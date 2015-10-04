/**
 * @file hashtable.h
 *
 * @brief Hash table definition
 *
 * Hashes strings to void pointers
 *
 * @author Lars Wander
 */ 

#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

struct _htable;
typedef struct htable htable_t;

htable_t *htable_new();
int htable_insert(htable_t *ht, char *key, void *value);
int htable_lookup(htable_t *ht, char *key, void **value);
int htable_delete(htable_t *ht, char *key, void **value);
void htable_free(htable_t *ht, void (*free_value)(void *));

#endif /* _HASH_TABLE_H_ */
