# Overview of BitTorrent Protocol

The following document attempts to summarize the specifications and process for the BitTorrent File Transfer protocol BTP/1.0 as given by http://jonas.nitro.dk/bittorrent/bittorrent-rfc.html . In particular, we go into detail about the structure of the MetaInfo file and the Peer-Wire Protocol side of the protocol due to us having the tracker not in scope at this time. This document will have the specifications for the format of parts such as the handshake, messages, etc.

The process of torrenting requires a metainfo file containing information about the file(s) to be torrented. This file is used to ensure the file(s) being transferred are correct. BTP/1.0 requires communication between peers of two classes: those that are serving the file and those who are requesting the file.

# Metainfo file

A single metainfo file should allow both clients and peers to communicate and send/receive file chunks.  This file should contain some sort of information (info dictionary) including the size of the actual file, file name, file pieces (or some method to obtain all the file chunks), and the size of each piece. A metainfo file should have the extension `.torrent` and can be provided from a web page.

The specific tags for a metainfo file include:
- ‘Announce’: location of tracker
- ‘Info’: this key points to a dictionary about the files, which includes different information for single and multi-file torrents

## Required keys for ‘info’ for single file torrents:
- ‘Length’: integer value indicating length of file in bytes
- 'Name’: string name of file
- ‘Piece length’: integer indicating the number of bytes in each piece.
- ‘Pieces’: String value that indicates the concatenation of the 20-byte SHA1 hash values for all pieces in the torrent. I.e. first 20 bytes of the string represent the SHA1 value used to verify piece index 0.

## Required keys for ‘info’ for Multi-file torrents.
- ‘Files’: a dictionary within the ‘info dictionary. The following keys are required for each one.
- ‘Length’: integer indicated total length of file in bytes.
- ‘Path’: list of string elements that specify the path of file relative to topmost directory
- ‘Name’: string name of file
- ‘Piece length’: integer indicating the number of bytes in each piece.
- ‘Pieces’: String value that indicates the concatenation of the 20-byte SHA1 hash values for all pieces in the torrent. I.e. first 20 bytes of the string represent the SHA1 value used to verify piece index 0.

# Splitting up file(s) to be sent

Split a file up into chunks that are then encoded and sent to the client, with a header containing information about the chunk (checksum, size, which file chunk, etc.)

The number of pieces is indicated in the metainfo file. Size of each piece in the torrent is fixed, and given by the integer division:

```
Fixed_piece_size = size_of_torrent / number_of_pieces
```
Generally the recommendation for size is given by whatever allows you to keep the metainfo file under 70 kb.

These pieces are generally verified using a SHA1 hash.

# Peer Wire Protocol:
How to communicate with neighboring peers for the purpose of sharing file content. This happens after the peer has read a metainfo file and has gathered info about other peers with which it can communicate.
Note, there is no standard algorithm for selecting elements from a clients neighboring peers with whom to share pieces.

## Handshaking:
Local peer opens port to listen for connections from remote peers. This port is reported to tracker, and there is no standard port.
Any remote peer wishing to communicate w/ the local peer must open a TCP connection to this port, and perform a handshake operation.
Handshake MUST occur prior to any other data being sent from the remote peer. The local peer MUST NOT send any data back to the remote peer before a well-constructed handshake has been recognized.
If the handshake violates these rules, the local peer MUST close the connection w/ the remote peer.

The handshake is a string of bytes with the following structure:

```
| Name Length | Protocol Name | Reserved | Info Hash | Peer ID |
```

Where the terms are defined as the following:
- ‘Name length’: unsigned value of the first byte represents the length of the character string containing the protocol name. This number is 19 in BTP/1.0.
- ‘Protocol name’: This is a character string which MUST contain the exact name of the protocol in ASCII. This is used to identify which version of BTP the remote peer is using. If they differ, the connection is to be dropped.  In BTP/1.0 this is ‘BitTorrent protocol’.
- ‘Reserved’: Eight bytes that are for future extensions, should be read without interpretation. Can all be set to 0.
- ‘Info Hash’: The following 20 bytes should be interpreted as the 20-byte SHA1 of the info key of the metainfo file. This serves to make sure that the torrent files are the same. Drop the connection if both peers are not using the same value; a different value could mean the peer is not interested anymore.
- ‘Peer ID’: The last 20 bytes are used as the self-designated name of the peer.  The local peer must use this name to identify the connection afterwards. If this name matches any that the local peer knows, then the connection must be dropped.

## Message Communication

Following the handshake, both ends of the TCP channel may send messages in a asynchronous fashion. PWP also have the dual purpose of updating the state of neighboring peers w/ regard to changes in the local peer as well as transferring data blocks between neighboring peers.

