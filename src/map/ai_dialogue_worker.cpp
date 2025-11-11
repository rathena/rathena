// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ai_dialogue_worker.hpp"
#include "ai_dialogue_queue.hpp"

#include <chrono>
#include <thread>
#include <sstream>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <common/showmsg.hpp>
#include <common/timer.hpp>

using json = nlohmann::json;

/**
 * Constructor
 */
AIDialogueWorker::AIDialogueWorker(AIDialogueQueue* q, const AIDialogueWorkerConfig& cfg)
    : running(false)
    , queue(q)
    , config(cfg)
    , total_requests_processed(0)
    , total_requests_succeeded(0)
    , total_requests_failed(0)
    , total_retries(0)
{
    if (!queue) {
        ShowError("AI Dialogue Worker: Queue pointer is null!\n");
        return;
    }
    
    ShowStatus("AI Dialogue Worker: Initialized with %d threads (bridge: %s:%d)\n", 
               config.num_threads, config.bridge_url.c_str(), config.bridge_port);
}

/**
 * Destructor
 */
AIDialogueWorker::~AIDialogueWorker() {
    stop();
    ShowStatus("AI Dialogue Worker: Shutdown complete\n");
}

/**
 * Start worker threads
 */
void AIDialogueWorker::start() {
    if (running.load()) {
        ShowWarning("AI Dialogue Worker: Already running\n");
        return;
    }
    
    running.store(true);
    
    // Create worker threads
    for (int i = 0; i < config.num_threads; i++) {
        worker_threads.emplace_back(&AIDialogueWorker::worker_loop, this, i);
    }
    
    ShowStatus("AI Dialogue Worker: Started %d worker threads\n", config.num_threads);
}

/**
 * Stop worker threads
 */
