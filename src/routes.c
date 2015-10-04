/**
 * @file routes.c
 *
 * @author Lars Wander (lars.wander@gmail.com)
 *
 * @brief Resource request -> file mapping is handled here.
 */

#include <lib/hashtable.h>

static htable_t _routes;

/**
 * @brief Setup routing table 
 */
int init_routes() {
    return htable_init(&_routes);
}

/**
 * @brief Add a route entry. Whenever source is encountered, dest is served 
 */
int add_route(char *source, char *dest) {
    return htable_insert(&_routes, source, (void *)dest);    
}

/**
 * @brief Lookup a route entry.
 */
int lookup_route(char *source, char **dest) {
    return htable_lookup(&routes, source, (void **)dest);
}
