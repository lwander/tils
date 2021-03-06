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
 * @file src/routes.c
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
int tils_routes_init() {
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
int tils_route_add(char *source, char *dest) {
    return htable_insert(_routes, source, (void *)dest);    
}

/**
 * @brief Lookup a route entry.
 */
int tils_route_lookup(char *source, char **dest) {
    return htable_lookup(_routes, source, (void **)dest);
}

/**
 * @brief free all route resources
 */
void tils_routes_cleanup() {
    htable_free(_routes, NULL);
}
