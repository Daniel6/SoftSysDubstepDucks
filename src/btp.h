#ifndef BTP_H_ /* Include Guard*/ 
#define BTP_H_

//Standard Libraries. 
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <openssl/sha.h>
#include <byteswap.h>
#include <poll.h>

#include "bencode.c"
#include "bencode.h"
//Defining bool types.
#define TRUE 0
#define FALSE 1

//Defining Status of Connection:

#define NOREQUEST -1


//Define message IDs -> Might want to do an enumeration instead.
#define CHOKE 0
#define UNCHOKE 1
#define INTERESTED 2
#define UNINTERESTED 3
#define HAVE 4
#define BITFIELD 5
#define REQUEST 6
#define PIECE 7
#define CANCEL 8

#define STATEMSGSIZE 5
#define HAVEMSGSIZE 9
#define REQUESTMSGSIZE 17
#define CANCELMSGSIZE 17

#define CONNECTIONSTATUS 7
#define OWNINTERESTED 6
#define OWNCHOKE 5
#define PEERINTERESTED 4
#define PEERCHOKE 3
#define PENDINGREQUEST 2

//Maximums
#define MAX_BACKLOGSIZE 10
#define MAX_PEERS 10
#define MAX_CONNECTIONS 500

//SOME HARDCODED STUFF -> mainly protocol specific variables.
const char* BTPROTOCOL = "BitTorrent protocol";
const unsigned char PROTOCOLNAMELENGTH = 19;
const unsigned int FULLHANDSHAKELENGTH = 68;

//Some hardcoded things to be removed later
#define LISTENER_PORT_NUMBER     30000
#define SERVER_ADDRESS  "127.0.0.1"
#define FILE_TO_SEND    "testfile.txt"


typedef struct connection_info {
	/*Status
	bit 7 = connection status
	bit 6 = ownInterested
	bit 5 = ownChoke
	bit 4 = peerInterested
	bit 3 = peerChoke
	bit 2 = pending request
	*/
	char status_flags;
	int requested_piece;
	int piece_to_send;
	char * peerBitfield;
} Connections;

typedef struct node {
    int val;
    struct node *next;
} Node;


#include "btp.c"

//Inherent assumption that hash and char are length 20 arrays. 
void initialize_connection(Connections* connection_to_initialize, int total_pieces_in_file);
char* construct_handshake(char * hash, char * id);
int server_socket_wrapper(struct sockaddr_in * server_addr, char * server_address, int port_number);
int client_socket_wrapper(struct sockaddr_in * remote_addr, char * server_address, int port_number);
void print_hex_memory(void *mem, int size);
int verify_handshake(char* handshakeToVerify, char* clientFileSHA1);
int count_char_bits(char b);
int count_bitfield_bits(char * bitfield, int bitfieldLen);
char * construct_bitfield_message(char * bitfield, int bitfieldLen);
char * construct_state_message(unsigned char msgID);
char * construct_have_message(int piece_index);
char * construct_request_message(int piece_index, int blockoffset, int blocklength);
char * construct_piece_message(int piece_index, int blockoffset, int piece_len, char *piece);
char * construct_cancel_message(int piece_index, int blockoffset, int blocklength);
int peerContainsUndownloadedPieces(char * peer_buffer, char* own_buffer,int bitfieldLen);
void print_bits ( void* buf, size_t size_in_bytes );

Node *make_node(int val, Node *next);
int get_val(Node *node);
Node *assign_pieces(struct connection_info *connections, int num_connections, int piece_statuses[], int num_pieces);
char *get_piece_from_file(int fd, int piece_num, int piece_len);
void error(char *msg);
int say(int socket, char *s);
int read_in(int socket, char *buf, int len);

int configure_socket();
void Verify_handshake(char* buffer, char * file_sha);
char *  Set_peerBitfield(char * buffer, int bitfieldMsgLength, int total_pieces);
void Send_interested(int client_socket, Connections* connection);
void Send_uninterested(int client_socket, Connections* connection);
void Send_unchoked(int client_socket, Connections* connection);
void Send_request(int client_socket, Connections* connection, int piece_index);
void Send_piece(int client_socket, Connections* connection, int piece_index, int file_d, int piece_len);
void Set_Flag(Connections* connection, int flag, int state);


#endif //BTP_H_