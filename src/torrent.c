#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stddef.h>

#include "btp.h"
#include "tracker.h"


typedef struct connection_info{
	char ownInterested;
	char ownChoked;
	char peerInterested;
	char peerChoked;
	char *peerBitfield;
	int requested_piece;
	int piece_to_send;
} Connections;

int main(int argc, char *argv[]) {
	char buffer[BUFSIZ];
	Connections connections_list[10];
	struct sock_in peer[MAX_PEERS];
	struct pollfd fds[MAX_CONNECTIONS];

	char *my_bitfield;



	char file_name[] = "62-Q2.mp3.torrent";
	bt_info_t *ans = decodeFile(file_name);

	/*
	 * Do lots of stuff until it is time for main while loop
	 *
	*/ 

	
	int timeout = 60000; // 1 min in ms
	int max_peer_index = 0;
	int i = 0;
	char * peer_handshake = malloc(FULLHANDSHAKELENGTH);
	int test;
	while (1) {
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
    						client[i].fd = connect_fd;
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
					connections[i].connected = 0;
				} else if (bytes_received == FULLHANDSHAKELENGTH) {
					// handle handshake
				} else {
					// they sent you a PWP 
					char *bufPtr = &buffer;
                    int *msgLength = malloc(4);
                    char *msgID = malloc(1);

                    // determind what kind of message they sent
                    char *msg = get_next_msg(bufPtr, msgID, msgSize);
                    bufPtr += *msgSize;

                    // respond accordingly
                    switch(*msgID) {
                        case(CHOKE):
                            connections[i]->ownChoked = 1;
                            printf("I am choked\n");
                            break;
                        case(UNCHOKE):
                            connections[i]->ownChoked = 0;
                            printf("I am unchoked\n");
                            break;
                        case(INTERESTED):
                            connections[i]->peerInterested = 1;
                            printf("Peer is interested\n");            
                            break;
                        case(UNINTERESTED):
                            connections[i]->peerInterested = 0;
                            printf("Peer is uninterested\n");
                            break;
                        case(HAVE):
                            //Reminder for the future -> write this into a function. 
                            int *test = malloc(4);
                            memcpy(test, msg1+5, 4);
                            int byte_of_piece = *test/8;
                            int bit_of_piece = *test%8;
                            char * byte_of_interest = peerBitfield;
                            byte_of_interest = peerBitfield + byte_of_piece ;
                            *byte_of_interest |= 1 << (7-bit_of_piece);
                            free(test);
                            print_bits(peerBitfield, 2);

                            printf("have tho\n");            
                            break;
                        case(BITFIELD):
                            //Technically this should never happen. 
                            memcpy(connections[i]->bitfield, msg1+5, total_pieces);
                            printf("bitfield tho\n");
                            break;
                        case(REQUEST):

                            // Some kind of way to put on queue this send command. 
                        	int piece_index;
                        	memcpy(&(piece_index), msg1 + 5, 4);
                        	connections[i]->piece_to_send = piece_index;
                        	int sent = 0;
                            printf("request tho\n");
                            break;              
                        case(PIECE):
                            // Insert saving files code here lol. + Parsing the thing + how to deal 
                            // with blocks within a piece?
                        	int piece_index;
                        	memcpy(&(piece_index), msg1 + 5, 4);
                        	char *piece = 
                        	if (verify_piece()) {
                        		write_piece(main_fd, piece_index, piece_size_bytes, char *buffer);
                            	connections[i]->received = 1;
                        	}
                            printf("piece tho\n");
                            break;
                        case(CANCEL):
                            // Need to cancel the queue reponse. 
                        	int piece = 0;
                        	connections[i]->piece_to_send = -1;
                        	int sent = 0;
                            printf("cancel tho\n");
                            break;
                        default:
                    }
                    recieved_bytes -= *msgSize;
                    free(msg1); 
				}
			}

			if (fds[i].revents & POLLOUT) {
				// if we have requested a piece:
					// check if they have requested a piece
				// if we haven't requested a piece:
					// send an interested message

				if (connections[i]->pending_request) {
					// we already requested from them, waiting for it to be sent
					// so we should send them something if they are interested

					if (!connections[i]->sent) {
						// we have not sent them anything so check steps
						// necessary to send something

						if (connections[i]->peerInterested) {
							if (!connections[i]->choked) {
								if (connections[i]->piece_to_send >= 0) {
									// they are interested, unchoked, and have
									// requested a piece, so send a piece message

								}
							} else {
								// they are interested but choked, so unchoke them
							}
						}
					}
					


				} else {
					// we haven't requested anything
					// do they have a piece we need??
					// if so:
					// if choked 
						// send interested
					// else
						// if interested
							// send request
						// else 
							// send interested
					if (connections[i]->choked) {
						// we are choked, so ask them to unchoke
						// by sending an interested message

					} else {
						// we are unchoked so 
					}
				}
			}

		}
	}
}