all: client.out tracker.out

client.out: client.c
	gcc -o client.out client.c

tracker.out: tracker2.c
	gcc -o tracker.out tracker2.c -pthread

clean: 
	rm client.out tracker.out