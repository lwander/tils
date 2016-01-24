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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <routes.h>
#include <util.h>
#include <worker_thread.h>
#include <tils.h>


int main(int argc, char *argv[]) {
    int server_fd = 0;
    int res = 0;
    int port = 80;

    if (argc > 1) {
        char *end = NULL;
        long res = strtol(argv[1], &end, 10);
        if (*end != '\0' || res < 0 || res >= (1 << 16) - 1) {
            fprintf(stderr, ERROR "Invalid port %s\n", argv[1]);
            exit(-1);
        }
        port = (int)res;
    }

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

    fprintf(stdout, "Opening connection on port %d...\n", port);
    if ((server_fd = init_server(port)) < 0) {
        fprintf(stdout, "failed.\n");
        res = -1;
        goto cleanup_routes;
    }

    fprintf(stdout, "Starting working threads...\n");
    start_thread_pool(server_fd);

    close(server_fd);

cleanup_routes:
    cleanup_routes();

    fprintf(stdout, "Goodbye\n");
    return res;
}
