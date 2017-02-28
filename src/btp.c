#include "btp.h"


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

/*
 * Verify a handshake message against a SHA1 hash.
 * If the handshake is verified to be correct, return 0.
 * If the 'Name Length' field of the handshake does not match
 * the SHA hash, return -1.
 * If the 'Protocol Name' field is incorrect, return -2.
 * If the 'Info Hash' field is incorrect, return -3.
 */
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

/*
 * Create and return a BITFIELD message, given the actual bitfield
 * and its length.
 *
 * bitfield: the bitfield to be made into the message
 * bitfieldLen: the length of the bitfield to add to the message
 *
 * returns: a constructed BITFIELD message
 */
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

/*
 * Create either a CHOKE, UNCHOKE, INTERESTED, or UNINTERESTED
 * message based on the value of msgID.
 *
 * msgID: char representing the ID of the state of the message to make
 *
 * returns: the created message
 */
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


//Construct a have msg. 
//Takes in the index of the piece that is desired. 
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

//Construct a request message.
//piece_index: Location of piece
//blockoffset: The location of the specific part of the piece that is requested
//blocklength: The length of the block to transfer.
//Returns: a Request Msg. 
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


//Construct a piece message.
//piece_index: Location of piece
//blockoffset: The location of the specific part of the piece that is requested
//piece_len: The length of the piece being transferred transfer.
//piece: The actual piece, length 'piece_len'
//Returns: a Piece Msg. 
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

/*
 * Construct and return a CANCEL message.
 *
 * piece_index: the index of the piece to cancel the request for
 * blockoffset: offset for the block 
 */
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

/*
 * Parses a buffer to extract information about the message it stores.
 *
 * bufPtr: pointer to a buffer storing the message to parse
 * msgID: pointer to a buffer to extract the ID of the message into
 * msgSize: pointer to an into to extract the size of the message into
 */
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

/*
 * Compare two buffers, returning 1 if the peer's buffer has high
 * bits that the own buffer does not have high.
 *
 * peer_buffer: a peer's buffer, to be checked for existence of bits not in own_buffer
 * own_buffer: your own buffer
 * bit_pieces: number of bits to compare in the two buffers
 */
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

/*
 * Initialize a Connections with the standard default flag values
 *
 * connection_to_initialize: pointer to the Connections to be initialized
 * total_pieces_in_file: number of pieces in the file that this Connections
 *                       will be torrenting
 */
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

/*
 * Verifies a handshake against a SHA hash of a file.
 * Exits if the handshake is not verified.
 *
 * buffer: char buffer containing a HANDSHAKE message sent from a peer
 * file_sha: SHA to verify against
 */
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
 * Create and return a char* bitfield message
 *
 * buffer: buffer containing the message
 * bitfieldMsgLength: int length of the BITFIELD message
 * total_pieces: int of total number of pieces in the file
 */
char *  Set_peerBitfield(char * buffer, int bitfieldMsgLength, int total_pieces)
{
    char * bitfield_message = malloc(bitfieldMsgLength);
    memcpy(bitfield_message, buffer+FULLHANDSHAKELENGTH, bitfieldMsgLength);
    return bitfield_message;
}

/*
 * Send an INTERESTED message to a given client socket
 *
 * client_socket: fd of the socket to send to
 * connection: pointer to Connections for the given client
 */
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

/*
 * Send an UNINTERESTED message to a given client socket
 *
 * client_socket: fd of the socket to send to
 * connection: pointer to Connections for the given client
 */
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

/*
 * Send an UNCHOKED message to a given client socket
 *
 * client_socket: fd of the socket to send to
 * connection: pointer to Connections for the given client
 */
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

/*
 * Send a REQUEST message to a given client socket
 *
 * client_socket: fd of the socket to send to
 * connection: pointer to Connections for the given client
 * piece_index: index of the piece to request
 */
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

/*
 * Send a PIECE message to a given client socket
 *
 * client_socket: fd of the socket to send to
 * connection: pointer to Connections for the given client
 * piece_index: index of the piece to send
 * file_d: fd of the file to get the piece from
 * piece_len: length of the piece
 */
void Send_piece(int client_socket, Connections *connection, int piece_index, int file_d, int piece_len) {
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

/*
 * Set a given flag in the Connections data structure's status_flag bitfield
 *
 * connection: Connections data structure to set the flag of
 * flag: int representing which flag in the status_flag to set
 * state: int of what to set flag to
 */
void Set_Flag(Connections* connection, int flag, int state)
{
    if(state == 1)
        connection->status_flags |= 1<<flag;
    else if(state==0)
        connection->status_flags &= 0<<flag;
    else
        printf("Bad State");
}