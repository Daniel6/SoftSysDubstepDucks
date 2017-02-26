# BitTorrent Client in C
Team Dubstep Ducks

Project 1 for the Software Systems class

## Quickstart
For setup and usage instructions see our [README](./README.md)

## Abstract
The team attempted to create a peer to peer file transfer system modelled on the BitTorrent protocol. Peer to peer file transfer systems have several advantages over traditional server to client transfers. They are very resilient to node outages, as each client is capable of transferring the whole file if needed. They are faster, because the amount of data each client has to transfer is much less than the overall size of the file, so the transfer rate is mostly limited by the download capacity of the client. However, to enable this peer to peer behaviour, the clients must share a set of metadata in common, which contains information such as the size of the file and the IP addresses of other peers. 

This system includes a tracker server and torrent client distribution. 

The purpose of the tracker server is to accept torrent client communications, and relay the IP addresses of connected clients. This is done so that each torrent client may know the addresses of its peers.

The torrent client is responsible for the partitioning and transfer of the desired file between itself and the other clients. Depending on the number of available clients, each client may only be responsible for transferring a segment of the file.

## Background
Our project resides in well-explored territory. There are quite a few torrent clients under development, and ours does nothing new compared to those. However, the inner mechanisms for these systems are not publicised as they are proprietary. Only the messaging protocols are publicly known. Thus, knowing the format in which messages should be sent, the team wanted to explore how these messages are handled by the program. 

## Implementation
We chose to implement two major facets of a peer to peer system, a centralized tracker server, and a client that would be run on many machines. The central server's purpose is to keep track of active clients, relay this list to the clients, and serve the torrent metadata file to the clients. The client's responsibility is to talk to other clients and manage file requests that it sends to other clients.

## Results
Ultimately we were unable to complete the project as originally envisioned. However, we did complete many of the subcomponents that we would integrate to complete the final product. The list of working components includes the tracker server, client handshake routine, and torrent file generating process.

This project was intended to be a learning exercise for the team to familiarize with the C language and get hands on experience. We accomplished the learning goals which we established at the creation of the project, those being to:

* learn about networking in C and how messages are handled on a low level
* learn about multiprocessing in C and managing shared resources
* learn how to write clean and readable C code

So while the project was not as functional as we had hoped, it still proved to be a valuable learning experience and was successful in allowin the team to gain in depth experience in their fields of interest.
