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

typedef int bool;
#define true 1
#define false 0

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

int read_in(int socket, char *buf, int len) {
  /* treat the socket stream as a regular IO stream, 
     so we can do character IO */
  FILE *fp = fdopen(socket, "r");
  int i = 0, ch;
  
  /* eat any leading whitespace */
  while (isspace(ch = fgetc(fp)) && ch != EOF);
  if (ferror(fp))
    error("fgetc");
  
  while (ch != '\n' && ch != EOF) {
    if (i < len)
      buf[i++] = ch;
    ch = fgetc(fp);
  }
  if (ferror(fp))
    error("fgetc");
  
  /* terminate the string, eating any trailing whitespace */
  while (isspace(buf[--i])) {
    buf[i] = '\0';
  }

  return strlen(buf);
}

int main(int argc, char *argv[]) {
  int connect_d = 0, rc = 0;
  char intro_msg[] = "Torrent Tracker Server\nType 'j' to join\n";
  
  if (catch_signal(SIGINT, handle_shutdown) == -1)
    error("Setting interrupt handler");

  int port = 30000;
  listener_d = open_listener_socket();
  bind_to_port(listener_d, port);
  
  if (listen(listener_d, 10) == -1)
    error("Can't listen");

  printf("Waiting for connection on port %d\n", port);

  char buf[2];

  static struct sockaddr_storage client_address;
  static unsigned int address_size = sizeof(client_address);
  
  int client_socket;
  pthread_t client_thread_id;
  while ((client_socket = accept(listener_d, (struct sockaddr *)&client_address, &address_size)) != -1) {
    struct handler_args args;
    args.socket_filedef = client_socket;
    args.ip = inet_ntoa(((struct sockaddr_in *)&client_address)->sin_addr);
    pthread_create(&client_thread_id, NULL, handler, (void *)&args);
  }
  // if ((s = accept(listener_d, (struct sockaddr *)&client_address, &address_size)) == -1)
  //   error("Can't open client socket");
    

  // char *ip = inet_ntoa(((struct sockaddr_in *)&client_address)->sin_addr);

  // struct client_socket sock;
  // sock.file_descriptor = s;
  // sock.ip = ip;
  // return sock;

  // connect_d = sock.file_descriptor;
  // char *ip = sock.ip;


  return 0;
}

void *handler(void *arguments) {
  struct handler_args *args = arguments;
  int client_socket = args->socket_filedef;
  char *ip = args->ip;
  fprintf(stdout, "Client %s connected\n", ip);
  if (say(client_socket, "Torrent Tracker\n") == -1) {
    close(client_socket);
  }

  char buf[255];
  read_in(client_socket, buf, sizeof(buf));
  fprintf(stdout, "%s\n", buf[0]);
  // if (strlen(buf) == 1) {
  //   if (buf[0] == 'j') {
  //     fprintf(stdout, "Client join: %s\n");
  //   }
  // }

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
