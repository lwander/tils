/*
 *  This file is part of tils.
 *
 *  tils is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  tils is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with tils.  If not, see <http://www.gnu.org/licenses/>
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

#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <lib/logging.h>
#include <tils/io_util.h>
#include <tils/serve.h>
#include <tils/accept.h>
#include <tils/worker_thread.h>
#include <tils/tils.h>

#include "worker_thread_private.h"

static tils_wt_t _worker_threads[THREAD_COUNT];

/**
 * @brief assign the current thread to a single core
 */
void _tils_sched_thread(tils_wt_t *self) {
    int core_cnt = sysconf(_SC_NPROCESSORS_ONLN);
    int assigned_core = self->id % core_cnt;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(assigned_core, &cpuset);

    pthread_t thread = pthread_self();
    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset) != 0) {
        log_warn("Couldn't bind thread %d", self->id);
    } else {
        log_info("Bound thread %d to core %d", self->id, assigned_core);
    }
}


/**
 * @brief Wait to be connected to a client, then handle the client's request,
 *        and repeat.
 *
 * @param server_fd The socket fd to accept connections on
 */
void *_tils_handle_connections(void *_self) {
    tils_wt_t *self = (tils_wt_t *)_self;
    _tils_sched_thread(self);

    struct sockaddr_in ip4client;
    unsigned int ip4client_len = sizeof(ip4client);

    int client_fd = 0;

    char addr_buf[INET_ADDRSTRLEN];
    tils_conn_t *conn = NULL;
    tils_http_request_t *request = NULL;
    tils_conn_buf_t *conn_buf = self->conns;

    fd_set read_fs;
    int nfds = 0;

    int i = 0;
    while (1) {
        i++;
        struct timeval timeout = { .tv_sec = 5, .tv_usec = 0 };
        FD_ZERO(&read_fs);

        /* Are we the leader? */
        if (self->server_fd >= 0) {
            /* If so, listen on the incomming connection port */
            FD_SET(self->server_fd, &read_fs);
            nfds = self->server_fd;
        } else {
            /* This may be risky */
            FD_SET(self->read_fd, &read_fs);
        //    if (self->read_fd > nfds)
            nfds = self->read_fd;
        }

        /* Do cleanup, and simultaneously record connections in our fd_set. */
        for (int i = 0; i < tils_conn_buf_size(conn_buf); i++) {
            tils_conn_buf_at(conn_buf, i, &conn);
            if (conn == NULL || conn->state == CONN_CLEAN)
                continue;

            /* Update connection state. */
            tils_conn_check_alive(conn);

            if (conn->state == CONN_DEAD) {
                tils_conn_close(conn);
                continue;
            }

            FD_SET(conn->client_fd, &read_fs);
            if (conn->client_fd > nfds)
                nfds = conn->client_fd;
        }
        
        int res = 0;
        if (UNLIKELY((res = select(nfds + 1, &read_fs, 
                            NULL, NULL, &timeout)) < 0)) {
            log_err("Select failed.");
            exit(-1);
        }

        /* 0 means no file descriptors are active and the timeout woke us up. */
        if (res == 0)
            continue;

        /* If our server_fd (leader token) is positive, 
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
                log_err("Failed to pass token.");
                exit(-1);
            }

            self->server_fd = -1;

            /* Load the IP address for logging purposes. */
            inet_ntop(AF_INET, (const void *)&ip4client.sin_addr, addr_buf,
                    INET_ADDRSTRLEN);

            /* Keep alive can fail.
             * TODO if the error hints at a larger problem, do something here.
             */
            tils_socket_keepalive(client_fd);

            if (UNLIKELY(tils_fd_nonblocking(client_fd) < 0)) {
                /* If non blocking fails, every call to `accept' will take too
                 * long. This connection is then no longer viable. */
                close(client_fd);
            } else {
                conn = tils_conn_buf_push(conn_buf, client_fd, addr_buf);
                request = tils_accept_request(conn);
                if (request) {
                    tils_conn_revitalize(conn);
                    tils_serve_resource(conn, request);
                }
            }
        }

        /* Is it our turn to become leader? */
        if (FD_ISSET(self->read_fd, &read_fs)) {
            if (read(self->read_fd, &self->server_fd, sizeof(int)) <= 0) {
                log_err("Failed to get token");
                exit(-1);
            }
            assert(self->server_fd >= 0);
        }

        /* Respond to sockets that are ready to be read from. */
        for (int i = 0; i < tils_conn_buf_size(conn_buf); i++) {
            tils_conn_buf_at(conn_buf, i, &conn);
            if (conn == NULL || !FD_ISSET(conn->client_fd, &read_fs))
                continue;

            request = tils_accept_request(conn);
            if (request) {
                tils_conn_revitalize(conn);
                tils_serve_resource(conn, request);
            }
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
void tils_start_thread_pool(int server_fd) {
    int pipefd[2];
    int conns_per_thread = get_open_fd_limit() / THREAD_COUNT;

    for (;;) {
        int client_fd = accept4(server_fd, NULL, NULL, SOCK_NONBLOCK);
        if (UNLIKELY(client_fd < 0)) {
            log_warn("accept4 failure");
            continue;
        }
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pipe(pipefd) < 0) {
            log_err("Failed to create pipe between threads");
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

        tils_conn_buf_init(&_worker_threads[i].conns, conns_per_thread);
        _worker_threads[i].size = 0;

        /* THREAD_COUNT - 1 is the calling thread. */
        if (i == THREAD_COUNT - 1)
            _tils_handle_connections((void *)&_worker_threads[i]);
        else 
            pthread_create(&_worker_threads[i].thread, NULL, _tils_handle_connections,
                    (void *)&_worker_threads[i]);
    } 
}
