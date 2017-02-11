/*
  Client code
  Daniel Bishop

  Connects to tracker server and gets the ips of all other peers in the network.
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdarg.h>

#define TRACKER_IP "127.0.0.1"
#define TRACKER_PORT 30000

void joinTracker(int tracker_socket);
void requestPeers(int tracker_socket);
char *buildMsg(char *msg);
int sendMsg(int tracker_socket, char *data);
char *recvMsg(int tracker_socket);

int main() {
  fprintf(stdout, "Torrent Client Initializing.\nPress 'j' to join or 'e' to exit.\n");
  char *line = NULL;
  size_t len = 0;
  ssize_t line_length;
  struct sockaddr_in tracker_addr;

  tracker_addr.sin_family = AF_INET;
  tracker_addr.sin_port = htons(TRACKER_PORT);

  inet_pton(AF_INET, TRACKER_IP, &(tracker_addr.sin_addr));
  int tracker_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (tracker_socket < 0) {
    fprintf(stderr, "Error creating socket: error %d\n", tracker_socket);
    exit(1);
  }

  while ((line_length = getline(&line, &len, stdin)) != -1) {
    if (strncmp(line, "e", 1) == 0) {
      fprintf(stdout, "Exiting.\n");
      return 0;
    } else if (strncmp(line, "j", 1) == 0) {
      fprintf(stdout, "Registering as available peer...\n");
      joinTracker(tracker_socket);
    } else if (strncmp(line, "r", 1) == 0) {
      fprintf(stdout, "Requesting peer list...\n");
      requestPeers(tracker_socket);
    }
  }

  return 0;
}

void requestPeers(int tracker_socket) {
  char *data = buildMsg("list");
  char *recv_msg;

  if (sendMsg(tracker_socket, data) == 0) {
    recv_msg = recvMsg(tracker_socket);
    if (strncmp(recv_msg, "joined", 6) == 0) {
      fprintf(stdout, "Successfully joined as available peer.\n");
    } else {
      fprintf(stderr, "[Error %s] ", recv_msg);
      fprintf(stderr, "Error joining as available peer. Please try again later.\n");
    }
    free(recv_msg);
  }

  free(data);
}

void joinTracker(int tracker_socket) {
  char *data = buildMsg("join");
  char *recv_msg;

  if (sendMsg(tracker_socket, data) == 0) {
    recv_msg = recvMsg(tracker_socket);
    if (strncmp(recv_msg, "joined", 6) == 0) {
      fprintf(stdout, "Successfully joined as available peer.\n");
    } else {
      fprintf(stderr, "[Error %s] ", recv_msg);
      fprintf(stderr, "Error joining as available peer. Please try again later.\n");
    }
    free(recv_msg);
  }

  free(data);
}

char *buildMsg(char *msg) {
  char *data = malloc(sizeof(msg) + 1);
  int *data_len = malloc(1);
  *data_len = sizeof(msg);
  memcpy(data, &data_len, 1);
  memcpy(data + 1, &msg, sizeof(msg));

  return data;
}

int sendMsg(int tracker_socket, char *data) {
  if (send(tracker_socket, data, sizeof(data), MSG_NOSIGNAL) == -1) {
    fprintf(stderr, "[Error %s] ", strerror(errno));
    fprintf(stderr, "Error communicating with tracker server. Please try again later.\n");
    return -1;
  }
  return 0;
}

char *recvMsg(int tracker_socket) {
  ssize_t recv_len;
  char *buffer = malloc(BUFSIZ);
  if ((recv_len = recv(tracker_socket, buffer, BUFSIZ, 0)) == -1) {
    fprintf(stderr, "[Error %s] ", strerror(errno));
    fprintf(stderr, "Error receiving data from tracker server. Please try again later.\n");
  }

  return buffer;
}