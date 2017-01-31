#include "btp.h"




int main(int argc, char ** argv)
{
	char * whatever = construct_cancel_message(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	print_hex_memory(whatever, 17);

	return 0;
}
