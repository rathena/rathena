// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ai_dialogue_worker.hpp"
#include "ai_dialogue_queue.hpp"
#include "../aiworld/aiworld_ipc.hpp"
#include "../aiworld/aiworld_utils.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <sstream>
#include <atomic>
#include <vector>
#include <string>

#include <common/showmsg.hpp>
#include <common/timer.hpp>

using json = nlohmann::json;
using namespace aiworld;

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
    , zmq_client(nullptr)
{
    if (!queue) {
        ShowError("AI Dialogue Worker: Queue pointer is null!\n");
        return;
    }
    zmq_client = std::make_unique<AIWorldIPCClient>(config.zmq_endpoint);
    if (!zmq_client->connect()) {
        ShowError("AI Dialogue Worker: Failed to connect to AIWorld server via ZeroMQ (%s)\n", config.zmq_endpoint.c_str());
    }
    ShowStatus("AI Dialogue Worker: Initialized with %d threads (ZeroMQ: %s)\n", 
               config.num_threads, config.zmq_endpoint.c_str());
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
        if (!queue->pop_request(req, 1000)) {
            continue;
        }
        if (config.debug_logging) {
            ShowDebug("AI Dialogue Worker %d: Processing request from char_id=%u\n", 
                      worker_id, req.char_id);
        }
        AIDialogueResponse resp = process_request(req);
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
 * Process a single AI dialogue request (ZeroMQ IPC)
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
        std::string ai_response = call_aiworld_server(req, success, error_msg);
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
        if (should_retry(retry_count, error_msg)) {
            retry_count++;
            total_retries++;
            ShowWarning("AI Dialogue Worker: Request failed for char_id=%u, retrying (%d/%d): %s\n",
                        req.char_id, retry_count, config.max_retries, error_msg.c_str());
            apply_retry_delay(retry_count);
        } else {
            resp.success = false;
            resp.npc_response = "";
            resp.error_message = error_msg;
            ShowWarning("AI Dialogue Worker: Request failed for char_id=%u (no retry): %s\n",
                        req.char_id, error_msg.c_str());
            return resp;
        }
    }
    resp.success = false;
    resp.npc_response = "";
    resp.error_message = "Maximum retry attempts exceeded";
    ShowError("AI Dialogue Worker: Max retries exceeded for char_id=%u\n", req.char_id);
    return resp;
}

/**
 * Call AIWorld server via ZeroMQ IPC (replaces HTTP)
 */
std::string AIDialogueWorker::call_aiworld_server(const AIDialogueRequest& req, bool& success, std::string& error_msg) {
    success = false;
    error_msg = "";
    if (!zmq_client || !zmq_client->is_connected()) {
        error_msg = "ZeroMQ client not connected";
        return "";
    }
    // Build JSON payload
    json payload;
    payload["npc_id"] = req.npc_id;
    payload["char_id"] = req.char_id;
    payload["player_message"] = req.player_message;
    payload["source"] = "npc_dialogue";
    payload["account_id"] = req.account_id;
    payload["request_time"] = req.request_time;
    payload["type"] = "dialogue";
    // AIWorldMessage now expects IPCMessageType (enum class) as first argument
    // No cast needed, constructor signature matches
    AIWorldMessage msg(IPCMessageType::AI_ACTION_REQUEST, generate_correlation_id(), payload);
    if (!zmq_client->send_message(msg)) {
        error_msg = "ZeroMQ send failed";
        return "";
    }
    // Wait for response (blocking)
    AIWorldMessage resp_msg;
    if (!zmq_client->receive_message(resp_msg, true)) {
        error_msg = "ZeroMQ receive failed";
        return "";
    }
    try {
        auto& resp_payload = resp_msg.payload;
        if (resp_payload.contains("payload") && resp_payload["payload"].contains("dialogue")) {
            success = true;
            return resp_payload["payload"]["dialogue"].get<std::string>();
        } else if (resp_payload.contains("payload") && resp_payload["payload"].contains("message")) {
            success = true;
            return resp_payload["payload"]["message"].get<std::string>();
        } else {
            error_msg = "Invalid response format: missing dialogue/message";
            return "";
        }
    } catch (const std::exception& e) {
        error_msg = std::string("JSON parse error: ") + e.what();
        return "";
    }
}

/**
 * Retry logic
 */
bool AIDialogueWorker::should_retry(int retry_count, const std::string& error_msg) {
    if (retry_count >= config.max_retries) {
        return false;
    }
    if (error_msg.find("timeout") != std::string::npos ||
        error_msg.find("connection") != std::string::npos ||
        error_msg.find("ZeroMQ") != std::string::npos) {
        return true;
    }
    return false;
}

/**
 * Apply exponential backoff delay before retry
 */
void AIDialogueWorker::apply_retry_delay(int retry_count) {
    int delay_ms = config.retry_delay_ms * (1 << (retry_count - 1));
    if (delay_ms > 10000) delay_ms = 10000;
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
    if (total == 0) return 0.0;
    uint64 succeeded = total_requests_succeeded.load();
    return (double)succeeded / (double)total;
}