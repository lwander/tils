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
 * @file src/lib/hashtable.c
 *
 * @brief Hash table implementation
 *
 * The hash table is an array linked lists. Elements that hash to the
 * same value are stored in the same list inside the array.
 *
 * @author Lars Wander
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lib/hashtable.h>
#include <err.h>
#include <util.h>

#include "hashtable_private.h"

/**
 * @brief Compute index into hashtable for given key
 *
 * @param ht Hash table the index is computed for
 * @param key Key being hashed
 *
 * @return Hashed key -> index
 */
unsigned int htable_hash(htable_t *ht, char *key) {
    int i = 0;
    unsigned int hash = 0;
    char ch;
    while ((ch = key[i]) != '\0') {
        hash += (HASH_LINEAR * ch + HASH_OFFSET);
        i++;
        if (i > HTABLE_MAX_KEY_LEN)
            break;
    }

    return hash % ht->table_size;
}

htable_t *htable_new() {
    htable_t *res = calloc(sizeof(htable_t), 1);
    if (res == NULL)
        return NULL;

    res->table = calloc(sizeof(hnode_t *), HTABLE_INIT_SIZE);
    res->table_size = HTABLE_INIT_SIZE;

    if (res->table == NULL) {
        free(res);
        return NULL;
    }

    return res;
}

/**
 * @brief Insert (key, value) into ht. Will overwrite if a (key, value') pair
 *        exists already
 *
 * @param ht Hashtable being inserted into
 * @param key Key being associated with value
 * @param value Value being inserted
 *
 * @return 0 on success, ERR_* otherwise
 */
int htable_insert(htable_t *ht, char *key, void *value) {
    if (ht == NULL)
        return ERR_INP;

    int ind = htable_hash(ht, key);
    hnode_t **hnode_p = ht->table + ind;
    while (*hnode_p != NULL) {
        /* Overwrite old value on collision */
        if (strncmp((*hnode_p)->key, key, HTABLE_MAX_KEY_LEN) == 0) {
            (*hnode_p)->value = value;
            return 0;
        }

        hnode_p = &(*hnode_p)->next;
    }

    /* Key wasn't found, so allocate a fresh node */
    *hnode_p = malloc(sizeof(hnode_t));
    if (*hnode_p == NULL)
        return ERR_MEM_ALLOC;

    size_t nlen;
    MIN(nlen, strlen(key), HTABLE_MAX_KEY_LEN);
    if (((*hnode_p)->key = malloc(nlen + 1)) == NULL)
        goto cleanup_hnode;

    strncpy((*hnode_p)->key, key, nlen + 1);
    (*hnode_p)->key[nlen] = '\0';

    (*hnode_p)->value = value;

    (*hnode_p)->next = NULL;

    return 0;

cleanup_hnode:
    free(*hnode_p);
    return ERR_MEM_ALLOC;
}

/**
 * @brief Find (key, value) in ht
 *
 * @param ht Hash table being searched
 * @param key Key associated with value being searched
 * @param[out] value Pointer to where value will be stored if not NULL
 *
 * @return 0 on success, -1 if key was not found, ERR_* otherwise
 */
int htable_lookup(htable_t *ht, char *key, void **value) {
    if (ht == NULL)
        return ERR_INP;

    int ind = htable_hash(ht, key);
    hnode_t **hnode_p = ht->table + ind;
    while (*hnode_p != NULL) {
        if (strncmp((*hnode_p)->key, key, HTABLE_MAX_KEY_LEN) == 0) {
            if (value != NULL)
                *value = (*hnode_p)->value;
            return 0;
        }

        hnode_p = &(*hnode_p)->next;
    }

    return -1;
}

/**
 * @brief Delete (key, value) in ht
 *
 * @param ht Hash table being modified
 * @param key Key being deleted
 * @param value[out] Places deleted value in here if not NULL
 *
 * @return 0 on success, -1 if key was not found, ERR_* otherwise
 */
int htable_delete(htable_t *ht, char *key, void **value) {
    if (ht == NULL)
        return ERR_INP;

    int ind = htable_hash(ht, key);
    hnode_t **hnode_p = ht->table + ind;
    while (*hnode_p != NULL) {
        if (strncmp((*hnode_p)->key, key, HTABLE_MAX_KEY_LEN) == 0) {
            if (value != NULL)
                *value = (*hnode_p)->value;

            hnode_t *next = (*hnode_p)->next;
            free((*hnode_p)->key);
            free(*hnode_p);
            *hnode_p = next;
            return 0;
        }

        hnode_p = &(*hnode_p)->next;
    }

    return -1;
}


/**
 * @brief Free the entire hash table
 *
 * @param ht Hash table to be freed
 * @param free_val custome function for free htable values (NULL if not-used)
 *
 */
void htable_free(htable_t *ht, void (*free_val)(void *)) {
    int i;
    for (i = 0; i < ht->table_size; i++) {
        hnode_t *hnode_p = ht->table[i];
        while (hnode_p != NULL) {
            hnode_t *next = hnode_p->next;
            free(hnode_p->key);
            if (free_val != NULL)
                (*free_val)(hnode_p->value);

            free(hnode_p);
            hnode_p = next;
        }
    }

    free(ht->table);
    free(ht);
}
