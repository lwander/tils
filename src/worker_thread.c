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

#include "worker_thread_private.h"

/**
 * @file src/worker_thread.c
 *
 * @brief Worker thread implemention
 *
 * The idea is to minimize context switches. The way this is done is to
 * dedicate 1 thread per core, and to have each thread manage it's own set
 * of connections - accepting, reading, and writing are all done on each
 * thread.
 *
 * @author Lars Wander
 */

static _worker_threads[THREAD_COUNT];

/**
 * @brief Process the type of HTTP request, and respond accordingly.
 *
 * @param client_fd File descriptor of client sending data.
 */
void accept_request(int client_fd) {
    char request[REQUEST_BUF_SIZE];
    char word[WORD_BUF_SIZE];
    char resource[WORD_BUF_SIZE];
    int request_len = 0;
    int index = 0;
    int word_len = 0;
    http_request_t http_request;

    /* First grab the full HTTP request */
    request_len = read(client_fd, request, REQUEST_BUF_SIZE);

    if (request_len == 0)
        return;

    /* Start parsing it word by word */
    index = next_word(request, request_len, 0);
    word_len = read_word(request, REQUEST_BUF_SIZE, word, WORD_BUF_SIZE, index);
    http_request = request_type(word, word_len);

    index = next_word(request, request_len, index + word_len);
    word_len = read_word(request, REQUEST_BUF_SIZE, resource, WORD_BUF_SIZE,
            index);

    serve_resource(client_fd, http_request, resource);
    return;
}

/**
 * @brief Wait to be connected to a client, then handle the client's request,
 *        and repeat.
 *
 * @param server_fd The socket fd to accept connections on
 */
void *handle_connections(void *server_fd) {
    struct sockaddr_in ip4client;
    unsigned int ip4client_len = sizeof(ip4client);
    int client_fd = 0;
    int optval = 1;
    socklen_t optlen = sizeof(optval);

    while (1) {
        if ((client_fd = accept(*((int *)server_fd),
                        (struct sockaddr *)&ip4client, &ip4client_len)) < 0) {
            fprintf(stderr, "Error, connection dropped (errno %d)", errno);
        } else {
            /* Set keepalive status */
            if (setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
                fprintf(stderr, "Unable to set keepalive to %d (errno %d)\n", optval, errno);
            } else {
                while (1) {
                    accept_request(client_fd);
                }

            }
            close(client_fd);
        }
    }

    return NULL;
}

/**
 * @brief Prepare the thread pool
 *
 * @param server_fd The server socket to listen on
 */
void init_thread_pol(int server_fd) {
    for (int i = 0; i < THREAD_COUNT; i++) {
        _worker_threads[i].server_fd = server_fd;
        _worker_threads[i].conns = NULL;
    }
}
