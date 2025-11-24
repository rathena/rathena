/**
 * aiworld_http_client.cpp
 * HTTP Client Implementation
 */

#include "aiworld_http_client.hpp"
#include "aiworld_utils.hpp"
#include <httplib.h>
#include <thread>
#include <atomic>

namespace aiworld {

/**
 * Private implementation (PIMPL pattern)
 */
struct AIWorldHTTPClient::Impl {
    std::string base_url;
    std::string host;
    int port;
    int timeout_ms;
    int max_retries;
    
    // Persistent HTTP client with connection pooling
    std::shared_ptr<httplib::Client> persistent_client;
    mutable std::mutex client_mutex;
    
    // Statistics
    std::atomic<size_t> total_requests{0};
    std::atomic<size_t> successful_requests{0};
    std::atomic<size_t> failed_requests{0};
    std::atomic<size_t> retried_requests{0};
    std::atomic<double> total_latency_ms{0.0};
    
    mutable std::mutex stats_mutex;
    
    Impl(const std::string& url, int timeout, int retries)
        : base_url(url), timeout_ms(timeout), max_retries(retries) {
        parse_url(url);
        initialize_persistent_client();
    }
    
    void parse_url(const std::string& url) {
        // Simple URL parsing: http://host:port
        size_t protocol_end = url.find("://");
        if (protocol_end == std::string::npos) {
            host = "127.0.0.1";
            port = 8000;
            return;
        }
        
        size_t host_start = protocol_end + 3;
        size_t port_start = url.find(":", host_start);
        
        if (port_start != std::string::npos) {
            host = url.substr(host_start, port_start - host_start);
            port = std::stoi(url.substr(port_start + 1));
        } else {
            host = url.substr(host_start);
            port = 8000;
        }
    }
    
    void initialize_persistent_client() {
        std::lock_guard<std::mutex> lock(client_mutex);
        persistent_client = std::make_shared<httplib::Client>(host, port);
        
        // Enable HTTP keep-alive for connection reuse
        persistent_client->set_keep_alive(true);
        
        // Configure timeouts
        persistent_client->set_connection_timeout(0, timeout_ms * 1000);
        persistent_client->set_read_timeout(timeout_ms / 1000, (timeout_ms % 1000) * 1000);
        persistent_client->set_write_timeout(timeout_ms / 1000, (timeout_ms % 1000) * 1000);
        
        log_info("HTTP connection pool initialized with keep-alive");
    }
    
    std::shared_ptr<httplib::Client> get_client() {
        std::lock_guard<std::mutex> lock(client_mutex);
        
        // Verify client is still valid, recreate if needed
        if (!persistent_client) {
            log_warning("Persistent client lost, recreating...");
            persistent_client = std::make_shared<httplib::Client>(host, port);
            persistent_client->set_keep_alive(true);
            persistent_client->set_connection_timeout(0, timeout_ms * 1000);
            persistent_client->set_read_timeout(timeout_ms / 1000, (timeout_ms % 1000) * 1000);
            persistent_client->set_write_timeout(timeout_ms / 1000, (timeout_ms % 1000) * 1000);
        }
        
        return persistent_client;
    }
};

AIWorldHTTPClient::AIWorldHTTPClient(
    const std::string& base_url,
    int timeout_ms,
    int max_retries
) : pimpl_(std::make_unique<Impl>(base_url, timeout_ms, max_retries)) {
    log_info("AIWorldHTTPClient initialized: " + base_url);
}

AIWorldHTTPClient::~AIWorldHTTPClient() {
    log_info("AIWorldHTTPClient destroyed");
}

HTTPResponse AIWorldHTTPClient::get(const std::string& endpoint) {
    return execute_request("GET", endpoint);
}

HTTPResponse AIWorldHTTPClient::post(const std::string& endpoint, const nlohmann::json& json_body) {
    std::string body = json_body.dump();
    return execute_request("POST", endpoint, body);
}

HTTPResponse AIWorldHTTPClient::execute_request(
    const std::string& method,
    const std::string& endpoint,
    const std::string& body
) {
    HTTPResponse response;
    pimpl_->total_requests++;
    
    auto start_time = std::chrono::steady_clock::now();
    
    int attempt = 0;
    while (attempt <= pimpl_->max_retries) {
        try {
            // Reuse persistent client with connection pooling
            auto client = pimpl_->get_client();
            
            httplib::Result res;
            if (method == "GET") {
                res = client->Get(endpoint.c_str());
            } else if (method == "POST") {
                httplib::Headers headers = {
                    {"Content-Type", "application/json"}
                };
                res = client->Post(endpoint.c_str(), headers, body, "application/json");
            } else {
                response.error_message = "Unsupported HTTP method: " + method;
                break;
            }
            
            if (res) {
                response.status_code = res->status;
                response.body = res->body;
                response.success = (res->status >= 200 && res->status < 300);
                
                if (response.success) {
                    pimpl_->successful_requests++;
                    break;
                } else {
                    response.error_message = "HTTP " + std::to_string(res->status);
                }
            } else {
                auto err = res.error();
                response.error_message = "Connection error: " + httplib::to_string(err);
            }
            
        } catch (const std::exception& e) {
            response.error_message = "Exception: " + std::string(e.what());
        }
        
        // Retry logic
        if (!response.success && attempt < pimpl_->max_retries) {
            pimpl_->retried_requests++;
            attempt++;
            log_warning("Request failed, retrying (" + std::to_string(attempt) + "/" + 
                       std::to_string(pimpl_->max_retries) + "): " + response.error_message);
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * attempt));
        } else {
            break;
        }
    }
    
    // Update statistics
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    pimpl_->total_latency_ms += duration.count();
    
    if (!response.success) {
        pimpl_->failed_requests++;
        log_error("HTTP " + method + " " + endpoint + " failed: " + response.error_message);
    } else {
        log_debug("HTTP " + method + " " + endpoint + " succeeded (" + 
                 std::to_string(duration.count()) + "ms)");
    }
    
    return response;
}

bool AIWorldHTTPClient::is_healthy() {
    auto response = get("/health");
    return response.success;
}

AIWorldHTTPClient::Stats AIWorldHTTPClient::get_stats() const {
    Stats stats;
    stats.total_requests = pimpl_->total_requests.load();
    stats.successful_requests = pimpl_->successful_requests.load();
    stats.failed_requests = pimpl_->failed_requests.load();
    stats.retried_requests = pimpl_->retried_requests.load();
    
    if (stats.total_requests > 0) {
        stats.avg_latency_ms = pimpl_->total_latency_ms.load() / stats.total_requests;
    }
    
    return stats;
}

} // namespace aiworld