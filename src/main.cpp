#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

// Conditional compilation for cross-platform compatibility
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <io.h>
    #include <fcntl.h>
    #pragma comment(lib, "Ws2_32.lib")
    #define CLOSESOCKET closesocket
    #define SOCKLEN int
#else
    #include <cerrno>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #define CLOSESOCKET close
    #define SOCKLEN socklen_t
#endif

void die(const char *s) {
    #ifdef _WIN32
    std::cerr << s << ": " << WSAGetLastError() << std::endl;
    #else
    perror(s);
    #endif
    exit(1);
}

void stream2Socket(int sock) {
    char buffer[262144];
    std::size_t haveRead;
    while ((haveRead = std::fread(buffer, sizeof(buffer[0]), sizeof(buffer), stdin))) {
        if (send(sock, buffer, (int) haveRead, 0) < 0) {
            die("send failed");
        }
    }
}

void socket2Stream(int sock) {
    char buffer[262144];
    int length;
    while ((length = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        std::cout.write(buffer, length);
    }
}

void client_mode(const char *host, int port, bool inbound) {
    struct sockaddr_in server;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) die("Could not create socket");

    server.sin_addr.s_addr = inet_addr(host);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        die("connect failed");
    }

    if (inbound) {
        socket2Stream(sock);
    } else {
        stream2Socket(sock);
    }
    CLOSESOCKET(sock);
}

void server_mode(int port, bool inbound) {
    int listener, sock;
    struct sockaddr_in server, client;
    SOCKLEN client_len = sizeof(client);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) die("Could not open socket");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(listener, (struct sockaddr *)&server, sizeof(server)) < 0) {
        die("bind failed");
    }

    listen(listener, 1);
    sock = accept(listener, (struct sockaddr *)&client, &client_len);
    if (sock < 0) die("accept failed");

    if (inbound) {
        socket2Stream(sock);
    } else {
        stream2Socket(sock);
    }
    CLOSESOCKET(sock);
    CLOSESOCKET(listener);
}

void usage(const char * name, const char * error)
{
    if (error[0] != 0) {
        std::cerr << error << "\n\n";
    }
    std::cerr << "Usage as client:\n"
              << name << " [-r] <host> <port>\n\n"
              << "Usage as server:\n"
              << name << " -l [-r] <port>\n\n"
              << "Where -r - reverse transfer (inbound for client and outbound for server)\n";
    exit(error[0] == 0 ? 0 : 1);
}

#define USAGE(a) usage(argv[0], (a))

int main(int argc, char *argv[]) {
    if (argc < 3)
    {
        USAGE("");
    }

    bool server= false;
    bool reverse= false;
    bool hostNeeded= true;
    char * host;
    int port= -1;

    for (int i= 1; i < argc; i++) {
        if (std::strcmp(argv[i], "-l") == 0) {
            server= true;
            hostNeeded= false;
        } else if (std::strcmp(argv[i], "-r") == 0) {
            reverse= true;
        } else if (argv[i][0] == '-') {
            std::cerr << "Unrecognized argument " << i << std::endl;
            return 1;
        } else if (hostNeeded) {
            host= argv[i];
            hostNeeded= false;
        } else {
            port= std::atoi(argv[i]);
        }
    }

    if (hostNeeded) {
        USAGE("Could not find host");
    } else if (port <= 0) {
        USAGE("Could not find port");
    }

    bool inbound= server ^ reverse;

    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        die("WSAStartup failed");
    }
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    #endif

    if (server) {
        server_mode(port, inbound);
    } else {
        client_mode(host, port, inbound);
    }
    return 0;

    #ifdef _WIN32
    WSACleanup();
    #endif
    return 0;
}
