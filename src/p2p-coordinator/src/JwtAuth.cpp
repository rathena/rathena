#include "JwtAuth.h"
#include "../../include/Logger.h"

JwtAuth::JwtAuth(const std::string& secret)
    : secret_(secret) {}

bool JwtAuth::ValidateToken(const std::string& token, std::string& error) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_})
            .with_issuer("ai-mmorpg-coordinator");
        verifier.verify(decoded);
        LOG_INFO("JWT token validated successfully");
        return true;
    } catch (const std::exception& ex) {
        error = ex.what();
        LOG_WARN("JWT token validation failed: " + error);
        return false;
    }
}