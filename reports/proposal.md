##Creating a BitTorrent Client in C
Daniel Bishop, Andrew Pan, Thuc Tran, Jordan Van Duyne

###Project Goal

We are attempting to create our own peer to peer file transfer network similar to how popular torrenting systems like BitTorrent operate today. Peer to peer file transfer provides several advantages over traditional FTP. For instance, the load of transferring the file is distributed over many clients instead of loaded onto one server. The network is also resistant to outages, as even if a few peers go offline, the others can pick up the slack.

Our minimum viable product is a client to server file transport system that follows the BitTorrent Protocol as specified here. This is achievable given the chapters from Head First C regarding simple networking, and we are confident that we will be able to create this file transport system within the timeframe for this project.

Improvements that extend this MVP would be implementing a protocol for requesting specific parts of a file at a time, the use of a tracker server to coordinate peer to peer connections, the use of multiple peers to seed download data and provide redundancies, error-checking and re-requests. 

A perpendicular improvement would be to also implement a P2P Chatroom so that you can send text to everybody else on the network. 


###Project Backlog

[Our Trello Board](https://trello.com/b/vVNhx3WF/softsysdubstepducks)

###Project System Map

![System Interaction Map](https://github.com/Daniel6/SoftSysDubstepDucks/blob/master/images/torrent_system_plan.jpg)

System interaction map for BitTorrent client

![Client Flowchart](https://github.com/Daniel6/SoftSysDubstepDucks/blob/master/images/client_flowchart.jpg)

Flowchart of actions for client to perform

###Roadblocks

1. Olin network blocks torrent traffic so our network must be local using a network switch.  
Allen and Ben have provided us with a network switch so that we may do this.
2. Our individual computer’s firewall may prevent us from connecting to each other easily.  
So far we have been able to communicate each other, but we will keep in mind that we may need to forward some ports later.
3. Unfamiliarity with content (Breaking up files, bencoding, SHA1 hashing, networking mixups, etc).  
We have yet to do this, but are consulting lots of resources about the process.

###Annotated Bibliography

[https://github.com/bschmaltz/c-p2p-chat](https://github.com/bschmaltz/c-p2p-chat)

- Peer-to-peer chat client written in C. Allows for grouping of clients. Uses tracker-clients framework that we can repurpose to set up file distribution. Easy to read.

[http://beej.us/guide/bgnet/output/html/multipage/index.html](http://beej.us/guide/bgnet/output/html/multipage/index.html)

- Widely regarded text for socket and networking programming. Provides explanation on the basics of working with sockets and networking, then details the protocols of IPv6.

[http://www.kristenwidman.com/blog/33/how-to-write-a-bittorrent-client-part-1/](http://www.kristenwidman.com/blog/33/how-to-write-a-bittorrent-client-part-1/)

- This source lists the basic steps that are involved in writing a BitTorrent client.  It discusses from a high-level how various aspects of the client should be implemented and how peers can communicate.

[http://jonas.nitro.dk/bittorrent/bittorrent-rfc.html](http://jonas.nitro.dk/bittorrent/bittorrent-rfc.html)

- This document provides a detailed description of the BitTorrent Protocol version 1.0 referred to as "BTP/1.0". We modeled our own protocol after the standard specifications detailed here.

[https://www.cs.swarthmore.edu/~aviv/classes/f12/cs43/labs/lab5/lab5.pdf](https://www.cs.swarthmore.edu/~aviv/classes/f12/cs43/labs/lab5/lab5.pdf)

- This PDF is a description of an assignment at Swarthmore to write a BitTorrent client. This document provides some details about how the BitTorrent Protocol is specified and provides information about which aspects of BitTorrent are extraneous and can be a stretch goal for our project.

[http://stackoverflow.com/questions/11952898/c-send-and-receive-file](http://stackoverflow.com/questions/11952898/c-send-and-receive-file)

- Community example of using C system library to send and receive files. Basic buffering for transfer, worth looking into for example on how to do file transfers over network.

[http://www.bittorrent.org/beps/bep_0000.html](http://www.bittorrent.org/beps/bep_0000.html)

- List of documents about BitTorrent Protocol. These documents will be helpful for understanding the constraints that are required for BTP, and the intention and ideas behind the project.  

[https://allenkim67.github.io/bittorrent/2016/05/04/how-to-make-your-own-bittorrent-client.html](https://allenkim67.github.io/bittorrent/2016/05/04/how-to-make-your-own-bittorrent-client.html)

- This document is a description of how to write a torrent client in node.js. It can serve as a guide for determining the kind of mentality and process needed for p2p file transfer. In particular, we will be focusing on using this to better understand the protocol’s process and the mechanics that underlie how to properly send and receive files. 
