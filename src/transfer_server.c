// Server Code
// http://stackoverflow.com/questions/11952898/c-send-and-receive-file

#include "btp.h"


int main(int argc, char **argv) {
  int server_socket;
  int peer_socket;
  socklen_t sock_len;
  ssize_t len;
  struct sockaddr_in server_addr;
  struct sockaddr_in peer_addr;
  int fd;
  int sent_bytes = 0;
  char file_size[256];
  struct stat file_stat;
  off_t offset;
  int remain_data;
  char buffer[BUFSIZ];
  /* Create server socket + construct server_addr struct*/
  server_socket = server_socket_wrapper(&server_addr, SERVER_ADDRESS, PORT_NUMBER);



  /* Bind */
  if ((bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) == -1) {
    fprintf(stderr, "Error on bind --> %s", strerror(errno));

    exit(EXIT_FAILURE);
  }

  /* Listening to incoming connections */
  if ((listen(server_socket, 5)) == -1) {
    fprintf(stderr, "Error on listen --> %s", strerror(errno));

    exit(EXIT_FAILURE);
  }


  fd = open(FILE_TO_SEND, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Error opening file --> %s", strerror(errno));

    exit(EXIT_FAILURE);
  }

  /* Get file stats */
  if (fstat(fd, &file_stat) < 0) {
    fprintf(stderr, "Error fstat --> %s", strerror(errno));

    exit(EXIT_FAILURE);
  }

//  fprintf(stdout, "File Size: \n%d bytes\n", (uint32_t)file_stat.st_size);

  sock_len = sizeof(struct sockaddr_in);

//Initially setting up a bitfield representing the parts of the file.
  int piece_size = 16;
  char * bitfield = malloc(file_stat.st_size/piece_size/8);
  int total_pieces = file_stat.st_size/piece_size;
  memset(bitfield, 255, total_pieces);
  
  int bitfieldMsgLength = 4 + 1 + total_pieces/8;

  /* Accepting incoming peers */
  peer_socket = accept(server_socket, (struct sockaddr *)&peer_addr, &sock_len);
  if (peer_socket == -1) {
    fprintf(stderr, "Error on accept --> %s", strerror(errno));

    exit(EXIT_FAILURE);
  }



  char * sha = "AAAAAAAAAAAAAAAAAAA1";
  len = recv(peer_socket, buffer, BUFSIZ, 0);
  //print_hex_memory(buffer, 68);
//  printf("\n len is %d\n", len);
  int test = verify_handshake(buffer, sha);
  printf("%d\n", test);
  if(test)
  {
    fprintf(stderr, "Error on handshake ---> %d\n", test);
    exit(EXIT_FAILURE);
  }
  printf("Verified handshake\n");

  //Tries to send own handshake back + attach current bitfield knowledge. 
  // Idea is to not have to worry about recieving bf_message or not in the transfer_client;
  char * own_handshake = construct_handshake(sha, sha);
  char * bf_message = construct_bitfield_message(bitfield, total_pieces/8);

  char * intro_msg = malloc(FULLHANDSHAKELENGTH+bitfieldMsgLength);
  memcpy(intro_msg, own_handshake, FULLHANDSHAKELENGTH);
  memcpy(intro_msg+FULLHANDSHAKELENGTH, bf_message, bitfieldMsgLength);

  if(send(peer_socket, intro_msg, bitfieldMsgLength+FULLHANDSHAKELENGTH, 0)==-1)
  {
    fprintf(stderr, "Sending BF message failed");
    exit(EXIT_FAILURE);

  }
  printf("Sent bf_msaage\n");
  print_hex_memory(bf_message, 5+ total_pieces/8);
  printf("END SERVER\n");

  close(peer_socket);
  close(server_socket);

  return 0;
}