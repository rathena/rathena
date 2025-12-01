/**
 * aiworld_http_client.hpp
 * HTTP Client for C++ → Python AI Service Communication
 * 
 * Provides thread-safe HTTP client with:
 * - Connection pooling
 * - Timeout handling
 * - Retry logic
 * - Error recovery
 */

#ifndef AIWORLD_HTTP_CLIENT_HPP
#define AIWORLD_HTTP_CLIENT_HPP

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>

namespace aiworld {

/**
 * HTTP Response structure
 */
struct HTTPResponse {
    int status_code = 0;
    std::string body;
    bool success = false;
    std::string error_message;
    
    nlohmann::json json() const {
        if (body.empty()) return nlohmann::json::object();
        try {
            return nlohmann::json::parse(body);
        } catch (...) {
            return nlohmann::json::object();
        }
    }
};

/**
 * Thread-safe HTTP Client with connection pooling
 */
class AIWorldHTTPClient {
public:
    /**
     * Constructor
     * @param base_url Base URL of the AI service (e.g., "http://127.0.0.1:8000")
     * @param timeout_ms Request timeout in milliseconds (default: 3000ms)
     * @param max_retries Maximum number of retry attempts (default: 2)
     */
    AIWorldHTTPClient(
        const std::string& base_url = "http://127.0.0.1:8000",
        int timeout_ms = 3000,
        int max_retries = 2
    );
    
    ~AIWorldHTTPClient();
    
    /**
     * Perform HTTP GET request
     * @param endpoint API endpoint path (e.g., "/api/npc/NPC001/state")
     * @return HTTPResponse with status and body
     */
    HTTPResponse get(const std::string& endpoint);
    
    /**
     * Perform HTTP POST request with JSON body
     * @param endpoint API endpoint path
     * @param json_body Request body as JSON
     * @return HTTPResponse with status and body
     */
    HTTPResponse post(const std::string& endpoint, const nlohmann::json& json_body);
    
    /**
     * Check if client is healthy
     */
    bool is_healthy();
    
    /**
     * Get client statistics
     */
    struct Stats {
        size_t total_requests = 0;
        size_t successful_requests = 0;
        size_t failed_requests = 0;
        size_t retried_requests = 0;
        double avg_latency_ms = 0.0;
    };
    
    Stats get_stats() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    
    HTTPResponse execute_request(
        const std::string& method,
        const std::string& endpoint,
        const std::string& body = ""
    );
};

} // namespace aiworld

// --- Global HTTP Client Management (C-linkage for plugin use) ---
void aiworld_init_http_client(const std::string& base_url = "http://127.0.0.1:8000");
aiworld::AIWorldHTTPClient* aiworld_get_http_client();
void aiworld_shutdown_http_client();

// --- Script Command Registration ---
void aiworld_register_http_script_commands();

#endif // AIWORLD_HTTP_CLIENT_HPP