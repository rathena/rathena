// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
/**
 * @file http_client.hpp
 * @brief Production-grade HTTP client with connection pooling, retry logic, and circuit breaker
 * 
 * Features:
 * - Connection pooling (max 10 connections)
 * - Automatic retry with exponential backoff (max 3 attempts)
 * - Circuit breaker pattern (5 failures → open for 60s)
 * - Thread-safe operations
 * - Comprehensive error handling
 */

#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <atomic>
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <curl/curl.h>

namespace AIBridge {

/**
 * @brief HTTP response structure
 */
struct HTTPResponse {
    int status_code{0};          ///< HTTP status code (200, 404, etc.)
    std::string body;             ///< Response body
    bool success{false};          ///< Whether request succeeded
    std::string error_message;    ///< Error message if failed
    int64_t latency_ms{0};       ///< Request latency in milliseconds
};

/**
 * @brief Connection pool statistics
 */
struct ConnectionPoolStats {
    size_t total_connections{0};
    size_t active_connections{0};
    size_t idle_connections{0};
    size_t total_requests{0};
    size_t failed_requests{0};
    double avg_latency_ms{0.0};
    double pool_reuse_rate{0.0};
};

/**
 * @brief CURL handle wrapper with RAII
 */
class CURLHandle {
public:
    explicit CURLHandle();
    ~CURLHandle();
    
    // Delete copy constructor and copy assignment
    CURLHandle(const CURLHandle&) = delete;
    CURLHandle& operator=(const CURLHandle&) = delete;
    
    // Allow move semantics
    CURLHandle(CURLHandle&& other) noexcept;
    CURLHandle& operator=(CURLHandle&& other) noexcept;
    
    CURL* get() const noexcept { return handle_; }
    bool is_valid() const noexcept { return handle_ != nullptr; }
    void reset(); ///< Reset handle for reuse
    
private:
    CURL* handle_{nullptr};
};

/**
 * @brief Connection pool for CURL handles
 * 
 * Manages a pool of reusable CURL connections to minimize overhead
 * Thread-safe with mutex protection
 */
class ConnectionPool {
public:
    /**
     * @brief Construct connection pool
     * @param max_size Maximum number of connections in pool (default: 10)
     * @param keep_alive Enable HTTP keep-alive (default: true)
     */
    explicit ConnectionPool(size_t max_size = 10, bool keep_alive = true);
    ~ConnectionPool();
    
    /**
     * @brief Acquire a connection from the pool
     * @return CURL handle or nullptr if pool exhausted
     */
    std::unique_ptr<CURLHandle> acquire();
    
    /**
     * @brief Return a connection to the pool
     * @param handle Connection to return
     */
    void release(std::unique_ptr<CURLHandle> handle);
    
    /**
     * @brief Get pool statistics
     * @return ConnectionPoolStats structure
     */
    ConnectionPoolStats get_stats() const;
    
    /**
     * @brief Clear all connections from pool
     */
    void clear();
    
private:
    const size_t max_size_;
    const bool keep_alive_;
    std::deque<std::unique_ptr<CURLHandle>> pool_;
    mutable std::mutex mutex_;
    
    // Statistics
    std::atomic<size_t> total_requests_{0};
    std::atomic<size_t> pool_hits_{0};
    std::atomic<size_t> failed_requests_{0};
    std::atomic<int64_t> total_latency_ms_{0};
};

/**
 * @brief Circuit breaker states
 */
enum class CircuitState {
    CLOSED,      ///< Normal operation, requests allowed
    OPEN,        ///< Too many failures, requests blocked
    HALF_OPEN    ///< Testing if service recovered
};

/**
 * @brief Circuit breaker implementation
 * 
 * Protects against cascading failures by temporarily blocking requests
 * when too many failures are detected
 */
class CircuitBreaker {
public:
    /**
     * @brief Construct circuit breaker
     * @param failure_threshold Number of failures before opening (default: 5)
     * @param timeout_ms Time to wait before testing recovery (default: 60000ms)
     * @param half_open_requests Number of test requests in half-open state (default: 3)
     */
    explicit CircuitBreaker(
        size_t failure_threshold = 5,
        int64_t timeout_ms = 60000,
        size_t half_open_requests = 3
    );
    
