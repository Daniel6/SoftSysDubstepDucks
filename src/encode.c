#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#include "bencode.h"

#define BUFFSIZE 10000
#define PIECELENGTH 256

//	Run this file with two arguments: the file you wish to create a torrent for, and the ip of the tracker server for your torrent file

// Compile with:
// gcc encode.c bencode.h -l ssl -l crypto -o encode.out -g

//	Returns the bencoded format of an input string
char *encodeString(char *input){
	char *ans = malloc(80);
	snprintf(ans, 100, "%zu", strlen(input));
	strncat(ans, ":", BUFFSIZE);
	strncat(ans, input, BUFFSIZE);
	return ans;
}

//	Returns the bencoded format of an input integer as a string
char *encodeInt(int *input){
	char ans[80] = "i";
	char num[80];
	sprintf(num, "%d", input);
	strncat(ans, num, BUFFSIZE);
	strncat(ans, "e", BUFFSIZE);
	return ans;
}

//	Returns the bencoded format of an input dictionary
//	Currently takes "dict" (it's actually just a list) of already bencoded values and returns a string bencoded dict
char *encodeDict(char *input, int size){
	char ans[BUFFSIZE] = "d";
	int i;
	for(i = 0; i < size; i ++){
		strncat(ans, input[i], BUFFSIZE);
	}
	strncat(ans, "e", BUFFSIZE);
	return ans;
}

// Given a file, creates a string with info for file name, length, piece length, piece num
char *infoDict(char *filename){
	char ans[BUFFSIZE] = "d";
	char tempAns[BUFFSIZE] = "";
	FILE *file = fopen(filename, "r");
	//	Standard strings to act as keys for the bencoded dictionary
	strncat(ans, encodeString("length"), BUFFSIZE);
	fseek(file, 0, SEEK_END);
	strncat(ans, encodeInt(ftell(file)), BUFFSIZE);
	rewind(file);
	strncat(ans, encodeString("name"), BUFFSIZE);
	strncat(ans, encodeString(filename), BUFFSIZE);
	strncat(ans, encodeString("piece length"), BUFFSIZE);
	strncat(ans, encodeInt(PIECELENGTH), BUFFSIZE);
	// Each sha is 20 characters per file piece
	strncat(ans, encodeString("pieces"), BUFFSIZE);
	char buffer[PIECELENGTH];
	unsigned char sha[SHA_DIGEST_LENGTH];
	char pieceSHAs[BUFFSIZE] = "";
	//	Goes through the file for a given piece size and hashes each file piece
	while(fread(buffer, 1, PIECELENGTH, file) == PIECELENGTH){
		SHA1(buffer, strlen(buffer), sha);
		strncat(pieceSHAs, sha, BUFFSIZE);
	}
	// Hash one last time after while loop has ended to clear the buffer
	SHA1(buffer, strlen(buffer), sha);
	strncat(pieceSHAs, sha, BUFFSIZE);

	//	Add all file piece hashes to the return string
	strncat(ans, encodeString(pieceSHAs), BUFFSIZE);
	strncat(ans, "e", BUFFSIZE);
	fclose(file);
	return ans;
}

//	Given a file name and address to act as the tracker server, creates a torrent file for the original file and writes it to disk
char *encodeFile(char *target, char *address){
	char fileContents = NULL;
	FILE *f = fopen(target, "r");
	char output[100] = "";
	strcpy(output, target);
	strncat(output, ".torrent", BUFFSIZE);
	FILE *torrentFile = fopen(output, "w");
	//	A "dictionary" for storing bencoded values which is really just an array, but treat as keys in even indices mapping to the next consecutive index
	char dict[4][80];
	int dictLength = 4;
	char ans[BUFFSIZE] = "";
	char *tempAns = malloc(80);
	char *info = malloc(BUFFSIZE);
	//	Standard strings to act as keys for the bencoded dictionary
	strcpy(dict[0], encodeString("announce"));
	strcpy(dict[1], encodeString(address));
	strcpy(dict[2], encodeString("info"));
	//	Info dictionary for specific meta file info
	strcpy(dict[3], infoDict(target));
	char finalAns[BUFFSIZE] = "d";
	//	Get all info from the "dictionary", and concatenate into a string
	int i;
	for(i = 0; i < dictLength; i++){
		strncat(finalAns, dict[i], BUFFSIZE);
	}
	strncat(finalAns, "e", BUFFSIZE);
	//	Print the string to file
	fprintf(torrentFile, "%s", finalAns);
	fclose(f);
	fclose(torrentFile);
	return finalAns;
}

//	Creates a SHA1 for a file
char *fileSHA(char *target){
	FILE *file = fopen(target, "r");
	fseek(file, 0, SEEK_END);
	//	Obtain file length
	long fileLength = ftell(file);
	rewind(file);
	char buffer[BUFFSIZE] = "";
	//	Write all file contents into a buffer
	fread(buffer, fileLength, 1, file);
	unsigned char ans[SHA_DIGEST_LENGTH] = "";
	//	Obtain the SHA1 of the file contents
	SHA1(buffer, fileLength, ans);
	fclose(file);
	return ans;
}

int main(int argc, char*argv[]){
	printf("Original: Hello world\n");
	printf("Bencoded String: %s\n\n", encodeString("Hello world"));

	printf("Original: 12345\n");
	printf("Bencoded Int: %s\n\n", encodeInt(12345));

	char *torrentFile = malloc(BUFFSIZE);
	torrentFile = encodeFile(argv[1], argv[2]);
	printf("Torrent file of your input file: \n%s\n\n", torrentFile);

	printf("SHA1 of torrent file: \n%s\n", fileSHA(argv[1]));
}