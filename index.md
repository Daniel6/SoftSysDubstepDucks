# BitTorrent Client in C
Team Dubstep Ducks

Project 1 for the Software Systems class

## Abstract
The team attempted to create a peer to peer file transfer system modelled on the BitTorrent protocol. Peer to peer file transfer systems have several advantages over traditional server to client transfers. They are very resilient to node outages, as each client is capable of transferring the whole file if needed. They are faster, because the amount of data each client has to transfer is much less than the overall size of the file, so the transfer rate is mostly limited by the download capacity of the client. However, to enable this peer to peer behaviour, the clients must share a set of metadata in common, which contains information such as the size of the file and the IP addresses of other peers. 

This system includes a tracker server and torrent client distribution. 

The purpose of the tracker server is to accept torrent client communications, and relay the IP addresses of connected clients. This is done so that each torrent client may know the addresses of its peers.

The torrent client is responsible for the partitioning and transfer of the desired file between itself and the other clients. Depending on the number of available clients, each client may only be responsible for trasnferring a segment of the file.
