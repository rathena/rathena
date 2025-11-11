// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ai_dialogue_queue.hpp"

#include <chrono>

#include <common/showmsg.hpp>

/**
 * Constructor
 */
AIDialogueQueue::AIDialogueQueue() 
    : total_requests_queued(0)
    , total_responses_queued(0)
    , total_requests_dropped(0)
{
    ShowStatus("AI Dialogue Queue: Initialized\n");
}

/**
 * Destructor
 */
AIDialogueQueue::~AIDialogueQueue() {
    clear_all();
    ShowStatus("AI Dialogue Queue: Shutdown complete\n");
}

/**
 * Push a request to the queue (thread-safe)
 * @param req The request to queue
 * @return true if queued successfully, false if queue is full
 */
bool AIDialogueQueue::push_request(const AIDialogueRequest& req) {
    std::lock_guard<std::mutex> lock(request_mutex);
    
    if (request_queue.size() >= MAX_REQUEST_QUEUE_SIZE) {
        ShowWarning("AI Dialogue Queue: Request queue full! Dropping request from char_id=%u\n", req.char_id);
        total_requests_dropped++;
        return false;
    }
    
    request_queue.push(req);
    total_requests_queued++;
    
    // Notify waiting worker threads
    request_cv.notify_one();
    
    ShowDebug("AI Dialogue Queue: Queued request from char_id=%u (queue size: %zu)\n", 
              req.char_id, request_queue.size());
    
    return true;
}

/**
 * Pop a request from the queue (thread-safe, blocking with timeout)
 * @param req Output parameter for the popped request
 * @param timeout_ms Maximum time to wait in milliseconds
 * @return true if request was popped, false if timeout or queue empty
 */
bool AIDialogueQueue::pop_request(AIDialogueRequest& req, int timeout_ms) {
    std::unique_lock<std::mutex> lock(request_mutex);
    
    // Wait for request or timeout
    if (request_queue.empty()) {
        auto timeout = std::chrono::milliseconds(timeout_ms);
        if (!request_cv.wait_for(lock, timeout, [this] { return !request_queue.empty(); })) {
            return false; // Timeout
        }
    }
    
    if (request_queue.empty()) {
        return false;
    }
    
    req = request_queue.front();
    request_queue.pop();
    
    ShowDebug("AI Dialogue Queue: Popped request for char_id=%u (queue size: %zu)\n", 
              req.char_id, request_queue.size());
    
    return true;
}

/**
 * Get current request queue size (thread-safe)
 */
size_t AIDialogueQueue::request_queue_size() {
    std::lock_guard<std::mutex> lock(request_mutex);
    return request_queue.size();
}

/**
 * Push a response to the queue (thread-safe)
 * @param resp The response to queue
 * @return true if queued successfully, false if queue is full
 */
bool AIDialogueQueue::push_response(const AIDialogueResponse& resp) {
    std::lock_guard<std::mutex> lock(response_mutex);
    
    if (response_queue.size() >= MAX_RESPONSE_QUEUE_SIZE) {
        ShowWarning("AI Dialogue Queue: Response queue full! Dropping response for char_id=%u\n", resp.char_id);
        return false;
    }
    
    response_queue.push(resp);
    total_responses_queued++;
    
    ShowDebug("AI Dialogue Queue: Queued response for char_id=%u (queue size: %zu)\n", 
              resp.char_id, response_queue.size());
    
    return true;
}

/**
 * Pop a response from the queue (thread-safe, non-blocking)
 * @param resp Output parameter for the popped response
 * @return true if response was popped, false if queue empty
 */
bool AIDialogueQueue::pop_response(AIDialogueResponse& resp) {
    std::lock_guard<std::mutex> lock(response_mutex);
    
    if (response_queue.empty()) {
        return false;
    }
    
    resp = response_queue.front();
    response_queue.pop();
    
    ShowDebug("AI Dialogue Queue: Popped response for char_id=%u (queue size: %zu)\n", 
              resp.char_id, response_queue.size());
    
    return true;
}

/**
 * Get current response queue size (thread-safe)
 */
size_t AIDialogueQueue::response_queue_size() {
    std::lock_guard<std::mutex> lock(response_mutex);
    return response_queue.size();
}

/**
 * Get total requests queued since startup
 */
uint64 AIDialogueQueue::get_total_requests_queued() const {
    return total_requests_queued.load();
}

/**
 * Get total responses queued since startup
 */
uint64 AIDialogueQueue::get_total_responses_queued() const {
    return total_responses_queued.load();
}

/**
 * Get total requests dropped due to queue full
 */
uint64 AIDialogueQueue::get_total_requests_dropped() const {
    return total_requests_dropped.load();
}

/**
 * Clear all queues (thread-safe)
 */
void AIDialogueQueue::clear_all() {
    {
        std::lock_guard<std::mutex> lock(request_mutex);
        while (!request_queue.empty()) {
            request_queue.pop();
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(response_mutex);
        while (!response_queue.empty()) {
            response_queue.pop();
        }
    }
    
    ShowStatus("AI Dialogue Queue: All queues cleared\n");
}

/**
 * Check if request queue is full
 */
bool AIDialogueQueue::is_request_queue_full() const {
    return request_queue.size() >= MAX_REQUEST_QUEUE_SIZE;
}

/**
 * Check if response queue is full
 */
bool AIDialogueQueue::is_response_queue_full() const {
    return response_queue.size() >= MAX_RESPONSE_QUEUE_SIZE;
}

