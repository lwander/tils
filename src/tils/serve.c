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
 * @file src/serve.c
 *
 * @brief File & response serving implementation.
 *
 * @author Lars Wander
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

#include <tils/serve.h>
#include <tils/request.h>
#include <tils/routes.h>
#include <tils/io_util.h>

#include "serve_private.h"

/**
 * @brief Send data & vaargs to client
 *
 * @param client_fd The client being communicated with.
 * @param msg The message with format specifiers to be sent.
 * @param ... variable args being formated into msg.
 */
void _tils_serve_to_client(int client_fd, char *msg, ...) {
    va_list ap;
    char buf[REQUEST_BUF_SIZE + 1];

    va_start(ap, msg);
    vsnprintf(buf, sizeof(buf), msg, ap);
    va_end(ap);

    send(client_fd, buf, strlen(buf), 0);
}

/**
 * @brief Get the content type by filename extension.
 *
 * @param filename The filename being examined.
 * @param filename_len The length of the filename in question.
 *
 * @return The content filetype - TEXT if unsure
 */
char *_tils_get_content_type(char *filename, int filename_len) {
    int p_ind = filename_len - 1;
    while (p_ind > 0 && filename[p_ind] != '.')
        p_ind--;

    p_ind += 1;

    if (strncmp("html", filename + p_ind, filename_len - p_ind) == 0) {
        return HTML;
    } else if (strncmp("css", filename + p_ind, filename_len - p_ind) == 0) {
        return CSS;
    } else if (strncmp("js", filename + p_ind, filename_len - p_ind) == 0) {
        return JS;
    } else {
        return TEXT;
    }
}

/**
 * @brief Send the unimplemented header to the client
 *
 * @param client_fd The client being communicated with
 */
void _tils_serve_unimplemented(tils_conn_t *conn) {
    _tils_serve_to_client(conn->client_fd, (char *)msg_unimplemented);
}

/**
 * @brief Send the unimplemented header to the client
 *
 * @param client_fd The client being communicated with
 */
void _tils_serve_not_found(tils_conn_t *conn) {
    _tils_serve_to_client(conn->client_fd, (char *)msg_not_found);
}

void _tils_serve_file(tils_conn_t *conn, int file_fd, char *content_type, 
        int size) {
    _tils_serve_to_client(conn->client_fd, (char *)header_file, content_type, 
            size);

    char buf[REQUEST_BUF_SIZE];
    int res = 0;

    int sum_total = 0;
    while ((res = read(file_fd, buf, REQUEST_BUF_SIZE)) > 0) {
        int total = 0;
        while (res > 0) {
            int sent = send(conn->client_fd, &buf[total], res, 0);

            /* Mark connection as dead to be cleaned up later */
            if (sent <= 0) {
                conn->state = CONN_DEAD;
                return;
            }

            res -= sent;
            total += sent;
        }

        sum_total += total;
    }
} 

/**
 * @brief Serve a resource to the input connection based on the request.
 *
 * @param conn The connection being served the resource.
 * @param http_request The request specifying the resource.
 */
void tils_serve_resource(tils_conn_t *conn, tils_http_request_t *http_request) {
    tils_http_request_e request_type = http_request->request_type;
    char *resource = http_request->resource;

    if (request_type != TILS_GET) {
        _tils_serve_unimplemented(conn);
        return;
    }

    char *remap_resource;
    int file_fd;
    int size;

    /* Find if we are allowed to serve this resource */
    if (tils_route_lookup(resource, &remap_resource) == 0 &&
            (file_fd = open(remap_resource, O_RDONLY)) >= 0) {
        if ((size = tils_fd_size(file_fd)) < 0)
            return;

        char *content_type = _tils_get_content_type(remap_resource, 
                strlen(remap_resource));

        _tils_serve_file(conn, file_fd, content_type, size);
        close(file_fd);
    } else {
        _tils_serve_not_found(conn);
    }
}
