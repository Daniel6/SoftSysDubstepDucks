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

int fd_write;

/*
void *set_bitfield(int *fd, char *file_name, char *bitfield, int num_pieces, int piece_len, char *piece_shas) {
	// check if file exists
	if (access(file_name, F_OK) != -1) {
	    // file exists
	    for (int i = 0; i < num_pieces; i++) {
	    	// save the piece
			char piece[piece_len] = get_piece(fd, i, piece_len);
			unsigned char hash[SHA_DIGEST_LENGTH];
			SHA1(piece, piece_len, hash);
			char expected_piece_sha[SHA_DIGEST_LENGTH];
			memcpy(expected_piece_sha, &piece_shas[i * (piece_len + 1)], piece_len);
			if (strcmp(hash, piece_shas[i])) {
				// add to bitfield
			}
		}

	} else {
		// file doesn't exist
		// open up file for reading and writing
		if ((fd = open(file_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR)) < 0) {
		        error("Error opening file to torrent");
		}
	}
}
*/
/*
int verify_piece(unsigned char *piece, unsigned char *expected_sha, int piece_length) {
	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1(piece, piece_length, hash);
	return memcmp(hash, expected_sha, SHA_DIGEST_LENGTH);
}
*/
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

int main(int argc, char *argv[]) {
	int fd;
	if ((fd = open("test.txt", O_RDONLY)) < -1) {
		// error opening file
		return 1;
	}
	
	int fd_write;
	if ((fd_write = open("write.txt", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR)) < 0) {
		// error opening write file
		return 1;
	}
	
	char* piece = get_piece(fd, 1, 5);
	printf("%s\n", piece);
	write_piece(fd_write, 1, 5, piece);

	/*
	unsigned char expect[SHA_DIGEST_LENGTH];
	unsigned char *hi = "hi";
	SHA1(hi, 2, expect);
	int i = verify_piece(hi, expect, 2);
	printf("%i", i);*/
	return 0;
}
