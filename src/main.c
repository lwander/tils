/**
 * @file main.c
 *
 * @brief Instantiate server here
 *
 * @author Lars Wander (lars.wander@gmail.com)
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <errno.h>

#include <routes.h>

#define HTTP_PORT (80)
#define NUM_THREADS (4)
#define REQUEST_BUF_SIZE (1 << 10)
#define WORD_BUF_SIZE (1 << 7)

#define SERVER_STRING "Server: lwander-c-http/0.0.1\r\n"

typedef enum {
    GET,
    POST,
    PUT,
    HEAD,
    OPTIONS,
    DELETE,
    TRACE,
    CONNECT,
    UNKNOWN
} http_request_t;

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
int read_word(char *ibuf, int ibuf_len, char *obuf, int obuf_len,
        int word_start) {
    int len = 0;
    int i = word_start;
    while (!isspace((int)ibuf[i]) && i < ibuf_len && len < obuf_len - 1) {
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
int next_word(char *buf, int buf_len, int index) {
    while (isspace((int)buf[index]) && index < buf_len)
        index++;
    return index;
}

/**
 * @brief Get the request type from an HTTP header.
 *
 * @param request The HTTP request in string form.
 * @param request_len The size of the request (cannot be greater than the size
 *                    of the buffer).
 *
 * @return The request type.
 */
http_request_t request_type(char *request, int request_len) {
    if (strncmp("GET", request, request_len) == 0)
        return GET;
    else if (strncmp("POST", request, request_len) == 0)
        return POST;
    else if (strncmp("PUT", request, request_len) == 0)
        return PUT;
    else if (strncmp("HEAD", request, request_len) == 0)
        return HEAD;
    else if (strncmp("OPTIONS", request, request_len) == 0)
        return OPTIONS;
    else if (strncmp("DELETE", request, request_len) == 0)
        return DELETE;
    else if (strncmp("TRACE", request, request_len) == 0)
        return TRACE;
    else if (strncmp("CONNECT", request, request_len) == 0)
        return CONNECT;
    else
        return UNKNOWN;
}

/**
 * @brief Send data & vaargs to client
 *
 * @param client_fd The client being communicated with.
 * @param msg The message with format specifiers to be sent.
 * @param ... variable args being formated into msg.
 */
void send_to_client(int client_fd, char *msg, ...) {
    va_list ap;
    char buf[REQUEST_BUF_SIZE + 1];

    va_start(ap, msg);
    vsnprintf(buf, sizeof(buf), msg, ap);
    va_end(ap);

    send(client_fd, buf, strlen(buf), 0);
}

/**
 * @brief Send the unimplemented header to the client
 *
 * @param client_fd The client being communicated with
 */
void serve_unimplemented(int client_fd) {
    send_to_client(client_fd, "HTTP/1.0 501 Method Not Implemented\r\n");
    send_to_client(client_fd, SERVER_STRING);
    send_to_client(client_fd, "Content-Type: text/html\r\n");
    send_to_client(client_fd, "\r\n");
    send_to_client(client_fd, "<html><head><title>Whoops</title></head>\r\n");
    send_to_client(client_fd, "<body>Method type not supported :(</body>\r\n");
    send_to_client(client_fd, "</html>\r\n");
}

/**
 * @brief Send the unimplemented header to the client
 *
 * @param client_fd The client being communicated with
 */
void serve_not_found(int client_fd) {
    send_to_client(client_fd, "HTTP/1.0 404 Not Found\r\n");
    send_to_client(client_fd, SERVER_STRING);
    send_to_client(client_fd, "Content-Type: text/html\r\n");
    send_to_client(client_fd, "\r\n");
    send_to_client(client_fd, "<html><head><title>Whoops</title></head>\r\n");
    send_to_client(client_fd, "<body>Resource not found :(</body>\r\n");
    send_to_client(client_fd, "</html>\r\n");
}

void serve_file(int client_fd, FILE *file) {
    send_to_client(client_fd, "HTTP/1.0 404 Not Found\r\n");
    send_to_client(client_fd, SERVER_STRING);
    send_to_client(client_fd, "Content-Type: text/html\r\n");
    send_to_client(client_fd, "\r\n");

    char buf[REQUEST_BUF_SIZE];
    do {
        fgets(buf, sizeof(buf), file);
        send_to_client(client_fd, buf);
    } while (!feof(file));
}

