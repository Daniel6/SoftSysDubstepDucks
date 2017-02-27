all: client tracker tests

client: torrent_client.c
	gcc -o torrent_client.out torrent_client.c -std=gnu99

tracker: tracker_serv.c
	gcc -o tracker_serv.out tracker_serv.c

tests: tracker_tests.c
	gcc -o tracker_tests.out tracker_tests.c
	./tracker_tests.out

clean: 
	rm torrent_client.out tracker_serv.out tracker_tests.out