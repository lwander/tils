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
 * @file src/request.c
 *
 * @brief Request parsing implementation.
 *
 * @author Lars Wander
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <tils/request.h>
#include <lib/util.h>

/**
 * @brief Get the request type from an HTTP header.
 *
 * @param request The HTTP request in string form.
 * @param request_len The size of the request (cannot be greater than the size
 *                    of the buffer).
 *
 * @return The request type.
 */
tils_http_request_e _tils_request_type(char *request, int request_len) {
    if (strncmp("GET", request, request_len) == 0)
        return TILS_GET;
    else if (strncmp("POST", request, request_len) == 0)
        return TILS_POST;
    else if (strncmp("PUT", request, request_len) == 0)
        return TILS_PUT;
    else if (strncmp("HEAD", request, request_len) == 0)
        return TILS_HEAD;
    else if (strncmp("OPTIONS", request, request_len) == 0)
        return TILS_OPTIONS;
    else if (strncmp("DELETE", request, request_len) == 0)
        return TILS_DELETE;
    else if (strncmp("TRACE", request, request_len) == 0)
        return TILS_TRACE;
    else if (strncmp("CONNECT", request, request_len) == 0)
        return TILS_CONNECT;
    else
        return TILS_UNKNOWN;
}

/**
 * @brief Read a whitespace delinated word out of ibuf and into obuf.
 *
 * @param ibuf The input buffer being read from.
 * @param ibuf_len The input buffer length.
 * @param obuf The output buffer being read from.
 * @param obuf_len The output buffer length.
 * @param word_start The start of the word being read.
 *
 * @return The length of the word that was read.
 */
int _read_word(char *ibuf, int ibuf_len, char *obuf, int obuf_len,
        int word_start) {
    int len = 0;
    int i = word_start;
    while (i < ibuf_len && len < obuf_len - 1 && !isspace((int)ibuf[i])) {
        obuf[len] = ibuf[i];
        i++;
        len++;
    }

    obuf[len] = '\0';
    return len;
}

/**
 * @brief Finds the next non-whitespace character starting at index.
 *
 * @param buf The buffer being searched.
 * @param buf_len The length of the buffer being searched.
 * @param index The index the search starts from.
 *
 * @return The index of the first non-whitespace charcter after index.
 */
int _next_word(char *buf, int buf_len, int index) {
    while (index < buf_len && isspace((int)buf[index]))
        index++;
    return index;
}

/**
 * @brief Frees a tils request.
 *
 * @param request Request to free.
 */
void tils_free_request(tils_http_request_t *request) {
    if (request == NULL)
        return;
    if (request->resource != NULL)
        free(request->resource);
    free(request);
}

/**
 * @brief Parse an incoming HTTP request.
 *
 * @param request A buffer containing the actual request.
 * @param request_len The length of the request buffer.
 *
 * @return The parsed request as a `tils_http_request_t`.
 */
tils_http_request_t *tils_parse_request(char *request, int request_len) {
    char word[WORD_BUF_SIZE];
    char resource[WORD_BUF_SIZE];
    int word_len = 0;
    tils_http_request_e request_type = TILS_UNKNOWN;
    tils_http_request_t *result = malloc(sizeof(tils_http_request_t));
    if (result == NULL)
        return result;

    /* Start parsing it word by word */
    /* TODO More robust parsing of requests */
    int index = _next_word(request, request_len, 0);
    word_len = _read_word(request, REQUEST_BUF_SIZE, word, WORD_BUF_SIZE, index);
    request_type = _tils_request_type(word, word_len);

    index = _next_word(request, request_len, index + word_len);
    word_len = _read_word(request, REQUEST_BUF_SIZE, resource, WORD_BUF_SIZE,
            index);

    result->request_type = request_type;
    result->resource = malloc(word_len + 1);
    strncpy(result->resource, resource, word_len);
    result->resource[word_len] = '\0';

    return result;
}
