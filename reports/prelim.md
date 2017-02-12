## Preliminary Report 
##### 2/13/2017

### Project Goal
The goal of this project is to create a peer to peer file transfer network. Basic features of this network would be the ability to add and drop clients at will, and transfer files from multiple peers to a single peer faster than a traditional FTP request. Additional features of this project may include enhanced error detection or redundant file transfers to accommodate peers unexpectedly going offline. These features would bring us closer to the functionality of the BitTorrent system.

### Trello Cards
#### Sprint 1
Readings completed individually:
* HFC 1
* HFC 2
* HFC 3
* HFC 11  

Project related work:
* Define network protocols
  * [protocol](https://github.com/Daniel6/SoftSysDubstepDucks/blob/master/protocol.md)
* Implement file transfer
* Establish client-server connection between two computers
* Develop handshake and verification  

#### Sprint 2
Readings completed individually:
* TOS 1
* TOS 2
* TOS 3
* TOS 4  

Project related work:
* Set up torrent file parsing for the partition of files
* Deconstruction and reconstruction of files
* Piece-peer request assigner
* Set up tracker server and client

### Preliminary Results

The tracker server and client have established basic communications. The client is able to register itself on the tracker, and also receive a list of all other registered peers. This is enough to move forwards with although it could benefit from more robustness.  
![example](../images/tracker-example.jpg)
