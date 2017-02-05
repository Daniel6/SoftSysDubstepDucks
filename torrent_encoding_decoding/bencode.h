/*
 * C implementation of a bencode decoder.
 * This is the format defined by BitTorrent:
 *  http://wiki.theory.org/BitTorrentSpecification#bencoding
 *
 * The only external requirements are a few [standard] function calls and
 * the long long type.  Any sane system should provide all of these things.
 *
 * This is released into the public domain.
 * Written by Mike Frysinger <vapier@gmail.com>.
 *
 *
 * Edited for CS43 @ Swarthmore College by Adam Aviv
 */

/* USAGE:
 *  - pass the string full of the bencoded data to be_decode()
 *  - parse the resulting tree however you like
 *  - call be_free() on the tree to release resources
 */

#ifndef _BENCODE_H
#define _BENCODE_H
#define FILE_NAME_MAX 1024

/*enumerate for the different types of ben_node*/
typedef enum {
	BE_STR,
	BE_INT,
	BE_LIST,
	BE_DICT,
} be_type;

/*predefined for compiler checks*/
struct be_dict;
struct be_node;

/*
 * XXX: the "val" field of be_dict and be_node can be confusing ...
 */

typedef struct be_dict {
  char *key; //key of a dict
  struct be_node *val; //val of a dict
} be_dict;

typedef struct be_node {
  be_type type; //type of the node, e.g., a string or a list
  union { //node can store all of these types
    char *s; // a stirng
    long long i; // a long long integer
    struct be_node **l; //a pointer to an array of be_nodes representing a list
    struct be_dict *d; //a dictionary 
  } val; //this union is stored in val
} be_node;

//holds information about a torrent file
typedef struct {
  char announce[FILE_NAME_MAX]; //url of tracker
  char name[FILE_NAME_MAX]; //name of file
  int piece_length; //number of bytes in each piece
  int length; //length of the file in bytes
  int num_pieces; //number of pieces, computed based on above two values

  //pointer to 20 byte data buffers of the sha1sum of each of the pieces:
  char ** piece_hashes; 
} bt_info_t;

long long be_str_len(be_node *node);

be_node *be_decode(const char *bencode);
be_node *be_decoden(const char *bencode, long long bencode_len);
void be_free(be_node *node);

//dump out the be_node encoding starting from the top
void be_dump(be_node *node);

be_node * load_be_node(char * torrent_file);
#endif
