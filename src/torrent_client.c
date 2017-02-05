#include "btp.h"

int main(int argc, char ** argv)
{
	struct connection_info connections[10];
	char buffer[BUFSIZ];

	struct sockaddr_in peer[MAX_PEERS];
	struct pollfd fds[MAX_CONNECTIONS];
	memset(fds, 0 , sizeof(fds));
	//Important Global Variables for right now. 
	int sock_len = sizeof(struct sockaddr *);
	// Listener socket information
	int listener_socket; 
	struct sockaddr_in listener_addr;
	// For now, SERVER ADDRESS and PORT_NUMBER need to be defined 
	// Somehow specifically for the computer -> not just homing it. 
	char * own_ip = SERVER_ADDRESS+1;
	int own_port = LISTENER_PORT_NUMBER;
	//parse torrent file here
	/*Output Ithink includes 
	announce string -> tracker URI, 
	length -> length of files in bytes
	name of file -> name of file
	piece length -> integer for # of bytes in each piece
	# of pieces -> implicit from pieces detail
	hashes for each part of file -> in a string
	*/ 

	//Unknown question atm: how to make an array that contains the 
	//hashes of the string?
	//Below code is temporary until somebody writes a thing to get 
	//and parse torrent file. 
	char * announce = "127.0.0.1 8000";
	char * announce_ips = "???";
	char * file_name = "testfile.txt";
	int file_length = 256;
	int piece_size_bytes = 16;
	int total_pieces_in_file = file_length/piece_size_bytes;
	int bitfieldLen = total_pieces_in_file/8;

	//total_pieces_in_file/8 = number of bytes allocated for containing a
	//bitfield where each bit represents each piece in torrent.
	char * bitfield_of_current_pieces = malloc(bitfieldLen);
	//Initializing bitfield to zero. 
	memset(bitfield_of_current_pieces, 0, bitfieldLen);
	//Length of bitfieldMsgs for when sending to other peers. 
	int bitfieldMsgLength = 4 + 1 + bitfieldLen;
	//Construct Handshake 
	//Own_id will probably be best to be input as an argument to program. Must
	//Be unique across the network. 
	// file_sha should be the sha for the torrent file. 
	char * file_sha = "AAAAAAAAAAAAAAAAAAA1";
	char * own_id = "00000000000000000001";
	char * own_handshake = construct_handshake(file_sha, own_id);

	
	//Future question: should we also have a char array to handle piece status?

	//Somewhere here probably should be a way to check to see if
	//File is already there/partiall downloaded. 
	/*
		CODE TO CHECK FOR EXISTANCE. 
		should output a bitfield w/ values that correspond to owned pieces. 
	*/
	//Output should probablybe some kind of way of dealing w/ a file 
	//file descriptors and such. 


	//Tracker interaction here: Assumptino of some kind of char array list 
	//returned for each IP + total number of peers
	int num_of_peers = 1;

	//Need array lol
	int port_number = 30001;
	char * ip = "127.0.0.1";

	//Creating a listening socket, has room for 10 connections in backlog. decided to have it 
	//on last connection
	listener_socket = server_socket_wrapper(&listener_addr, own_ip, own_port);
	fds[0].fd = listener_socket;
	fds[0].events = POLLIN|POLLOUT;


	int i = 0;
	int peers_to_connect_to = num_of_peers;
	if(num_of_peers > MAX_CONNECTIONS-1){
		peers_to_connect_to = MAX_CONNECTIONS-1;
	}
	char * ip_buf = malloc(16*sizeof(char));

	for(i; i<peers_to_connect_to; i++){
		//Create a new socket address from peer -> do we need this? Might not.
		//Mostly because of the fact tahat we rewrite peers. But just in case.
		struct sockaddr_in *remote_addr = &peer[i];
		int client_socket = client_socket_wrapper(&remote_addr, ip, port_number);
		fds[i+1].fd = client_socket;
		fds[i+1].fd = POLLIN|POLLOUT;

		initialize_connection(&connections[i], total_pieces_in_file);

		/* Connect to the server */
		if (connect(client_socket, &remote_addr, sizeof(struct sockaddr)) == -1) {
			fprintf(stderr, "Error on connect --> %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if(send(client_socket, own_handshake, FULLHANDSHAKELENGTH,0)==-1){
			fprintf(stderr, "Error on send --> %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		uint32_t recieved_bytes = 0;
		recieved_bytes = recv(client_socket, buffer, BUFSIZ, 0);

		char * peer_handshake = malloc(FULLHANDSHAKELENGTH);
		int test;
		//I really dislike the following code. 

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
			test = verify_handshake(peer_handshake, file_sha);
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
			test = verify_handshake(peer_handshake, file_sha);
		    // Testing for issues on handshake.
			if(test)
			{
				fprintf(stderr, "Error on handshake ---> %d\n", test);
				exit(EXIT_FAILURE);
			}
	   	    //If there is a bitfield option, set bitfield. 
			memcpy(bitfield_message, buffer+FULLHANDSHAKELENGTH, bitfieldMsgLength);
			memcpy(connections[i].peerBitfield, bitfield_message+5, total_pieces_in_file);
			memset(buffer, 0, sizeof(buffer));

			if(0<peerContainsUndownloadedPieces(connections[i].peerBitfield, bitfield_of_current_pieces, bitfieldLen))
			{
				char * interested = construct_state_message(INTERESTED);
				connections[i].ownInterested = TRUE;
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
	}




	return 0;
}