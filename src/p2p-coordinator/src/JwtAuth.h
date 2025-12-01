#pragma once

#include <string>
#include <jwt-cpp/jwt.h>

/**
 * JwtAuth - Handles JWT token validation and authentication for coordinator endpoints.
 */
class JwtAuth {
public:
    JwtAuth(const std::string& secret);

    // Validates a JWT token and returns true if valid, false otherwise.
    bool ValidateToken(const std::string& token, std::string& error);

private:
    std::string secret_;
};