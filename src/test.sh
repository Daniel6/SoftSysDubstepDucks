gcc transfer_client.c -o client
gcc transfer_server.c -o server

./server &
./client