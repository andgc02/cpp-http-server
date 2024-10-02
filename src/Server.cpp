#include "Server.h"
#include <iostream>
#include <cstring>
#include <thread>

Server::Server(int port) : port(port) {
    #ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        exit(EXIT_FAILURE);
    }
    #endif

    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    #ifdef _WIN32
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    #else
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    #endif

    int opt = 1;
    #ifdef _WIN32
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "setsockopt failed: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    #else
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    #endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    #ifdef _WIN32
    if (bind(server_fd, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    #else
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    #endif
}

Server::~Server() {
    #ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
    #else
    close(server_fd);
    #endif
}

void Server::start() {
    #ifdef _WIN32
    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    #else
    if (listen(server_fd, 10) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    #endif

    std::cout << "Server is listening on port " << port << std::endl;

    while (true) {
        int addrlen = sizeof(address);
        #ifdef _WIN32
        SOCKET client_socket = accept(server_fd, (SOCKADDR*)&address, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }
        #else
        int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("Accept");
            continue;
        }
        #endif

        std::thread(&Server::handleClient, this, client_socket).detach();
    }
}

void Server::handleClient(
    #ifdef _WIN32
    SOCKET
    #else
    int
    #endif
    client_socket) {
    char buffer[1024] = {0};
    #ifdef _WIN32
    int valread = recv(client_socket, buffer, sizeof(buffer), 0);
    if (valread == SOCKET_ERROR) {
        std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        return;
    }
    #else
    ssize_t valread = read(client_socket, buffer, sizeof(buffer));
    if (valread < 0) {
        perror("Read");
        close(client_socket);
        return;
    }
    #endif

    std::string response = generateHttpResponse();

    #ifdef _WIN32
    int sendResult = send(client_socket, response.c_str(), response.size(), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
    }
    closesocket(client_socket);
    #else
    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
    #endif
}

std::string Server::generateHttpResponse() {
    std::string html_content = "<html><body><h1>Hello, World!</h1></body></html>";
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + std::to_string(html_content.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        html_content;
    return response;
}
