/*
 * Determine which piece to ask for after establishing
 * connection with an IP.
 */

/*
 * Pieces is an int array, index represents piece number
 * Element can be 
 */

#include <stdio.h>
#include <stdlib.h>

#define NOT_REQUESTED = 0
#define REQUESTED = 1
#define HAVE = 2

typedef struct node {
    int val;
    struct node *next;
} Node;


struct connection_info{
	char ownInterested;
	char ownChoked;
	char peerInterested;
	char peerChoked;
	char * peerBitfield;
	int sent_request;
};

int num_pieces = 5;

int piece_statuses[] = {0, 1, 2, 0, 1};
/* Makes a new Linked List node structure.
 *  
 * val: value to store in the node.
 * next: pointer to the next node
 * 
 * returns: pointer to a new node
 */
Node *make_node(int val, Node *next) {
	Node *node = malloc(sizeof(Node));
    node->val = val;
    node->next = next;
    return node;
}

/*
 * Returns the value of a Node.
 * 
 * node: node to get value from
 *
 * returns: int that is the value of node
 */
int get_val(Node *node) {
	return node->val;
}

/*
 * Given a peer's bitfield and an array of client's (your) current piece statuses,
 * return the first piece the peer has that you have not requested.
 *
 * peer_bitfield: array containing bits representing if the peer has a piece
 * pieces: client's current request/have statuses of pieces
 *
 * returns: piece number that should be requested from peer
 */
int get_piece(char *peer_bitfield, int piece_statuses[], int num_pieces) {
	for (int i = 0; i < num_pieces; i++) {
		if (!piece_statuses[i] && peer_bitfield[i]) {
			// if client has not yet requested the piece
			// and the peer has the piece, request that
			// piece
			return i;
		}
	}

	return -1;
}


/*
 * Determine what pieces should be requested from which peers
 *
 * connections: a list of peer connections
 * num_connections: total number of connections
 * pieces: array of current piece statuses
 * num_pieces: total number of pieces:
 *
 * returns: head of a Linked List, where value is piece number
 * 	        and the order of the nodes is the order of the peers
 * 	        in connections
 */
Node *assign_pieces(struct connection_info *connections, int num_connections, int *piece_statuses[], int num_pieces) {
	Node *head = make_node(-1, NULL);
	Node *curr = head;

	// assign a piece to every peer connections
	for (int i = 0; i < num_connections; i++) {
		// set value of ith node to piece number that 
		// should be requested from ith peer connection
		curr->val = get_piece(connections[i].peerBitfield, *piece_statuses, num_pieces);

		if (i != num_connections - 1) {
			// don't make a new next node if
			// this is the last connection
			curr->next = make_node(-1, NULL);
			*curr = *(curr->next);
		}
	}

	return head;
}

int main() {
	return 0;
}






