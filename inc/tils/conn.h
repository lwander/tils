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
 * @file inc/conn.h
 *
 * @brief Connection implemnation declaration
 *
 * @author Lars Wander
 */

#ifndef _TILS_CONN_H_
#define _TILS_CONN_H_

#include <arpa/inet.h>

#define TTL (60)

#define TILS_CONN_BUF_ELEM_AT(i, s) ((i) % (s))
#define TILS_CONN_BUF_ELEM_NEXT(c, s) (TILS_CONN_BUF_ELEM_AT((c) + 1, (s)))

typedef enum tils_conn_state_e {
    /* Connection is totally closed, no dangling resources */
    CONN_CLEAN = 0,

    /* Connection is running normally */
    CONN_ALIVE,

    /* Connection should be closed and marked as clean afterwards */
    CONN_DEAD,

    /* Not a connection */
    CONN_NONE
} tils_conn_state;

/**
 * @brief A single connection handled by a single thread
 */
typedef struct tils_conn {
    /* Last alive time (used for keepalive). */
    double last_alive;

    /* fd corresponding to socket client is on. */
    int client_fd;

    /* ipv4 address of client - used for logging purposes. */
    char addr_buf[INET_ADDRSTRLEN];

    /* Connection can be marked as dead and cleaned up lazily using this flag.
     */
    tils_conn_state state;
} tils_conn_t;

void tils_conn_revitalize(tils_conn_t *conn);
int tils_conn_check_alive(tils_conn_t *conn);
tils_conn_state tils_conn_close(tils_conn_t *conn);

#endif /* _TILS_CONN_H_ */
