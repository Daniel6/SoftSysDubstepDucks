#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

#define TRUE 0
#define FALSE 1

//These should be constants that are in a header/global (probably not global)
const char* BTPROTOCOL = "BitTorrent protocol";
const int PROTOCOLNAMELENGTH = 19;
const char* TESTSHA1 = "AAAAAAAAAAAAAAAAAAA1";
const int FULLHANDSHAKELENGTH = 68;


int verify_handshake(char* handshakeToVerify, char* clientFileSHA1);
char * construct_handshake(char hash[20], char id[20]);
void print_hex_memory(void* mem, int size);

int main(int argc, char *argv[])
{

char * handshake = construct_handshake(TESTSHA1, TESTSHA1);
printf("%s\n", handshake);
//printf("%s\n", test);
//When abstracting this to a library function, this is unnecessary. 
//char *handshakeToVerify = strdup(argv[1]);
//handshakeToVerify[0] = 19;
//const char* sha1 = "ASDASDA";
//verify_handshake(handshakeToVerify, sha1);

//free(handshakeToVerify);
	return 0;
}

char * construct_handshake(char hash[20], char id[20])
{
	char * handshake = malloc(FULLHANDSHAKELENGTH);
	memcpy(handshake, &PROTOCOLNAMELENGTH, 1);
	memcpy(handshake+1, BTPROTOCOL, PROTOCOLNAMELENGTH);
	memset(handshake+1+PROTOCOLNAMELENGTH, 0, 8);	

	print_hex_memory(handshake, 68);
	memcpy(handshake+1+PROTOCOLNAMELENGTH+8, hash, 20);

	print_hex_memory(handshake, 68);
	memcpy(handshake+1+PROTOCOLNAMELENGTH+20+8, id, 20);
	print_hex_memory(handshake, 68);
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

	int peerProtocolNameLength = (int) handshakeToVerify[0];
	//Verify name length
	if(peerProtocolNameLength - PROTOCOLNAMELENGTH)
		return 4;

	//Verify Protocol Name is the same.
	char * peerProtocolName = malloc(PROTOCOLNAMELENGTH);
	memcpy(peerProtocolName, handshakeToVerify+1, PROTOCOLNAMELENGTH);
	if (!strcmp(peerProtocolName, BTPROTOCOL))
	{
		free(peerProtocolName);
		return 5;
	}

	char * peerSHA1 = malloc(SHA_DIGEST_LENGTH);
	memcpy(peerSHA1, handshakeToVerify+1+PROTOCOLNAMELENGTH+8, 20); 

	/*Verify if the SHA1 hash for info key is the same. 
	Note that in this case, we will probably need to revise this for when we 
	have multiple files downloading/contained. There needs to be some way of
	verifying/checking from the list of torrents has at least one key whos
	SHA1 matches. -> Search a dictionary/hashmap by SHA1?
	*/
	if(!strcmp(peerSHA1, TESTSHA1))
	{
		free(peerSHA1);
		free(peerProtocolName);
		return 6;
	}
	 
	return TRUE;
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

