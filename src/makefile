all: torrent tracker tests encode

torrent: torrent.c
	clang-3.8 -o torrent.out torrent.c file_constructor.c util.c -lssl -lcrypto -std=gnu99

tracker: tracker_serv.c
	gcc -o tracker_serv.out tracker_serv.c

tests: tracker_tests.c
	gcc -o tracker_tests.out tracker_tests.c
	./tracker_tests.out

encode: encode.c
	gcc encode.c bencode.h -l ssl -l crypto -o encode.out

clean: 
	rm torrent_client.out tracker_serv.out tracker_tests.out torrent.out