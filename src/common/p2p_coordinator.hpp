// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// P2P Coordinator interface for rAthena networking
// This file provides the interface and stubs for P2P coordinator integration.
// To be extended with a real P2P implementation (e.g., libp2p, custom coordinator).

#ifndef P2P_COORDINATOR_HPP
#define P2P_COORDINATOR_HPP

#include <string>
#include <cstdint>

// P2P connection options
struct P2POptions {
    std::string peer_id;
    std::string bootstrap_addr;
    int timeout = 5;
    bool silent = false;
    int backlog = 16;
};

// P2P connection handle (opaque)
struct P2PConnection;

// Initialize P2P subsystem (singleton/global)
bool p2p_init();

// Finalize P2P subsystem
void p2p_finalize();

// Start a P2P listener, returns fd or -1 on error
int p2p_listen(const std::string& address, uint16_t port, const P2POptions& options);

// Connect to a P2P peer, returns fd or -1 on error
int p2p_connect(const std::string& address, uint16_t port, const P2POptions& options);

// Send data over P2P connection
ssize_t p2p_send(int fd, const void* data, size_t len);

// Receive data from P2P connection
ssize_t p2p_recv(int fd, void* buffer, size_t len);

// Close a P2P connection
void p2p_close(int fd);

// Utility: check if fd is a P2P connection
bool is_p2p_fd(int fd);

#endif // P2P_COORDINATOR_HPP