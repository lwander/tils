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

#define SERVER_STRING "Server: lwander-c-http/0.0.1\r\n"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <util.h>

#ifndef _SERVE_H_
#define _SERVE_H_

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

typedef enum {
    HTML,
    CSS,
    JS,
    TEXT
} content_t;

#endif /* _SERVE_H_ */
