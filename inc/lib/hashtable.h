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
 * @file inc/lib/hashtable.h
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