PWP messages fall into two different Categories:

### State-oriented messages
These messages serve the purpose of informing peers of changes in the state of neighboring peers. These messages MUST be sent whenever a change occurs in a peer’s state, regardless of the state of other peers. The following messages fall into this category: Interested, Uninterested, Choked, Unchoked, Have,and Bitfield.

### Data-Oriented Messages
These messages handle the requesting and sending of data. The following messages fall into this category: Request, Cancel, and Piece


## Peer States:
At each end of a connection, a peer must maintain two state flags, choked and interested.

- Choked: When true, this flag means that the choked peer is not allowed to request data.
- Interested: When true, this flag means a peer is interested in requesting data from another peer. This indicates that the peer will start requesting blocks if it is unchoked.

A choked peer MUST not send any data oriented messages, but is free to send messages to the peer that blocked it.	 If a peer chokes a remote peer, it MUST also discard any unanswered requests for blocks previously received from the remote peer.

An unchoked peer is allowed to send data-oriented messages to the remote peer. It is left to implementation how many peers a peer is allowed to choke/unchoke.

## Peer wire messages:
All integer members of a PWP message are encoded as a 4-byte big-endian number.

PWP messages have the following structure:

```
| Message Length | Message ID | Payload |
```

- ‘Message length’: Integer denoting the length of the message. (messages of size 0 may be sent as keep alive messages)
- ‘Message ID’: This is a one byte value, indicating type of message (BTP/1.0 specifies 9 different messages’
- ‘Payload’: variable length stream of bytes.

Any violations to the above SHOULD be dropped.

## Messages:

### Choke
ID 0, no payload. A peer sends this message to a remote peer to inform them that it is being choked.

### Unchoke
ID 1, no payload. A peer sends this message to a remote peer to inform that it is no longer being choked.

### Interested
ID 2, no payload.A peer sends this message to a remote peer to inform the remote peer of its desire to request data.

### Uninterested
ID 3, no payload. A peer sends this message to a remote peer to inform it that it is not interested in any pieces from the remote peer.

### Have
ID 4, payload of length 4. The payload is a number denoting the index of a piece that the peer has successfully downloaded and validated. A peer receiving this message must validate the index, and drop the connection if index index is not within the expected bounds. Also, a peer receiving this message MUST send an interested message to the sender if indeed it lacks the piece announced. Further, it MAY also send a request for the piece.

### Bitfield
ID 5, variable payload length. The payload is a bitfield representing the pieces that the sender has successfully downloaded, with the high bit in the first byte corresponding to piece index 0. If a bit is cleared, it is to be interpreted as a missing piece. A peer MUST send this message immediately after the handshake operation, and MAY choose not to send it if it has no pieces at all. This message MUST not be sent at any other time during the communication.  

### Request
ID 6, payload of length 12. The payload is 3 integer values indicating a block within a piece that the sender is interested in downloading from the recipient. The recipient MUST only send the piece messages to a sender that has already requested it, and only in accordance to the above rules w.r.t. The choke and interested states.This has the following structure:

```
| Piece Index | Block Offset | Block Length |
```
### Piece
ID 7, variable payload. The payload holds 2 integers indicated which piece and what offset the block data is derived from. The data length is implicit and can be calculated by subtracting 9 from the total message length. The payload has the following structure:
```
| Piece Index | Block Offset | Block Data |
```

### Cancel
ID 8, payload of length 12. The payload is a 3 integer value indicating a block within a piece that the sender has requested for, but is no longer interested in. The recipient MUST erase the request information upon receiving this message. The payload has the following structure:

```
| Piece Index | Block Offset | Block Length |
```

## Piece Selection Strategy:
BTP/1.0 does not force a particular order for selecting pieces. From experience, doing rarest-first is ideal. To find rarest piece, a client must calculate for each piece index, the number of times this index is true in the bitfield vectors of all neighboring peers. The piece w/ lowest sum is selected for requesting.


# Client to peer

Client has to initiate communication with the peer and receive an affirmative response before sending any file piece requests to the peer.  Client will then send request messages for a specific file chunk, and will wait until it has received a chunk from the peer (after which we could send a confirmation message? May or may not be necessary, since if the file chunk checksums don’t match the client can just make another request for the same chunk).  Client should be able to keep track of different requests made to different peers, and store the received file chunks to be later reassembled.

# Peer to client

Peer waits until a client attempts to begin communication, and sends an affirmative response if the peer is able to send file chunks for the requested torrent file.  Peer then waits for client to request a file chunk, parses the request message, and sends the encoded file chunk with a header message containing relevant information.  Peer should only be trying to send one file chunk at a time a client.

# Peer to peer
