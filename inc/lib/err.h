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
 * @file inc/err.h
 *
 * @brief Error macros & defines
 *
 * @author Lars Wander 
 */

#ifndef _ERR_H_
#define _ERR_H_

#include <assert.h>

void err_set_msg(const char *msg);
void err_report(const char *msg, int err, ...);
const char *err_to_string(int err);

#define _DEBUG_ (1)

#define ERR_OOB (-3)

#define ERR_INP (-8)
#define ERR_CORRUPT (-9)
#define ERR_FILE_ACTION (-10)
#define ERR_MEM_ALLOC (-11)
#define ERR_SEMANTICS (-12)
#define ERR_BAD_PARSE (-13)
#define ERR_UNBOUND_VAR (-14)

#endif /* _ERR_H_ */
