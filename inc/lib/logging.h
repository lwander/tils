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

#ifndef _LOGGING_H_
#define _LOGGING_H_

#define ANSI_BOLD    "\x1b[1m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_RESET   "\x1b[0m"
#define ANSI_CLEAR   "\r\x1b[K"

#define INFO   ANSI_BOLD " [INFO] " ANSI_RESET
#define WARN   ANSI_BOLD ANSI_YELLOW " [WARN] " ANSI_RESET
#define ERROR  ANSI_BOLD ANSI_RED "[ERROR] " ANSI_RESET

#define MAX_LOG_LENGTH  (256)
#define MAX_TIME_LENGTH  (26)
#define MAX_ERRNO_LENGTH  (128)

void log_info(const char *format, ...);
void log_warn(const char *format, ...);
void log_err(const char *format, ...);

#endif /* _LOGGING_H_ */
