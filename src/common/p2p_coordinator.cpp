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
#include <fcntl.h>
#include <set>

// Global P2P initialization flag
static bool p2p_initialized = false;

// Track P2P file descriptors for is_p2p_fd()
static std::set<int> p2p_fds;

/**
 * Initialize P2P subsystem (singleton/global)
 * Sets up the P2P networking layer with TCP fallback
 * For full P2P: Integrate with libp2p or custom P2P library
 */
bool p2p_init() {
    if (p2p_initialized) {
        return true;
    }
    
    // TCP fallback: No special initialization needed
    // For full P2P: Initialize libp2p context here
    // Example: libp2p_init() or custom P2P bootstrap
    
    p2p_initialized = true;
    
    std::cout << "[P2P] Initialized with TCP fallback mode" << std::endl;
    std::cout << "[P2P] For full P2P networking, integrate libp2p or custom P2P library" << std::endl;
    std::cout << "[P2P] Benefits: NAT traversal, DHT discovery, encrypted streams" << std::endl;
    
    return true;
}

/**
 * Finalize P2P subsystem
 * Cleanup resources when shutting down
 */
void p2p_finalize() {
    if (!p2p_initialized) {
        return;
    }
    
    // Close all tracked P2P connections
    for (int fd : p2p_fds) {
        if (fd >= 0) {
            close(fd);
        }
    }
    p2p_fds.clear();
    
    // TCP fallback: No cleanup needed
    // For full P2P: Cleanup libp2p context
    
    p2p_initialized = false;
    std::cout << "[P2P] Finalized P2P subsystem" << std::endl;
}

// Production-grade P2P TCP listener
int p2p_listen(const std::string& address, uint16_t port, const P2POptions& options) {
    if (!p2p_initialized) {
        if (!p2p_init()) {
            std::cerr << "[P2P] Failed to initialize P2P subsystem" << std::endl;
            return -1;
        }
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "[P2P] Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }
    
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Set non-blocking mode for async operation
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    }
    
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
    
    p2p_fds.insert(sockfd);
    
    if (!options.silent) {
        std::cout << "[P2P] Listening on " << address << ":" << port << std::endl;
    }
    
    return sockfd;
}

// Production-grade P2P TCP connect
int p2p_connect(const std::string& address, uint16_t port, const P2POptions& options) {
    if (!p2p_initialized) {
        if (!p2p_init()) {
            std::cerr << "[P2P] Failed to initialize P2P subsystem" << std::endl;
            return -1;
        }
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "[P2P] Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }
    
    // Set non-blocking mode for connect with timeout
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    }
    
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address.c_str());
    
    int connect_result = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    
    if (connect_result < 0 && errno != EINPROGRESS) {
        std::cerr << "[P2P] Failed to connect: " << strerror(errno) << std::endl;
        close(sockfd);
        return -1;
    }
    
    // Wait for connection with timeout
    if (errno == EINPROGRESS) {
        fd_set write_fds;
        FD_ZERO(&write_fds);
        FD_SET(sockfd, &write_fds);
        
        struct timeval timeout;
        timeout.tv_sec = options.timeout;
        timeout.tv_usec = 0;
        
        int select_result = select(sockfd + 1, nullptr, &write_fds, nullptr, &timeout);
        
        if (select_result == 0) {
            if (!options.silent) {
                std::cerr << "[P2P] Connection timeout after " << options.timeout << " seconds" << std::endl;
            }
            close(sockfd);
            return -1;
        } else if (select_result < 0) {
            if (!options.silent) {
                std::cerr << "[P2P] Select error: " << strerror(errno) << std::endl;
            }
            close(sockfd);
            return -1;
        }
        
        // Check if connection succeeded
        int error = 0;
        socklen_t error_len = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &error_len) < 0 || error != 0) {
            if (!options.silent) {
                std::cerr << "[P2P] Connection failed: " << strerror(error) << std::endl;
            }
            close(sockfd);
            return -1;
        }
    }
    
    // Restore blocking mode for normal I/O
    fcntl(sockfd, F_SETFL, flags);
    
    p2p_fds.insert(sockfd);
    
    if (!options.silent) {
        std::cout << "[P2P] Connected to " << address << ":" << port << std::endl;
    }
    
    return sockfd;
}

/**
 * Send data over P2P connection
 * TCP fallback: Standard send() with MSG_NOSIGNAL
 * Full P2P: Encrypted stream send with libp2p
 */
ssize_t p2p_send(int fd, const void* data, size_t len) {
    if (fd < 0 || !data || len == 0) {
        return -1;
    }
    
    // TCP fallback: Use standard send
    // For full P2P: Use libp2p stream write
    ssize_t sent = send(fd, data, len, MSG_NOSIGNAL);
    
    if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        std::cerr << "[P2P] Send error: " << strerror(errno) << std::endl;
    }
    
    return sent;
}

/**
 * Receive data from P2P connection
 * TCP fallback: Standard recv()
 * Full P2P: Decrypted stream receive
 */
ssize_t p2p_recv(int fd, void* buffer, size_t len) {
    if (fd < 0 || !buffer || len == 0) {
        return -1;
    }
    
    // TCP fallback: Use standard recv
    // For full P2P: Use libp2p stream read
    ssize_t received = recv(fd, buffer, len, 0);
    
    if (received < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        std::cerr << "[P2P] Receive error: " << strerror(errno) << std::endl;
    }
    
    return received;
}

/**
 * Close a P2P connection
 * TCP fallback: Standard close()
 * Full P2P: Graceful stream close with libp2p
 */
void p2p_close(int fd) {
    if (fd >= 0) {
        p2p_fds.erase(fd);
        // TCP fallback: Standard close
        // For full P2P: libp2p_stream_close() or similar
        close(fd);
    }
}

/**
 * Check if fd is a P2P connection
 * TCP fallback: Check if fd is in tracked set
 * Full P2P: Check if fd is managed by libp2p
 */
bool is_p2p_fd(int fd) {
    return p2p_fds.find(fd) != p2p_fds.end();
}
