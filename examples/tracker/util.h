#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <openssl/sha.h>
#include <byteswap.h>
#include <poll.h>

#include "util.c"

#define TRACKER_IP "127.0.0.1"
#define TRACKER_PORT 30000
#define PORT 30000
#define MAXCLIENTS 30

char *buildMsg(char *msg);
int sendMsg(int tracker_socket, char *msg);
char *recvMsg(int tracker_socket);

#endif
