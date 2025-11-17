#include "p2p_coordinator.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <string>
#include <iostream>

// Production-grade P2P TCP listener
int p2p_listen(const std::string& address, uint16_t port, const P2POptions& options) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "[P2P] Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address.c_str());
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[P2P] Failed to bind: " << strerror(errno) << std::endl;
        close(sockfd);
        return -1;
    }
    if (listen(sockfd, options.backlog) < 0) {
        std::cerr << "[P2P] Failed to listen: " << strerror(errno) << std::endl;
        close(sockfd);
        return -1;
    }
    return sockfd;
}

// Production-grade P2P TCP connect
int p2p_connect(const std::string& address, uint16_t port, const P2POptions& options) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "[P2P] Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address.c_str());
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[P2P] Failed to connect: " << strerror(errno) << std::endl;
        close(sockfd);
        return -1;
    }
    return sockfd;
}