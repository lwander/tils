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

#include <stdlib.h>

#include "queue_private.h"

/**
 * @brief Allocate a fresh, empty queue
 */
queue_t *queue_new() {
    queue_t * res = (queue_t *)calloc(sizeof(queue_t), 1);

    return res;
}
