#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <openssl/sha.h>


int verify_piece(char piece[], unsigned char expected_sha[], int piece_length);
char *get_piece(int fd, int piece_num, int piece_len);
void write_piece(int fd, int piece_num, int piece_len, char *buffer);
char*  set_initial_bitfield(int *fd, char *file_name, int num_pieces, int piece_len, char **piece_shas);
