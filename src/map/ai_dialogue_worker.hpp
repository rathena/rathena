// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef AI_DIALOGUE_WORKER_HPP
#define AI_DIALOGUE_WORKER_HPP

#include <thread>
#include <vector>
#include <atomic>
#include <string>
#include <memory>
#include "../aiworld/aiworld_ipc.hpp"

#include <common/cbasetypes.hpp>

// Forward declarations
class AIDialogueQueue;
struct AIDialogueRequest;
struct AIDialogueResponse;

/**
 * AI Dialogue Worker Configuration
 */
struct AIDialogueWorkerConfig {
    std::string zmq_endpoint;      // ZeroMQ endpoint for AIWorld server (e.g., "tcp://127.0.0.1:5555")
    int32 num_threads;             // Number of worker threads (4-8 recommended)
    int32 request_timeout_ms;      // Request timeout in milliseconds
    int32 max_retries;             // Maximum retry attempts for failed requests
    int32 retry_delay_ms;          // Delay between retries in milliseconds
    bool debug_logging;            // Enable verbose debug logging
};

/**
 * AI Dialogue Worker Thread Pool
 * Manages background threads that process AI dialogue requests via ZeroMQ IPC
 */
class AIDialogueWorker {
private:
    // Worker threads
    std::vector<std::thread> worker_threads;
    std::atomic<bool> running;

    // Queue reference (not owned)
    AIDialogueQueue* queue;

    // Configuration
    AIDialogueWorkerConfig config;

    // Statistics
    std::atomic<uint64> total_requests_processed;
    std::atomic<uint64> total_requests_succeeded;
    std::atomic<uint64> total_requests_failed;
    std::atomic<uint64> total_retries;

    // ZeroMQ IPC client
    std::unique_ptr<aiworld::AIWorldIPCClient> zmq_client;

    // Worker thread main loop
    void worker_loop(int worker_id);

    // Process a single request
    AIDialogueResponse process_request(const AIDialogueRequest& req);

    // Call AIWorld server via ZeroMQ IPC
    std::string call_aiworld_server(const AIDialogueRequest& req, bool& success, std::string& error_msg);

    // Retry logic
    bool should_retry(int retry_count, const std::string& error_msg);
    void apply_retry_delay(int retry_count);

public:
    AIDialogueWorker(AIDialogueQueue* q, const AIDialogueWorkerConfig& cfg);
    ~AIDialogueWorker();

    // Lifecycle
    void start();
    void stop();
    bool is_running() const;

    // Configuration
    void set_config(const AIDialogueWorkerConfig& cfg);
    AIDialogueWorkerConfig get_config() const;

    // Statistics
    uint64 get_total_requests_processed() const;
    uint64 get_total_requests_succeeded() const;
    uint64 get_total_requests_failed() const;
    uint64 get_total_retries() const;
    double get_success_rate() const;
};

#endif /* AI_DIALOGUE_WORKER_HPP */