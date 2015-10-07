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
