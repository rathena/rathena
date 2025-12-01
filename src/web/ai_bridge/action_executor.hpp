// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
/**
 * @file action_executor.hpp
 * @brief Action executor for AI service → game actions
 * 
 * Features:
 * - Polls for pending actions every 1 second
 * - Executes actions via existing rAthena APIs
 * - Reports execution results back to AI service
 * - Thread-safe with dedicated polling thread
 * - Validates actions before execution
 * - Comprehensive logging
 */

#ifndef ACTION_EXECUTOR_HPP
#define ACTION_EXECUTOR_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <common/cbasetypes.hpp>

#include "http_client.hpp"

namespace AIBridge {

/**
 * @brief Action types that can be executed
 */
enum class ActionType {
    SPAWN_MONSTER,     ///< Spawn monster at location
    SPAWN_NPC,         ///< Create dynamic NPC
    GIVE_REWARD,       ///< Give item/exp to player
    BROADCAST,         ///< Server-wide announcement
    CREATE_QUEST,      ///< Create dynamic quest
    SET_MAP_FLAG,      ///< Set map property (hazard, weather)
    MOVE_NPC,          ///< Move NPC to new location
    REMOVE_NPC,        ///< Remove dynamic NPC
    UPDATE_ECONOMY,    ///< Update shop prices
    UNKNOWN            ///< Unknown/invalid action
};

/**
 * @brief AI action structure
 */
struct AIAction {
    std::string action_id;  ///< Unique action ID from AI service
    ActionType type{ActionType::UNKNOWN};
    std::string payload;    ///< JSON payload with action parameters
    uint32 target_player_id{0};  ///< Optional player target
    std::string target_map;      ///< Optional map target
    int32 priority{0};           ///< Action priority
    
    /**
     * @brief Parse action from JSON string
     * @param json_str JSON string
     * @return true on success
     */
    bool from_json(const std::string& json_str);
    
    /**
     * @brief Convert action result to JSON
     * @param success Execution result
     * @param message Result message
     * @return JSON string
     */
    std::string result_to_json(bool success, const std::string& message) const;
};

/**
 * @brief Action executor statistics
 */
struct ActionExecutorStats {
    size_t total_polled{0};
    size_t total_executed{0};
    size_t total_failed{0};
    size_t current_pending{0};
    double avg_execution_time_ms{0.0};
    int64_t last_poll_time_ms{0};
};

/**
 * @brief Action executor class
 * 
 * Polls AI service for pending actions and executes them in-game
 * Thread-safe with dedicated polling thread
 */
class ActionExecutor {
public:
    /**
     * @brief Construct action executor
     * @param http_client HTTP client for requests
     * @param poll_interval_ms Time between polls (default: 1000ms)
     * @param max_concurrent Maximum concurrent executions (default: 5)
     */
    explicit ActionExecutor(
        std::shared_ptr<HTTPClient> http_client,
        int64_t poll_interval_ms = 1000,
        size_t max_concurrent = 5
    );
    
    ~ActionExecutor();
    
    /**
     * @brief Start executor thread
     * @return true on success
     */
    bool start();
    
    /**
     * @brief Stop executor thread
     */
    void stop();
    
    /**
     * @brief Get executor statistics
     * @return ActionExecutorStats
     */
    ActionExecutorStats get_stats() const;
    
    /**
     * @brief Check if executor is running
     * @return true if running
     */
    bool is_running() const;
    
    /**
     * @brief Manually execute a single action (blocking)
     * @param action Action to execute
     * @return true on success
     */
    bool execute_action(const AIAction& action);
    
private:
    void polling_thread_func();
    std::vector<AIAction> poll_actions();
    bool execute_spawn_monster(const AIAction& action);
    bool execute_spawn_npc(const AIAction& action);
    bool execute_give_reward(const AIAction& action);
    bool execute_broadcast(const AIAction& action);
    bool execute_create_quest(const AIAction& action);
    bool execute_set_map_flag(const AIAction& action);
    bool execute_move_npc(const AIAction& action);
    bool execute_remove_npc(const AIAction& action);
    bool execute_update_economy(const AIAction& action);
    bool report_execution_result(const AIAction& action, bool success, const std::string& message);
    
    std::shared_ptr<HTTPClient> http_client_;
    const int64_t poll_interval_ms_;
    const size_t max_concurrent_;
    
    // Polling thread
    std::unique_ptr<std::thread> polling_thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> should_stop_{false};
    
    // Statistics
    std::atomic<size_t> total_polled_{0};
    std::atomic<size_t> total_executed_{0};
    std::atomic<size_t> total_failed_{0};
    std::atomic<int64_t> total_execution_time_ms_{0};
    std::atomic<int64_t> last_poll_time_ms_{0};
    
    mutable std::mutex stats_mutex_;
};

} // namespace AIBridge

#endif // ACTION_EXECUTOR_HPP