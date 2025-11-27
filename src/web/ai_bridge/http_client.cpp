// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "http_client.hpp"

#include <algorithm>
#include <cstring>
#include <sstream>

#include <common/showmsg.hpp>
#include <common/timer.hpp>

namespace AIBridge {

// ============================================================================
// CURLHandle Implementation
// ============================================================================

CURLHandle::CURLHandle() : handle_(curl_easy_init()) {
    if (handle_) {
        // Set default options for thread safety
        curl_easy_setopt(handle_, CURLOPT_NOSIGNAL, 1L);
    }
}

CURLHandle::~CURLHandle() {
    if (handle_) {
        curl_easy_cleanup(handle_);
        handle_ = nullptr;
    }
}

CURLHandle::CURLHandle(CURLHandle&& other) noexcept : handle_(other.handle_) {
    other.handle_ = nullptr;
}

CURLHandle& CURLHandle::operator=(CURLHandle&& other) noexcept {
    if (this != &other) {
        if (handle_) {
            curl_easy_cleanup(handle_);
        }
        handle_ = other.handle_;
        other.handle_ = nullptr;
    }
    return *this;
}

void CURLHandle::reset() {
    if (handle_) {
        curl_easy_reset(handle_);
        // Restore thread-safe option
        curl_easy_setopt(handle_, CURLOPT_NOSIGNAL, 1L);
    }
}

// ============================================================================
// ConnectionPool Implementation
// ============================================================================

ConnectionPool::ConnectionPool(size_t max_size, bool keep_alive)
    : max_size_(max_size), keep_alive_(keep_alive) {
    // Pre-allocate pool with some connections
    const size_t initial_size = std::min(size_t(2), max_size);
    for (size_t i = 0; i < initial_size; ++i) {
        auto handle = std::make_unique<CURLHandle>();
        if (handle->is_valid()) {
            pool_.push_back(std::move(handle));
        }
    }
}

ConnectionPool::~ConnectionPool() {
    clear();
}

std::unique_ptr<CURLHandle> ConnectionPool::acquire() {
    total_requests_++;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Try to get from pool first
    if (!pool_.empty()) {
        auto handle = std::move(pool_.front());
        pool_.pop_front();
        pool_hits_++;
        return handle;
    }
    
    // Pool empty, create new connection if under limit
    auto handle = std::make_unique<CURLHandle>();
    if (!handle->is_valid()) {
        failed_requests_++;
        ShowError("[ConnectionPool] Failed to create new CURL handle\n");
        return nullptr;
    }
    
    return handle;
}

void ConnectionPool::release(std::unique_ptr<CURLHandle> handle) {
    if (!handle || !handle->is_valid()) {
        return;
    }
    
    // Reset handle for reuse
    handle->reset();
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Only return to pool if under max size
    if (pool_.size() < max_size_) {
        pool_.push_back(std::move(handle));
    }
    // Otherwise let it be destroyed
}

ConnectionPoolStats ConnectionPool::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ConnectionPoolStats stats;
    stats.total_connections = pool_.size();
    stats.idle_connections = pool_.size();
    stats.active_connections = 0; // Simplified - would need tracking
    stats.total_requests = total_requests_.load();
    stats.failed_requests = failed_requests_.load();
    
    if (total_requests_ > 0) {
        stats.pool_reuse_rate = static_cast<double>(pool_hits_) / total_requests_ * 100.0;
        stats.avg_latency_ms = static_cast<double>(total_latency_ms_) / total_requests_;
    }
    
    return stats;
}

void ConnectionPool::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    pool_.clear();
}

// ============================================================================
// CircuitBreaker Implementation
// ============================================================================

CircuitBreaker::CircuitBreaker(
    size_t failure_threshold,
    int64_t timeout_ms,
    size_t half_open_requests
)
    : failure_threshold_(failure_threshold),
      timeout_ms_(timeout_ms),
      half_open_requests_(half_open_requests) {
}

