#include "btp.h"


int server_socket_wrapper(struct sockaddr_in *server_addr_p, char * server_address, int port_number)
	{
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
    fprintf(stderr, "Error creating socket --> %s", strerror(errno));

    exit(EXIT_FAILURE);
  }

 /* Zeroing server_addr struct */
  memset(server_addr_p, 0, sizeof(*server_addr_p));
  server_addr_p->sin_family = AF_INET;
  inet_pton(AF_INET, server_address, &(server_addr_p->sin_addr));
  server_addr_p->sin_port = htons(port_number);
  int reuse = 1;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1)
    error("Can't set the reuse option on the socket"); 

  return server_socket;
}

// Useful piece of support code when debugging some of this stuff
// Context was using it as a way to dump hex of char arrays easily.
void print_hex_memory(void *mem, int size) {
  int i;
  unsigned char *p = (unsigned char *)mem;
  for (i=0;i<size;i++) {
    printf("0x%02x ", p[i]);
  }
  printf("\n");
}

char * construct_handshake(char* hash, char* id)
{
	//Generates a Handshake assumiing that hash and id point to 20 byte arrays 
	char * handshake = malloc(FULLHANDSHAKELENGTH);
	memcpy(handshake, &PROTOCOLNAMELENGTH, 1);
	memcpy(handshake+1, BTPROTOCOL, PROTOCOLNAMELENGTH);
	memset(handshake+1+PROTOCOLNAMELENGTH, 0, 8);	
	memcpy(handshake+1+PROTOCOLNAMELENGTH+8, hash, 20);
	memcpy(handshake+1+PROTOCOLNAMELENGTH+20+8, id, 20);

	return handshake;
}

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
	 
	return TRUE;
}

char *construct_bitfield_message(char * bitfield, int bitfieldLen)
{

	char * msg = malloc(4 + 1 + bitfieldLen);
	int *msgLen = malloc(sizeof(int));
	*msgLen = bitfieldLen+1;
	memcpy(msg, msgLen, 4);
	msg[4] = (char) BITFIELD;
	memcpy(msg+5, bitfield, bitfieldLen);
	free(msgLen);
	return msg;
}

char * construct_state_msg(unsigned char msgID)
{
	char * msg = malloc(5);
	int * msgLen = malloc(4);
	*msgLen = 1;
	memcpy(msg, msgLen, 4);
	memcpy(msg+4, &msgID, 1);
	free(msgLen);
	return msg;

}


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
	*msgLen = 17;
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

char * construct_cancel_message(int piece_index, int blockoffset, int blocklength)
{
	//MsgLen = 4 bytes for msg_len 1 for msgID, 12 for piece, block_off, block_len
	char * msg = malloc(17);
	int * msgLen = malloc(4);
	*msgLen = 17;
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