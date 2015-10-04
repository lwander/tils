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
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

#define HTTP_PORT (80)
#define NUM_THREADS (4)

/**
 * @brief Process the type of HTTP request, and respond accordingly.
 *
 * @param client_fd File descriptor of client sending data
 */
void accept_request(int client_fd) {
    char *buf = NULL;
    size_t buf_len = 0;
    int read = 0;
    while ((read = getline(&buf, &buf_len, client_fd)) > 0) {
        printf("%d - %s", read, buf);
    }
    return; 
}

/**
 * @brief Wait to be connected to a client, then handle the client's request, 
 *        and repeat.
 *
 * @param server_fd The socket fd to accept connections on
 */
void handle_connections(void *server_fd) {
    struct sockaddr_in ip4client;
    unsigned int ip4client_len = sizeof(ip4client);
    int client_fd;
    while (1) {
        if ((client_fd = accept(*((int *)server_fd), 
                        (struct sockaddr *)&ip4client, &ip4client_len)) < 0) {
            fprintf(stderr, "Error, connection dropped (errno %d)", errno);
        } else {
            accept_request(client_fd);
        }
    }
}

/**
 * @brief Bind server to HTTP TCP socket. 
 *
 * @return -1 on error, the socket file descriptor otherwise.
 */
int init_server() {
    struct sockaddr_in ip4server;
    ip4server.sin_family = AF_INET; /* Address family internet */
    ip4server.sin_port = htons(HTTP_PORT); /* Bind to port 80 */
    ip4server.sin_addr.s_addr = htonl(INADDR_ANY);  /* Bind to any interface */

    /* Get a file descriptor for our socket */
    int server_fd = 0;
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
    printf("Opening connection... ");
    int server_fd = 0;
    if ((server_fd = init_server()) < 0) {
        printf("failed.\n");
        return -1;
    } else {
        printf("done.\n");
    }

    close(server_fd);
    return 0;
}