bool CircuitBreaker::allow_request() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    CircuitState current_state = state_.load();
    
    switch (current_state) {
        case CircuitState::CLOSED:
            return true;
            
        case CircuitState::OPEN: {
            // Check if timeout has elapsed
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();
            
            if (now - last_failure_time_ >= timeout_ms_) {
                transition_to_half_open();
                return true;
            }
            return false;
        }
            
        case CircuitState::HALF_OPEN:
            // Allow limited requests in half-open state
            return success_count_ < half_open_requests_;
            
        default:
            return false;
    }
}

void CircuitBreaker::record_success() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    CircuitState current_state = state_.load();
    
    if (current_state == CircuitState::HALF_OPEN) {
        success_count_++;
        if (success_count_ >= half_open_requests_) {
            transition_to_closed();
        }
    } else if (current_state == CircuitState::CLOSED) {
        // Reset failure count on success
        failure_count_ = 0;
    }
}

void CircuitBreaker::record_failure() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    failure_count_++;
    last_failure_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    
    CircuitState current_state = state_.load();
    
    if (current_state == CircuitState::HALF_OPEN) {
        // Failure in half-open state, go back to open
        transition_to_open();
    } else if (current_state == CircuitState::CLOSED && failure_count_ >= failure_threshold_) {
        transition_to_open();
    }
}

CircuitState CircuitBreaker::get_state() const {
    return state_.load();
}

size_t CircuitBreaker::get_failure_count() const {
    return failure_count_.load();
}

void CircuitBreaker::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    transition_to_closed();
}

void CircuitBreaker::transition_to_open() {
    state_ = CircuitState::OPEN;
    ShowWarning("[CircuitBreaker] Circuit opened after %zu failures\n", failure_count_.load());
}

void CircuitBreaker::transition_to_half_open() {
    state_ = CircuitState::HALF_OPEN;
    success_count_ = 0;
    ShowInfo("[CircuitBreaker] Circuit half-open, testing recovery...\n");
}

void CircuitBreaker::transition_to_closed() {
    state_ = CircuitState::CLOSED;
    failure_count_ = 0;
    success_count_ = 0;
    ShowInfo("[CircuitBreaker] Circuit closed, normal operation resumed\n");
}

// ============================================================================
// HTTPClient Implementation
// ============================================================================

HTTPClient::HTTPClient(
    const std::string& base_url,
    int64_t timeout_ms,
    size_t max_retries
)
    : base_url_(base_url),
      timeout_ms_(timeout_ms),
      max_retries_(max_retries),
      pool_(std::make_unique<ConnectionPool>()),
      circuit_breaker_(std::make_unique<CircuitBreaker>()) {
    
    // Initialize CURL globally (thread-safe after this)
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    ShowInfo("[HTTPClient] Initialized with base_url=%s, timeout=%lldms, max_retries=%zu\n",
             base_url_.c_str(), timeout_ms_, max_retries_);
}

HTTPClient::~HTTPClient() {
    pool_->clear();
    curl_global_cleanup();
}

// Static CURL write callback
size_t HTTPClient::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), total_size);
    return total_size;
}

HTTPResponse HTTPClient::get(
    const std::string& endpoint,
    const std::vector<std::pair<std::string, std::string>>& headers
) {
    return retry_request("GET", endpoint, "", headers);
}

HTTPResponse HTTPClient::post(
    const std::string& endpoint,
    const std::string& json_body,
    const std::vector<std::pair<std::string, std::string>>& headers
) {
    return retry_request("POST", endpoint, json_body, headers);
}

