// Client Code
// http://stackoverflow.com/questions/11952898/c-send-and-receive-file

#include "btp.h"

#define FILENAME        "./client_side.txt"

int main(int argc, char **argv) {
  int client_socket;
  ssize_t len;
  struct sockaddr_in remote_addr;
  char buffer[BUFSIZ];
  int file_size;
  FILE *received_file;
  int remain_data = 0;

  /* Zeroing remote_addr struct */
  memset(&remote_addr, 0, sizeof(remote_addr));
  /* Construct remote_addr struct */
  remote_addr.sin_family = AF_INET;
  inet_pton(AF_INET, SERVER_ADDRESS, &(remote_addr.sin_addr));
  remote_addr.sin_port = htons(PORT_NUMBER);
  /* Create client socket */
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1) {
    fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));

    exit(EXIT_FAILURE);
  }
  /* Connect to the server */
  if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1) {
    fprintf(stderr, "Error on connect --> %s\n", strerror(errno));

    exit(EXIT_FAILURE);
  }

  file_size = 256;
  //Initializing Own Bitfield for torrent file
  int piece_size = 16;
  int total_pieces = file_size/piece_size;
  char * ownBitfield = malloc(total_pieces/8);
  memset(ownBitfield, 0, total_pieces);
  //Initialize Peer Bitfield for torrent file;
  char * peerBitfield = malloc(total_pieces/8);
  memset(peerBitfield, 0, total_pieces); 

  //Declare bitfield message length, maps from total pieces to the number of 
  //bits needed to represent it. (Hence the total_pieces/8) -> 8 being # of 
  //bits in a char.


  int bitfieldMsgLength = 4 + 1 + total_pieces/8;
  int bitfieldLen = total_pieces/8;

  printf("Client %d, %d\n", total_pieces, bitfieldMsgLength);

  //Construct Handshake + SHA1 Hash
  char * sha = "AAAAAAAAAAAAAAAAAAA1";
  char * own_handshake = construct_handshake(sha, sha);
  //Tries to send handshake.
  if(send(client_socket, own_handshake, 68,0)==-1){
    fprintf(stderr, "Error on send --> %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  uint32_t recieved_bytes = 0;

  recieved_bytes = recv(client_socket, buffer, BUFSIZ, 0);

  char * peer_handshake = malloc(FULLHANDSHAKELENGTH);
  int test;
  printf("Client recieved_bytes = %d\n", recieved_bytes);
  if(recieved_bytes == -1)
    {
      fprintf(stderr, "Error on reception");
      exit(EXIT_FAILURE);
    }
    else if(recieved_bytes == 0)
    {
      fprintf(stdout, "Dropped Connection\n");
      exit(EXIT_FAILURE);
    }

    else if(recieved_bytes == FULLHANDSHAKELENGTH)
    {
      memcpy(peer_handshake, buffer, 68);
      test = verify_handshake(peer_handshake, sha);
      // Testing for issues on handshake.
      if(test)
      {
      fprintf(stderr, "Error on handshake ---> %d\n", test);
      exit(EXIT_FAILURE);
      }
      memset(buffer, 0, sizeof(buffer));
      free(peer_handshake);
    }
    else if( recieved_bytes == FULLHANDSHAKELENGTH + bitfieldMsgLength)
    {

      char * bitfield_message = malloc(bitfieldMsgLength);
      memcpy(peer_handshake, buffer, FULLHANDSHAKELENGTH);
      test = verify_handshake(peer_handshake, sha);
      // Testing for issues on handshake.
      if(test)
      {
      fprintf(stderr, "Error on handshake ---> %d\n", test);
      exit(EXIT_FAILURE);
      }
      //If there is a bitfield option, set bitfield. 
      memcpy(bitfield_message, buffer+FULLHANDSHAKELENGTH, bitfieldMsgLength);
      memcpy(peerBitfield, bitfield_message+5, total_pieces);
      memset(buffer, 0, sizeof(buffer));


      //Free handshake  + memory after completion. -> Don't think we need it afterwards
      free(bitfield_message);
      free(peer_handshake);
    };

    printf("Client %d\n", count_bitfield_bits(peerBitfield, bitfieldLen));
    char * bufPtr;

 


 
  received_file = fopen(FILENAME, "w");
  if (received_file == NULL) {
    fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));

    exit(EXIT_FAILURE);
  }

  remain_data = file_size;

  while (((len = recv(client_socket, buffer, BUFSIZ, 0)) > 0) && (remain_data > 0)) {
    fwrite(buffer, sizeof(char), len, received_file);
    remain_data -= len;
    fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", (int) len, remain_data);
  }
//      print_hex_memory(buffer, 100);
  fclose(received_file);

  close(client_socket);

  return 0;
}
