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
 * @file src/lib/queue.c
 *
 * @brief Queue data structure implementation
 *
 * The queue is a single ring buffer that maintains atomicity by only reporting
 * an insertion as successful by incrementing the end of the buffer only
 * when the insertion has completed. This way any removal will fail until we
 * can guarantee there is data to remove. Similarly, a removal will only
 * increment the start of the buffer when the removal has completed, this way
 * we will not fail to reject an insertion (due to the queue being full) when
 * the first element of the queue hasn't been removed yet.
 *
 * @author Lars Wander 
 */

#include <lib/queue.h>
#include <lib/logging.h>

#include <errno.h>
#include <stdlib.h>
#include <stdatomic.h>

#include "queue_private.h"

/**
 * @brief Allocate a fresh, empty queue
 *
 * @param capacity The max capacity of the queue
 */
queue_t *queue_new(int capacity) {
    queue_t *res = (queue_t *)calloc(sizeof(queue_t), 1);
    if (res == NULL) {
        goto cleanup_none;
    }

    res->buf = (void **)calloc(sizeof(void *), capacity);
    if (res->buf == NULL) {
        goto cleanup_res;
    }

    res->capacity = capacity;
    return res;

cleanup_res:
    free(res);

cleanup_none:
    return NULL;
}

/**
 * @brief Checks if the given queue is full
 *
 * @param q The queue to check
 *
 * The explicit assumption here is that this queue is single conusmer single
 * producer. This is critical because `q->size` can change between the access
 * of size, and the comparison with `q->capacity`. This operation is only
 * called by the lone consumer, so if it falsely reports that the queue is
 * full, the worst case scenario is we reject an insertion.
 *
 * @return 0 if not full, EXFULL if full.
 */
int _queue_full(queue_t *q) {
    if (q->size == q->capacity) {
        return 0;
    } else {
        return EXFULL;
    }
}

/**
 * @brief Insert an element
 *
 * @param q The queue to insert into
 * @param v The value to insert
 *
 * @return 0 on success, error code otherwise
 */
int queue_insert(queue_t *q, void *v) {
    int res = 0;
    if ((res = _queue_full(q)) != 0) {
        return res;
    }

    q->buf[q->end] = v;
    q->end = (q->end + 1) % q->capacity;

    return 0;
}
