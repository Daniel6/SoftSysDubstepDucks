#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stddef.h>

#include "btp.h"
#include "file_constructor.h"
#include "tracker.h"

int main(int argc, char ** argv)
{
	char buffer[BUFSIZ];
	connection_info connections[10];
	struct sockaddr_in peer[MAX_PEERS];
	struct pollfd fds[MAX_CONNECTIONS];


	memset(fds, 0 , sizeof(fds));


	//Important Global Variables for right now. 
	//	int sock_len = sizeof(struct sockaddr *);
	// Listener socket information
	int listener_socket; 
	struct sockaddr_in listener_addr;
	// For now, SERVER ADDRESS and PORT_NUMBER need to be defined 
	// Somehow specifically for the computer -> not just homing it. 
	char * own_ip = SERVER_ADDRESS;
	int own_port = LISTENER_PORT_NUMBER;
	//Parse torrent file. 

	char target[] = "moby_dick.txt.torrent";
	char length[80];
	bt_info_t *ans =  decodeFile(target);

	int file_destination;
	char * bitfield_of_current_pieces =	set_initial_bitfield(&file_destination, ans->name, ans->num_pieces, ans->piece_length, ans-> piece_hashes);

	//Parse tracker info

	 char *tracker_ip = "10.7.8.50";
	 printf("Tracker IP set to: %s\n", tracker_ip);
	 printf("%s\n", tracker_ip);
	//char *tracker_ip = malloc(16);
	//memcpy(tracker_ip, ans->announce, 16);
	//

	//	char * announce = "127.0.0.1 8000";
	//	char * announce_ips = "???";
	//	char * file_name = "testfile.txt";
	
	int file_length = 256;
	int piece_size_bytes = 16;
	int total_pieces_in_file = file_length/piece_size_bytes;
	//Truncating division. This only works right now for total pieces
	//divisible by 8. 
	int bitfieldLen = total_pieces_in_file/sizeof(char);
	if(total_pieces_in_file%8 != 0)
	{
		bitfieldLen++;		
	}
	print_bits(bitfield_of_current_pieces, bitfieldLen);


	//Length of bitfieldMsgs for when sending to other peers. 
	int bitfieldMsgLength = 4 + 1 + bitfieldLen;
	//Construct Handshake 
	//Own_id will probably be best to be input as an argument to program. Must
	//Be unique across the network. 
	//file_sha should be the sha for the torrent file. 
	char * file_sha = "AAAAAAAAAAAAAAAAAAA1";
	char * own_id = "00000000000000000001";
	char * own_handshake = construct_handshake(file_sha, own_id);

	
	//Somewhere here probably should be a way to check to see if
	//File is already there/partiall downloaded. 
	/*
		CODE TO CHECK FOR EXISTANCE. 
		should output a bitfield w/ values that correspond to owned pieces. 
	*/
	//Output should probablybe some kind of way of dealing w/ a file 
	//file descriptors and such. 

	printf("Starting tracker stuff\n");
	//===============================================================================================
	//Tracker interaction here: Assumption of some kind of char array list 
	//Creates tracker socket.
	struct sockaddr_in tracker_addr;
	tracker_addr.sin_family = AF_INET;
    tracker_addr.sin_port = htons(TRACKER_PORT);
    inet_pton(AF_INET, tracker_ip, &(tracker_addr.sin_addr));
	 int tracker_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tracker_socket < 0) {
	    fprintf(stderr, "Error creating socket: error %d\n", tracker_socket);
    	exit(1);
  	}

	if (connect(tracker_socket, (struct sockaddr *)&tracker_addr, sizeof(struct sockaddr)) == -1) {
    	fprintf(stderr, "[Error %s] ", strerror(errno));
    	fprintf(stderr, "Error connecting to tracker.\n");
    	exit(1);
  	}
	if (strlen(tracker_ip) <= 1) {
    	fprintf(stdout, "Connected to local tracker.\n");
  	} 
  	else {
 	    fprintf(stdout, "Connected to tracker at address: %s\n", tracker_ip);
  	}

  	//Used to create a buffer to contain the IP's of each peer.
	char *peer_buf = calloc(MAX_PEERS,sizeof(char)*16);
	int *num_of_peers=calloc(1, sizeof(int));
	*num_of_peers = 0;

	requestPeers(tracker_socket, peer_buf, num_of_peers);
	printf("Done with tracker stuff\n");
    printf("%d\n", *num_of_peers);
    printf("%d\n", (*num_of_peers)*16);
	char *peers [MAX_PEERS];
  	//Iteration variable
  	int j;
  	for (j = 0; j < *num_of_peers; j++) {
   		peers[j] = malloc(16);
  		memcpy(peers[j], peer_buf + (16 * j), 16);
  	}
  	
    for (j = 0; j < *num_of_peers; j++) {
  		printf("Peer %d: %s\n", j, peers[j]);	
 	}

    printf("Num peers: %d\n", *num_of_peers);

	//returned for each IP + total number of peers
    //================================================================================================

    //Creates a listener port.
	int port_number = LISTENER_PORT_NUMBER;
	
	//Following should be deprecated soon, was used to test connections on this computer.
	char * ip = "127.0.0.1";

	//Creating a listening socket, has room for 10 connections in backlog. decided to have it 
	//on last connection. This will be used as the 0 index in our file descriptor table 
	//for polling
	//listener_socket = server_socket_wrapper(&listener_addr, own_ip, own_port);
	//fds[0].fd = listener_socket;
	//fds[0].events = POLLIN|POLLOUT;

	//This refers to ensuring that we don't have more peers than our max allowed
	//connections. 
	int peers_to_connect_to = *num_of_peers;
	if(*num_of_peers > MAX_CONNECTIONS-1){
		peers_to_connect_to = MAX_CONNECTIONS-1;
	}

	//For accesing the peer array in connections[i], i = 1+fdsindex for the 
	//corresponding socket
	//Basically, given the list of IP's that we can connect to, we add it to the 
	//file descriptor table to monitor. 
	int i;
	for(i=0; i<peers_to_connect_to; i++){
		//Create a new socket address from peer -> do we need this? Might not.
		//Mostly because of the fact tahat we rewrite peers. But just in case.
		struct sockaddr_in *remote_addr = &peer[i];
		int client_socket = client_socket_wrapper((struct sockaddr_in *)&remote_addr, ip, port_number);

		
		fds[i+1].fd = client_socket;
		//Need to double check
		fds[i+1].events = POLLIN|POLLOUT;
		//Initialize the associated connecti
		initialize_connection(&connections[i], total_pieces_in_file);

		/* Connect to the server */
		if (connect(client_socket,  &remote_addr, sizeof(struct sockaddr)) == -1) {
			fprintf(stderr, "Error on connect --> %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("connected\n");

		//Send Handshake
		if(send(client_socket, own_handshake, FULLHANDSHAKELENGTH,0)==-1){
			fprintf(stderr, "Error on send --> %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("Sent Handshake\n");
	
		uint32_t recieved_bytes = 0;
		recieved_bytes = recv(client_socket, buffer, BUFSIZ, 0);
		if(recieved_bytes == -1)
		{
			fprintf(stderr, "Error on reception");
			exit(EXIT_FAILURE);
		}
		else if(recieved_bytes == 0)
		{	
		//If the connection was dropped.
			fprintf(stdout, "Dropped Connection\n");
			exit(EXIT_FAILURE);
		}
		else if(recieved_bytes == FULLHANDSHAKELENGTH)
		{
	
			Verify_handshake(buffer, file_sha);
			memset(buffer, 0,sizeof(buffer));
		}	
		else if( recieved_bytes == FULLHANDSHAKELENGTH + bitfieldMsgLength)
		{
			Verify_handshake(buffer, file_sha);
			//Set Peer Bitfield

			connections[i].peerBitfield = Set_peerBitfield(buffer, bitfieldMsgLength, total_pieces_in_file);

			//Check to see if peer had undownloaded pieces.
			if(0<peerContainsUndownloadedPieces(connections[i].peerBitfield, bitfield_of_current_pieces, bitfieldLen))
			{
				Send_interested(client_socket, &connections[i]);	
			}
			else{
				//Send uninterested message
				Send_uninterested(client_socket, &connections[i]);
			}
			//Clear buffer
			memset(buffer, 0, sizeof(buffer));

		};	
	}

		print_bits(&connections[i].peerBitfield, bitfieldMsgLength);

		printf("hello\n");
	for(;;)
	{
		int rv = poll(fds, POLLIN|POLLOUT, 60*1000);
		if(rv == -1){
			fprintf(stderr, "Poll failed--> %s\n", strerror(errno));
			exit(EXIT_FAILURE);		
		}
		else if(rv ==0)
		{
			printf("No response after 1 minute");
		}
		else{
			//Deal w/ listener


			//Deal w/ every other one. 



		}




	}



	return 0;
}
