#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stddef.h>

#include "btp.h"
#include "tracker.h"

int main(int argc, char ** argv) {

    char buffer[BUFSIZ];
    struct connection_info connections[10];
    struct sockaddr_in peer[MAX_PEERS];
    struct pollfd fds[MAX_CONNECTIONS]; // sockets for polling

    memset(fds, 0, sizeof(fds)); // set fds to 0?? WHY


    /* Important Global Variables for right now. */

    int sock_len = sizeof(struct sockaddr *);
    // Listener socket information
    int listener_socket; 
    struct sockaddr_in listener_addr;

    // For now, SERVER ADDRESS and PORT_NUMBER need to be defined 
    // Somehow specifically for the computer -> not just homing it. 
    char *own_ip = SERVER_ADDRESS;
    int own_port = LISTENER_PORT_NUMBER;
    //Parse torrent file. 

    char target[] = "62-Q2.mp3.torrent";
    char length[80];
    // bt_info_t *ans =  decodeFile(target);

    //Parse tracker info
    char *tracker_ip = "10.7.88.53";
    printf("%s\n", tracker_ip);
    //char *tracker_ip = malloc(16);
    //memcpy(tracker_ip, ans->announce, 16);

    //  char * announce = "127.0.0.1 8000";
    //  char * announce_ips = "???";
     char * file_name = "testfile.txt";
    
    int file_length = 256;
    int piece_size_bytes = 16;
    int total_pieces_in_file = file_length/piece_size_bytes;
    //Truncating division. This only works right now for total pieces
    //divisible by 8. 
    int bitfieldLen = total_pieces_in_file/sizeof(char);
    char *bitfield_of_current_pieces = malloc(bitfieldLen);
    memset(bitfield_of_current_pieces, 0, bitfieldLen);


    //Length of bitfieldMsgs for when sending to other peers. 
    int bitfieldMsgLength = 4 + 1 + bitfieldLen;
    //Construct Handshake 
    //Own_id will probably be best to be input as an argument to program. Must
    //Be unique across the network. 
    //file_sha should be the sha for the torrent file. 
    char *file_sha = "AAAAAAAAAAAAAAAAAAA1";
    char *own_id = "00000000000000000001";
    char *own_handshake = construct_handshake(file_sha, own_id);

    
    //Somewhere here probably should be a way to check to see if
    //File is already there/partiall downloaded. 
    /*
        TODO: CODE TO CHECK FOR EXISTANCE. 
        should output a bitfield w/ values that correspond to owned pieces. 
    */
    //Output should probablybe some kind of way of dealing w/ a file 
    //file descriptors and such.

    // if the file does not exist, create it
    int main_fd;
    set_bitfield(&main_fd, file_name, &bitfield_of_current_pieces, total_pieces_in_file, piece_size_bytes);
    if ((main_fd = open(file_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR)) < 0) {
        error("Error opening file to torrent");
    }



    printf("Starting tracker stuff\n");
    //===============================================================================================
    //Tracker interaction here: Assumption of some kind of char array list 
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
    } else {
        fprintf(stdout, "Connected to tracker at address: %s\n", tracker_ip);
    }

    char *peer_buf = calloc(MAX_PEERS,sizeof(char)*16);
    int *num_of_peers=calloc(1, sizeof(int));
    *num_of_peers = 0;

    requestPeers(tracker_socket, peer_buf, num_of_peers);
    printf("Done with tracker stuff\n");
    printf("%d\n", *num_of_peers);
    printf("%d\n", (*num_of_peers)*16);

    char *peers [MAX_PEERS];
    for (int j = 0; j < *num_of_peers; j++) {
        peers[j] = malloc(16);
        memcpy(peers[j], peer_buf + (16 * j), 16);
        printf("Peer %d: %s\n", j, peers[j]); 
    }

    printf("Num peers: %d\n", *num_of_peers);

        //returned for each IP + total number of peers
      //================================================================================================

    //Need array lol
    int port_number = LISTENER_PORT_NUMBER;
    char *ip = "127.0.0.1";
    //Creating a listening socket, has room for 10 connections in backlog. decided to have it 
    //on last connection
    listener_socket = server_socket_wrapper(&listener_addr, own_ip, own_port);
    fds[0].fd = listener_socket;
    fds[0].events = POLLIN | POLLOUT;


    int peers_to_connect_to = num_of_peers;
    if (num_of_peers > MAX_CONNECTIONS-1) {
        peers_to_connect_to = MAX_CONNECTIONS-1;
    }
