gcc transfer_client.c -Wall -o client
gcc transfer_server.c -Wall -o server

./server &
./client