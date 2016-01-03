/*
 *  This file is part of c-http.
 *
 *  c-http is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  c-http is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with c-http.  If not, see <http://www.gnu.org/licenses/>
 */

/**
 * @file src/worker_thread.c
 *
 * @brief Worker thread implemention
 *
 * The idea is to minimize context switches. The way this is done is to
 * dedicate 1 thread per core, and to have each thread manage its own set
 * of connections - accepting, reading, and writing are all done on each
 * thread.
 *
 * @author Lars Wander
 */

#define _GNU_SOURCE

#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <socket_util.h>
#include <serve.h>
#include <worker_thread.h>

#include "worker_thread_private.h"

static wt_t _worker_threads[THREAD_COUNT];

/**
 * @brief Process the type of HTTP request, and respond accordingly.
 * * @param conn Connection being communicated with
 */
void accept_request(conn_t *conn) {
    char request[REQUEST_BUF_SIZE];
    char word[WORD_BUF_SIZE];
    char resource[WORD_BUF_SIZE];
    int request_len = 0;
    int index = 0;
    int word_len = 0;
    http_request_t http_request;

    /* First grab the full HTTP request */
    request_len = recv(conn->client_fd, request, REQUEST_BUF_SIZE, 0);

    if (request_len <= 0)
        return;

    conn_revitalize(conn);

    /* Start parsing it word by word */
    /* TODO More robust parsing of requests */
    index = next_word(request, request_len, 0);
    word_len = read_word(request, REQUEST_BUF_SIZE, word, WORD_BUF_SIZE, index);
    http_request = request_type(word, word_len);

    index = next_word(request, request_len, index + word_len);
    word_len = read_word(request, REQUEST_BUF_SIZE, resource, WORD_BUF_SIZE,
            index);

    fprintf(stdout, ANSI_BLUE "%s -> " ANSI_RESET ANSI_BOLD "%s " ANSI_RESET
            "%s\n", conn->addr_buf, word, resource);
    serve_resource(conn, http_request, resource);
    return;
}

/**
 * @brief Wait to be connected to a client, then handle the client's request,
 *        and repeat.
 *
 * @param server_fd The socket fd to accept connections on
 */
