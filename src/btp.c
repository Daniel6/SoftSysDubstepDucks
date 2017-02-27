#include "btp.h"

int configureSocket() {
  int reuse = 1;
  int listening_socket = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in listening_addr;
  listening_addr.sin_family = PF_INET;
  listening_addr.sin_port = (in_port_t)htons(30000);
  listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1) {
    fprintf(stderr, "Can't set the 'reuse' option on the socket.\n");
    exit(1);
  }
    
  if (bind(listening_socket, (struct sockaddr *)&listening_addr, sizeof(listening_addr)) == -1) {
    fprintf(stderr, "Can't bind to socket.\n");
    exit(1);
  }

//Sets up a server socket for listening. 
int server_socket_wrapper(struct sockaddr_in *server_addr_p, char * server_address, int port_number)
    {
    //Create + test socket. 
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
    fprintf(stderr, "Error creating socket --> %s", strerror(errno));
    exit(EXIT_FAILURE);
     }
    int sock_len = sizeof(struct sockaddr_in);
    /* Zeroing server_addr struct */
    memset(server_addr_p, 0, sizeof(*server_addr_p));
    server_addr_p->sin_family = AF_INET;
    inet_pton(AF_INET, server_address, &(server_addr_p->sin_addr));
    server_addr_p->sin_port = htons(port_number);

    /*This setting options to allow for reuse of port from other clients. 
    Basically making the port not sticky lol*/
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1){
      fprintf(stderr, "Error on setting option --> %s", strerror(errno));
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


//Sets up a socket to ping other clients with. 

int client_socket_wrapper(struct sockaddr_in * remote_addr, char * server_address, int port_number){
  /* Zeroing remote_addr struct */
    memset(remote_addr, 0, sizeof(struct sockaddr_in));
  /* Construct remote_addr struct */
    remote_addr->sin_family = AF_INET;
    inet_pton(AF_INET, server_address, &(remote_addr->sin_addr));
    remote_addr->sin_port = htons(port_number);
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  /* Create client socket */
    if (client_socket == -1) {
        fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    return client_socket;
}


// Useful piece of support code when debugging some of this stuff
// Context was using it as a way to dump hex of char arrays easily.
void print_hex_memory(void *mem, int size) {
  int i;
  //Iterates over each byte in mem. 
  unsigned char *p = (unsigned char *)mem;
  for (i=0;i<size;i++) {
    printf("0x%02x ", p[i]);
  }
  printf("\n");
}

// Support code to print the binary representation of some memory.
// source: http://stackoverflow.com/questions/35364772/how-to-print-memory-bits-in-c
void print_bits ( void* buf, size_t size_in_bytes )
{
    char* ptr = (char*)buf;
    //Iterate over each bit in memory, first by byte, then by bit. 
    for (size_t i = 0; i < size_in_bytes; i++) {
        for (short j = 7; j >= 0; j--) {
            printf("%d", (ptr[i] >> j) & 1);
        }
        printf(" ");
    }
    printf("\n");
}


//Constructs handshake used for verification with other computers. 
char * construct_handshake(char* hash, char* id)
{
    //Generates a Handshake assumiing that hash and id point to 20 byte arrays 
    char * handshake = malloc(FULLHANDSHAKELENGTH);
    //First part of handshake is name length
    memcpy(handshake, &PROTOCOLNAMELENGTH, 1);
    //Next is the description/text for the protocol.
    memcpy(handshake+1, BTPROTOCOL, PROTOCOLNAMELENGTH);
    //Next 8 bytes is unimplemented, but would refer to special features. 
    memset(handshake+1+PROTOCOLNAMELENGTH, 0, 8);   
    //Copy the 20 byte sha 1 hash of the file 
    memcpy(handshake+1+PROTOCOLNAMELENGTH+8, hash, 20);
    //Copy user id. Still need to modify this. 
    memcpy(handshake+1+PROTOCOLNAMELENGTH+20+8, id, 20);

    return handshake;
}

//Actual verifies if the handshake is correct. 
int verify_handshake(char* handshakeToVerify, char* clientFileSHA1)
{
/* 
    Verifying handshakes is done by the following checks:
        1. Name length
        2. Protocol Name
        3. Info hash
*/ 
    unsigned char peerProtocolNameLength = (unsigned char) handshakeToVerify[0];
    //Verify name length
    if(peerProtocolNameLength - PROTOCOLNAMELENGTH)
        return -1;

    //Verify Protocol Name is the same.
    char * peerProtocolName = malloc(PROTOCOLNAMELENGTH+1);
    memcpy(peerProtocolName, handshakeToVerify+1, PROTOCOLNAMELENGTH);
    if (memcmp(peerProtocolName, BTPROTOCOL,20))
    {
        free(peerProtocolName);
        return -2;
    }

    char * peerSHA1 = malloc(SHA_DIGEST_LENGTH);
    memcpy(peerSHA1, handshakeToVerify+1+PROTOCOLNAMELENGTH+8, 20); 

    /*Verify if the SHA1 hash for info key is the same. 
    Note that in this case, we will probably need to revise this for when we 
    have multiple files downloading/contained. There needs to be some way of
    verifying/checking from the list of torrents has at least one key whos
    SHA1 matches. -> Search a dictionary/hashmap by SHA1?
    */
    if(memcmp(peerSHA1, clientFileSHA1, 20))
    {
        free(peerSHA1);
        free(peerProtocolName);
        return -3;
    }
     
    return 0;
}

//Create a bitfield message, which contains a bitfield of length bitfieldLen
//That can deal w/ the length of the object. 
char *construct_bitfield_message(char * bitfield, int bitfieldLen)
{
    //allocate 4 bytes for total message size, 1 byte for status, and bitfield length
    char * msg = malloc(4 + 1 + bitfieldLen);
    int *msgLen = malloc(sizeof(int));
    *msgLen = bitfieldLen+1;
    memcpy(msg, msgLen, 4);
    //Set status byte (4) of msg. 
    msg[4] = (char) BITFIELD;
    memcpy(msg+5, bitfield, bitfieldLen);
    free(msgLen);
    return msg;
}
//Create state msg. 
char * construct_state_message(unsigned char msgID)
{
    char * msg = malloc(5);
    int * msgLen = malloc(4);
    *msgLen = 1;
    memcpy(msg, msgLen, 4);
    memcpy(msg+4, &msgID, 1);
    free(msgLen);
    return msg;

}

//Count bits in your byte
int count_char_bits(char b)
{
    //BK's method for counting bits in a char. 
    // Works by counting the number of bits needed to have the number equal 0. 
    // Sourced from http://www.keil.com/support/docs/194.htm
    int count;
    for (count = 0; b != 0; count++)
    {
       b &= b - 1; // this clears the LSB-most set bit
    }

    return count;
}

//Count bits in your bitfield. 
int count_bitfield_bits(char * bitfield, int bitfieldLen)
{
    int count = 0;
    int x = 0;
    for(x =0; x<bitfieldLen; x++)
    {
        count += count_char_bits(*(bitfield+x));
    }
    return count;


}

//Construct a have msg. 
char * construct_have_message(int piece_index)
{
    char * msg = malloc(9);
    int * msgLen = malloc(4);
    *msgLen = 5;
    int * piece_id = malloc(4);
    char * msgID = malloc(1);
    *msgID = HAVE;
    *piece_id = piece_index;
    memcpy(msg, msgLen, 4);
    memcpy(msg+4, msgID, 1);
    memcpy(msg+5, piece_id, 4);
    free(msgLen);
    free(msgID);
    free(piece_id);
    return msg;
}

char * construct_request_message(int piece_index, int blockoffset, int blocklength)
{
    //MsgLen = 4 bytes for msg_len 1 for msgID, 12 for piece, block_off, block_len
    char * msg = malloc(17);
    int * msgLen = malloc(4);
    *msgLen = 13;
    int * piece_id = malloc(4);
    int * block_off = malloc(4);
    int * block_len = malloc(4);
    char * msgID = malloc(1);

    *msgID = REQUEST;
    *piece_id = piece_index;
    *block_off = blockoffset;
    *block_len = blocklength;
    memcpy(msg, msgLen, 4);
    memcpy(msg+4, msgID, 1);
    memcpy(msg+5, piece_id, 4);
    memcpy(msg+9, block_off, 4);
    memcpy(msg+13, block_len, 4);


    free(msgLen);
    free(msgID);
    free(piece_id);
    free(block_off);
    free(block_len);
    return msg;
}

char * construct_piece_message(int piece_index, int blockoffset, int piece_len, char *piece)
{
    //MsgLen = 4 bytes for msg_len 1 for msgID, 8 for piece, block_off
    char * msg = malloc(13 + piece_len);
    int * msgLen = malloc(4);
    *msgLen = 9 + piece_len;
    int * piece_id = malloc(4);
    int * block_off = malloc(4);
    char * msgID = malloc(1);

    *msgID = PIECE;
    *piece_id = piece_index;
    *block_off = blockoffset;
    memcpy(msg, msgLen, 4);
    memcpy(msg+4, msgID, 1);
    memcpy(msg+5, piece_id, 4);
    memcpy(msg+9, block_off, 4);
    memcpy(msg+13, piece, piece_len);


    free(msgLen);
    free(msgID);
    free(piece_id);
    free(block_off);
    return msg;
}

char * construct_cancel_message(int piece_index, int blockoffset, int blocklength)
{
    //MsgLen = 4 bytes for msg_len 1 for msgID, 12 for piece, block_off, block_len
    char * msg = malloc(17);
    int * msgLen = malloc(4);
    *msgLen = 13;
    int * piece_id = malloc(4);
    int * block_off = malloc(4);
    int * block_len = malloc(4);
    char * msgID = malloc(1);

    *msgID = CANCEL;
    *piece_id = piece_index;
    *block_off = blockoffset;
    *block_len = blocklength;
    memcpy(msg, msgLen, 4);
    memcpy(msg+4, msgID, 1);
    memcpy(msg+5, piece_id, 4);
    memcpy(msg+9, block_off, 4);
    memcpy(msg+13, block_len, 4);


    free(msgLen);
    free(msgID);
    free(piece_id);
    free(block_off);
    free(block_len);
    return msg;
}

//Parses the buffer for the next msg , assuming there is no corruption in the buffer. 
//This has no error handlign right now. 
char* get_next_msg(char * bufPtr,char * msgID, int* msgSize)
{

  memcpy(msgSize, bufPtr, 4);

  char * message = malloc(*msgSize+4);
  memcpy(message, bufPtr, *msgSize+4);
  *msgSize += 4;
  *msgID = *(bufPtr+4);
  bufPtr = bufPtr +*msgSize;
  
  return message;
}


int peerContainsUndownloadedPieces(char * peer_buffer, char* own_buffer,int bit_pieces)
{
    int x;
    int count = 0;
    for(x = 0; x< bit_pieces; x++)
        {
        char * tester = malloc(1);
        char common_bits = *(peer_buffer+x)&*(own_buffer+x);
        
        if((*(peer_buffer+x) - common_bits)!=0)
            return 1;

        printf("count bits%d\n", count_char_bits(*tester));
        free(tester);   
    }
    return 0;
}


void initialize_connection(Connections* connection_to_initialize, int total_pieces_in_file)
{
    //Connection status
    char initial = 0<<7| //Connection status = 0
                   0<<6| //ownInterested = 0
                   1<<5| //ownChoke = 1
                   0<<4| //peerInterested = 0
                   1<<3| //PeerChoked = 1
                   0<<2;//Pending Request

    connection_to_initialize->status_flags |=initial;
    connection_to_initialize->requested_piece = -1;
    connection_to_initialize->piece_to_send = -1;
    connection_to_initialize->peerBitfield = malloc(total_pieces_in_file/8);
    memset(connection_to_initialize->peerBitfield, 0,total_pieces_in_file/8);
}

void Verify_handshake(char* buffer, char * file_sha)
{
    int test;    
    char * peer_handshake = malloc(FULLHANDSHAKELENGTH);
    memcpy(peer_handshake, buffer, FULLHANDSHAKELENGTH);
    test = verify_handshake(peer_handshake, file_sha);
    if(test){
        fprintf(stderr, "Error on handshake ---> %d\n", test);
        exit(EXIT_FAILURE);
    }
    memset(buffer, 0, sizeof(buffer));
    free(peer_handshake);    
}


char *  Set_peerBitfield(char * buffer, int bitfieldMsgLength, int total_pieces)
{
    char * bitfield_message = malloc(bitfieldMsgLength);
    memcpy(bitfield_message, buffer+FULLHANDSHAKELENGTH, bitfieldMsgLength);
    return bitfield_message;
}

<<<<<<< HEAD
/*
 * Get a piece from a file
 *
 * fd: file descriptor
 * piece_num: which piece to get from the file (0 indexed)
 * piece_len: length of pieces
 *
 * returns: char array of the piece data
 */
char *get_piece_from_file(int fd, int piece_num, int piece_len) {

    char buffer[piece_len];
    // seek to correct position in the file
    if (lseek(fd, piece_num * piece_len, SEEK_SET) < 0){
        // error seeking
        exit(-1);
    }

    // read from file into buffer
    if (read(fd, buffer, piece_len) != piece_len) {
        exit(-1);
    }

    printf("%s\n", buffer);
    //char *piece = (char *) mallloc(sizeof(char) * piece_len);
    return strcpy(malloc(sizeof(char) * piece_len), buffer);
    //return piece;
}

/*
 * Write to a file from a buffer
 *
 * fd: file descriptor
 * piece_num: what number piece will be written
 * piece_len: length of the piece being written
 * buffer: char array of what will be written
 */
void write_piece(int fd, int piece_num, int piece_len, char *buffer) {
    // seek to correct position in the file
    if (lseek(fd, piece_num * piece_len, SEEK_SET) < 0){
        // error seeking
        exit(-1);
        return;
    }

    if (write(fd, strcat(buffer, "\n"), piece_len) != piece_len) {
        printf("Success writing to the file!!!!!\n");
        return;
    }
}

/* Makes a new Linked List node structure.
 *  
 * val: value to store in the node.
 * next: pointer to the next node
 * 
 * returns: pointer to a new node
 */
Node *make_node(int val, Node *next) {
    Node *node = malloc(sizeof(Node));
    node->val = val;
    node->next = next;
    return node;
}

/*
 * Returns the value of a Node.
 * 
 * node: node to get value from
 *
 * returns: int that is the value of node
 */
int get_val(Node *node) {
    return node->val;
}

/*
 * Given a peer's bitfield and an array of client's (your) current piece statuses,
 * return the first piece the peer has that you have not requested.
 *
 * peer_bitfield: array containing bits representing if the peer has a piece
 * pieces: client's current request/have statuses of pieces
 *
 * returns: piece number that should be requested from peer
 */
int get_piece(char *peer_bitfield, int piece_statuses[], int num_pieces) {
    for (int i = 0; i < num_pieces; i++) {
        if (!piece_statuses[i] && peer_bitfield[i]) {
            // if client has not yet requested the piece
            // and the peer has the piece, request that
            // piece
            return i;
        }
    }

    return -1;
}


/*
 * Determine what pieces should be requested from which peers
 *
 * connections: a list of peer connections
 * num_connections: total number of connections
 *
 * returns: head of a Linked List, where value is piece number
 *          and the order of the nodes is the order of the peers
 *          in connections
 */
Node *assign_pieces(struct connection_info *connections, int num_connections, int piece_statuses[], int num_pieces) {
    Node *head = make_node(-1, NULL);
    Node *curr = head;

    // assign a piece to every peer connections
    for (int i = 0; i < num_connections; i++) {
        // set value of ith node to piece number that 
        // should be requested from ith peer connection
        curr->val = get_piece(connections[i].peerBitfield, piece_statuses, num_pieces);

        if (i != num_connections - 1) {
            // don't make a new next node if
            // this is the last connection
            curr->next = make_node(-1, NULL);
            *curr = *(curr->next);
        }
    }

    return head;
}

void error(char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

int say(int socket, char *s) {
    int result = send(socket, s, strlen(s), 0);
    if (result == -1) {
        // don't call error()
        // don't want to stop server if there's a problem with just one client
        fprintf(stderr, "%s: %s\n", "Error talking to the client", strerror(errno));
    }
    return result;
}

int read_in(int socket, char *buf, int len) {
    char *s = buf;
    int slen = len;
    int c = recv(socket, s, slen, 0);
    // keep reading until no more chars or reach '\n'
    while ((c > 0) && (s[c-1] != '\n')) {
        s += c;
        slen -= c;
        c = recv(socket, s, slen, 0);
    }
    if (c < 0) {
        // in case there's an error
        return c;
    } else if (c == 0) {
        // nothing read, so send an empty string
        buf[0] = '\0';
    } else {
        // replace '\r' with '\0'
        s[c-1]='\0';
    }
    return len - slen;
}






void Verify_handshake(char* buffer, char * file_sha)
{
    int test;    
    char * peer_handshake = malloc(FULLHANDSHAKELENGTH);
    memcpy(peer_handshake, buffer, FULLHANDSHAKELENGTH);
    test = verify_handshake(peer_handshake, file_sha);
    if(test){
        fprintf(stderr, "Error on handshake ---> %d\n", test);
        exit(EXIT_FAILURE);
    }
    memset(buffer, 0, sizeof(buffer));
    free(peer_handshake);    
}


char *  Set_peerBitfield(char * buffer, int bitfieldMsgLength, int total_pieces)
{
    char * bitfield_message = malloc(bitfieldMsgLength);
    memcpy(bitfield_message, buffer+FULLHANDSHAKELENGTH, bitfieldMsgLength);
    return bitfield_message;
}

void Send_interested(int client_socket, Connections* connection)
{
    char * interested = construct_state_message(INTERESTED);
    connection->status_flags |= 1<<OWNINTERESTED;
    //Send interested message.
    if(send(client_socket, interested, STATEMSGSIZE,0)==-1){
        fprintf(stderr, "Error on send --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    free(interested);
}

void Send_uninterested(int client_socket, Connections* connection)
{
    connection->status_flags |= 0<<OWNINTERESTED;
    char * uninterested = construct_state_message(UNINTERESTED);
    if(send(client_socket, uninterested, STATEMSGSIZE,0)==-1){
        fprintf(stderr, "Error on send --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    free(uninterested); 
}

void Send_unchoked(int client_socket, Connections* connection)
{
    connection->status_flags |= 0<<PEERCHOKE;
    char * unchoke = construct_state_message(UNCHOKE);
    if(send(client_socket, unchoke, STATEMSGSIZE,0)==-1){
        fprintf(stderr, "Error on send --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    free(unchoke); 
}

void Send_request(int client_socket, Connections* connection, int piece_index) {
   char *request = construct_request_message(piece_index, 0, 0);
   if(send(client_socket, request, REQUESTMSGSIZE, 0) == -1){
        fprintf(stderr, "Error on send --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    connection->status_flags |= 1<<PENDINGREQUEST;
    connection->requested_piece = piece_index;
    free(request); 
}

void Send_piece(int client_socket, Connections* connection, int piece_index, int file_d, int piece_len) {
   char *piece = get_piece_from_file(file_d, piece_index, piece_len);
   char *piece_msg = construct_piece_message(piece_index, 0, piece_len, piece);
   if(send(client_socket, piece_msg, 13 + piece_len, 0) == -1) {
        fprintf(stderr, "Error on send --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    connection->status_flags |= 1<<PENDINGREQUEST;
    connection->requested_piece = piece_index;
    free(piece_msg); 
}

void Set_Flag(Connections* connection, int flag, int state)
{
    if(state ==1)
        connection->status_flags |= 1<<flag;
    else if(state==0)
        connection->status_flags &= 0<<flag;
    else
        printf("Bad State");
}