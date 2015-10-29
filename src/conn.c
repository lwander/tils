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
#include <errno.h>

#include <conn.h>
#include <util.h>

/**
 * @brief Initialize a new connection.
 *
 * @param client_fd The client connection this connection listens to.
 *
 * @return The new connection object. NULL on error.
 */
void conn_new(int client_fd, char *addr_buf, conn_t *conn) {
    conn->client_fd = client_fd;
    conn->last_alive = TIME_NOW;
    conn->state = CONN_ALIVE;
    conn->file_fd = -1;
    memcpy(conn->addr_buf, addr_buf, sizeof(conn->addr_buf));
}

/**
 * @brief Update the last time a connection was heard from.
 *
 * @param conn The connection being updated.
 */
void conn_revitalize(conn_t *conn) {
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
int conn_check_alive(conn_t *conn) {
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
conn_state conn_close(conn_t *conn) {
    conn_state res = conn->state;
    if (res != CONN_CLEAN) {
        close(conn->client_fd);

        if (conn->file_fd >= 0) {
            close(conn->file_fd);
        }

        /* TODO Log forced death here */
        conn->state = CONN_CLEAN;
    }

    return res;
}

/**
 * @brief Push a connection onto the connection buffer
 *
 * @param conn_buf The connection buffer being added to.
 * @param client_fd Socket connection is on.
 * @param addr_buf Address of connection for logging.
 *
 * @return The connection that was added
 */
conn_t *conn_buf_push(conn_buf_t *conn_buf, int client_fd, char *addr_buf) {
    /* First safely evict existing connection (if we are full) */
    if (conn_buf->size == CONNS_PER_THREAD)
        conn_buf_pop(conn_buf);

    conn_new(client_fd, addr_buf, &conn_buf->conns[conn_buf->end]);
    conn_t *res = &conn_buf->conns[conn_buf->end];
    conn_buf->end = CONN_BUF_ELEM_NEXT(conn_buf->end);
    conn_buf->size++;

    return res;
}

/**
 * @brief Pop a connection (without ever seeing what it was), and close it
 *        in the process.
 *
 * @param conn_buf The connection buffer being popped from.
 *
 * @return The state of the connection that was popped prior to the
 *         operation being carried out, unless the buffer was empty,
 *         in which case return CONN_NONE.
 */
conn_state conn_buf_pop(conn_buf_t *conn_buf) {
    if (conn_buf->size == 0)
        return CONN_NONE;

    conn_state res = conn_buf->conns[conn_buf->start].state;
    conn_close(&conn_buf->conns[conn_buf->start]);
    conn_buf->start = CONN_BUF_ELEM_NEXT(conn_buf->start);
    conn_buf->size--;

    return res;
}

/**
 * @brief Peek at the given index into the connection buffer.
 *
 * @param conn_buf The connection buffer being examined.
 * @param i The index of the lookup.
 * @param[out] conn_buf A non-null pointer that will store the result of the
 *             lookup. If we specify an invalid index, it will point to NULL.
 */
void conn_buf_at(conn_buf_t *conn_buf, int i, conn_t **conn) {
    if (i >= conn_buf->size)
        *conn = NULL;
    else
        *conn = &conn_buf->conns[CONN_BUF_ELEM_AT(conn_buf->start + i)];
}

/**
 * @brief Initialize a connection buffer.
 *
 * The goal of this is to minimize memory allocations while the server is
 * running by preallocating all the resources we will use.
 *
 * @param conn_buf A pointer to the connection buffer to be initialized
 */
void conn_buf_init(conn_buf_t **conn_buf) {
    *conn_buf = calloc(sizeof(conn_buf_t), 1);

    if (*conn_buf == NULL) {
        fprintf(stderr, ERROR "No memory for initialization of connection "
                "buffers (%s).\n", strerror(errno));
        exit(-1);
    }
}

/**
 * @brief Get the number of elements in the connection buffer.
 *
 * @param conn_buf The connection buffer being examined.
 */
int conn_buf_size(conn_buf_t *conn_buf) {
    return conn_buf->size;
}
