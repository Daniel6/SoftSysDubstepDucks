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


	client_socket = client_socket_wrapper(&remote_addr, SERVER_ADDRESS, LISTENER_PORT_NUMBER);

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
	memset(ownBitfield, 0, total_pieces/8);
  //Initialize Peer Bitfield for torrent file;
	char * peerBitfield = malloc(total_pieces/8);
	memset(peerBitfield, 0, total_pieces/8); 
	char * piece_status = malloc(total_pieces);

  //Declare bitfield message length, maps from total pieces to the number of 
  //bits needed to represent it. (Hence the total_pieces/8) -> 8 being # of 
  //bits in a char.
	int bitfieldMsgLength = 4 + 1 + total_pieces/8;
	int bitfieldLen = total_pieces/8;
  //
	char ownInterested = FALSE;
	char ownChoked = TRUE;
	char peerInterested = FALSE;
	char peerChoked = TRUE;

  //Construct Handshake + SHA1 Hash
	char * sha = "AAAAAAAAAAAAAAAAAAA1";
	char * own_handshake = construct_handshake(sha, sha);
  //Tries to send handshake.
	if(send(client_socket, own_handshake, FULLHANDSHAKELENGTH,0)==-1){
		fprintf(stderr, "Error on send --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	uint32_t recieved_bytes = 0;
	recieved_bytes = recv(client_socket, buffer, BUFSIZ, 0);

	char * peer_handshake = malloc(FULLHANDSHAKELENGTH);
	int test;
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

		if(0<peerContainsUndownloadedPieces(peerBitfield, ownBitfield, bitfieldLen))
		{
			char * interested = construct_state_message(INTERESTED);

			ownInterested = TRUE;
			if(send(client_socket, interested, STATEMSGSIZE,0)==-1){
				fprintf(stderr, "Error on send --> %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			free(interested);
		}
		else{
			char * uninterested = construct_state_message(UNINTERESTED);
			if(send(client_socket, uninterested, STATEMSGSIZE,0)==-1){
				fprintf(stderr, "Error on send --> %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			free(uninterested);	
		}
	  //Free handshake  + memory after completion. -> Don't think we need it afterwards
		free(bitfield_message);
		free(peer_handshake);
	};





	received_file = fopen(FILENAME, "w");
	if (received_file == NULL) {
		fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}




	recieved_bytes = recv(client_socket, buffer, BUFSIZ, 0);
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
	//bufPtr has direct access to buffer. Reminder to not mess it up
	//accidently. 
	char * bufPtr = &buffer;
	int *  msgSize = malloc(4);
	char * msgID = malloc(1);
	//INTERPRET MESSAGES HERE.
	while(recieved_bytes > 0)
	{ 
		char * msg1 = get_next_msg(bufPtr, msgID, msgSize);
		bufPtr += *msgSize;
		switch(*msgID)
		{
			case(CHOKE):
			ownChoked = 1;
			printf("choked tho\n");
			break;
			case(UNCHOKE):
			ownChoked = 0;
			printf("unchoked tho\n");
			break;
			case(INTERESTED):
			peerInterested = 1;
			printf("interested tho\n");            
			break;
			case(UNINTERESTED):
			peerInterested = 0;
			printf("uninterested tho\n");
			break;
			case(HAVE):
			;
			//Reminder for the future -> write this into a function. 
			int *test = malloc(4);
			memcpy(test, msg1+5, 4);
			int byte_of_piece = *test/8;
			int	bit_of_piece = *test%8;
			char * byte_of_interest = peerBitfield;
			byte_of_interest = peerBitfield + byte_of_piece ;
			*byte_of_interest |= 1 << (7-bit_of_piece);
			free(test);
			print_bits(peerBitfield, 2);

			printf("have tho\n");            
			break;
			case(BITFIELD):
			//Technically this should never happen. 
			memcpy(peerBitfield, msg1+5, total_pieces);
			printf("bitfield tho\n");
			break;
			case(REQUEST):

			//Some kind of way to put on queue this send command. 
			printf("request tho\n");
			break;              
			case(PIECE):
			//Insert saving files code here lol. + Parsing the thing + how to deal 
			//with blocks within a piece?

			printf("piece tho\n");
			break;
			case(CANCEL):
			//Need to cancel the queue reponse. 

			printf("cancel tho\n");
			break;
			default:
			;
		}
		recieved_bytes -= *msgSize;
		free(msg1);          


		//Create uninterested message. 
		if(0==peerContainsUndownloadedPieces(peerBitfield, ownBitfield, bitfieldLen))
		{
			char * uninterested = construct_state_message(UNINTERESTED);
			ownInterested = FALSE;
			if(send(client_socket, uninterested, STATEMSGSIZE,0)==-1){
				fprintf(stderr, "Error on send --> %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			free(uninterested);
		}





	}

	free(msgSize);
	free(msgID);


	free(piece_status);
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
