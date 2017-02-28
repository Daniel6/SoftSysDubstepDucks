#ifndef UTIL_H_ /* Include Guard*/ 
#define UTIL_H_

//Standard Libraries. 
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <openssl/sha.h>
#include <byteswap.h>
#include <poll.h>



typedef struct node {
    int val;
    struct node *next;
} Node;


int configure_socket();
void print_bits ( void* buf, size_t size_in_bytes );
int client_socket_wrapper(struct sockaddr_in * remote_addr, char * server_address, int port_number);
void print_hex_memory(void *mem, int size);
int count_char_bits(char b);
int count_bitfield_bits(char * bitfield, int bitfieldLen);



#endif //UTIL_H_