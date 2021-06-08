#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/select.h>

#include "util.h"

static int err_base_val = 1;

int make_server(short port, int maxqueue, int nonblock)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("Socket creation error");
        exit(err_base_val);
    }
    fprintf(stderr, "Socket made.\n");

    if (nonblock) {
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Binding error");
        exit(err_base_val + 1);
    }
    fprintf(stderr, "Socket binded.\n");

    if (listen(fd, maxqueue) < 0) {
        perror("Listening error");
        exit(err_base_val + 2);
    }
    fprintf(stderr, "Socket listened.\n");

    return fd;
}

int accept_connection(int fd, int nonblock)
{
    struct sockaddr_in cli_addr;
    socklen_t cli_len;

    int clientfd = accept(fd, (struct sockaddr *) &cli_addr, &cli_len);
    if (clientfd < 0) {
        perror("Accepting error");
        exit(err_base_val + 3);
    }
    printf("Connection accepted.\n");

    if (nonblock) {
        setsockopt(clientfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
        fcntl(clientfd, F_SETFL, fcntl(clientfd, F_GETFL, 0) | O_NONBLOCK);
    }

    return clientfd;
}

int make_client(char *serv_hostname, short serv_port, int nonblock)
{
    struct hostent *entry = gethostbyname(serv_hostname);
    if (entry == NULL) {
        herror("Host resolve error");
        exit(err_base_val + 4);
    }
    fprintf(stderr, "Host resolved.\n");

    int serv_fd;
    if ((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(err_base_val + 5);
    }
    fprintf(stderr, "Socket created.\n");

    if (nonblock) {
        setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
        fcntl(serv_fd, F_SETFL, fcntl(serv_fd, F_GETFL, 0) | O_NONBLOCK);
    }

    struct sockaddr_in serv_addr;
    socklen_t serv_len = sizeof(serv_addr);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, entry->h_addr_list[0], entry->h_length);
    serv_addr.sin_port = htons(serv_port);

    if (connect(serv_fd, (struct sockaddr *) &serv_addr, serv_len) < 0) {
        perror("Connect error");
        exit(err_base_val + 6);
    }

    fprintf(stderr, "Connected.\n");

    return serv_fd;
}

void send_wrapper(int fd, char *msg, char *errmsg)
{
    fprintf(stderr, "%s: Sending...\n", errmsg);
    fprintf(stderr, "%s: Content: %s\n", errmsg, msg);
    if (send(fd, msg, strlen(msg), 0) < 0) {
        perror(errmsg);
        exit(err_base_val + 7);
    }
    fprintf(stderr, "%s: Sent.\n", errmsg);
}

void recv_wrapper(int fd, char *msg, int size, char *errmsg)
{
    fprintf(stderr, "%s: Receiving...\n", errmsg);
    memset(msg, 0, size);
    int status;
    if ((status = recv(fd, msg, size, 0)) < 0) {
        perror(errmsg);
        exit(err_base_val + 8);
    } else if (status == 0) {
        shutdown(fd, SHUT_RDWR);
        close(fd);
        msg = NULL;
        fprintf(stderr, "Socket hung up.\n");
        exit(err_base_val + 9);
    }

    fprintf(stderr, "%s: Content: %s\n", errmsg, msg);
    fprintf(stderr, "%s: Received.\n", errmsg);
}

void make_file(char *buffer, char *filename)
{
    FILE *file = fopen(filename, "w");
    fprintf(file, "%s\n", buffer);
    fclose(file);
}

void make_buffer(char *buffer, char *filename)
{
    FILE *file = fopen(filename, "r");
    char line[1024] = { 0 };
    while (fgets(line, sizeof(line), file)) {
        strncat(buffer, line, strlen(line));
        memset(line, 0, sizeof(line));
    }
    fclose(file);
}
