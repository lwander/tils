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
 * @file src/socket_util.c
 *
 * @brief Abstract more common socket manipulation here
 *
 * @author Lars Wander (lars.wander@gmail.com)
 */

#include <socket_util.h>

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

/**
 * @brief set keepalive state for input socket
 *
 * @param sock The socket being modified
 *
 * @return 0 on success, < 0 otherwise
 */
int socket_keepalive(int sock) {
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    /* Set keepalive status */
    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        fprintf(stderr, "Unable to set keepalive to %d (errno %d)\n",
                optval, errno);
        return -1;
    }

    return 0;
}

/**
 * @brief Set socket to not block on accept
 *
 * @param sock The socket being modified
 *
 * @return 0 on success, < 0 otherwise
 */
int socket_nonblocking(int sock) {
    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) < 0) {
        fprintf(stderr, "Unable set status to non-blocking (errno %d)\n",
                errno);
        return -1;
    }

    return 0;
}

/**
 * @brief Set socket to not block on accept
 *
 * @param sock The socket being modified
 *
 * @return 0 on success, < 0 otherwise
 */
int socket_blocking(int sock) {
    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK) < 0) {
        fprintf(stderr, "Unable set status to non-blocking (errno %d)\n",
                errno);
        return -1;
    }

    return 0;
}