HTTPResponse HTTPClient::retry_request(
    const std::string& method,
    const std::string& endpoint,
    const std::string& body,
    const std::vector<std::pair<std::string, std::string>>& headers
) {
    HTTPResponse last_response;
    
    for (size_t attempt = 0; attempt <= max_retries_; ++attempt) {
        // Check circuit breaker before attempting
        if (!circuit_breaker_->allow_request()) {
            last_response.success = false;
            last_response.error_message = "Circuit breaker is open";
            last_response.status_code = 503; // Service Unavailable
            ShowWarning("[HTTPClient] Request blocked by circuit breaker\n");
            return last_response;
        }
        
        // Execute request
        last_response = execute_request(method, endpoint, body, headers);
        
        // Check if successful
        if (last_response.success) {
            circuit_breaker_->record_success();
            successful_requests_++;
            return last_response;
        }
        
        // Record failure in circuit breaker
        circuit_breaker_->record_failure();
        
        // Don't retry on client errors (4xx)
        if (last_response.status_code >= 400 && last_response.status_code < 500) {
            ShowDebug("[HTTPClient] Client error %d, not retrying\n", last_response.status_code);
            break;
        }
        
        // Retry with backoff if not last attempt
        if (attempt < max_retries_) {
            int64_t backoff_ms = calculate_backoff_ms(attempt);
            ShowDebug("[HTTPClient] Retry %zu/%zu after %lldms backoff\n",
                     attempt + 1, max_retries_, backoff_ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(backoff_ms));
        }
    }
    
    failed_requests_++;
    return last_response;
}

HTTPResponse HTTPClient::execute_request(
    const std::string& method,
    const std::string& endpoint,
    const std::string& body,
    const std::vector<std::pair<std::string, std::string>>& headers
) {
    auto start_time = std::chrono::steady_clock::now();
    
    HTTPResponse response;
    std::string url = base_url_ + endpoint;
    
    // Acquire connection from pool
    auto handle = pool_->acquire();
    if (!handle || !handle->is_valid()) {
        response.error_message = "Failed to acquire connection from pool";
        response.status_code = 0;
        return response;
    }
    
    CURL* curl = handle->get();
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms_);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_ms_);
    
    // Set method and body
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    } else {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }
    
    // Build header list
    struct curl_slist* header_list = nullptr;
    header_list = curl_slist_append(header_list, "Accept: application/json");
    if (method == "POST") {
        header_list = curl_slist_append(header_list, "Content-Type: application/json");
    }
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    
    // Set write callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
    
    // Execute request
    CURLcode res = curl_easy_perform(curl);
    
    // Get response code
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    response.status_code = static_cast<int>(response_code);
    
    // Calculate latency
    auto end_time = std::chrono::steady_clock::now();
    response.latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time
    ).count();
    
    // Cleanup
    curl_slist_free_all(header_list);
    
    // Check result
    if (res != CURLE_OK) {
        response.success = false;
        response.error_message = curl_easy_strerror(res);
        ShowDebug("[HTTPClient] Request failed: %s\n", response.error_message.c_str());
    } else if (response.status_code >= 200 && response.status_code < 300) {
        response.success = true;
    } else {
        response.success = false;
        response.error_message = "HTTP " + std::to_string(response.status_code);
        ShowDebug("[HTTPClient] HTTP error %d\n", response.status_code);
    }
    
    // Return connection to pool
    pool_->release(std::move(handle));
    
    return response;
}

int64_t HTTPClient::calculate_backoff_ms(size_t attempt) const {
    // Exponential backoff: 1s, 2s, 4s, 8s, 16s
    int64_t base_delay = 1000;
    int64_t max_delay = 16000;
    int64_t delay = base_delay * (1 << attempt);
    return std::min(delay, max_delay);
}

ConnectionPoolStats HTTPClient::get_pool_stats() const {
    return pool_->get_stats();
}

CircuitState HTTPClient::get_circuit_state() const {
    return circuit_breaker_->get_state();
}

void HTTPClient::reset_circuit_breaker() {
    circuit_breaker_->reset();
}

bool HTTPClient::is_healthy() const {
    return circuit_breaker_->get_state() == CircuitState::CLOSED;
}

} // namespace AIBridge