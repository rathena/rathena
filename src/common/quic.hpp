// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// QUIC protocol support interface for rAthena networking
// This file provides the interface and stubs for QUIC protocol integration.
// To be extended with a real QUIC implementation (e.g., msquic, quiche).

#ifndef QUIC_HPP
#define QUIC_HPP

#include <string>
#include <cstdint>

// QUIC connection options
struct QuicOptions {
    std::string cert_file;
    std::string key_file;
    int timeout = 5;
    bool silent = false;
};

// QUIC connection handle (opaque)
struct QuicConnection;

// Initialize QUIC library (singleton/global)
bool quic_init();

// Finalize QUIC library
void quic_finalize();

// Create a QUIC server socket, returns fd or -1 on error
int quic_listen(const std::string& address, uint16_t port, const QuicOptions& options);

// Connect to a QUIC server, returns fd or -1 on error
int quic_connect(const std::string& address, uint16_t port, const QuicOptions& options);

// Send data over QUIC connection
ssize_t quic_send(int fd, const void* data, size_t len);

// Receive data from QUIC connection
ssize_t quic_recv(int fd, void* buffer, size_t len);

// Close a QUIC connection
void quic_close(int fd);

// Utility: check if fd is a QUIC connection
bool is_quic_fd(int fd);

#endif // QUIC_HPP