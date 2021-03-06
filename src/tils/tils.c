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
 * @file src/lw-http.c
 *
 * @brief Instantiate server here
 *
 * @author Lars Wander (lars.wander@gmail.com)
 */

#define _GNU_SOURCE

#include <stdio.h>

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <lib/util.h>
#include <lib/logging.h>
#include <tils/io_util.h>

static int _fd_limit;

/**
 * @brief Increase open file descriptors to max
 *
 * @return Number of possible open file descriptors, -1 on error.
 */
rlim_t set_open_fd_limit() {
    struct rlimit r;

    if (getrlimit(RLIMIT_NOFILE, &r) < 0)
        log_err("Unable to get file descriptor limit");

    if (r.rlim_cur < r.rlim_max)
        r.rlim_cur = r.rlim_max;

    if (setrlimit(RLIMIT_NOFILE, &r) < 0)
        log_err("Unable to set file descriptor limit");

    return r.rlim_cur;
}

int get_open_fd_limit() {
    if (_fd_limit == 0) {
        _fd_limit = set_open_fd_limit();
    }
    return _fd_limit;
}

/**
 * @brief Bind server to HTTP TCP socket.
 *
 * @return -1 on error, the server file descriptor otherwise.
 */
int init_server(int port) {
    struct sockaddr_in ip4server;
    int server_fd = 0;
    _fd_limit = 0;

    ip4server.sin_family = AF_INET; /* Address family internet */
    ip4server.sin_port = htons(port); /* Bind to given port */
    ip4server.sin_addr.s_addr = htonl(INADDR_ANY);  /* Bind to any interface */

    /* Get a file descriptor for our socket */
    if ((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        log_err("Unable to create socket");
        goto fail;
    }

    if (tils_socket_keepalive(server_fd) < 0) {
        goto fail;
    }

    /* At first block, because we don't need to spin waiting for connections
     * if we know there are none */
    if (tils_fd_nonblocking(server_fd) < 0) {
        goto fail;
    }

    /* Bind the socket file descriptor to our network interface */
    if (bind(server_fd, (struct sockaddr *)&ip4server, sizeof(ip4server)) < 0) {
        log_err("Unable to bind socket");
        goto cleanup_socket;
    }

    /* Listen for connections on this socket. AFAIK, the second argument
     * (backlog) is a suggestion, not a hard value. */
    if (listen(server_fd, 16) < 0) {
        log_err("Unable to listen on socket");
        goto cleanup_socket;
    }

    return server_fd;


cleanup_socket:
    close(server_fd);

fail:
    return -1;
}
