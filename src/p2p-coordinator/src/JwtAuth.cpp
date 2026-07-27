#include "JwtAuth.h"
#include <jwt-cpp/jwt.h>
#include <chrono>

JwtAuth::JwtAuth(const std::string& secret) : secret_(secret) {}

bool JwtAuth::ValidateToken(const std::string& token, std::string& error) {
    try {
        // Decode the token without verification first to check structure
        auto decoded = jwt::decode(token);
        
        // Verify the token signature using the shared secret
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_})
            .with_issuer("p2p-coordinator");
        
        verifier.verify(decoded);
        
        // Check expiration
        auto exp = decoded.get_expires_at();
        auto now = std::chrono::system_clock::now();
        if (exp < now) {
            error = "Token has expired";
            return false;
        }
        
        error.clear();
        return true;
    } catch (const jwt::token_verification_exception& e) {
        error = std::string("Token verification failed: ") + e.what();
        return false;
    } catch (const std::exception& e) {
        error = std::string("Token validation error: ") + e.what();
        return false;
    }
}
