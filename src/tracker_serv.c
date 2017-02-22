/*
  Tracker server rev.2
  Daniel Bishop

  Handles client connections and adds them to list of peers. Serves list to clients.
*/

#include "tracker.h"

struct client {
  char *ip;
  struct client *next;
};

void sendClients(int socket, struct client *head, int numClients);
void addIfAbsent(struct client *head, char *ip, int *numClients);
int configureSocket();

int main(int argc, char *argv[]) {
  fprintf(stdout, "Torrent Tracker Initializing.\n");

  int numClients = 0;
  int listening_socket = configureSocket();

  printf("Waiting for connection on port %d\n", TRACKER_PORT);

  static struct sockaddr_storage client_address;
  static unsigned int address_size = sizeof(client_address);
  int client_socket;

  struct client *c_head;

  // Repeatedly wait for incoming connections
  while ((client_socket = accept(listening_socket, (struct sockaddr *)&client_address, &address_size)) != -1) {
    printf("======\n");
    // Capture the client's IP address
    char *ip = inet_ntoa(((struct sockaddr_in *)&client_address)->sin_addr);

    fprintf(stdout, "Client %s connected.\n", ip);

    // Char pointer to store incoming bytes
    char *recv_msg;
    recv_msg = recvMsg(client_socket);

    if (recv_msg[0] == 0) {
      // When the client socket closes, a byte of value 0 is sent
      fprintf(stdout, "Detected client %s shutdown.\n", ip);
    } else {
      if (strncmp(recv_msg + 1, "list", 4) == 0) {
        // Add the client to the linked list of clients
        if (numClients == 0) {
          // Add client as head AND tail of linked list
          c_head = malloc(sizeof(struct client));
          c_head->ip = ip;
          c_head->next = 0; // Initialize next pointer to NULL
          c_tail = c_head;
          numClients = 1;
        } else {
          // Append client to end of list if it is not present
          addIfAbsent(c_head, ip, &numClients);
        }

        fprintf(stdout, "Client %s joined, %d clients total.\n", ip, numClients);
        // Store the ip's of every client in a buffer and send that buffer to the client
        sendClients(client_socket, c_head, numClients);
      }
    }
    free(recv_msg);
  }
  close(client_socket);
  return 0;
}

/*
  Given a linked list of clients, add a new client if the client's IP
  is not represented in the list.
  If a new client is added to the list, update numClients accordingly.
  O(n) time complexity
*/
void addIfAbsent(struct client *head, char *ip, int *numClients) {
  printf("Add if absent\n");
  struct client *curr_node;
  curr_node = head;
  if (curr_node->ip == ip) {
    // Special case for when linked list contains only 1 node
    return;
  }
  while (curr_node->next != 0) {
    if (curr_node->ip == ip) {
      return;
    } else {
      curr_node = curr_node->next;
    }
  }

  // When the while loop ends, curr_node should point to the tail of the list
  struct client *new_node;
  new_node = malloc(sizeof(struct client));
  new_node->ip = ip;
  new_node->next = 0;
  curr_node->next = new_node;
  (*numClients)++;
}

int configureSocket() {
  int reuse = 1;
  int listening_socket = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in listening_addr;
  listening_addr.sin_family = PF_INET;
  listening_addr.sin_port = (in_port_t)htons(TRACKER_PORT);
  listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);

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

  return listening_socket;
}

/*
  Given a linked list of clients, write their ip's to a buffer and send
  it over the socket given.
*/
void sendClients(int socket, struct client *head, int numClients) {
  if (numClients <= 0) {
    // Do not attempt to send no clients
    return;
  }

  // Bundle all client ip's into one message
  char *data = malloc(1 + (IP_SIZE * numClients));
  memcpy(data, &numClients, 1);

  struct client *curr_node;
  curr_node = head;
  int counted_nodes = 1;
  // Ensure that the length of the linked list given is equal to the
  // number of clients given
  while (curr_node-> next != 0) {
    curr_node = curr_node->next;
    counted_nodes++;
  }
  if (counted_nodes != numClients) {
    fprintf(stderr, "Unexpected number of clients (%d expected %d)\n", counted_nodes, numClients);
    return;
  }

  int i;
  curr_node = head;
  // Copy all ip's into data buffer for sending
  for (i = 0; i < numClients; i++) {
    memcpy(data + 1 + (IP_SIZE * i), curr_node->ip, IP_SIZE);
    curr_node = curr_node->next;
  }

  sendData(socket, data, IP_SIZE * numClients);

  fprintf(stdout, "Done sending.\n");
}
