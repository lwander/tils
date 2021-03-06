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
 * @file inc/util.h
 *
 * @brief Basic utility functions / macros
 *
 * @author Lars Wander
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <time.h>

#define USE_BENCH 

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
#define REQUEST_BUF_SIZE (1 << 12)
#define WORD_BUF_SIZE (1 << 7)

#define TIME_NOW (((double) (clock())) / CLOCKS_PER_SEC)

#define LIKELY(x)       __builtin_expect((x),1)
#define UNLIKELY(x)     __builtin_expect((x),0)

#endif /* _UTIL_H_ */
