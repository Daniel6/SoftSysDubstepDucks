/*
  Tracker server rev.2
  Daniel Bishop

  Handles client connections and adds them to list of peers. Serves list to clients.
*/

#include "tracker.h"

// Global flag to enable or disable debugging print statements
int debugging = 0;

int main(int argc, char *argv[]) {

  // Determine if debugging command line argument is present
  if (argc > 1) {
    // 1 command line arg is always provided: the program name
    int i;
    for (i = 1; i < argc; i++) {
      if (strncmp(argv[i], "debug", 5) == 0) {
        printf("Debugging print statements enabled.\n");
        debugging = 1; // Enable debugging mode
      }
    }
  }

  if (debugging){
    printf("Torrent Tracker Initializing.\n");
  }

  int numClients = 0;
  int listening_socket = configureSocket();

  if (debugging) {
    printf("Waiting for connection on port %d\n", TRACKER_PORT);
  }

  static struct sockaddr_storage client_address;
  static unsigned int address_size = sizeof(client_address);
  int client_socket;

  Client *c_head;
  Client *c_tail;

  // Repeatedly wait for incoming connections
  while ((client_socket = accept(listening_socket, (struct sockaddr *)&client_address, &address_size)) != -1) {
    // Capture the client's IP address
    char *ip = inet_ntoa(((struct sockaddr_in *)&client_address)->sin_addr);

    if (debugging) {
      printf("======\n");
      printf("Client %s connected.\n", ip);
    }

    // Char pointer to store incoming bytes
    char *recv_msg;
    recv_msg = recvMsg(client_socket);

    if (recv_msg[0] == 0) {
      // When the client socket closes, a byte of value 0 is sent
      if (debugging) {
        printf("Detected client %s shutdown.\n", ip);
      }
    } else {
      if (strncmp(recv_msg + 1, "list", 4) == 0) {
        // Add the client to the linked list of clients
        if (numClients == 0) {
          // Add client as head AND tail of linked list
          c_head = malloc(sizeof(Client));
          c_head->ip = ip;
          c_head->next = 0; // Initialize next pointer to NULL
          c_tail = c_head;
          numClients = 1;
        } else {
          // Append client to end of list if it is not present
          addIfAbsent(c_head, ip, &numClients);
        }
        if (debugging) {
          printf("Client %s joined, %d clients total.\n", ip, numClients);
        }
        // Store the ip's of every client in a buffer and send that buffer to the client
        sendClients(client_socket, c_head, numClients);
      }
    }
    free(recv_msg);
  }
  close(client_socket);
  destroyClientList(c_head);
  return 0;
}
