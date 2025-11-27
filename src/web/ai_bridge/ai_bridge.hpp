// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
/**
 * @file ai_bridge.hpp
 * @brief Main Bridge Layer coordinator
 * 
 * Coordinates all Bridge Layer components:
 * - HTTP Client with connection pooling and circuit breaker
 * - Event Dispatcher for game → AI events
 * - Action Executor for AI → game actions
 * - State Synchronizer for bidirectional sync
 * - Configuration Manager
 * 
 * Provides unified interface for NPC scripts and game code
 */

#ifndef AI_BRIDGE_HPP
#define AI_BRIDGE_HPP

#include <memory>
#include <string>

#include <common/cbasetypes.hpp>

#include "action_executor.hpp"
#include "config_manager.hpp"
#include "event_dispatcher.hpp"
#include "http_client.hpp"

namespace AIBridge {

/**
 * @brief Bridge Layer coordinator (Singleton)
 * 
 * Main entry point for all Bridge Layer functionality
 * Manages lifecycle of all sub-components
 */
class BridgeLayer {
public:
    /**
     * @brief Get singleton instance
     * @return BridgeLayer reference
     */
    static BridgeLayer& instance();
    
    /**
     * @brief Initialize Bridge Layer
     * @param config_path Path to configuration file
     * @return true on success
     */
    bool initialize(const std::string& config_path = "conf/ai_bridge.conf");
    
    /**
     * @brief Shutdown Bridge Layer
     */
    void shutdown();
    
    /**
     * @brief Check if initialized
     * @return true if initialized
     */
    bool is_initialized() const;
    
    /**
     * @brief Get HTTP client
     * @return Shared pointer to HTTP client
     */
    std::shared_ptr<HTTPClient> get_http_client() const;
    
    /**
     * @brief Get event dispatcher
     * @return Shared pointer to event dispatcher
     */
    std::shared_ptr<EventDispatcher> get_event_dispatcher() const;
    
    /**
     * @brief Get action executor
     * @return Shared pointer to action executor
     */
    std::shared_ptr<ActionExecutor> get_action_executor() const;
    
    /**
     * @brief Get configuration manager
     * @return Reference to configuration manager
     */
    ConfigManager& get_config() const;
    
    // ========================================================================
    // Convenience functions for NPC scripts
    // ========================================================================
    
    /**
     * @brief Simple HTTP GET (synchronous)
     * @param url Full URL or endpoint path
     * @param timeout_ms Optional timeout override
     * @return Response body or empty string on error
     */
    std::string http_get(const std::string& url, int64_t timeout_ms = 0);
    
    /**
     * @brief Simple HTTP POST (synchronous)
     * @param url Full URL or endpoint path
     * @param json_body JSON request body
     * @param timeout_ms Optional timeout override
     * @return Response body or empty string on error
     */
    std::string http_post(const std::string& url, const std::string& json_body, int64_t timeout_ms = 0);
    
    /**
     * @brief Queue game event for dispatch
     * @param event Event to queue
     * @return true if queued successfully
     */
    bool queue_event(const GameEvent& event);
    
    /**
     * @brief Helper: Queue player login event
     */
    bool on_player_login(uint32 player_id, const std::string& map_name);
    
    /**
     * @brief Helper: Queue player logout event
     */
    bool on_player_logout(uint32 player_id);
    
    /**
     * @brief Helper: Queue monster kill event
     */
    bool on_monster_kill(uint32 player_id, uint32 monster_id, const std::string& map_name);
    
    /**
     * @brief Helper: Queue MVP kill event
     */
    bool on_mvp_kill(uint32 player_id, uint32 mvp_id, const std::string& map_name);
    
    /**
     * @brief Helper: Queue daily reset event
     */
    bool on_daily_reset();
    
    /**
     * @brief Helper: Queue map change event
     */
    bool on_map_change(uint32 player_id, const std::string& from_map, const std::string& to_map);
    
    /**
     * @brief Helper: Queue NPC interaction event
     */
    bool on_npc_interaction(uint32 player_id, uint32 npc_id, const std::string& map_name);
    
    /**
     * @brief Helper: Queue quest complete event
     */
    bool on_quest_complete(uint32 player_id, uint32 quest_id);
    
    /**
     * @brief Get system status summary
     * @return JSON string with status of all components
     */
    std::string get_status_json() const;
    
    /**
     * @brief Reload configuration
     * @return true on success
     */
    bool reload_config();
    
private:
    BridgeLayer() = default;
    ~BridgeLayer() = default;
    
    // Delete copy and move
    BridgeLayer(const BridgeLayer&) = delete;
    BridgeLayer& operator=(const BridgeLayer&) = delete;
    BridgeLayer(BridgeLayer&&) = delete;
    BridgeLayer& operator=(BridgeLayer&&) = delete;
    
    bool initialized_{false};
    
    std::shared_ptr<HTTPClient> http_client_;
    std::shared_ptr<EventDispatcher> event_dispatcher_;
    std::shared_ptr<ActionExecutor> action_executor_;
};

// Global convenience functions for C-style integration
extern "C" {
    /**
     * @brief Initialize Bridge Layer (C wrapper)
     * @param config_path Configuration file path
     * @return 1 on success, 0 on failure
     */
    int32 ai_bridge_initialize(const char* config_path);
    
    /**
     * @brief Shutdown Bridge Layer (C wrapper)
     */
    void ai_bridge_shutdown();
    
    /**
     * @brief Simple HTTP GET (C wrapper)
     * @param url URL to fetch
     * @param output_buffer Buffer to store response
     * @param buffer_size Size of output buffer
     * @return Length of response or -1 on error
     */
    int32 ai_bridge_http_get(const char* url, char* output_buffer, size_t buffer_size);
    
    /**
     * @brief Simple HTTP POST (C wrapper)
     * @param url URL to post to
     * @param json_body JSON body
     * @param output_buffer Buffer to store response
     * @param buffer_size Size of output buffer
     * @return Length of response or -1 on error
     */
    int32 ai_bridge_http_post(const char* url, const char* json_body, char* output_buffer, size_t buffer_size);
}

} // namespace AIBridge

#endif // AI_BRIDGE_HPP