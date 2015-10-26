/*
 *  This file is part of c-http.
 *
 *  c-http is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
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

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <socket_util.h>
#include <serve.h>
#include <worker_thread.h>

#include "worker_thread_private.h"

static wt_t _worker_threads[THREAD_COUNT];

/**
 * @brief Pop a connection from a worker threads queue of connections.
 *
 * @param self The worker thread being modified.
 *
 * @return The connection least recently checked. NULL If there are none.
 */
conn_t *wt_pop_conn(wt_t *self) {
    if (self == NULL || self->conns == NULL)
        return NULL;

    conn_t *res = self->conns;
    self->conns = self->conns->next;
    self->size--;

    /* Did we just pop the only connection? */
    if (self->last_conn == res) {
        self->last_conn = NULL;
        self->conns = NULL;
        assert(self->size == 0);
    }

    return res;
}

/**
 * @brief Push a connection onto a worker threads queue.
 *
 * @param self The worker thread being modified.
 * @param conn The connection being enqueued.
 */
void wt_push_conn(wt_t *self, conn_t *conn) {
    if (self == NULL || conn == NULL) {
        return;
    }
    if (self->last_conn == NULL) {
        self->last_conn = conn;
        self->conns = conn;
    } else {
        self->last_conn->next = conn;
        self->last_conn = conn;
    }
    self->size++;
}

/**
 * @brief Free input connection.
 *
 * @param conn Connection to be freed.
 */
void free_conn(conn_t *conn) {
    if (conn == NULL)
        return;

    close(conn->client_fd);
    free(conn);
}

/**
 * @brief Process the type of HTTP request, and respond accordingly.
 *
 * @param conn Connection being communicated with
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

    revitalize_conn(conn);

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
    int server_fd = self->server_fd;
    conn_t *conn = NULL;
    char addr_buf[INET_ADDRSTRLEN];

    /* Alternate between binding new connections and reading from old ones */
    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&ip4client, 
                        &ip4client_len)) > 0) {

            /* Load the IP address for logging purposes */
            inet_ntop(AF_INET, (const void *)&ip4client.sin_addr, addr_buf, 
                    INET_ADDRSTRLEN);

            /* Keep alive can fail
             * TODO if the error hints at a larger problem, do something here
             */
            socket_keepalive(client_fd);

            conn = new_conn(client_fd, addr_buf);
            if (conn == NULL) {
                close(client_fd);
            } else if (fd_nonblocking(client_fd) < 0) {
                /* If non blocking fails, every call to `accept' will take too
                 * long. This connection is then no longer viable */
                free_conn(conn);
            } else {
                accept_request(conn);
                wt_push_conn(self, conn);
                fprintf(stdout, ANSI_BOLD ANSI_GREEN  "-->  " ANSI_BLUE "%s\n" 
                        ANSI_RESET, addr_buf);
            }
        }

        conn = wt_pop_conn(self);
        if (conn == NULL) {
            assert(self->size == 0);
            continue;
        }

        /* Lazy cleanup is preformed here, meaning, if a connection 
         * encountered an error, or hasn't been heard from in sometime, we
         * delete it */
        if (!conn_is_alive(conn)) {
            fprintf(stdout, ANSI_BOLD ANSI_YELLOW "-/-> " ANSI_BLUE "%s\n" 
                    ANSI_RESET, addr_buf);
            free_conn(conn);
            continue;
        }

        accept_request(conn);
        wt_push_conn(self, conn);
    }

    return NULL;
}

/**
 * @brief Run the thread pool - the master thread is roped into this as well.
 *
 * @param server_fd The server socket to listen on
 */
void start_thread_pool(int server_fd) {
    for (int i = 0; i < THREAD_COUNT; i++) {
        _worker_threads[i].server_fd = server_fd;
        _worker_threads[i].conns = NULL;
        _worker_threads[i].last_conn = NULL;
        _worker_threads[i].id = i + 1;
        _worker_threads[i].size = 0;
        pthread_create(&_worker_threads[i].thread, NULL, handle_connections, 
                (void *)&_worker_threads[i]); 
    }

    wt_t master;
    master.server_fd = server_fd;
    master.conns = NULL;
    master.last_conn = NULL;
    master.id = 0;
    master.size = 0;
    fprintf(stdout, "done.\n");
    handle_connections((void*)&master);
}
