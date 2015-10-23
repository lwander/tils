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
 * @file src/worker_thread_private.c
 *
 * @brief 'secret' details of worker_thread implementation go here.
 *
 * @author Lars Wander (lars.wander@gmail.com)
 */

#ifndef _WORKER_THREAD_PRIVATE_H_
#define _WORKER_THREAD_PRIVATE_H_

#include <util.h>

#include <pthread.h>

#define THREAD_COUNT (4)

static double _ttl = 600;

typedef struct conn {
    /* Linked list of connections */
    struct conn *next;

    /* Last alive time (used for keepalive) */
    double last_alive;

    int client_fd;
} conn_t;

/**
 * @brief Worker thread struct implementation.
 */
typedef struct {
    /* Aligned to the nearest cache line since each thread will be hammering
     * this struct, and we don't want constant cross processor cache eviction
     * that will occur if all of our thread objects are too close on the same
     * cache line.
     */
    pthread_t thread __attribute__((aligned(CACHE_LINE_SIZE)));

    /* List of managed connections */
    conn_t *conns;
    
    /* Last managed connection here */
    conn_t *last_conn;

    /* File descriptor to listen to new connections on */
    int server_fd;
} wt_t;

#endif /* _WORKER_THREAD_PRIVATE_H_ */
