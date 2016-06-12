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
 * @file src/io_util.c
 *
 * @brief Abstract more common IO handler manipulation here
 *
 * @author Lars Wander (lars.wander@gmail.com)
 */


#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <lib/util.h>
#include <lib/logging.h>
#include <tils/io_util.h>

/**
 * @brief set keepalive state for input socket
 *
 * @param sock The socket being modified
 *
 * @return 0 on success, < 0 otherwise
 */
int tils_socket_keepalive(int sock) {
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    /* Set keepalive status */
    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        log_err("Unable to set keepalive to %d", optval);
        return -1;
    }

    return 0;
}

/**
 * @brief Set fd to not block on accept/read/recv/send
 *
 * @param fd The fd being modified
 *
 * @return 0 on success, < 0 otherwise
 */
int tils_fd_nonblocking(int fd) {
    int res;
    if ((res = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) |
                    O_NONBLOCK)) < 0) {
        log_err("Unable set status to non-blocking");
        return res;
    }

    return 0;
}

/**
 * @brief Set fd to block on accept/read/recv/etc
 *
 * @param fd The fd being modified
 *
 * @return 0 on success, < 0 otherwise
 */
int tils_fd_blocking(int fd) {
    int res;
    if ((res = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) &
                    ~O_NONBLOCK)) < 0) {
        log_err("Unable set status to blocking");
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
int tils_fd_size(int fd) {
    struct stat st;
    int res;
    if ((res = fstat(fd, &st)) < 0) {
        log_err("Getting file info");
        return res;
    }

    return st.st_size;
}
