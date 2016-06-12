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
 * @file inc/request.c
 *
 * @brief Request handling implementation
 *
 * @author Lars Wander
 */

#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <tils/conn.h>

typedef enum {
    TILS_GET,
    TILS_POST,
    TILS_PUT,
    TILS_HEAD,
    TILS_OPTIONS,
    TILS_DELETE,
    TILS_TRACE,
    TILS_CONNECT,
    TILS_UNKNOWN
} tils_http_request_e;

typedef struct {
    tils_http_request_e request_type;
    char *resource;
} tils_http_request_t;

tils_http_request_t *tils_parse_request(char *request, int request_len);
void tils_free_request(tils_http_request_t *request);

#endif /* _REQUEST_H_ */
