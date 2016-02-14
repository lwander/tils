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
 * @file src/accept.c
 *
 * @brief Request handler implementation.
 *
 * @author Lars Wander
 */

#include <tils/accept.h>
#include <tils/serve.h>

/**
 * @brief Process the HTTP request, and respond accordingly.
 * 
 * @param conn Connection being communicated with
 */
tils_http_request_t *tils_accept_request(tils_conn_t *conn) {
    char request[REQUEST_BUF_SIZE];
    int request_len = 0;
    tils_http_request_t *http_request;

    /* First grab the full HTTP request */
    request_len = recv(conn->client_fd, request, REQUEST_BUF_SIZE, 0);

    if (request_len <= 0)
        return NULL;

    http_request = tils_parse_request(request, request_len);

    if (http_request != NULL)
        fprintf(stdout, ANSI_BLUE "%s -> " ANSI_RESET "%s\n", 
                conn->addr_buf, http_request->resource);
    return http_request;
}
