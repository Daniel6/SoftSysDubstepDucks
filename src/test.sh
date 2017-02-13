clang-3.8 transfer_client.c -Wall -o client -g
clang-3.8 transfer_server.c -Wall -o server -g

./server &
./client