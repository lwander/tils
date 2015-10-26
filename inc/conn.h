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
 * @file inc/conn.h
 *
 * @brief Connection implemnation declaration
 *
 * @author Lars Wander
 */

#ifndef _CONN_H_
#define _CONN_H_

#include <arpa/inet.h>

#define CONNS_PER_THREAD (1000)

/**
 * @brief A single connection handled by a single thread
 */
typedef struct conn {
    /* Last alive time (used for keepalive). */
    double last_alive;

    /* fd corresponding to socket client is on. */
    int client_fd;

    /* fd corresponding to file being served (-1 if no such file). */
    int file_fd;

    /* ipv4 address of client - used for logging purposes. */
    char addr_buf[INET_ADDRSTRLEN];

    /* connection can be marked as dead and cleaned up lazily using this flag.
     */
    int alive;
} conn_t;

/**
 * @brief Used to store all connections that belong to a thread
 *
 * Implemented as a ring buffer. The idea is that when a thread hits its 
 * max number of connections - we can through away the oldest to make room
 * for the next.
 */
typedef struct conn_buf {
    /* All connections in buffer */
    conn_t conns[CONNS_PER_THREAD];

    /* Index of first connection held. */
    int start;

    /* Index of first unused connection. */
    int end;
} conn_buf_t;

#endif /* _CONN_H_ */
