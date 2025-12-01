#pragma once

#include <functional>
#include <string>
#include <memory>
#include "JwtAuth.h"

// Forward declaration for WebSocket connection handle
class WebSocketConnection;

class WebSocketServer {
public:
    using MessageHandler = std::function<void(const std::string&, std::shared_ptr<WebSocketConnection>)>;

    WebSocketServer(const std::string& host, int port, std::shared_ptr<JwtAuth> jwtAuth);

    // Start the server (blocking call)
    void run();

    // Stop the server
    void stop();

    // Set handler for authenticated messages
    void setMessageHandler(MessageHandler handler);

private:
    std::string host_;
    int port_;
    std::shared_ptr<JwtAuth> jwtAuth_;
    MessageHandler messageHandler_;
    bool running_;
};