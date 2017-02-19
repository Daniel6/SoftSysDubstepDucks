/* Code from Head First C.

 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

// function prototypes
void send_clients(int connect_d);
void add_client(char *ip);
void* handler(void *arg);

#define PORT 30000;

int listener_d = 0;

int num_clients = 0;
struct client {
  char *ip;
};
struct client clients[10];

struct handler_args {
  int socket_filedef;
  char *ip;
};

void error(char *msg) {
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(1);
}

int catch_signal(int sig, void (*handler)(int)) {
  struct sigaction action;
  action.sa_handler = handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  return sigaction(sig, &action, NULL);
}

void handle_shutdown(int sig) {
  if (listener_d)
    close(listener_d);

  fprintf(stderr, "Bye!\n");
  exit(EXIT_SUCCESS);
}

int open_listener_socket(void) {
  int s = socket(PF_INET, SOCK_STREAM, 0);
  if (s == -1)
    error("Can't open listener socket");
  return s;
}

struct client_socket {
  int file_descriptor; // file descriptor for socket connection to client
  char *ip;
};

void bind_to_port(int socket, int port) {
  struct sockaddr_in name;
  name.sin_family = PF_INET;
  name.sin_port = (in_port_t)htons(port);
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  int reuse = 1;
    
  if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1)
    error("Can't set the 'reuse' option on the socket");
    
  if (bind(socket, (struct sockaddr *)&name, sizeof(name)) == -1)
    error("Can't bind to socket");
}

int say(int socket, char *s) {
  int result = send(socket, s, strlen(s), 0);
  if (result == -1)
    fprintf(stderr, "%s: %s\n", "Error talking to the client", strerror(errno));
  return result;
}

int main(int argc, char *argv[]) {
  int connect_d = 0, rc = 0;
  char intro_msg[] = "Torrent Tracker Server\nType 'j' to join\n";
  
  if (catch_signal(SIGINT, handle_shutdown) == -1)
    error("Setting interrupt handler");

  listener_d = open_listener_socket();
  bind_to_port(listener_d, PORT);
  
  if (listen(listener_d, 10) == -1)
    error("Can't listen");

  printf("Waiting for connection on port %d\n", PORT);

  static struct sockaddr_storage client_address;
  static unsigned int address_size = sizeof(client_address);
  
  int client_socket;
  pthread_t client_thread_id;
  while ((client_socket = accept(listener_d, (struct sockaddr *)&client_address, &address_size)) != -1) {
    // Spawn new thread to handle each incoming connection
    struct handler_args args;
    args.socket_filedef = client_socket;
    args.ip = inet_ntoa(((struct sockaddr_in *)&client_address)->sin_addr);
    pthread_create(&client_thread_id, NULL, handler, (void *)&args);
  }

  return 0;
}

/*
  Read input over network socket and return a pointer to a char array containing the
  message. Input is expected to have a leading byte representing the length of the message
  and to be no longer than 9 bytes total (including leading byte).
*/
char *read_in(int socket) {
  char *temp_buf = malloc(9);
  int data_length = recv(socket, temp_buf, size, 0);
  int msg_length = buf[0];
  char *buf = malloc(msg_length);
  if (data_length - msg_length > 0 && msg_length <= bufsize) {
    memcpy(buf, temp_buf + 1, 1);
  }
  free(temp_buf);

  return buf;

  // FILE *fp = fdopen(socket, "r");
  // int i = 0;
  // int ch;
  // // Eat leading whitespaces
  // while (isspace(ch = fgetc(fp)) && ch != EOF);
  // if (ferror(fp))
  //   error("fgetc");

  // while (ch != '\n' && ch != EOF) {
  //   if (i < size) {
  //     buf[i] = (char) ch;
  //     i++;
  //   }
  //   ch = fgetc(fp);
  // }
  // if (ferror(fp))
  //   error("fgetc");

  // /* terminate the string, eating any trailing whitespace */
  // while (isspace(buf[--i])) {
  //   buf[i] = '\0';
  // }
}

/*
  Check if client is connected to socket.
  Returns 1 if client is connected, 0 if not.
*/
int is_connected(int socket) {
  char *buf = malloc(2);
  int readval = read(socket, buf, sizeof(buf) - 1);
  free(buf);
  if (readval == 0) {
    return 0;
  }
  return 1;
}

/* 
  Handle a client connection. Read data sent by client and close the socket
  if the client becomes unresponsive.
*/
void *handler(void *arguments) {
  struct handler_args *args = arguments;
  int client_socket = args->socket_filedef;
  char *ip = args->ip;
  fprintf(stdout, "Client %s connected\n", ip);
  if (say(client_socket, "Torrent Tracker\n") == -1) {
    close(client_socket);
  } else {
    if (is_connected(client_socket)) {
      char *buf = read_in(client_socket);

      int i;
      for (i = 0; i < sizeof(*buf); i++) {
        fprintf(stdout, "%c\n", buf[i]);
      }

      fprintf(stdout, "Data length: %d\n", sizeof(*buf));
      if (sizeof(*buf) == 1) {
        if ((char)buf[0] == 'j') {
          fprintf(stdout, "First char: %c\n", buf[0]);
        }
      }
    } else {
      fprintf(stdout, "Client %s disconnected\n", ip);
      close(client_socket);
    }
  }
}

void send_clients(int connect_d) {
  int i;
  fprintf(stdout, "num clients %d\n", num_clients);
  for (i = 0; i < num_clients; i++) {
    fprintf(stdout, "client %d\n", i);
    char buf[256];
    sprintf(buf, "%s\n", clients[i].ip);
    fprintf(stdout, "Sent: %s\n", buf);
    if (say(connect_d, buf) == -1) {
      fprintf(stderr, "Error transmitting connected clients.\n");
    }
  }
}

void add_client(char *ip) {
  if (!client_exists(ip)) {
    struct client new_client;
    new_client.ip = ip;
    clients[num_clients] = new_client;
    num_clients++;
  }
}

bool client_exists(char *ip) {
  int i;
  for (i = 0; i < num_clients; i++) {
    if (clients[i].ip == ip) {
      return true;
    }
  }
  return false;
}