//  char * ip_buf = malloc(16*sizeof(char));

    //For accesing the peer array in connections[i], i = 1+fdsindex for the 
    //corresponding socket
    //Flipping int i right before peers causes things to break somehow. 
    //WHY?????
    // int i;

    // handshake with all of your connections
    // (not including listener socket)
    for(int i=0; i < peers_to_connect_to; i++) {
        //Create a new socket address from peer -> do we need this? Might not.
        //Mostly because of the fact tahat we rewrite peers. But just in case.
        struct sockaddr_in *remote_addr = &peer[i];
        int client_socket = client_socket_wrapper((struct sockaddr_in *)&remote_addr, ip, port_number);
        fds[i+1].fd = client_socket;
        fds[i+1].events = POLLIN|POLLOUT;

        initialize_connection(&connections[i], total_pieces_in_file);

        /* Connect to the server */
        if (connect(client_socket,  &remote_addr, sizeof(struct sockaddr)) == -1) {
            fprintf(stderr, "Error on connect --> %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        printf("connected\n");
        if(send(client_socket, own_handshake, FULLHANDSHAKELENGTH,0)==-1){
            fprintf(stderr, "Error on send --> %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("connected\n");
        
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
        }  
        
    }

    // assign the pieces to the different peers
    Node assigned_pieces = assign_pieces(connections, peers_to_connect_to, own_bitfield, total_pieces_in_file);

    // placeholder vars for when listener socket
    // is being contacted by a client peer
    struct sockaddr_storage client_addr;
    unsigned int address_size = sizeof(client_addr);


    // main while loop:
    //   forever check all of your connections until
    //   you have downloaded the whole file or all that
    //   is available to you and until you have served as
    //   much of the file as you are willing to serve
    while (1) {
        // int rv = poll(fds, POLLIN|POLLOUT, 60*1000);
        int rv = poll(fds, peers_to_connect_to, 60*1000);
        if (rv == -1) {
            fprintf(stderr, "Poll failed--> %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        } else if (rv == 0) {
            printf("Time out occured, no response after 1 minute");
        } else {
            // check listener: has anyone contacted you?
            // do you need to send someone something?

            if (fds[0].revents & POLLIN) {
                int connect_d = accept(listener_socket, (struct sockaddr *)&client_addr, &address_size);
                if (connect_d == -1) {
                    error("Listener can't open another socket");
                }

                // read in the message being sent to you
                int bytes_received = read_in(connect_d, buffer, BUFSIZ);
                if (recieved_bytes == -1) {
                    fprintf(stderr, "Error on reception");
                    exit(EXIT_FAILURE);
                } else if (recieved_bytes == 0) {
                    fprintf(stdout, "Dropped Connection\n");
                    exit(EXIT_FAILURE);
                } else if(recieved_bytes == FULLHANDSHAKELENGTH) {
                    // peer sent a handshake
                    // gotta parse the handshake, verify, send back own handshake

                    char *peer_handshake = malloc(FULLHANDSHAKELENGTH); // buffer to store
                    int test_failed_status;
                    memcpy(peer_handshake, buffer, 68);
                    test_failed_status = verify_handshake(peer_handshake, file_sha); // 0 if the handshake was verified FIX THIS B/C ?!?!?!
                    if (test_failed_status) {
                        // handshake was bad (failed test)
                        // so close this connection
                        fprintf(stderr, "Error on handshake ---> %d\n", test_failed_status);
                        close(connect_d);
                        exit(EXIT_FAILURE);
                    } else {
                        // their handshake was legit
                        memset(buffer, 0, sizeof(buffer));
                        free(peer_handshake);

                        // so send back your handshake
                        if (say(client_socket, own_handshake) != -1) {
                            // and then send your bitfield
                            say(client_socket, bitfield_of_current_pieces);
                        }
                    }
                    
        
                } else {
                    // they sent a message from PWP
                    // gotta respond accordingly

                }  


            // deal with other communications:
            // you are acting as client, peers are servers
            for (int i = 1; i < peers_to_connect_to; i++) {
                if (fds[i].revents & POLLIN) {

                    int bytes_received = read_in(fds[i].fd, buffer, BUFSIZ);
                    if (bytes_received == EOF) {
                        error("Can't read from server");
                    } else if (bytes_received == 0) {
                        fprintf(stdout, "Dropped Connection\n");
                        exit(EXIT_FAILURE);
                    } else if (bytes_received == FULLHANDSHAKELENGTH) {
                        // yay they responded to my handshake!

                        // verify their handshake

                        // if they're verified to be okay, also read in their bitfield
                        // figure out how to respond
                    } else {
                        // they sent you a PWP
                        char *bufPtr = &buffer;
                        int *msgLength = malloc(4);
                        char *msgID = malloc(1);

                        char * msg1 = get_next_msg(bufPtr, msgID, msgSize);
                        bufPtr += *msgSize;
                        switch(*msgID) {
                            case(CHOKE):
                                connections[i].ownChoked = 1;
                                printf("I am choked\n");
                                break;
                            case(UNCHOKE):
                                connections[i].ownChoked = 0;
                                printf("I am unchoked\n");
                                break;
                            case(INTERESTED):
                                connections[i].peerInterested = 1;
                                printf("Peer is interested\n");            
                                break;
                            case(UNINTERESTED):
                                connections[i].peerInterested = 0;
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
                                memcpy(peerBitfield, msg1+5, total_pieces);
                                printf("bitfield tho\n");
                                break;
                            case(REQUEST):

                                // Some kind of way to put on queue this send command. 
                                printf("request tho\n");
                                break;              
                            case(PIECE):
                                // Insert saving files code here lol. + Parsing the thing + how to deal 
                                // with blocks within a piece?
                                write_piece(int fd, int piece_num, piece_size_bytes, char *buffer);

                                printf("piece tho\n");
                                break;
                            case(CANCEL):
                                // Need to cancel the queue reponse. 

                                printf("cancel tho\n");
                                break;
                            default:
                            ;
                        }
                        recieved_bytes -= *msgSize;
                        free(msg1);          
                    }








                    int bytes_received = recv(fds[i].fd, buffer, BUFSIZ, 0);

                    char *bufPtr = &buffer;
                    int *msgLength = malloc(4);
                    char *msgID = malloc(1);
                    // buffer[bytes_received] = '\0';
                    while (bytes_received) {
                        if (bytes_received == EOF) {
                            error("Can't read from server");
                        } else if (bytes_received == 0) {
                            fprintf(stdout, "Dropped Connection\n");
                            exit(EXIT_FAILURE);
                        }

                    }






                }
            }


        }




    }



    return 0;
}