    /**
     * @brief Check if request should be allowed
     * @return true if request allowed, false if circuit is open
     */
    bool allow_request();
    
    /**
     * @brief Record successful request
     */
    void record_success();
    
    /**
     * @brief Record failed request
     */
    void record_failure();
    
    /**
     * @brief Get current circuit state
     * @return CircuitState
     */
    CircuitState get_state() const;
    
    /**
     * @brief Get failure count
     * @return Number of consecutive failures
     */
    size_t get_failure_count() const;
    
    /**
     * @brief Reset circuit breaker to closed state
     */
    void reset();
    
private:
    void transition_to_open();
    void transition_to_half_open();
    void transition_to_closed();
    
    const size_t failure_threshold_;
    const int64_t timeout_ms_;
    const size_t half_open_requests_;
    
    std::atomic<CircuitState> state_{CircuitState::CLOSED};
    std::atomic<size_t> failure_count_{0};
    std::atomic<size_t> success_count_{0};
    std::atomic<int64_t> last_failure_time_{0};
    mutable std::mutex mutex_;
};

/**
 * @brief Production-grade HTTP client
 * 
 * Features:
 * - Connection pooling for performance
 * - Automatic retry with exponential backoff
 * - Circuit breaker for fault tolerance
 * - Thread-safe operations
 * - Comprehensive logging
 */
class HTTPClient {
public:
    /**
     * @brief Construct HTTP client
     * @param base_url Base URL for all requests (e.g., "http://192.168.0.100:8000")
     * @param timeout_ms Request timeout in milliseconds (default: 5000)
     * @param max_retries Maximum retry attempts (default: 3)
     */
    explicit HTTPClient(
        const std::string& base_url,
        int64_t timeout_ms = 5000,
        size_t max_retries = 3
    );
    
    ~HTTPClient();
    
    /**
     * @brief Perform HTTP GET request
     * @param endpoint Endpoint path (e.g., "/api/v1/health")
     * @param headers Optional additional headers
     * @return HTTPResponse
     */
    HTTPResponse get(
        const std::string& endpoint,
        const std::vector<std::pair<std::string, std::string>>& headers = {}
    );
    
    /**
     * @brief Perform HTTP POST request
     * @param endpoint Endpoint path
     * @param json_body JSON request body
     * @param headers Optional additional headers
     * @return HTTPResponse
     */
    HTTPResponse post(
        const std::string& endpoint,
        const std::string& json_body,
        const std::vector<std::pair<std::string, std::string>>& headers = {}
    );
    
    /**
     * @brief Get connection pool statistics
     * @return ConnectionPoolStats
     */
    ConnectionPoolStats get_pool_stats() const;
    
    /**
     * @brief Get circuit breaker state
     * @return CircuitState
     */
    CircuitState get_circuit_state() const;
    
    /**
     * @brief Reset circuit breaker
     */
    void reset_circuit_breaker();
    
    /**
     * @brief Check if client is healthy
     * @return true if circuit is closed and pool is available
     */
    bool is_healthy() const;
    
private:
    HTTPResponse execute_request(
        const std::string& method,
        const std::string& endpoint,
        const std::string& body,
        const std::vector<std::pair<std::string, std::string>>& headers
    );
    
    HTTPResponse retry_request(
        const std::string& method,
        const std::string& endpoint,
        const std::string& body,
        const std::vector<std::pair<std::string, std::string>>& headers
    );
    
    int64_t calculate_backoff_ms(size_t attempt) const;
    
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
    
    const std::string base_url_;
    const int64_t timeout_ms_;
    const size_t max_retries_;
    
    std::unique_ptr<ConnectionPool> pool_;
    std::unique_ptr<CircuitBreaker> circuit_breaker_;
    
    // Statistics
    std::atomic<size_t> total_requests_{0};
    std::atomic<size_t> successful_requests_{0};
    std::atomic<size_t> failed_requests_{0};
};

} // namespace AIBridge

#endif // HTTP_CLIENT_HPP