void *handle_connections(void *_self) {
    wt_t *self = (wt_t *)_self;

    struct sockaddr_in ip4client;
    unsigned int ip4client_len = sizeof(ip4client);

    int client_fd = 0;

    char addr_buf[INET_ADDRSTRLEN];
    conn_t *conn = NULL;
    conn_buf_t *conn_buf = self->conns;

    fd_set read_fs;
    int nfds = 0;

    int i = 0;
    while (1) {
        struct timeval timeout = { .tv_sec = 5, .tv_usec = 0 };
        i++;
        FD_ZERO(&read_fs);

        /* Are we the leader? */
        if (self->server_fd >= 0) {
            FD_SET(self->server_fd, &read_fs);
            nfds = self->server_fd;
        }

        FD_SET(self->read_fd, &read_fs);
        if (self->read_fd > nfds)
            nfds = self->read_fd;

        /* Do cleanup, and simultaneously record connections in our fd_set. */
        for (int i = 0; i < conn_buf_size(conn_buf); i++) {
            conn_buf_at(conn_buf, i, &conn);
            if (conn == NULL || conn->state == CONN_CLEAN)
                continue;

            /* Update connection state. */
            conn_check_alive(conn);

            if (conn->state == CONN_DEAD) {
                fprintf(stdout, ANSI_BOLD ANSI_YELLOW "-/-> " ANSI_BLUE "%s\n"
                        ANSI_RESET, conn->addr_buf);
                conn_close(conn);
                continue;
            }

            FD_SET(conn->client_fd, &read_fs);
            if (conn->client_fd > nfds)
                nfds = conn->client_fd;

            /* Listen to the file_fd as well, since we are using non-blocking
             * IO to serve resources (meaning a fetch from disk could have 
             * stopped halfway through, and we don't want to have every other
             * connection wait until it's done, so we register it here). */
            if (LIKELY(conn->file_fd >= 0))
                FD_SET(conn->file_fd, &read_fs);
            if (conn->file_fd > nfds)
                nfds = conn->file_fd;
        }

        int res = 0;
        if (UNLIKELY((res = select(nfds + 1, &read_fs, 
                            NULL, NULL, &timeout)) < 0)) {
            fprintf(stderr, ERROR "Select failed (%s).\n", strerror(errno));
            exit(-1);
        }
        
        if (i % LOG_FREQ == 0)
            fprintf(stdout, INFO "[%d] on %d\n", self->id, res);
        /* 0 means no file descriptors are active and the timeout woke us up. */
        if (res == 0)
            continue;

        /* If our server_fd (leader token)  is positive, 
         * we can accept connections */
        if (self->server_fd >= 0 && FD_ISSET(self->server_fd, &read_fs) &&
                (client_fd = accept(self->server_fd, 
                                    (struct sockaddr *)&ip4client,
                                    &ip4client_len)) > 0) {
            /* First pass the leader token on to the next thread. 
             * This wakes up the next thread in the token chain, causing it 
             * to listen for unopened connections. If it is already awake,
             * it will either: 
             * 1. Discover it can read from read_fd, and start listening on
             *    it's newly assigned server_fd.
             * 2. Not read from read_fd, call select, and wake up at once. */
            if (write(self->write_fd, &self->server_fd, sizeof(int)) <= 0) {
                fprintf(stderr, ERROR "Failed to pass token (%s).\n", 
                        strerror(errno));
                exit(-1);
            }
            self->server_fd = -1;

            /* Load the IP address for logging purposes. */
            inet_ntop(AF_INET, (const void *)&ip4client.sin_addr, addr_buf,
                    INET_ADDRSTRLEN);

            /* Keep alive can fail.
             * TODO if the error hints at a larger problem, do something here.
             */
            socket_keepalive(client_fd);

            if (UNLIKELY(fd_nonblocking(client_fd) < 0)) {
                /* If non blocking fails, every call to `accept' will take too
                 * long. This connection is then no longer viable. */
                close(client_fd);
            } else {
                conn = conn_buf_push(conn_buf, client_fd, addr_buf);
                accept_request(conn);
                fprintf(stdout, ANSI_BOLD ANSI_GREEN  "-->  " ANSI_BLUE "%s\n"
                        ANSI_RESET, addr_buf);
            }
        }

        /* Is it our turn to become leader? */
        if (FD_ISSET(self->read_fd, &read_fs)) {
            if (read(self->read_fd, &self->server_fd, sizeof(int)) <= 0) {
                fprintf(stderr, ERROR "Failed to get token (%s).\n", 
                        strerror(errno));
                exit(-1);
            }
            assert(self->server_fd >= 0);
            fprintf(stdout, INFO "NEW LEADER");
        }

        /* Respond to sockets that are ready to be read from. */
        for (int i = 0; i < conn_buf_size(conn_buf); i++) {
            conn_buf_at(conn_buf, i, &conn);
            if (conn == NULL || !FD_ISSET(conn->client_fd, &read_fs))
                continue;

            accept_request(conn);
        }
    }

    /* Just for you, compiler. */
    return NULL;
}

/**
 * @brief Run the thread pool - the master thread is roped into this as well.
 *
 * @param server_fd The server socket to listen on.
 */
void start_thread_pool(int server_fd) {
    int pipefd[2];

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pipe(pipefd) < 0) {
            fprintf(stderr, ERROR "Failed to create pipe between threads "
                    "(%s).\n", strerror(errno));
            exit(-1);
        }

        _worker_threads[i].id = i;

        /* Connect writer. */
        _worker_threads[i].write_fd = pipefd[1];
        /* Connect reader. */
        _worker_threads[(i + 1) % THREAD_COUNT].read_fd = pipefd[0];

        /* At first, thread 0 will be the leader. */
        if (i == 0)
            _worker_threads[i].server_fd = server_fd;
        else 
            _worker_threads[i].server_fd = -1;

        conn_buf_init(&_worker_threads[i].conns);
        _worker_threads[i].size = 0;

        /* THREAD_COUNT - 1 is the calling thread. */
        if (i == THREAD_COUNT - 1)
            handle_connections((void *)&_worker_threads[i]);
        else 
            pthread_create(&_worker_threads[i].thread, NULL, handle_connections,
                    (void *)&_worker_threads[i]);
    } 
}
