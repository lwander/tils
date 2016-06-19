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
 * @return 0 if not full, 1 if full
 */
int _queue_full(queue_t *q) {
    // `memory_order_relaxed` because we don't access memory that can be
    // modified elsewhere in this function
    int size = atomic_load_explicit(&q->size, memory_order_relaxed);

    if (size == q->capacity) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Check if the queue is empty
 *
 * @param q The queue to check
 *
 * The explicit assumption here is that this queue is single conusmer single
 * producer. This is critical because `q->size` can change between the access
 * of size, and the comparison with 0. This operation is only
 * called by the lone consumer, so if it falsely reports that the queue is
 * empty, the worst case scenario is we reject a removal.
 *
 * @return 0 if not empty, 1 if empty
 */
int _queue_empty(queue_t *q) {
    // `memory_order_relaxed` because we don't access memory that can be
    // modified elsewhere in this function
    int size = atomic_load_explicit(&q->size, memory_order_relaxed);

    if (size == 0) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Insert an element
 *
 * @param q The queue to insert into
 * @param v The value to insert
 *
 * @return 0 on success, -1 if no room to insert
 */
int queue_insert(queue_t *q, void *v) {
    if (_queue_full(q)) {
        return -1;
    }

    q->buf[q->end] = v;
    q->end = (q->end + 1) % q->capacity;

    // Important that this operation acts as a memory barrier, to ensure we
    // have written to the queue by the time this operation completes.
    atomic_fetch_add(&q->size, 1);
    return 0;
}

/**
 * @brief Remove an element
 *
 * @param q The queue to insert into
 * @param v A pointer to memory that will hold the resulting value.
 *
 * @return 0 on success, -1 if empty
 */
int queue_remove(queue_t *q, void **v) {
    if (v == NULL) {
        return EINVAL;
    }

    if (_queue_empty(q)) {
        return -1;
    }

    *v = q->buf[q->start];
    q->start = (q->start + 1) % q->capacity;

    // Important that this operation acts as a memory barrier, to ensure we
    // have read from the queue by the time this operation completes.
    atomic_fetch_sub(&q->size, 1);
    return 0;
}
