# BitTorrent Client in C
Team Dubstep Ducks

Project 1 for the Software Systems class

## Description
This student project was created as part of an educational course. For more details, see the [final report](https://daniel6.github.io/SoftSysDubstepDucks/).

This project alows users to set up their own peer to peer file transfer system following the BitTorrent protocol. This distribution is not intended for rigorous use, and was instead designed as a learning exercise for the team. As such, the project is more than likely disfunctional, although it may still prove to be valuable as a reference.

## Setup and Usage
To compile the project from source, simply run `make` in the root directory.

Before running any clients, the tracker server must be set up. Executing the `tracker_serv.out` binary will initialize the tracker server on your machine. Make note of your machine's IP address, as this will need to be encoded in any torrent files that you distribute.

Now that the tracker server is up and running, you must generate a torrent file for the file that you want to distribute. Run `encode.out <name of file you want to distribute> <your IP here>` to generate a torrent file for the file specified. You will want to distribute `<file to distribute>.torrent` to all of the clients via your ftp of choice.

Each client must posess a copy of this torrent file in order to join the network, as it contains important metadata about the file being transferred.

To initialize a client, execute the `torrent_client.out` binary. This process will attempt to download the example file from any other active clients in the network.

## Authors
Daniel Bishop

Jordan Van Duyne

Thuc Tran

Andrew Pan

### Additional Libraries

bencode.c written by Mike Frysinger and edited by Adam Aviv

openssl/sha.h by OpenSSL


## License
MIT License

Copyright (c) [2017] [Daniel Bishop]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
