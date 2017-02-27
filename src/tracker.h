#ifndef TRACKER_H_
#define TRACKER_H_

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

// Tracker server constants
#define MAXCLIENTS 255
#define TRACKER_PORT 30001
#define IP_SIZE 16

// Linked list of clients definition for tracker server
typedef struct Client {
  char *ip;
  struct Client *next;
} Client;

// Shared functions
char *buildMsg(char *msg);
int sendMsg(int tracker_socket, char *msg);
int sendData(int socket, char *data, int len);
char *recvMsg(int tracker_socket);
// Client only functions
void joinTracker(int tracker_socket);
void requestPeers(int tracker_socket, char *peers, int *numPeers);
// Server only functions
void sendClients(int socket, Client *head, int numClients, char *ignore_ip);
void addIfAbsent(Client *head, char *ip, int *numClients);
void destroyClientList(Client *head);
int configureSocket();

#include "tracker.c"
#endif