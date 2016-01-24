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
 * @file inc/serve.c
 *
 * @brief Serving tools implementation
 *
 * @author Lars Wander
 */

#ifndef _SERVE_H_
#define _SERVE_H_

#define SERVER_STRING "Server: lwander-c-http/0.0.1\r\n"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <lib/util.h>
#include <tils/worker_thread.h>

typedef enum {
    GET,
    POST,
    PUT,
    HEAD,
    OPTIONS,
    DELETE,
    TRACE,
    CONNECT,
    UNKNOWN
} http_request_t;

#define HTML "text/html; charset=utf8"
#define CSS "text/css"
#define JS "application/javascript"
#define TEXT "text"

int read_word(char *ibuf, int ibuf_len, char *obuf, int obuf_len,
        int word_start);
int next_word(char *buf, int buf_len, int index);

http_request_t request_type(char *request, int request_len);
void serve_resource(tils_conn_t *conn, http_request_t http_request, char *resource);

#endif /* _SERVE_H_ */
