/**
 * @file routes.c
 *
 * @author Lars Wander (lars.wander@gmail.com)
 *
 * @brief Resource request -> file mapping is handled here.
 */

#include <stdlib.h>
#include <stdio.h>

#include <lib/hashtable.h>

static htable_t *_routes = NULL;

/**
 * @brief Setup routing table 
 */
int init_routes() {
    _routes = htable_new();
    if (_routes == NULL) {
        return -1;
    } else {
        return 0;
    }
}

/**
 * @brief Add a route entry. Whenever source is encountered, dest is served 
 */
int add_route(char *source, char *dest) {
    return htable_insert(_routes, source, (void *)dest);    
}

/**
 * @brief Lookup a route entry.
 */
int lookup_route(char *source, char **dest) {
    return htable_lookup(_routes, source, (void **)dest);
}
