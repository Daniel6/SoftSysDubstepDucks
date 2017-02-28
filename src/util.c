#include "util.h"


/*
Configures a socket for listening for impending communications

*/

int configure_socket() {
    int listening_socket = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in listening_addr;
    listening_addr.sin_family = PF_INET;
    listening_addr.sin_port = (in_port_t)htons(30000);
    listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //Set option for reuse. 
    int reuse = 1;
    if(setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1) {
        fprintf(stderr, "Can't set the 'reuse' option on the socket.\n");
        exit(1);
    }
    
    //Binds
    if(bind(listening_socket, (struct sockaddr *)&listening_addr, sizeof(listening_addr)) == -1) {
        fprintf(stderr, "Can't bind to socket.\n");
        exit(1);
    }

    //Listens
    if ((listen(listening_socket, 10)) == -1) {
        fprintf(stderr, "Error on listen --> %s", strerror(errno));
        exit(1);
    }

    return listening_socket;
}

/*
 * Set up and return a socket for communicating with other clients.
 *
 * remote_addr: address to connect to
 * server_address: address of the server
 * port_number: port of the connection
 */
int client_socket_wrapper(struct sockaddr_in * remote_addr, char * server_address, int port_number){
  /* Zeroing remote_addr struct */
    memset(remote_addr, 0, sizeof(struct sockaddr_in));
  /* Construct remote_addr struct */
    remote_addr->sin_family = AF_INET;
    inet_pton(AF_INET, server_address, &(remote_addr->sin_addr));
    remote_addr->sin_port = htons(port_number);
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  /* Create client socket */
    if (client_socket == -1) {
        fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return client_socket;
}


// Useful piece of support code when debugging some of this stuff
// Context was using it as a way to dump hex of char arrays easily.
void print_hex_memory(void *mem, int size) {
  int i;
  //Iterates over each byte in mem. 
  unsigned char *p = (unsigned char *)mem;
  for (i=0;i<size;i++) {
    printf("0x%02x ", p[i]);
  }
  printf("\n");
}

// Support code to print the binary representation of some memory.
// source: http://stackoverflow.com/questions/35364772/how-to-print-memory-bits-in-c
void print_bits ( void* buf, size_t size_in_bytes )
{
    char* ptr = (char*)buf;
    //Iterate over each bit in memory, first by byte, then by bit. 
    for (size_t i = 0; i < size_in_bytes; i++) {
        for (short j = 7; j >= 0; j--) {
            printf("%d", (ptr[i] >> j) & 1);
        }
        printf(" ");
    }
    printf("\n");
}

//Count bits in your byte
//b: Byte that you want to count the number of set bits in.
//Returns: # of set bits. 
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

//Count bits in your bitfield. 
//Bitfield: The bitfield of interest
//Bitfield Length: The size of the bitfield in bytes.
//Returns: Number of bits set in bitfield. 
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

