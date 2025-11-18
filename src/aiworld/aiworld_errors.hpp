#pragma once
#include <stdexcept>
#include <string>

namespace aiworld {

enum class ErrorCode {
    SUCCESS = 0,
    CONNECTION_FAILED = 100,
    TIMEOUT = 101,
    INVALID_JSON = 102,
    NPC_NOT_FOUND = 103,
    SERVICE_UNAVAILABLE = 104,
    PERMISSION_DENIED = 105,
    RATE_LIMITED = 106,
    INTERNAL_ERROR = 500
};

inline std::string errorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS: return "Success";
        case ErrorCode::CONNECTION_FAILED: return "Connection to AIWorld server failed";
        case ErrorCode::TIMEOUT: return "Request timeout";
        case ErrorCode::INVALID_JSON: return "Invalid JSON data";
        case ErrorCode::NPC_NOT_FOUND: return "NPC not found";
        case ErrorCode::SERVICE_UNAVAILABLE: return "AIWorld service unavailable";
        case ErrorCode::PERMISSION_DENIED: return "Permission denied";
        case ErrorCode::RATE_LIMITED: return "Rate limit exceeded";
        case ErrorCode::INTERNAL_ERROR: return "Internal server error";
        default: return "Unknown error";
    }
}

class APIException : public std::runtime_error {
public:
    APIException(ErrorCode code, const std::string& message)
        : std::runtime_error(message), error_code_(code) {}
    ErrorCode getErrorCode() const { return error_code_; }
private:
    ErrorCode error_code_;
};

class ConnectionException : public APIException {
public:
    using APIException::APIException;
};

class TimeoutException : public APIException {
public:
    using APIException::APIException;
};

class ValidationException : public APIException {
public:
    using APIException::APIException;
};

} // namespace aiworld