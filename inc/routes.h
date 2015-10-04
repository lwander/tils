/**
 * @file routes.h
 *
 * @author Lars Wander (lars.wander@gmail.com)
 *
 * @brief Routing definitions go here
 */

#ifndef _ROUTES_H_
#define _ROUTES_H_

int init_routes();
int add_route(char *source, char *dest);
int lookup_route(char *source, char **dest);

#endif /* _ROUTES_H_ */
