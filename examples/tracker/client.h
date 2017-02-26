#ifndef UTIL_H_
#define UTIL_H_

#include "client.c"

void execTorrent(int tracker_socket);
void joinTracker(int tracker_socket);
void requestPeers(int tracker_socket);

#endif