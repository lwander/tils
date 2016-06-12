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
 * @file inc/accept.c
 *
 * @brief Request handling implementation
 *
 * @author Lars Wander
 */

#ifndef _ACCEPT_H_
#define _ACCEPT_H_

#include <tils/conn.h>
#include <tils/request.h>

tils_http_request_t *tils_accept_request(tils_conn_t *conn);

#endif /* _ACCEPT_H_ */
