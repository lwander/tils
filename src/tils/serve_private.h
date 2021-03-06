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
 * @file src/serve_private.c
 *
 * @brief 'secret' details of serve implementation go here.
 *
 * @author Lars Wander (lars.wander@gmail.com)
 */

#ifndef _SERVE_PRIVATE_H_
#define _SERVE_PRIVATE_H_

const char* msg_unimplemented = "HTTP/1.1 501 Method Not Implemented\r\n"
SERVER_STRING
"Content-Type: text\r\n"
"Content-Length: 18\r\n"
"\r\n"
"Not implemented.\r\n";

const char *msg_not_found = "HTTP/1.1 404 Not Found\r\n"
SERVER_STRING
"Content-Type: text/html\r\n"
"Content-Length: 5\r\n"
"\r\n"
"404\r\n";

const char *header_file = "HTTP/1.1 200 OK\r\n"
SERVER_STRING
"Content-Type: %s\r\n"
"Content-Length: %d\r\n"
"Connection: keep-alive\r\n"
"\r\n";

#endif /* _SERVE_PRIVATE_H_ */
