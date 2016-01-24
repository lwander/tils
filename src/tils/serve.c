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
#include <tils/routes.h>
#include <tils/socket_util.h>

#include "serve_private.h"

/**
 * @brief Read a whitespace delinated word out of ibuf and into obuf.
 *
 * @param ibuf The input buffer being read from.
 * @param ibuf_len The input buffer length.
 * @param obuf The output buffer being read from.
 * @param obuf_len The output buffer length.
 * @param word_start The start of the word being read.
 *
 * @return The length of the word that was read.
 */
int read_word(char *ibuf, int ibuf_len, char *obuf, int obuf_len,
        int word_start) {
    int len = 0;
    int i = word_start;
    while (i < ibuf_len && len < obuf_len - 1 && !isspace((int)ibuf[i])) {
        obuf[len] = ibuf[i];
        i++;
        len++;
    }

    obuf[len] = '\0';
    return len;
}

/**
 * @brief Finds the next non-whitespace character starting at index.
 *
 * @param buf The buffer being searched.
 * @param buf_len The length of the buffer being searched.
 * @param index The index the search starts from.
 *
 * @return The index of the first non-whitespace charcter after index.
 */
int next_word(char *buf, int buf_len, int index) {
    while (index < buf_len && isspace((int)buf[index]))
        index++;
    return index;
}

/**
 * @brief Get the request type from an HTTP header.
 *
 * @param request The HTTP request in string form.
 * @param request_len The size of the request (cannot be greater than the size
 *                    of the buffer).
 *
 * @return The request type.
 */
http_request_t request_type(char *request, int request_len) {
    if (strncmp("GET", request, request_len) == 0)
        return GET;
    else if (strncmp("POST", request, request_len) == 0)
        return POST;
    else if (strncmp("PUT", request, request_len) == 0)
        return PUT;
    else if (strncmp("HEAD", request, request_len) == 0)
        return HEAD;
    else if (strncmp("OPTIONS", request, request_len) == 0)
        return OPTIONS;
    else if (strncmp("DELETE", request, request_len) == 0)
        return DELETE;
    else if (strncmp("TRACE", request, request_len) == 0)
        return TRACE;
    else if (strncmp("CONNECT", request, request_len) == 0)
        return CONNECT;
    else
        return UNKNOWN;
}

/**
 * @brief Send data & vaargs to client
 *
 * @param client_fd The client being communicated with.
 * @param msg The message with format specifiers to be sent.
 * @param ... variable args being formated into msg.
 */
void send_to_client(int client_fd, char *msg, ...) {
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
char *get_content_type(char *filename, int filename_len) {
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
void serve_unimplemented(tils_conn_t *conn) {
    fprintf(stdout, ANSI_BLUE "%s <- " ANSI_YELLOW ANSI_BOLD "501\n" ANSI_RESET,
            conn->addr_buf);
    send_to_client(conn->client_fd, (char *)msg_unimplemented);
}

/**
 * @brief Send the unimplemented header to the client
 *
 * @param client_fd The client being communicated with
 */
void serve_not_found(tils_conn_t *conn) {
    fprintf(stdout, ANSI_BLUE "%s <- " ANSI_RED ANSI_BOLD "404\n" ANSI_RESET,
            conn->addr_buf);
    send_to_client(conn->client_fd, (char *)msg_not_found);
}

void serve_file(tils_conn_t *conn, int file_fd, char *content_type, int size) {
    send_to_client(conn->client_fd, (char *)header_file, content_type, size);

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
        fprintf(stdout, ANSI_CLEAR INFO "[%d/%d bytes]", sum_total, size);
    }

    fprintf(stdout, "\n");
}

void serve_resource(tils_conn_t *conn, http_request_t http_request, char *resource) {
    if (http_request != GET) {
        serve_unimplemented(conn);
        return;
    }

    char *remap_resource;
    int file_fd;
    int size;

    /* Find if we are allowed to serve this resource */
    if (tils_route_lookup(resource, &remap_resource) == 0 &&
            (file_fd = open(remap_resource, O_RDONLY)) >= 0) {
        if ((size = fd_size(file_fd)) < 0)
            return;

        fprintf(stdout, ANSI_BLUE "%s <- " ANSI_GREEN ANSI_BOLD "200 "
                ANSI_RESET "%s\n", conn->addr_buf, remap_resource);

        char *content_type = get_content_type(remap_resource, 
                strlen(remap_resource));

        serve_file(conn, file_fd, content_type, size);
        close(file_fd);
    } else {
        serve_not_found(conn);
    }
}
