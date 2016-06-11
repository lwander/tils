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
 * @file lib/util.h
 *
 * @brief Benchmarking tools.
 *
 * @author Lars Wander
 */

#ifndef _BENCH_H_
#define _BENCH_H_

#include <sys/time.h>

#include <lib/util.h>

#ifdef USE_BENCH

#define INIT_BENCH struct timeval _b_start, _b_stop
#define START_BENCH gettimeofday(&_b_start, NULL)
#define STOP_BENCH gettimeofday(&_b_stop, NULL)
#define LOG_BENCH(r) printf(",- %s : %lu us\n", (r), \
        _b_stop.tv_usec - _b_start.tv_usec) \
    
#else

#define INIT_BENCH 
#define START_BENCH(r) 
#define STOP_BENCH 
#define LOG_BENCH 

#endif /* USE_BENCH */

#endif /* _BENCH_H_ */
