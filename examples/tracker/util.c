#include "util.h"

char *buildMsg(char *msg) {
  int len = strlen(msg);
  char *data = malloc(len + 1);
  memcpy(data, &len, 1);
  memcpy(data + 1, msg, len);
  return data;
}

int sendMsg(int tracker_socket, char *msg) {
  char *data = buildMsg(msg);
  if (send(tracker_socket, data, strlen(msg) + 1, MSG_NOSIGNAL) == -1) {
    fprintf(stderr, "[Error %s] ", strerror(errno));
    fprintf(stderr, "Error communicating with socket. Please try again later.\n");
    return -1;
  }
  return 0;
}

int sendData(int socket, char *data, int len) {
  if (send(socket, data, len, MSG_NOSIGNAL) == -1) {
    fprintf(stderr, "[Error %s] ", strerror(errno));
    fprintf(stderr, "Error communicating with socket. Please try again later.\n");
    return -1;
  }
  return 0;
}

char *recvMsg(int tracker_socket) {
  ssize_t recv_len;
  char *buffer = malloc(BUFSIZ);
  if ((recv_len = recv(tracker_socket, buffer, BUFSIZ, 0)) == -1) {
    fprintf(stderr, "[Error %s] ", strerror(errno));
    fprintf(stderr, "Error receiving data socket. Please try again later.\n");
  }

  return buffer;
}