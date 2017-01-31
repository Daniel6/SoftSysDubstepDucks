#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <byteswap.h>

#define CHOKE 0
#define UNCHOKE 1
#define INTERESTED 2
#define UNINTERESTED 3
#define HAVE 4
#define BITFIELD 5
#define REQUEST 6
#define PIECE 7
#define CANCEL 8

void parse_message( char* message);
void create_core_state_message(char* dst,uint8_t ID);
void create_have_message(char* dst, uint32_t piece_index);
void print_hex_memory(void *mem, int size);

int main(int argc, char *argv[])
{

	char *message = strdup(argv[1]);
	char *testStatusMessage = malloc(5);
	char *testHaveMessage = malloc(9);

	uint8_t test = 1;
	create_core_state_message(testStatusMessage, test);
	create_have_message(testHaveMessage, 0xabcdef12);

//	printf("%\n", testStatusMessage);
	print_hex_memory(testStatusMessage, 5);
	print_hex_memory(testHaveMessage, 9);


	message[0] = 1;
	message[1] = 0;
	message[2] = 0;
	message[3] = 0;
	//This won't work. can't get size of a pointer. 
	//message[4] = sizeof(argv[1])-1;
	message[4] = strlen(argv[1])-1;
	//	parse_message(message);

	free(testHaveMessage);
	free(testStatusMessage);
	free(message);
	return 0;
}

void create_core_state_message( char* dst, uint8_t ID)
{
	uint32_t new_id;
	//SUPER PLATFORM Specific. Should  probably check if works on our other 
	//Computers (This reverses the byte order). Should probably just do bit 
	//Magic on it later. 
//	new_id = __bswap_32(1);
	new_id = 1;
	memcpy(dst, (char*)&new_id, 4);
	*(dst+4) = ID;	
}

void create_have_message(char* dst, uint32_t piece_index)
{

	uint32_t new_id = 5;
//	new_id = __bswap_32(5);
	memcpy(dst, (char*)&new_id,4);
	*(dst+4) = HAVE;
	memcpy(dst+5, (char*)&piece_index, 4);


}


void parse_message(char* message)
{
	//Copy the message length to memory
	int* messageLength = malloc(4);
	memcpy(messageLength, message, 4);
	int messageID = message[4];

//Case to addresss the different kind of messages you can get. 
	switch(messageID)
	{
		case CHOKE:
			printf("%s\n", "CHOKE");
			break;
		case UNCHOKE:
			printf("%s\n", "UNCHOKE");
			break;
		case INTERESTED:
			printf("%s\n", "INTERESTED");
			break;
		case UNINTERESTED:
			printf("%s\n", "UNINTERESTED");		
			break;		
		case HAVE:
			printf("%s\n", "HAVE");
			break;
		case BITFIELD:
			printf("%s\n", "BITFIELD");
			break;
		case REQUEST:
			printf("%s\n", "REQUEST");
	
			break;
		case PIECE:
			printf("%s\n", "PIECE");
			break;
		case CANCEL:
			printf("%s\n", "CANCEL");
		
			break;
		default:
		;
	}
	free(messageLength);

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
