// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
/**
 * @file event_dispatcher.hpp
 * @brief Event dispatcher for game events → AI service communication
 * 
 * Features:
 * - Circular buffer queue (1000 events max)
 * - Batch dispatch (50 events OR 1 second intervals)
 * - Priority queue (urgent events sent immediately)
 * - Async dispatch (non-blocking)
 * - Dead letter queue for failed events
 * - Thread-safe operations
 */

#ifndef EVENT_DISPATCHER_HPP
#define EVENT_DISPATCHER_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "http_client.hpp"

namespace AIBridge {

/**
 * @brief Game event types
 */
enum class GameEventType {
    PLAYER_LOGIN,      ///< Player logs in
    PLAYER_LOGOUT,     ///< Player logs out
    NPC_INTERACTION,   ///< Player talks to NPC
    MONSTER_KILL,      ///< Monster defeated
    MVP_KILL,          ///< MVP boss defeated
    DAILY_RESET,       ///< Server daily reset (00:00)
    MAP_CHANGE,        ///< Player changes map
    ITEM_DROP,         ///< Item dropped
    QUEST_COMPLETE,    ///< Quest completed
    TRADE_COMPLETE,    ///< Trade between players
    CUSTOM             ///< Custom event type
};

/**
 * @brief Event priority levels
 */
enum class EventPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    URGENT = 3
};

/**
 * @brief Game event structure
 */
struct GameEvent {
    GameEventType type;
    EventPriority priority{EventPriority::NORMAL};
    std::string payload;  ///< JSON payload
    int64_t timestamp_ms{0};
    uint32 player_id{0};  ///< Optional player context
    std::string map_name; ///< Optional map context
    
    /**
     * @brief Convert event to JSON string
     * @return JSON representation
     */
    std::string to_json() const;
};

/**
 * @brief Event queue statistics
 */
struct EventQueueStats {
    size_t current_size{0};
    size_t total_queued{0};
    size_t total_dispatched{0};
    size_t total_failed{0};
    size_t total_batches{0};
    double avg_batch_size{0.0};
    double avg_dispatch_latency_ms{0.0};
};

/**
 * @brief Event dispatcher class
 * 
 * Collects game events and dispatches them to AI service in batches
 * Thread-safe with dedicated dispatch thread
 */
class EventDispatcher {
public:
    /**
     * @brief Construct event dispatcher
     * @param http_client HTTP client for requests
     * @param batch_size Maximum events per batch (default: 50)
     * @param batch_interval_ms Time between batches (default: 1000ms)
     * @param queue_size Maximum queue size (default: 1000)
     */
    explicit EventDispatcher(
        std::shared_ptr<HTTPClient> http_client,
        size_t batch_size = 50,
        int64_t batch_interval_ms = 1000,
        size_t queue_size = 1000
    );
    
    ~EventDispatcher();
    
    /**
     * @brief Start dispatcher thread
     * @return true on success
     */
    bool start();
    
    /**
     * @brief Stop dispatcher thread
     */
    void stop();
    
    /**
     * @brief Queue a game event for dispatch
     * @param event Event to queue
     * @return true if queued, false if queue full
     */
    bool queue_event(const GameEvent& event);
    
    /**
     * @brief Queue multiple events
     * @param events Vector of events
     * @return Number of events successfully queued
     */
    size_t queue_events(const std::vector<GameEvent>& events);
    
    /**
     * @brief Get queue statistics
     * @return EventQueueStats
     */
    EventQueueStats get_stats() const;
    
    /**
     * @brief Get current queue size
     * @return Number of events in queue
     */
    size_t get_queue_size() const;
    
    /**
     * @brief Check if dispatcher is running
     * @return true if running
     */
    bool is_running() const;
    
    /**
     * @brief Flush all pending events immediately
     */
    void flush();
    
    /**
     * @brief Set dead letter queue file path
     * @param filepath Path to DLQ file
     */
    void set_dlq_path(const std::string& filepath);
    
private:
    void dispatch_thread_func();
    bool dispatch_batch(const std::vector<GameEvent>& batch);
    void save_to_dlq(const std::vector<GameEvent>& events);
    std::string build_batch_json(const std::vector<GameEvent>& batch) const;
    
    std::shared_ptr<HTTPClient> http_client_;
    const size_t batch_size_;
    const int64_t batch_interval_ms_;
    const size_t queue_size_;
    
    // Event queue (circular buffer)
    std::deque<GameEvent> queue_;
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    // Dispatcher thread
    std::unique_ptr<std::thread> dispatch_thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> should_stop_{false};
    
    // Dead letter queue
    std::string dlq_path_;
    mutable std::mutex dlq_mutex_;
    
    // Statistics
    std::atomic<size_t> total_queued_{0};
    std::atomic<size_t> total_dispatched_{0};
    std::atomic<size_t> total_failed_{0};
    std::atomic<size_t> total_batches_{0};
    std::atomic<size_t> total_dropped_{0};
    std::atomic<int64_t> total_dispatch_latency_ms_{0};
};

/**
 * @brief Helper functions for event creation
 */
namespace EventFactory {
    GameEvent create_player_login(uint32 player_id, const std::string& map_name);
    GameEvent create_player_logout(uint32 player_id);
    GameEvent create_monster_kill(uint32 player_id, uint32 monster_id, const std::string& map_name);
    GameEvent create_mvp_kill(uint32 player_id, uint32 mvp_id, const std::string& map_name);
    GameEvent create_daily_reset();
    GameEvent create_map_change(uint32 player_id, const std::string& from_map, const std::string& to_map);
    GameEvent create_npc_interaction(uint32 player_id, uint32 npc_id, const std::string& map_name);
    GameEvent create_quest_complete(uint32 player_id, uint32 quest_id);
    GameEvent create_custom(const std::string& event_type, const std::string& payload, EventPriority priority = EventPriority::NORMAL);
}

} // namespace AIBridge

#endif // EVENT_DISPATCHER_HPP