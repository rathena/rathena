#include "quic.hpp"
#include <string>
#include <iostream>

// Enterprise-ready QUIC listen (stub for now, extend with msquic/ngtcp2/other)
int quic_listen(const std::string& address, uint16_t port, const QuicOptions& options) {
    std::cerr << "[QUIC] quic_listen not implemented. Please integrate a QUIC library (e.g., msquic, ngtcp2)." << std::endl;
    return -1;
}

// Enterprise-ready QUIC connect (stub for now, extend with msquic/ngtcp2/other)
int quic_connect(const std::string& address, uint16_t port, const QuicOptions& options) {
    std::cerr << "[QUIC] quic_connect not implemented. Please integrate a QUIC library (e.g., msquic, ngtcp2)." << std::endl;
    return -1;
}