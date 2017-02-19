/*
  Tracker server rev.2
  Daniel Bishop

  Handles client connections and adds them to list of peers. Serves list to clients.
*/

#include "tracker.h"

void sendClients(int socket, char *peersBuffer, int numClients);

int main(int argc, char *argv[]) {
  int listening_socket;
  int numClients;

  fprintf(stdout, "Torrent Tracker Initializing.\n");

  int reuse = 1;
  numClients = 0;

  struct sockaddr_in listening_addr;
  listening_addr.sin_family = PF_INET;
  listening_addr.sin_port = (in_port_t)htons(TRACKER_PORT);
  listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  listening_socket = socket(PF_INET, SOCK_STREAM, 0);

  if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1) {
    fprintf(stderr, "Can't set the 'reuse' option on the socket.\n");
    exit(1);
  }
    
  if (bind(listening_socket, (struct sockaddr *)&listening_addr, sizeof(listening_addr)) == -1) {
    fprintf(stderr, "Can't bind to socket.\n");
    exit(1);
  }

  if (listening_socket < 0) {
    fprintf(stderr, "Error creating socket: error %d\n", listening_socket);
    exit(1);
  }

  if (listen(listening_socket, 10) == -1) {
    fprintf(stderr, "Can't listen.\n");
    exit(1);
  }

  printf("Waiting for connection on port %d\n", TRACKER_PORT);

  static struct sockaddr_storage client_address;
  static unsigned int address_size = sizeof(client_address);
  int client_socket;

  char *peersBuffer = malloc(16 * MAXCLIENTS);

  while ((client_socket = accept(listening_socket, (struct sockaddr *)&client_address, &address_size)) != -1) {
    printf("======\n");
    char *ip = inet_ntoa(((struct sockaddr_in *)&client_address)->sin_addr);

    fprintf(stdout, "Client %s connected.\n", ip);
    char *recv_msg;
    recv_msg = recvMsg(client_socket);

    if (recv_msg[0] == 0) {
      fprintf(stdout, "Detected client %s shutdown.\n", ip);
    } else {
      if (strncmp(recv_msg + 1, "list", 4) == 0) {
        // Execute the "list" command
        memcpy(peersBuffer + (numClients * 16), ip, 16);
        numClients++;
        fprintf(stdout, "Client %s joined, %d clients total.\n", ip, numClients);
        sendClients(client_socket, peersBuffer, numClients);
      }
    }
    free(recv_msg);
  }
  close(client_socket);
  return 0;
}

void sendClients(int socket, char *peersBuffer, int numClients) {
  // Bundle all client ip's into one message
  char *data = malloc(1 + (16 * numClients));
  memcpy(data, &numClients, 1);
  int dl = 16 * numClients;

  memcpy((data + 1), peersBuffer, 16 * numClients);

  sendData(socket, data, dl);

  fprintf(stdout, "Done sending.\n");
}
