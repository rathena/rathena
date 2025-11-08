// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef AI_DIALOGUE_QUEUE_HPP
#define AI_DIALOGUE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <atomic>

#include <common/cbasetypes.hpp>
#include <common/timer.hpp>

/**
 * AI Dialogue Request Structure
 * Represents a player's text input to an AI-enabled NPC
 */
struct AIDialogueRequest {
    uint32 account_id;          // Player's account ID
    uint32 char_id;             // Player's character ID
    uint32 npc_id;              // NPC's game ID
    std::string npc_name;       // NPC identifier for AI service
    std::string player_message; // Player's text input
    t_tick request_time;        // When request was created
    int32 retry_count;          // Number of retry attempts
};

/**
 * AI Dialogue Response Structure
 * Represents the AI service's response to a player's message
 */
struct AIDialogueResponse {
    uint32 account_id;          // Player's account ID
    uint32 char_id;             // Player's character ID
    uint32 npc_id;              // NPC's game ID
    std::string npc_response;   // AI-generated dialogue text
    std::string emotion;        // Optional emotion/emote
    bool success;               // Whether request succeeded
    std::string error_message;  // Error details if failed
    t_tick response_time;       // When response was generated
    t_tick request_time;        // Original request time (for latency tracking)
};

/**
 * Thread-Safe AI Dialogue Queue System
 * Manages request/response queues for asynchronous AI dialogue processing
 */
class AIDialogueQueue {
private:
    // Request queue (player input waiting to be processed)
    std::queue<AIDialogueRequest> request_queue;
    std::mutex request_mutex;
    std::condition_variable request_cv;
    
    // Response queue (AI responses waiting to be sent to players)
    std::queue<AIDialogueResponse> response_queue;
    std::mutex response_mutex;
    
    // Queue size limits
    static const size_t MAX_REQUEST_QUEUE_SIZE = 1000;
    static const size_t MAX_RESPONSE_QUEUE_SIZE = 1000;
    
    // Statistics
    std::atomic<uint64> total_requests_queued;
    std::atomic<uint64> total_responses_queued;
    std::atomic<uint64> total_requests_dropped;
    
public:
    AIDialogueQueue();
    ~AIDialogueQueue();
    
    // Request queue operations
    bool push_request(const AIDialogueRequest& req);
    bool pop_request(AIDialogueRequest& req, int timeout_ms = 1000);
    size_t request_queue_size();
    
    // Response queue operations
    bool push_response(const AIDialogueResponse& resp);
    bool pop_response(AIDialogueResponse& resp);
    size_t response_queue_size();
    
    // Statistics
    uint64 get_total_requests_queued() const;
    uint64 get_total_responses_queued() const;
    uint64 get_total_requests_dropped() const;
    
    // Utility
    void clear_all();
    bool is_request_queue_full() const;
    bool is_response_queue_full() const;
};

#endif /* AI_DIALOGUE_QUEUE_HPP */

