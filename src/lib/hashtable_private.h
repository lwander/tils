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
