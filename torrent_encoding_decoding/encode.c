#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bencode.h"


char *encodeFile(char *target, char *ans){
	char *fileContents = NULL;
	FILE *f = fopen(target, "r");
	long long length;
	fread(fileContents, 1, length, f);

}

char *encodeString(char *input, char *ans){
	snprintf(ans, strlen(input), "%zu", 100);
	strcat(ans, ":");
	strcat(ans, input);
	return ans;
}

char *encodeInt(int *input, char *ans){
	ans = "i";
	char num[80];
	itoa(input, num, 10);
	strcat(ans, num);
	strcat(ans, "e");
	return ans;
}

char *encodeListString(char *input, char *ans){
	ans = "l";
	int i;
	for(i = 0; i < sizeof(input); i++){
		char *encodePiece;
		strcat(ans, encodeString(input[i], encodePiece));
	}
	strcat(ans, "e");
	return ans;
}

char *encodeListInt(char *input, char *ans){
	ans = "l";
	int i;
	for(i = 0; i < sizeof(input); i++){
		char *encodePiece;
		strcat(ans, encodeInt(input[i], encodePiece));
	}
	strcat(ans, "e");
	return ans;
}

char *encodeListList(char *input, char *ans){		// need a way to determine the type of data in the list, currently not working
	ans = "l";
	int i;
	for(i = 0; i < sizeof(input); i++){
		char *encodePiece;
		strcat(ans, encodeString(input[i], encodePiece));
	}
	strcat(ans, "e");
	return ans;
}

char *encodeListDict(char *input, char *ans){		// need a way to determine the type of data in the dict, currently not working
	ans = "l";
	int i;
	for(i = 0; i < sizeof(input); i++){
		char *encodePiece;
		strcat(ans, encodeString(input[i], encodePiece));
	}
	strcat(ans, "e");
	return ans;
}

char *encodeDict(char *input, char *ans){
	
}

int main(int argc, char*argv[]){

}