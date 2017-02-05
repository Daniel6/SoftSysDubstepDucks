#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bencode.h"

char *encodeFile(char *target){
	char *fileContents = NULL;
	FILE *f = fopen(target, "r");
	long long length;
	fread(fileContents, 1, length, f);
	
}

int main(int argc, char*argv[]){

}