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
 * @file src/lib/logging.c
 *
 * @brief Logging utilities
 *
 * @author Lars Wander
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>

#include <lib/logging.h>

void _fmt_time(char *buf, int buf_len) {
    time_t timer;
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);
    strftime(buf, buf_len, "%Y:%m:%d %H:%M:%S", tm_info);
}

void _log_format(const char *format, char *msg_buf, va_list ap) {
    char err_buf[MAX_LOG_LENGTH];
    char time_buf[MAX_TIME_LENGTH];

    _fmt_time(time_buf, MAX_TIME_LENGTH);

    if (errno != 0) {
        char errno_buf[MAX_ERRNO_LENGTH];
        strerror_r(errno, errno_buf, MAX_ERRNO_LENGTH);
        sprintf(err_buf, "[%s] \t%s (%s)", time_buf, format, errno_buf);
    } else {
        sprintf(err_buf, "[%s] \t%s", time_buf, format);
    }

    vsnprintf(msg_buf, MAX_LOG_LENGTH, err_buf, ap);
}

void log_info(const char *format, ...) {
    char msg_buf[MAX_LOG_LENGTH];

    va_list ap;
    va_start(ap, format);

    _log_format(format, msg_buf, ap);

    fprintf(stdout, INFO "%s\n", msg_buf);
    va_end(ap);
}

void log_warn(const char *format, ...) {
    char msg_buf[MAX_LOG_LENGTH];

    va_list ap;
    va_start(ap, format);

    _log_format(format, msg_buf, ap);

    fprintf(stdout, WARN "%s\n", msg_buf);
    va_end(ap);
}

void log_err(const char *format, ...) {
    char msg_buf[MAX_LOG_LENGTH];

    va_list ap;
    va_start(ap, format);

    _log_format(format, msg_buf, ap);

    fprintf(stdout, ERROR "%s\n", msg_buf);
    va_end(ap);
}
