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
 * @file inc/util.h
 *
 * @brief Basic utility functions / macros
 *
 * @author Lars Wander
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <time.h>

#define ANSI_BOLD    "\x1b[1m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_RESET   "\x1b[0m"
#define ANSI_CLEAR   "\r\x1b[K"

#define INFO   ANSI_BOLD "[INFO] " ANSI_RESET
#define ERROR  ANSI_BOLD ANSI_RED "[ERROR] " ANSI_RESET

#define MAX(res, a, b) \
   do { __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     res = (_a > _b ? _a : _b); } while (0)

#define MIN(res, a, b) \
   do { __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     res = (_a < _b ? _a : _b); } while (0)

#define CACHE_LINE_SIZE (64)

#define HTTP_PORT (80)
#define REQUEST_BUF_SIZE (1 << 10)
#define WORD_BUF_SIZE (1 << 7)

#define TIME_NOW (((double) (clock())) / CLOCKS_PER_SEC)

#endif /* _UTIL_H_ */
