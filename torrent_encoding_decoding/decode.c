#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bencode.c"
#include "bencode.h"


int main(int argc, char*argv[]){
	// char *target = malloc(80);
	// printf("Enter target file name: ");
	// fgets(target, 80, stdin);
	char target[] = "moby_dick.txt.torrent";
	bt_info_t *ans =  decodeFile(target);
	printf("Original file: %s\n", target);
	// sprintf(length, "%d", ans->length);
	printf("Announce: ");
	printf(ans->announce);
	printf("\n");
	printf("Name: ");
	printf(ans->name);
	printf("\n");
	printf("Piece Length: ");
	char piecelength[80];
	sprintf(piecelength, "%d", ans->piece_length);
	// printf(ans->piece_length);
	printf(piecelength);
	printf("\n");
	printf("Length: ");
	char length[80];
	sprintf(length, "%d", ans->length);
	// printf(ans->length);
	printf(length);
	printf("\n");
	printf("Number of Pieces: ");
	char pieces[80];
	sprintf(pieces, "%d", ans->num_pieces);
	// printf(ans->num_pieces);
	printf(pieces);
	printf("\n");
}