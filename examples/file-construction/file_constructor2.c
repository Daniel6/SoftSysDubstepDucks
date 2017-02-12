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

FILE * f_read;
FILE * f_write;

char *get_piece(int piece_num, int piece_len) {

	char buffer[piece_len];
	// seek to correct position in the file
	if (fseek(f_read, piece_num * piece_len, SEEK_SET) != 0){
		// error seeking
		puts("err fseek read");
		exit(-1);
	}

	// read from file into buffer
	if (fread(buffer, 1, piece_len + 1, f_read) != piece_len+1) {
		puts("err fread");
		exit(-1);
	}

	printf("%s\n", buffer);
	//char *piece = (char *) mallloc(sizeof(char) * piece_len);
	return strcpy(malloc(sizeof(char) * piece_len), buffer);
	//return piece;
}

void write_piece(int piece_num, int piece_len, char *buffer) {
	// seek to correct position in the file
	if (fseek(f_write, piece_num * piece_len, SEEK_SET) != 0){
		// error seeking
		puts("err fseek write");
		return;
	}

	// if (fwrite(buffer, 1, piece_len, f_write) != piece_len) {
	// 	puts("err fwrite");
	// 	return;
	// }

	fprintf(f_write, "%s\n", buffer);
	
}

int main(int argc, char *argv[]) {
	if ((f_read = fopen("test.txt", "r")) == NULL) {
		// error opening file
		puts("cannot open test");
		return 1;
	}
	
	if ((f_write = fopen("write.txt", "w")) == NULL) {
		// error opening write file
		puts("cannot open write");
		return 1;
	}
	
	char* piece = get_piece(1, 5);
	printf("%s\n", piece);
	write_piece(1, 5, piece);
	return 0;
}
