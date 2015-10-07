/*
 *  This file is part of c-http.
 *
 *      c-http is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      c-http is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 * 
 *      You should have received a copy of the GNU General Public License
 *      along with c-http.  If not, see <http://www.gnu.org/licenses/>
 */

/**
 * @file hastable_private.h
 *
 * @brief Hash table data structure internals
 *
 * @author Lars Wander 
 */

#ifndef _HASH_TABLE_PRIVATE_H_
#define _HASH_TABLE_PRIVATE_H_

/* Minimum table size : element count ratio */
#define HTABLE_LOAD_FACTOR (4)

/* Starting table size */
#define HTABLE_INIT_SIZE (16)

/* Linear hash constants to compute hash = HASH_LINEAR * key + HASH_OFFSET
 * Taken from CMU 15-122 (S12) HW6 hashmap code */
#define HASH_LINEAR 1664525
#define HASH_OFFSET 1013904223

#define HTABLE_MAX_KEY_LEN (1 << 10)

/**
 * @brief Node containing hash data & key pair
 */
typedef struct hnode {
    char *key;
    void *value;
    /* Pointer to next hnode in hashmap list */
    struct hnode *next;
} hnode_t;

typedef struct htable {
    /* Size of underlying array */
    int table_size;
    /* # of elements contained in the table */
    int elem_count;
    /* Array of pointers to hnodes */
    hnode_t **table;
} htable_t;

#endif /* _HASH_TABLE_PRIVATE_H_ */
