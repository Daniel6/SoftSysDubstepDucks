#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stddef.h>

#include "btp.h"
#include "tracker.h"


int main(int argc, char *argv[]) {
	char buffer[BUFSIZ];
	Connections connections[MAX_PEERS];
	struct sockaddr_in peer[MAX_PEERS];
	struct pollfd fds[MAX_PEERS];
	memset(fds, 0 , sizeof(fds));
	//Iteration variable
  	int j;
	int i;

	char file_name[] = "testfile.txt.torrent";
	bt_info_t *ans = decodeFile(file_name);

	int file_destination;
	char *bitfield_of_current_pieces = set_initial_bitfield(&file_destination, ans->name, ans->num_pieces, ans->piece_length, ans->piece_hashes);


	int piece_size_bytes = ans->piece_length;
	int total_pieces_in_file = ans->length;
	//Truncating division. This only works right now for total pieces
	//divisible by 8. 
	int bitfieldLen = total_pieces_in_file/sizeof(char);
	if (total_pieces_in_file%8 != 0) {
		bitfieldLen++;		
	}

	//Length of bitfieldMsgs for when sending to other peers. 
	int bitfieldMsgLength = 4 + 1 + bitfieldLen;
	//Construct Handshake 
	//Features invovling own_id is not implemented right now. 
	char *file_sha = "AAAAAAAAAAAAAAAAAAA1";
	char *own_id = "00000000000000000001";
	char *own_handshake = construct_handshake(file_sha, own_id);

	char *tracker_ip = malloc(16);
	memcpy(tracker_ip, ans->announce, 16);

	printf("\nStarting tracker intersection\n");

	// Tracker interaction here: Assumption of some kind of char array list 
	// Creates tracker socket.
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

  	// create a buffer to contain the IP's of each peer.
	char *peer_buf = calloc(MAX_PEERS, sizeof(char)*16);
	int *num_of_peers = calloc(1, sizeof(int));
	*num_of_peers = 0;

	requestPeers(tracker_socket, peer_buf, num_of_peers);
	printf("Done with tracker interaction\n");
    printf("%d\n", *num_of_peers);
    printf("%d\n", (*num_of_peers)*16);
	char *peers [MAX_PEERS];
  	for (j = 0; j < *num_of_peers; j++) {
   		peers[j] = malloc(16);
  		memcpy(peers[j], peer_buf + (16 * j), 16);
  	}
  	
    for (j = 0; j < *num_of_peers; j++) {
  		printf("Peer %d: %s\n", j, peers[j]);	
 	}

    printf("Num peers: %d\n", *num_of_peers);

	// This refers to ensuring that we don't have more peers than our max allowed
	// connections. 
	int peers_to_connect_to = *num_of_peers;
	if (*num_of_peers > MAX_CONNECTIONS-1) {
		peers_to_connect_to = MAX_CONNECTIONS - 1;
	}

	// set up the listener socket
	// (used for listening for new connections)
	int listener_socket = configure_socket(); 
	fds[0].fd = listener_socket;
	fds[0].events = POLLIN | POLLOUT;

	
	int timeout = 60000; // 1 min in ms
	int max_peer_index = 0;
	char *peer_handshake = malloc(FULLHANDSHAKELENGTH);

	for (i=0; i < peers_to_connect_to; i++) {
		// Create a new socket address from peer -> do we need this? Might not.
		// Mostly because of the fact tahat we rewrite peers. But just in case.
		struct sockaddr_in *remote_addr = &peer[i];
		int client_socket = client_socket_wrapper((struct sockaddr_in *)&remote_addr, peers[i], LISTENER_PORT_NUMBER);
		fds[i+1].fd = client_socket;
		fds[i+1].events = POLLIN | POLLOUT;
		//Initialize the associated connecti
		initialize_connection(&connections[i], total_pieces_in_file);

		/* Connect to the server */
		if (connect(client_socket,  &remote_addr, sizeof(struct sockaddr)) == -1) {
			fprintf(stderr, "Error on connect --> %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("connected\n");

		// Send Handshake
		if (send(client_socket, own_handshake, FULLHANDSHAKELENGTH, 0) == -1) {
			fprintf(stderr, "Error on send --> %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("Sent Handshake\n");
	

		uint32_t recieved_bytes = 0;
		recieved_bytes = recv(client_socket, buffer, BUFSIZ, 0);
		printf("just recv\n");
		if (recieved_bytes == -1) {
			fprintf(stderr, "Error on reception");
			exit(EXIT_FAILURE);
		} else if (recieved_bytes == 0) {	
		//If the connection was dropped.
			fprintf(stdout, "Dropped Connection\n");
			exit(EXIT_FAILURE);
		} else if (recieved_bytes == FULLHANDSHAKELENGTH) {
	
			Verify_handshake(buffer, file_sha);
			memset(buffer, 0,sizeof(buffer));
		} else if (recieved_bytes == FULLHANDSHAKELENGTH + bitfieldMsgLength) {
			Verify_handshake(buffer, file_sha);
			//Set Peer Bitfield

			connections[i].peerBitfield = Set_peerBitfield(buffer, bitfieldMsgLength, total_pieces_in_file);

			//Check to see if peer had undownloaded pieces.
			if (0 < peerContainsUndownloadedPieces(connections[i].peerBitfield, bitfield_of_current_pieces, bitfieldLen)) {
				Send_interested(client_socket, &connections[i]);	
			} else {
				//Send uninterested message
				Send_uninterested(client_socket, &connections[i]);
			}
			//Clear buffer
			memset(buffer, 0, sizeof(buffer));

		}
	}


	printf("hello\n");
	int* test = malloc(4);
	int *piece_index = malloc(4);

	while (1) {
		int num_have_pieces = 0;
		for (i = 0; i < total_pieces_in_file; i++) {
			if (bitfield_of_current_pieces[i] & 255 == 1) {
				num_have_pieces++;
			}
		}
		if (num_have_pieces == total_pieces_in_file) {
			printf("You have the entire file!\n");
		}

		int n_ready = poll(fds, max_peer_index + 1, timeout); // number of ready connections
		if (n_ready == -1) {
			perror("Poll error: "); 
		} else if (n_ready == 0) {
			printf("Timeout! No data after %i ms\n", timeout);
		} else {
			// check listener
			if (fds[0].revents & POLLIN) {
				struct sockaddr_storage client_addr;
    			unsigned int address_size = sizeof(client_addr);

    			// get the fd
    			int connect_fd;
    			if ((connect_fd = accept(listener_socket, (struct sockaddr *)&client_addr, &address_size)) == -1) {
    				fprintf(stderr, "Listener can't open another client socket\n");
    			} else {
    				// success opening socket

    				// save fd to connections array as first unused 
    				// element in fds (some connections might've closed)
    				// TODO: make into function called save_to_connections(fds, )
    				for (i = 1; i < MAX_CONNECTIONS; i++) {
    					if (fds[i].fd < 0) {
    						fds[i].fd = connect_fd;
    						if (i > max_peer_index) {
    							max_peer_index = 1;
    							peers_to_connect_to++;
    						}
    						break;
    					}
    				}

    			}

    			if (--n_ready <= 0) {
    				// no more fds are ready
    				continue;
    			}
    			
    		
			}

			// check all clients
			for (i = 1; i < max_peer_index; i++) {
				if (fds[i].fd < 0) {
					// this connection has been closed
					continue;
				}
                
				if (fds[i].revents & POLLIN) {
					int bytes_received = read_in(fds[i].fd, buffer, BUFSIZ);
					if (bytes_received == EOF) {
						fprintf(stderr, "Error reading from peer server\n");
					} else if (bytes_received == 0) {
						fprintf(stderr, "Peer server %i dropped connection\n", i);
						// close this socket
						close(fds[i].fd);
						fds[i].fd = -1;
						Set_Flag(&connections[i], CONNECTIONSTATUS,0);
					} else if (bytes_received == FULLHANDSHAKELENGTH) {
						Verify_handshake(buffer, file_sha);
						memset(buffer, 0, sizeof(buffer));

						// need to send our handshake and bitfield
						// this peer is a client b/c they only sent
						// handshake, not also a bitfield

						char * bf_message = construct_bitfield_message(bitfield_of_current_pieces, total_pieces_in_file/8);
						char * intro_msg = malloc(FULLHANDSHAKELENGTH+bitfieldMsgLength);
						memcpy(intro_msg, own_handshake, FULLHANDSHAKELENGTH);
						memcpy(intro_msg+FULLHANDSHAKELENGTH, bf_message, bitfieldMsgLength);

						if(send(fds[i].fd, intro_msg, bitfieldMsgLength+FULLHANDSHAKELENGTH, 0) == -1) {
							fprintf(stderr, "Sending BF message failed");
							exit(EXIT_FAILURE);
						}
					} else if (bytes_received == FULLHANDSHAKELENGTH + bitfieldMsgLength) {
						Verify_handshake(buffer, file_sha);
						//Set Peer Bitfield

						connections[i].peerBitfield = Set_peerBitfield(buffer, bitfieldMsgLength, total_pieces_in_file);

						//Check to see if peer had undownloaded pieces.
						if (0 < peerContainsUndownloadedPieces(connections[i].peerBitfield, bitfield_of_current_pieces, bitfieldLen)) {
							Send_interested(fds[i].fd, &connections[i]);	
						} else {
							//Send uninterested message
							Send_uninterested(fds[i].fd, &connections[i]);
						}
						//Clear buffer
						memset(buffer, 0, sizeof(buffer));

					} else {
						// they sent you a PWP 
						char *bufPtr = &buffer;
	                    int *msgLength = malloc(4);
	                    char *msgID = malloc(1);

	                    // determind what kind of message they sent
	                    char *msg = get_next_msg(bufPtr, msgID, msgLength);
	                    bufPtr += *msgLength;
	                    int *piece_index = malloc(4);

	                    // respond accordingly
	                    switch(*msgID) {
	                        case(CHOKE):
	                        	Set_Flag(&connections[i], OWNCHOKE, 1);
	                            printf("I am choked\n");
	                            break;
	                        case(UNCHOKE):
	                        	Set_Flag(&connections[i], OWNCHOKE, 0);
	                            printf("I am unchoked\n");
	                            break;
	                        case(INTERESTED):
	                         	Set_Flag(&connections[i], PEERINTERESTED, 1);
	                            printf("Peer is interested\n");            
	                            break;
	                        case(UNINTERESTED):
	                         	Set_Flag(&connections[i], PEERINTERESTED, 0);
	                            printf("Peer is uninterested\n");
	                            break;
	                        case(HAVE):
	                            //Reminder for the future -> write this into a function. 
	                            memcpy(test, msg+5, 4);                        
	                            int byte_of_piece = *test/8;
	                            int bit_of_piece = *test%8;
	                            char *byte_of_interest = connections[i].peerBitfield;
	                            byte_of_interest = connections[i].peerBitfield + byte_of_piece;
	                            *byte_of_interest |= 1 << (7-bit_of_piece);
	                            free(test);
	                            print_bits(connections[i].peerBitfield, 2);
	                            printf("have tho\n");            
	                            break;
	                        case(BITFIELD):
	                            //Technically this should never happen. 
	                            memcpy(connections[i].peerBitfield, msg+5, total_pieces_in_file);
	                            printf("bitfield tho\n");
	                            break;
	                        case(REQUEST):
	                            // Some kind of way to put on queue this send command.
	                        	memcpy(&(piece_index), msg + 5, 4);
	                        	connections[i].piece_to_send = piece_index;
	                        	int sent = 0;
	                            printf("request tho\n");
	                            break;              
	                        case(PIECE):
	                            // Insert saving files code here lol. + Parsing the thing + how to deal 
	                            // with blocks within a piece?
	                        	memcpy(&(piece_index), msg + 5, 4);
	                        	memcpy(&(buffer), msg + 5 + 5, piece_size_bytes);
	                        	if (verify_piece(buffer, ans->piece_hashes[+(i*SHA_DIGEST_LENGTH)])) {
	                        		write_piece(file_destination, piece_index, piece_size_bytes, buffer);
	                        		*bitfield_of_current_pieces |= 1 << *piece_index;
	                        		Set_Flag(&connections[i], PENDINGREQUEST, 0);
	                        	}
	                            printf("piece tho\n");
	                            break;
	                        case(CANCEL):
	                            // Need to cancel the queue reponse.
	                        	connections[i].piece_to_send = -1;
	                            printf("cancel tho\n");
	                            break;
	                        default:
	                        	printf("default\n");
	                    }
	                    free(msg); 
					}
				}

				if (fds[i].revents & POLLOUT) {
					// if we have requested a piece:
						// check if they have requested a piece
					// if we haven't requested a piece:
						// send an interested message

					int pending_request = (connections[i].status_flags >> PENDINGREQUEST) & 1;
					int own_interested = (connections[i].status_flags >> OWNINTERESTED) & 1;
					int own_choke = (connections[i].status_flags >> OWNCHOKE) & 1;

					// if peer interested and (we are not interested or we have a pending request)
					if ( ((connections[i].status_flags >> PEERINTERESTED) & 1) && (!own_interested || pending_request)) {
						// we already requested from them, waiting for it to be sent
						// or we do not want anything from them
						// so we should send them something if they are interested

						if (!((connections[i].status_flags >> PEERCHOKE) & 1)) {
							int piece_num = connections[i].piece_to_send;
								if (piece_num >= 0) {
									// they are interested, unchoked, and have
									// requested a piece, so send a piece message
									Send_piece(fds[i].fd, &connections[i], piece_num, file_destination, piece_size_bytes);
								}
						} else {
							// they are interested but choked, so unchoke them
							Send_unchoked(fds[i].fd, &connections[i]);
						}


					} else if (!pending_request) {
						// we haven't requested anything
						// do they have a piece we need??
						// if so:
							// if interested
								// if unchoked
									// send request
							// else
								// send interested
						// else:
							// send uninterested


						// if they have a piece we do not have
						if (0 < peerContainsUndownloadedPieces(connections[i].peerBitfield, bitfield_of_current_pieces, bitfieldLen)) {
							// if we are currently interested and not choked, request something
							if (own_interested) {
								if (!own_choke) {
									int piece_index;
									for (i = 0; i < total_pieces_in_file; i++) {
										int own_have = (*bitfield_of_current_pieces >> i) & 1;
										int peer_have = (*connections[i].peerBitfield >> i) & 1;
										if (!own_have && peer_have) {
											piece_index = i;
											break;
										}
									}
									Send_request(fds[i].fd, &connections[i], piece_index);
								}	
							} else {
								// we are currently not interested, send interested
								Send_interested(fds[i].fd, &connections[i]);
							}
						} else {
							// they do not have pieces we need
							Send_uninterested(fds[i].fd, &connections[i]);
						}
					}

				}

			}
		}
	}
}

