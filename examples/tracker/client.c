/*
  Client code
  Daniel Bishop

  Connects to tracker server and gets the ips of all other peers in the network.
*/

#include "util.h"

void joinTracker(int tracker_socket);
void requestPeers(int tracker_socket);

char *peers[255];
int numPeers;

int main() {
  fprintf(stdout, "Torrent Client Initializing.\nControls:\n    'j' to join\n    'l' to get peers\n    'e' to exit\n");
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

  if (connect(tracker_socket, (struct sockaddr *)&tracker_addr, sizeof(struct sockaddr)) == -1) {
    fprintf(stderr, "[Error %s] ", strerror(errno));
    fprintf(stderr, "Error connecting to tracker.\n");
    exit(1);
  }

  // Listen for commands
  while ((line_length = getline(&line, &len, stdin)) != -1) {
    if (strncmp(line, "e", 1) == 0) {
      fprintf(stdout, "Exiting.\n");
      return 0;
    } else if (strncmp(line, "j", 1) == 0) {
      fprintf(stdout, "Registering as available peer...\n");
      joinTracker(tracker_socket);
    } else if (strncmp(line, "l", 1) == 0) {
      fprintf(stdout, "Requesting peer list...\n");
      requestPeers(tracker_socket);
    }
  }

  if (tracker_socket) {
    close(tracker_socket);
  }
  return 0;
}

/*
  Request that the tracker server send all ips of other clients which have "joined"
  The response comes in many parts.
  The first part details how many ips will be sent, and by extension how many more messages are coming.
  Each extra message contains 1 ip.
*/
void requestPeers(int tracker_socket) {
  if (sendMsg(tracker_socket, "list") == 0) {
    char *recv_msg = recvMsg(tracker_socket); 
    int numClients = recv_msg[1];
    fprintf(stdout, "Server tells us there are %d peers.\n", numClients);

    int i;
    numPeers = 0;
    for (i = 0; i < numClients; i++) {
      recv_msg = recvMsg(tracker_socket);
      sendMsg(tracker_socket, "ready");
      if (recv_msg[0] == 0) {
        fprintf(stdout, "Tracker socket closed unexpectedly, exiting.\n");
        return;
      } else if (recv_msg[0] == 16) {
        char *ip = recv_msg + 1;
        fprintf(stdout, "Received peer: %s\n", ip);
        peers[numPeers] = ip; // Store received ips in array
        numPeers++;
      }
    }
    free(recv_msg);
  }

  fprintf(stdout, "Peer List:\n");

  int i;
  for (i = 0; i < numPeers; i++) {
    fprintf(stdout, " - %s\n", peers[i]);
  }
}

/*
  Request to add our ip to the list of connected ips to be returned when the peer list is requested.
*/
void joinTracker(int tracker_socket) {
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