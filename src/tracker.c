/*
  Functions relating to the communication between tracker and client
*/

#include "tracker.h"

/*
  Request that the tracker server send all ips of other clients which have "joined"
  The response comes in many parts.
  The first part details how many ips will be sent, and by extension how many more messages are coming.
  Each extra message contains 1 ip.
*/
void requestPeers(int tracker_socket, char *peers, int *numPeers) {
  fprintf(stdout, "Requesting peer list...\n");
  if (sendMsg(tracker_socket, "list") == 0) {
    char *recv_msg = recvMsg(tracker_socket);
    fprintf(stdout, "Sizeof data: %d\n", recv_msg[0]);
    int numClients = recv_msg[0];
    int i;
    *numPeers = 0;
    for (i = 0; i < numClients; i++) {
      char *peer = malloc(16);
      memcpy(peer, &recv_msg[1 + (16 * i)], 16);
      memcpy(peers + i, peer, 16);
      (*numPeers)++;
    }
    free(recv_msg);
  }

  fprintf(stdout, "Peer List:\n");

  int i;
  for (i = 0; i < *numPeers; i++) {
    fprintf(stdout, " - %s\n", peers[i]);
  }
}

/*
  Request to add our ip to the list of connected ips to be returned when the peer list is requested.
*/
void joinTracker(int tracker_socket) {
  fprintf(stdout, "Registering as available peer...\n");
  if (sendMsg(tracker_socket, "join") == 0) {
    char *recv_msg = recvMsg(tracker_socket);
    // The confirmation of a successful join is the reception of a "joined" message
    fprintf(stdout, "Received message of length: %d\n", recv_msg[0]);
    if (strncmp(recv_msg + 1, "joined", 6) == 0) {
      fprintf(stdout, "Successfully joined as available peer.\n");
    } else {
      unsigned int msg_len = recv_msg[0];
      int i;
      fprintf(stderr, "[Error ");
      for (i = 0; i < msg_len; i++) {
        fprintf(stderr, "%s", recv_msg[i + 1]);
      }
      fprintf(stderr, "] ");
      fprintf(stderr, "Error joining as available peer. Please try again later.\n");
    }
    free(recv_msg);
  }
}

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