void AIDialogueWorker::stop() {
    if (!running.load()) {
        return;
    }
    
    ShowStatus("AI Dialogue Worker: Stopping worker threads...\n");
    running.store(false);
    
    // Wait for all threads to finish
    for (auto& thread : worker_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    worker_threads.clear();
    
    ShowStatus("AI Dialogue Worker: All worker threads stopped\n");
}

/**
 * Check if worker is running
 */
bool AIDialogueWorker::is_running() const {
    return running.load();
}

/**
 * Worker thread main loop
 */
void AIDialogueWorker::worker_loop(int worker_id) {
    ShowInfo("AI Dialogue Worker: Thread %d started\n", worker_id);
    
    while (running.load()) {
        AIDialogueRequest req;
        
        // Try to get a request from queue (with timeout)
        if (!queue->pop_request(req, 1000)) {
            // Timeout or queue empty, continue loop
            continue;
        }
        
        if (config.debug_logging) {
            ShowDebug("AI Dialogue Worker %d: Processing request from char_id=%u\n", 
                      worker_id, req.char_id);
        }
        
        // Process the request
        AIDialogueResponse resp = process_request(req);
        
        // Queue the response
        if (!queue->push_response(resp)) {
            ShowWarning("AI Dialogue Worker %d: Failed to queue response for char_id=%u\n", 
                        worker_id, req.char_id);
        }
        
        total_requests_processed++;
        
        if (resp.success) {
            total_requests_succeeded++;
        } else {
            total_requests_failed++;
        }
    }
    
    ShowInfo("AI Dialogue Worker: Thread %d stopped\n", worker_id);
}

/**
 * Process a single AI dialogue request
 */
AIDialogueResponse AIDialogueWorker::process_request(const AIDialogueRequest& req) {
    AIDialogueResponse resp;
    resp.account_id = req.account_id;
    resp.char_id = req.char_id;
    resp.npc_id = req.npc_id;
    resp.request_time = req.request_time;
    resp.response_time = gettick();
    resp.success = false;
    
    int retry_count = 0;
    
    while (retry_count <= config.max_retries) {
        bool success = false;
        std::string error_msg;
        
        // Call AI service
        std::string ai_response = call_ai_service(
            req.npc_name, 
            req.char_id, 
            req.player_message, 
            success, 
            error_msg
        );
        
        if (success) {
            resp.success = true;
            resp.npc_response = ai_response;
            resp.error_message = "";
            
            if (config.debug_logging) {
                t_tick latency = resp.response_time - req.request_time;
                ShowDebug("AI Dialogue Worker: Request succeeded for char_id=%u (latency: %llums)\n", 
                          req.char_id, latency);
            }
            
            return resp;
        }

        // Request failed, check if we should retry
        if (should_retry(retry_count, error_msg)) {
            retry_count++;
            total_retries++;

            ShowWarning("AI Dialogue Worker: Request failed for char_id=%u, retrying (%d/%d): %s\n",
                        req.char_id, retry_count, config.max_retries, error_msg.c_str());

            apply_retry_delay(retry_count);
        } else {
            // Don't retry, return error response
            resp.success = false;
            resp.npc_response = "";
            resp.error_message = error_msg;

            ShowWarning("AI Dialogue Worker: Request failed for char_id=%u (no retry): %s\n",
                        req.char_id, error_msg.c_str());

            return resp;
        }
    }

    // Max retries exceeded
    resp.success = false;
    resp.npc_response = "";
    resp.error_message = "Maximum retry attempts exceeded";

    ShowError("AI Dialogue Worker: Max retries exceeded for char_id=%u\n", req.char_id);

    return resp;
}

/**
 * Call AI service via HTTP
 */
std::string AIDialogueWorker::call_ai_service(const std::string& npc_name, uint32 char_id,
                                               const std::string& message, bool& success,
                                               std::string& error_msg) {
    success = false;
    error_msg = "";

    try {
        // Create HTTP client
        httplib::Client client(config.bridge_url.c_str(), config.bridge_port);
        client.set_connection_timeout(0, config.request_timeout_ms * 1000); // seconds, microseconds
        client.set_read_timeout(config.request_timeout_ms / 1000, 0);
        client.set_write_timeout(config.request_timeout_ms / 1000, 0);

        // Build JSON request
        json request_json;
        request_json["npc_id"] = npc_name;
        request_json["char_id"] = char_id;
        request_json["message"] = message;
        request_json["source"] = "npc_dialogue";

        std::string request_body = request_json.dump();

        if (config.debug_logging) {
            ShowDebug("AI Dialogue Worker: Sending request to %s:%d/ai/chat/command\n",
                      config.bridge_url.c_str(), config.bridge_port);
            ShowDebug("AI Dialogue Worker: Request body: %s\n", request_body.c_str());
        }

        // Make HTTP POST request
        auto res = client.Post("/ai/chat/command", request_body, "application/json");

        if (!res) {
            error_msg = "HTTP request failed: " + httplib::to_string(res.error());
            return "";
        }

        if (res->status != 200) {
            std::ostringstream oss;
            oss << "HTTP " << res->status << ": " << res->body;
            error_msg = oss.str();
            return "";
        }

        // Parse JSON response
        json response_json = json::parse(res->body);

        if (config.debug_logging) {
            ShowDebug("AI Dialogue Worker: Response: %s\n", res->body.c_str());
        }

        // Extract dialogue text
        if (response_json.contains("response") && response_json["response"].is_string()) {
            success = true;
            return response_json["response"].get<std::string>();
        } else if (response_json.contains("dialogue") && response_json["dialogue"].is_string()) {
            success = true;
            return response_json["dialogue"].get<std::string>();
        } else if (response_json.contains("message") && response_json["message"].is_string()) {
            success = true;
            return response_json["message"].get<std::string>();
        } else {
            error_msg = "Invalid response format: missing dialogue text";
            return "";
        }

    } catch (const json::exception& e) {
        error_msg = std::string("JSON error: ") + e.what();
        return "";
    } catch (const std::exception& e) {
        error_msg = std::string("Exception: ") + e.what();
        return "";
    }
}

/**
 * Check if request should be retried
 */
bool AIDialogueWorker::should_retry(int retry_count, const std::string& error_msg) {
    if (retry_count >= config.max_retries) {
        return false;
    }

    // Retry on network errors, timeouts, and 5xx server errors
    if (error_msg.find("timeout") != std::string::npos ||
        error_msg.find("connection") != std::string::npos ||
        error_msg.find("HTTP 5") != std::string::npos) {
        return true;
    }

    // Don't retry on client errors (4xx)
    if (error_msg.find("HTTP 4") != std::string::npos) {
        return false;
    }

    // Retry on other errors
    return true;
}

/**
 * Apply exponential backoff delay before retry
 */
void AIDialogueWorker::apply_retry_delay(int retry_count) {
    // Exponential backoff: base_delay * 2^(retry_count - 1)
    int delay_ms = config.retry_delay_ms * (1 << (retry_count - 1));

    // Cap at 10 seconds
    if (delay_ms > 10000) {
        delay_ms = 10000;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
}

/**
 * Set configuration
 */
void AIDialogueWorker::set_config(const AIDialogueWorkerConfig& cfg) {
    config = cfg;
    ShowInfo("AI Dialogue Worker: Configuration updated\n");
}

/**
 * Get configuration
 */
AIDialogueWorkerConfig AIDialogueWorker::get_config() const {
    return config;
}

/**
 * Get total requests processed
 */
uint64 AIDialogueWorker::get_total_requests_processed() const {
    return total_requests_processed.load();
}

/**
 * Get total successful requests
 */
uint64 AIDialogueWorker::get_total_requests_succeeded() const {
    return total_requests_succeeded.load();
}

/**
 * Get total failed requests
 */
uint64 AIDialogueWorker::get_total_requests_failed() const {
    return total_requests_failed.load();
}

/**
 * Get total retry attempts
 */
uint64 AIDialogueWorker::get_total_retries() const {
    return total_retries.load();
}

/**
 * Get success rate (0.0 to 1.0)
 */
double AIDialogueWorker::get_success_rate() const {
    uint64 total = total_requests_processed.load();
    if (total == 0) {
        return 0.0;
    }

    uint64 succeeded = total_requests_succeeded.load();
    return (double)succeeded / (double)total;
}

