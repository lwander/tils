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
 * @file inc/io_util.h
 *
 * @brief Abstract more common IO handler manipulation here
 *
 * @author Lars Wander (lars.wander@gmail.com)
 */

int tils_socket_keepalive(int sock);
int tils_fd_nonblocking(int fd);
int tils_fd_blocking(int fd);
int tils_fd_size(int fd);
