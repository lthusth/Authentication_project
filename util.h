#ifndef __UTIL_H
#define __UTIL_H

#define NONBLOCK 1
#define BLOCK 0

int accept_connection(int fd, int nonblock);
int make_server(short serv_port, int maxqueue, int nonblock);
int make_client(char *serv_hostname, short serv_port, int nonblock);
void send_wrapper(int fd, char *msg, char *errmsg);
void recv_wrapper(int fd, char *buffer, int size, char *errmsg);

void make_file(char *buffer, char *filename);
void make_buffer(char *buffer, char *filename);

#endif