void serve_resource(int client_fd, http_request_t http_request, char *resource) {
    if (http_request != GET) {
        serve_unimplemented(client_fd);
        return;
    }

    char *translate_resource;
    FILE *file = NULL;

    if (lookup_route(resource + 1, &translate_resource) == 0)
        file = fopen(translate_resource, "r");
    else
        file = fopen(resource + 1, "r"); // Plus one to avoid "/" prefix

    if (file == NULL)
        serve_not_found(client_fd);
    else 
        serve_file(client_fd, file);
}

/**
 * @brief Process the type of HTTP request, and respond accordingly.
 *
 * @param client_fd File descriptor of client sending data.
 */
void accept_request(int client_fd) {
    char request[REQUEST_BUF_SIZE];
    char word[WORD_BUF_SIZE];
    char resource[WORD_BUF_SIZE];
    int request_len = 0;
    int index = 0;
    int word_len = 0;
    http_request_t http_request;

    /* First grab the full HTTP request */
    request_len = read(client_fd, request, REQUEST_BUF_SIZE);

    /* Start parsing it word by word */
    index = next_word(request, request_len, 0);
    word_len = read_word(request, REQUEST_BUF_SIZE, word, WORD_BUF_SIZE, index);
    http_request = request_type(word, word_len);

    index = next_word(request, request_len, index + word_len);
    word_len = read_word(request, REQUEST_BUF_SIZE, resource, WORD_BUF_SIZE,
            index);

    serve_resource(client_fd, http_request, resource);
    return;
}

/**
 * @brief Wait to be connected to a client, then handle the client's request,
 *        and repeat.
 *
 * @param server_fd The socket fd to accept connections on
 */
void *handle_connections(void *server_fd) {
    struct sockaddr_in ip4client;
    unsigned int ip4client_len = sizeof(ip4client);
    int client_fd;
    while (1) {
        if ((client_fd = accept(*((int *)server_fd),
                        (struct sockaddr *)&ip4client, &ip4client_len)) < 0) {
            fprintf(stderr, "Error, connection dropped (errno %d)", errno);
        } else {
            accept_request(client_fd);
            close(client_fd);
        }
    }

    return NULL;
}

/**
 * @brief Bind server to HTTP TCP socket.
 *
 * @return -1 on error, the socket file descriptor otherwise.
 */
int init_server() {
    struct sockaddr_in ip4server;
    int server_fd = 0;

    ip4server.sin_family = AF_INET; /* Address family internet */
    ip4server.sin_port = htons(HTTP_PORT); /* Bind to port 80 */
    ip4server.sin_addr.s_addr = htonl(INADDR_ANY);  /* Bind to any interface */

    /* Get a file descriptor for our socket */
    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Unable to create socket (errno %d)\n", errno);
        goto fail;
    }

    /* Bind the socket file descriptor to our network interface */
    if (bind(server_fd, (struct sockaddr *)&ip4server, sizeof(ip4server)) < 0) {
        fprintf(stderr, "Unable to bind socket (errno %d)\n", errno);
        goto cleanup_socket;
    }

    /* Listen for connections on this socket. AFAIK, the second argument
     * (backlog) is a suggestion, not a hard value. */
    if (listen(server_fd, 16) < 0) {
        fprintf(stderr, "Unable to listen on socket (errno %d)\n", errno);
        goto cleanup_socket;
    }

    return server_fd;


cleanup_socket:
    close(server_fd);

fail:
    return -1;
}

int main(int argc, char *argv[]) {
    pthread_t threads[NUM_THREADS];

    fprintf(stdout, "Building routes... ");
    if (init_routes() < 0) {
        fprintf(stdout, "failed.\n");
        goto cleanup_socket;
    }
    if (add_route("", "html/index.html") < 0) {
        fprintf(stdout, "failed.\n");
        goto cleanup_socket;
    }

    fprintf(stdout, "done.\n");

    fprintf(stdout, "Opening connection... ");
    int server_fd = 0;
    if ((server_fd = init_server()) < 0) {
        fprintf(stdout, "failed.\n");
        return -1;
    } else {
        fprintf(stdout, "done.\n");
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, handle_connections,
                (void *)(&server_fd));

    while (1) {
    }

cleanup_socket:
    close(server_fd);
    return 0;
}
