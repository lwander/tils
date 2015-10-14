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
 * @file inc/worker_thread.h
 *
 * @brief Declarations go here - see src/worker_thread.c for more info.
 *
 * @author Lars Wander (lars.wander@gmail.com)
 */

#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

/* We typedef these to allow the implementor to pick an implementation */
struct _worker_thread;
struct _connection;

typedef struct _worker_thread wt_t;
typedef struct _connection conn_t;

conn_t *wt_pop_conn(wt_t *self);
void wt_push_conn(wt_t *self, conn_t *conn);

int conn_is_alive(conn_t *conn);
conn_t *new_conn(int client_fd);
void revitalize_conn(conn_t *conn);

int start_thread_pool(int server_fd);

#endif /* _WORKER_THREAD_H_ */