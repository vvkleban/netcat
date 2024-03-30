A reasonable C++ netcat implementation for Windows and UNIX/Linux systems, where a single stream direction is used. By default it's from client to server. However with -r command line option the stream is reversed.

Usage as client:
netcat [-r] <host> <port>

Usage as server:
netcat -l [-r] <port>

Where -r - reverse transfer (inbound for client and outbound for server)
