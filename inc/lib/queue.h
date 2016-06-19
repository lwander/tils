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
 * @file lib/queue.h
 *
 * @brief Atomic, lock-free, single producer single consumer queue
 *
 * @author Lars Wander
 */

#ifndef _QUEUE_H_
#define _QUEUE_H_

struct _queue;
typedef struct queue queue_t;

queue_t *queue_new(int capacity);
int queue_insert(queue_t *q, void *v);
int queue_remove(queue_t *q, void **v);
void queue_free(queue_t *q, void (*free_value)(void *));

#endif /* _QUEUE_H_ */
