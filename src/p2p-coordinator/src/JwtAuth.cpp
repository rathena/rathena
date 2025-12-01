#include "JwtAuth.h"
JwtAuth::JwtAuth(const std::string& secret) : secret_(secret) {}

bool JwtAuth::ValidateToken(const std::string& token, std::string& error) {
    // Minimal stub: always return true for now
    error.clear();
    return true;
}