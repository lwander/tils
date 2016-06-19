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
 * @file src/lib/queue_private.h
 *
 * @brief Queue data structure internals
 *
 * @author Lars Wander 
 */

#ifndef _QUEUE_PRIVATE_H_
#define _QUEUE_PRIVATE_H_

typedef struct queue {
    /* Ring buffer is a list of void* pointers. */
    void **buf;
    
    /* First element in queue */
    int start;

    /* First non-empty element in queue */
    int end;

    /* Max size of the buffer */
    int capacity;

    /* Number of stored elements */
    int size;
} queue_t;

#endif /* _QUEUE_PRIVATE_H_ */
