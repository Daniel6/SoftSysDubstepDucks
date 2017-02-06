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
  server_socket = server_socket_wrapper(&server_addr, SERVER_ADDRESS, LISTENER_PORT_NUMBER);


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
  char * sha = "AAAAAAAAAAAAAAAAAAA1";

//Initially setting up a bitfield representing the parts of the file.
  int piece_size = 16;
  char * bitfield = malloc(file_stat.st_size/piece_size/8);
  int total_pieces = file_stat.st_size/piece_size;
  memset(bitfield, 128, total_pieces);
  
  int bitfieldMsgLength = 4 + 1 + total_pieces/8;
  printf("testing\n");
  /* Accepting incoming peers */
  peer_socket = accept(server_socket, (struct sockaddr *)&peer_addr, &sock_len);
  if (peer_socket == -1) {
    fprintf(stderr, "Error on accept --> %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  printf("testing\n");

  len = recv(peer_socket, buffer, BUFSIZ, 0);

  int test = verify_handshake(buffer, sha);
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

  // TESTING SCRIPTS DELETE BELOW LATER
  printf("testing sending things in buffer\n");

  char * bf = construct_bitfield_message(bitfield, total_pieces/8);
  char * chokeMsg = construct_state_message(CHOKE);
  char * unchokeMsg = construct_state_message(UNCHOKE);
  char * interestedMsg = construct_state_message(INTERESTED);
  char * uninterestedMsg = construct_state_message(UNINTERESTED);
  char * haveMsg = construct_have_message(0);
  char * requestMsg = construct_request_message(1,2,3);
  char * cancelMsg = construct_cancel_message(9,8,7);

  char * testmsg = malloc(STATEMSGSIZE*4 + HAVEMSGSIZE + REQUESTMSGSIZE + CANCELMSGSIZE+bitfieldMsgLength);
  memcpy(testmsg, bf, bitfieldMsgLength);
  memcpy(testmsg+bitfieldMsgLength, chokeMsg, STATEMSGSIZE);
  memcpy(testmsg+bitfieldMsgLength+STATEMSGSIZE, unchokeMsg, STATEMSGSIZE);
  memcpy(testmsg+bitfieldMsgLength+STATEMSGSIZE*2, interestedMsg, STATEMSGSIZE);
  memcpy(testmsg+bitfieldMsgLength+STATEMSGSIZE*3, uninterestedMsg, STATEMSGSIZE);
  memcpy(testmsg+bitfieldMsgLength+STATEMSGSIZE*4, haveMsg, HAVEMSGSIZE);
  memcpy(testmsg+bitfieldMsgLength+STATEMSGSIZE*4+HAVEMSGSIZE, requestMsg, REQUESTMSGSIZE);
  memcpy(testmsg+bitfieldMsgLength+STATEMSGSIZE*4+HAVEMSGSIZE+REQUESTMSGSIZE, cancelMsg, CANCELMSGSIZE);

  print_hex_memory(testmsg, STATEMSGSIZE*4 + HAVEMSGSIZE + REQUESTMSGSIZE + CANCELMSGSIZE+bitfieldMsgLength);
  

  if(send(peer_socket, testmsg, STATEMSGSIZE*4 + HAVEMSGSIZE + REQUESTMSGSIZE + CANCELMSGSIZE+bitfieldMsgLength, 0)==-1)
  {
    fprintf(stderr, "Sending BF message failed");
    exit(EXIT_FAILURE);

  }
  //END TESTING
  memset(buffer, 0, 20);
  len = recv(peer_socket, buffer, BUFSIZ, 0);
  print_hex_memory(buffer, 20);
  printf("Sent bf_msaage\n");
  print_hex_memory(bf_message, 5+ total_pieces/8);
  printf("END SERVER\n");

  close(peer_socket);
  close(server_socket);

  return 0;
}