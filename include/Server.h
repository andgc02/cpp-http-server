#ifndef SERVER_H
#define SERVER_H

#ifdef _WIN32
#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <netinet/in.h>
#include <unistd.h>
#endif

#include <string>

class Server {
public:
    explicit Server(int port);
    ~Server();
    void start();

private:
    #ifdef _WIN32
    SOCKET server_fd;
    #else
    int server_fd;
    #endif
    int port;
    struct sockaddr_in address;

    void handleClient(
        #ifdef _WIN32
        SOCKET
        #else
        int
        #endif
        client_socket);
    std::string generateHttpResponse();
};

#endif // SERVER_H
