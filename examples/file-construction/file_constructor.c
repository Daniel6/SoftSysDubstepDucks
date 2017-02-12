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

int fd_write;

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

}
