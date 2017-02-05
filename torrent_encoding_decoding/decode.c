#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bencode.h"


bt_info_t * decodeFile(char target[]){	// reads information from a torrent file and returns a bt_info_t with relevent torrent information
	be_node * node = load_be_node(target);	// creates a be_node from the torrent file
	bt_info_t * out = (bt_info_t *) malloc(1000);

	size_t i,j;
	be_node * currnode;
	be_node * infonode;

	// populate the bt_info_t object with information from the be_node
	for (i = 0; node->val.d[i].val; ++i) {
	    currnode = node -> val.d[i].val;
	    if(strcmp(node->val.d[i].key,"announce") == 0){
	      	strcpy(out->announce,currnode->val.s);
	    }
	    else if(strcmp(node->val.d[i].key,"info") == 0){
	      	for (j = 0; currnode->val.d[j].val; ++j) {
	        	infonode = currnode -> val.d[j].val;
	        	if(strcmp(currnode->val.d[j].key,"name") == 0)
	          		strcpy(out->name,infonode->val.s);
	        	else if(strcmp(currnode->val.d[j].key,"length") == 0)
	          		out->length = infonode->val.i;
	        	else if(strcmp(currnode->val.d[j].key,"piece length") == 0){
	          		out->piece_length = infonode->val.i;
			        // once we have the total length and piece length
			        // we can find the number of pieces req'd
			        out -> num_pieces = (out->length)/(out-> piece_length);
			        // handle partial pieces
		          	if (out->length%out->piece_length > 0)
		            	out -> num_pieces++; 
	        	}
		        else if(strcmp(currnode->val.d[j].key,"pieces") == 0)
		        {
		        	// here we malloc the pieces_hash buffer based
		        	// on the number of pieces we have
		        	out -> piece_hashes = (char **)malloc(out->num_pieces*sizeof(char *));
		        	int k;
		        	for(k=0;k<out->num_pieces;k++){//malloc a sha1 per piece
		            	out->piece_hashes[k] = (char *)malloc(20);
		            	memcpy(out->piece_hashes[k],infonode->val.s + 20*k,20);
		          	}
		        }
	      	}
	    }
  	}
	return out;
}

int main(int argc, char*argv[]){
	// char *target = malloc(80);
	// printf("Enter target file name: ");
	// fgets(target, 80, stdin);
	char target[] = "62-Q2.mp3.torrent";
	char length[80];
	bt_info_t *ans =  decodeFile(target);
	sprintf(length, "%d", ans->length);
	printf(length);
	printf("\n");
}