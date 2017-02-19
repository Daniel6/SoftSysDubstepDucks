/*
  Tracker server rev.2
  Daniel Bishop

  Handles client connections and adds them to list of peers. Serves list to clients.
*/

#include "tracker.h"

struct handler_args {
  int socket_filedef;
  char *ip;
};

int listening_socket;
char *clients[MAXCLIENTS];
int numClients;

void* handler(void *args);
int addClient(char *ip);
void sendClients(int socket);

int main(int argc, char *argv[]) {
  fprintf(stdout, "Torrent Tracker Initializing.\n");

  int reuse = 1;
  numClients = 0;

  struct sockaddr_in listening_addr;
  listening_addr.sin_family = AF_INET;
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
  pthread_t client_thread_id;
  while ((client_socket = accept(listening_socket, (struct sockaddr *)&client_address, &address_size)) != -1) {
    // Spawn new thread to handle each incoming connection
    struct handler_args args;
    args.socket_filedef = client_socket;
    args.ip = inet_ntoa(((struct sockaddr_in *)&client_address)->sin_addr);
    pthread_create(&client_thread_id, NULL, handler, (void *)&args);
  }

  return 0;
}

/*
  Handles communication with one client. Closes socket after processing command.
  TODO: Poll the client to detect remote disconnections
*/
void *handler(void *arguments) {
  struct handler_args *args = arguments;
  int client_socket = args->socket_filedef;
  char *ip = args->ip;
  fprintf(stdout, "Client %s connected.\n", ip);

  // TODO: iterate upon this to support the execution of multiple commands before closing the connection.
  char *recv_msg;
  while (1) {
    recv_msg = recvMsg(client_socket);
    // If we read a 0 from the socket, the socket has been closed on the client side.
    if (recv_msg[0] == 0) {
      fprintf(stdout, "Detected client %s shutdown.\n", ip);
      break;
    }

    if (strncmp(recv_msg + 1, "join", 4) == 0) {
      // Execute the "join" command
      fprintf(stdout, "Client %s is joining the peer list.\n", ip);
      if (addClient(ip) == 0) {
        sendMsg(client_socket, "joined");
      } else {
        sendMsg(client_socket, "failed to join");
      }
    } else if (strncmp(recv_msg + 1, "list", 4) == 0) {
      // Execute the "list" command
      addClient(ip);
      sendClients(client_socket);
    }
  }
  free(recv_msg);
  close(client_socket);
}

/*
  Add the connected client's ip address to the list of "joined" ips.
  This list is available upon request to any client.
*/
int addClient(char *ip) {
  if (numClients < MAXCLIENTS) {
    clients[numClients] = ip;
    numClients++;
    fprintf(stdout, "Client joined, %d clients total.\n", numClients);
    return 0;
  } else {
    fprintf(stderr, "Client tried to join but maximum number of clients is reached (%d out of %d).\n", numClients, MAXCLIENTS);
    return -1;
  }
}

void sendClients(int socket) {
  // Bundle all client ip's into one message
  char *data = malloc(1 + (16 * numClients));
  memcpy(data, &numClients, 1);
  int dl = 16 * numClients;

  int i;
  for (i = 0; i < numClients; i++) {
    fprintf(stdout, "Copying: %s\n", clients[i]);
    memcpy((data + 1) + (16 * i), clients[i], 16);
  }

  sendData(socket, data, dl);

  fprintf(stdout, "Done sending.\n");
}
