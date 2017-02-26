#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#include "bencode.h"

#define BUFFSIZE 10000

//maybe something about enum attached to a struct to determine the variable type for arbitrary bencoding
//include file sha, going to have to edit decode too
//gcc shatest.c -l ssl -l crypto -o shatest.out

char *encodeString(char *input, char *ansqwr){
	char *ans = malloc(80);
	snprintf(ans, 100, "%zu", strlen(input));
	// sprintf(ans, "%d", strlen(input));
	strncat(ans, ":", BUFFSIZE);
	strncat(ans, input, BUFFSIZE);
	return ans;
}

char *encodeInt(int *input, char *answer){
	char ans[80] = "i";
	char num[80];
	// itoa(input, num, 10);
	sprintf(num, "%d", input);
	strncat(ans, num, BUFFSIZE);
	strncat(ans, "e", BUFFSIZE);
	return ans;
}

char *encodeListString(char *input, char *answer){
	char ans[80] = "l";
	int i;
	for(i = 0; i < sizeof(input); i++){
		char *encodePiece;
		strncat(ans, encodeString(input[i], encodePiece), BUFFSIZE);
	}
	strncat(ans, "e", BUFFSIZE);
	return ans;
}

char *encodeListInt(char *input, char *answer){
	char ans[80] = "l";
	int i;
	for(i = 0; i < sizeof(input); i++){
		char *encodePiece;
		strncat(ans, encodeInt(input[i], encodePiece), BUFFSIZE);
	}
	strncat(ans, "e", BUFFSIZE);
	return ans;
}

char *encodeListList(char *input, char *answer){		// need a way to determine the type of data in the list, currently not working
	char ans[80] = "l";
	int i;
	for(i = 0; i < sizeof(input); i++){
		char *encodePiece;
		strncat(ans, encodeString(input[i], encodePiece), BUFFSIZE);
	}
	strncat(ans, "e", BUFFSIZE);
	return ans;
}

char *encodeListDict(char *input, char *answer){		// need a way to determine the type of data in the dict, currently not working
	char ans[80] = "l";
	int i;
	for(i = 0; i < sizeof(input); i++){
		char *encodePiece;
		strncat(ans, encodeString(input[i], encodePiece), BUFFSIZE);
	}
	strncat(ans, "e", BUFFSIZE);
	return ans;
}

char *encodeDict(char *input, int size, char *answer){	//currently takes "dict" (it's actually just a list) of already bencoded values and returns a string bencoded dict
	char ans[BUFFSIZE] = "d";
	int i;
	printf("hi11\n");
	fflush(stdout);
	for(i = 0; i < size; i ++){
		printf(input[i]);
		fflush(stdout);
		printf("hi111\n");
		fflush(stdout);
		strncat(ans, input[i], BUFFSIZE);
		printf("hilots\n");
		fflush(stdout);
	}
	printf("hi12\n");
	fflush(stdout);
	strncat(ans, "e", BUFFSIZE);
	return ans;
}

//it's another dict for info with file name, length, piece length, piece num, md5sum of file (not actually used by bittorrent but is is torrent files)
char *infoDict(char *filename, char *answer){
	char ans[BUFFSIZE] = "d";
	char tempAns[BUFFSIZE] = "";
	FILE *file = fopen(filename, "r");
	strncat(ans, encodeString("length", tempAns), BUFFSIZE);
	fseek(file, 0, SEEK_END);
	strncat(ans, encodeInt(ftell(file), tempAns), BUFFSIZE);
	rewind(file);
	strncat(ans, encodeString("name", tempAns), BUFFSIZE);
	strncat(ans, encodeString(filename, tempAns), BUFFSIZE);
	strncat(ans, encodeString("piece length", tempAns), BUFFSIZE);
	strncat(ans, encodeInt(262144, tempAns), BUFFSIZE);

	// uhhh guess each sha is 20 characters per file piece
	strncat(ans, encodeString("pieces", tempAns), BUFFSIZE);
	// need to actually break up files and sha that stuff
	char buffer[262144];
	unsigned char sha[SHA_DIGEST_LENGTH];
	char pieceSHAs[BUFFSIZE] = "";
	while(fread(buffer, 1, 262144, file) == 262144){
		SHA1(buffer, strlen(buffer), sha);
		strncat(pieceSHAs, sha, BUFFSIZE);
	}
	// one last time after while loop has ended to clear the buffer
	SHA1(buffer, strlen(buffer), sha);
	strncat(pieceSHAs, sha, BUFFSIZE);

	strncat(ans, encodeString(pieceSHAs, tempAns), BUFFSIZE);
	strncat(ans, "e", BUFFSIZE);
	return ans;
}

void populateDict(char *dict[], char *filename, char *address){	// this is now useless, yay
	char *tempAns = malloc(80);
	dict[0] = encodeString("announce", tempAns);
	dict[1] = encodeString(address, tempAns);
	dict[2] = encodeString("info", tempAns);
	char *info = malloc(BUFFSIZE);
	dict[3] = infoDict(filename, info);
}

char *encodeFile(char *target, char *address){
	char fileContents = NULL;
	FILE *f = fopen(target, "r");
	char output[100] = "";
	strcpy(output, target);
	strncat(output, ".torrent", BUFFSIZE);
	FILE *torrentFile = fopen(output, "w");
	char dict[4][80];
	int dictLength = 4;
	char ans[BUFFSIZE] = "";
	char *tempAns = malloc(80);
	char *info = malloc(BUFFSIZE);

	strcpy(dict[0], encodeString("announce", tempAns));
	strcpy(dict[1], encodeString(address, tempAns));
	strcpy(dict[2], encodeString("info", tempAns));
	strcpy(dict[3], infoDict(target, info));
	char finalAns[BUFFSIZE] = "d";
	int i;
	for(i = 0; i < dictLength; i++){
		// printf(dict[i]);
		fflush(stdout);
		strncat(finalAns, dict[i], BUFFSIZE);
	}
	strncat(finalAns, "e", BUFFSIZE);
	fprintf(torrentFile, "%s", finalAns);

	fclose(f);
	fclose(torrentFile);
}

int main(int argc, char*argv[]){
	// encodeFile("moby_dick.txt", "127.0.0.1");
	encodeFile(argv[1], argv[2]);
}