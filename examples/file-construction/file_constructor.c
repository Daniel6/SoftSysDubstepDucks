/*
 * Uses a .torrent metainfo file to break up a file into pieces
 * and to reconstruct pieces into a file.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include "file_constructor.h"




void set_initial_bitfield(int *fd, char *file_name, char *bitfield, int num_pieces, int piece_len, char *piece_shas) {
	// check if file exists
	int bitfield_len = num_pieces / 8; // 8 is size of byte
	memset(bitfield, 0, bitfield_len);
	if (access(file_name, F_OK) != -1) {
	    // file exists
	    char expected_piece_sha[SHA_DIGEST_LENGTH];
	    for (int i = 0; i < num_pieces; i++) {
	    	// save the piece
			char piece[piece_len] = get_piece(fd, i, piece_len);
			memcpy(expected_piece_sha, piece_shas+(i*SHA_DIGEST_LENGTH), SHA_DIGEST_LENGTH);
	
			if (verify_piece(piece, expected_piece_sha, piece_len)) {
				// add to bitfield
				int byte_index = i/8;
				int bit_index = i%8;
				bitfield[byte_index]|= 1<<bit_index;
			}
		}

	} else {
		// file doesn't exist
		// open up file for reading and writing
		if ((fd = open(file_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR)) < 0) {
		        fprintf(stderr, "Error opening file to torrent");
		}
	}
}


/*
 * Verifies if string has an expected SHA has.
 *
 * piece[]: the string to verify
 * expected_sha[]: the expected SHA has
 * piece_length: the length of piece
 *
 * returns: 0 if the piece is not correct, 1 if the piece is verified
 */
int verify_piece(char piece[], unsigned char expected_sha[], int piece_length) {
	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1(piece, piece_length, hash);
	return !memcmp(hash, expected_sha, SHA_DIGEST_LENGTH);
}

/*
 * Get a piece from a file
 *
 * fd: file descriptor
 * piece_num: which piece to get from the file (0 indexed)
 * piece_len: length of pieces
 *
 * returns: char array of the piece data
 */
char *get_piece(int fd, int piece_num, int piece_len) {

	char buffer[piece_len];
	// seek to correct position in the file
	if (lseek(fd, piece_num * piece_len, SEEK_SET) < 0){
		// error seeking
		exit(-1);
	}

	// read from file into buffer
	if (read(fd, buffer, piece_len) != piece_len) {
		exit(-1);
	}

	printf("%s\n", buffer);
	//char *piece = (char *) mallloc(sizeof(char) * piece_len);
	return strcpy(malloc(sizeof(char) * piece_len), buffer);
	//return piece;
}

/*
 * Write to a file from a buffer
 *
 * fd: file descriptor
 * piece_num: what number piece will be written
 * piece_len: length of the piece being written
 * buffer: char array of what will be written
 */
void write_piece(int fd, int piece_num, int piece_len, char *buffer) {
	// seek to correct position in the file
	if (lseek(fd, piece_num * piece_len, SEEK_SET) < 0){
		// error seeking
		exit(-1);
		return;
	}

	if (write(fd, strcat(buffer, "\n"), piece_len) != piece_len) {
		printf("Success writing to the file!!!!!\n");
		return;
	}
}
