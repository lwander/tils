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
 * @file inc/routes.h
 *
 * @author Lars Wander (lars.wander@gmail.com)
 *
 * @brief Routing definitions go here
 */

#ifndef _ROUTES_H_
#define _ROUTES_H_

int tils_routes_init();
void tils_routes_cleanup();
int tils_route_add(char *source, char *dest);
int tils_route_lookup(char *source, char **dest);

#endif /* _ROUTES_H_ */
