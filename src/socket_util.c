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
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <util.h>

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
        fprintf(stdout, ERROR "Unable to set keepalive to %d (%s)\n",
                optval, strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * @brief Set fd to not block on accept/read/recv/send
 *
 * @param sock The fd being modified
 *
 * @return 0 on success, < 0 otherwise
 */
int fd_nonblocking(int sock) {
    int res;
    if ((res = fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | 
                    O_NONBLOCK)) < 0) {
        fprintf(stdout, ERROR "Unable set status to non-blocking (%s)\n",
                strerror(errno));
        return res;
    }

    return 0;
}

/**
 * @brief Set fd to block on accept/read/recv/etc
 *
 * @param sock The fd being modified
 *
 * @return 0 on success, < 0 otherwise
 */
int fd_blocking(int sock) {
    int res;
    if ((res = fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) & 
                    ~O_NONBLOCK)) < 0) {
        fprintf(stdout, ERROR "Unable set status to blocking (%s)\n",
                strerror(errno));
        return res;
    }

    return 0;
}

/**
 * @brief Get file size for input fd 
 *
 * @param fd The file descriptor being examined
 *
 * @return size on success, < 0 on failure
 */
int fd_size(int fd) {
    struct stat st;
    int res;
    if ((res = fstat(fd, &st)) < 0) {
        fprintf(stdout, ERROR "Getting file info (%s)\n", strerror(errno));
        return res;
    }

    return st.st_size;
}
