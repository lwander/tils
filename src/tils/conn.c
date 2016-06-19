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
 * @file src/conn.c
 *
 * @brief Connection manipulation implementation. The key detail is the
 *        implementation of non-blocking IO facilitated by storing both
 *        files being served and their corresponding sockets.
 *
 * @author Lars Wander
 */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <lib/util.h>
#include <lib/logging.h>
#include <tils/conn.h>

/**
 * @brief Initialize a new connection.
 *
 * @param client_fd The client connection this connection listens to.
 *
 * @return The new connection object. NULL on error.
 */
void tils_conn_new(int client_fd, char *addr_buf, tils_conn_t *conn) {
    conn->client_fd = client_fd;
    conn->last_alive = TIME_NOW;
    conn->state = CONN_ALIVE;
    memcpy(conn->addr_buf, addr_buf, sizeof(conn->addr_buf));
}

/**
 * @brief Update the last time a connection was heard from.
 *
 * @param conn The connection being updated.
 */
void tils_conn_revitalize(tils_conn_t *conn) {
    conn->last_alive = TIME_NOW;
}

/**
 * @brief Check if connection is still alive.
 *
 * @param conn The connection being checked.
 *
 * This function also marks the result of the check in the connection state.
 *
 * @return 1 if true, 0 otherwise.
 */
int tils_conn_check_alive(tils_conn_t *conn) {
    if (conn->state == CONN_DEAD || conn->state == CONN_CLEAN) {
        return 0;
    } else if (TIME_NOW - conn->last_alive >= TTL) {
        conn->state = CONN_DEAD;
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief Close a connections file descriptors.
 *
 * @param conn The connection being closed.
 *
 * @return The original state of conn before close.
 */
tils_conn_state tils_conn_close(tils_conn_t *conn) {
    tils_conn_state res = conn->state;
    if (res != CONN_CLEAN) {
        close(conn->client_fd);

        /* TODO Log forced death here */
        conn->state = CONN_CLEAN;
    }

    return res;
}
