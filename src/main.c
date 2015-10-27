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
 * @file src/main.c
 *
 * @brief Instantiate server here
 *
 * @author Lars Wander (lars.wander@gmail.com)
 */

#define _GNU_SOURCE

static int _server_fd;

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <errno.h>

#include <routes.h>
#include <util.h>
#include <socket_util.h>
#include <worker_thread.h>

void sigint_handler(int unused) {
    fprintf(stdout, "Shutting down `gracefully'\n");
    close(_server_fd);
    exit(-1);
}

/**
 * @brief Bind server to HTTP TCP socket.
 *
 * @return -1 on error, the server file descriptor otherwise.
 */
int init_server() {
    struct sockaddr_in ip4server;
    int server_fd = 0;

    ip4server.sin_family = AF_INET; /* Address family internet */
    ip4server.sin_port = htons(HTTP_PORT); /* Bind to port 80 */
    ip4server.sin_addr.s_addr = htonl(INADDR_ANY);  /* Bind to any interface */

    /* Get a file descriptor for our socket */
    if ((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fprintf(stdout, ERROR "Unable to create socket "
                ANSI_RESET "(%s)\n", strerror(errno));
        goto fail;
    }

    if (socket_keepalive(server_fd) < 0) {
        goto fail;
    }

    /* At first block, because we don't need to spin waiting for connections
     * if we know there are none */
    if (fd_nonblocking(server_fd) < 0) {
        goto fail;
    }

    /* Bind the socket file descriptor to our network interface */
    if (bind(server_fd, (struct sockaddr *)&ip4server, sizeof(ip4server)) < 0) {
        fprintf(stdout, ERROR "Unable to bind socket "
                ANSI_RESET "(%s)\n", strerror(errno));
        goto cleanup_socket;
    }

    /* Listen for connections on this socket. AFAIK, the second argument
     * (backlog) is a suggestion, not a hard value. */
    if (listen(server_fd, 16) < 0) {
        fprintf(stdout, ERROR "Unable to listen on socket "
                ANSI_RESET "(%s)\n", strerror(errno));
        goto cleanup_socket;
    }

    return server_fd;


cleanup_socket:
    close(server_fd);

fail:
    return -1;
}

int main(int argc, char *argv[]) {
    int server_fd = 0;
    int res = 0;

    struct sigaction sigact;
    sigact.sa_handler = sigint_handler;
    sigaction(SIGINT, &sigact, NULL);

    fprintf(stdout, "Building routes...\n");
    if (init_routes() < 0 || 
           add_route("/", "html/index.html") < 0 ||
           add_route("/apple-touch-icon.png", "html/apple-touch-icon.png") < 0 ||
           add_route("/favicon.png", "html/favicon.png") < 0 ||
           add_route("/common.css", "html/common.css") < 0) {
        fprintf(stdout, "failed.\n");
        res = -1;
        goto cleanup_routes;
    } 

    fprintf(stdout, "Opening connection...\n");
    if ((server_fd = init_server()) < 0) {
        fprintf(stdout, "failed.\n");
        res = -1;
        goto cleanup_routes;
    }

    /* Necessary for sigint handler */
    _server_fd = server_fd;

    fprintf(stdout, "Starting working threads...\n");
    start_thread_pool(server_fd);

    close(server_fd);

cleanup_routes:
    cleanup_routes();

    fprintf(stdout, "Goodbye\n");
    return res;
}
