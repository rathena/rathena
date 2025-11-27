// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ai_bridge.hpp"

#include <sstream>

#include <common/showmsg.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace AIBridge {

BridgeLayer& BridgeLayer::instance() {
    static BridgeLayer instance;
    return instance;
}

bool BridgeLayer::initialize(const std::string& config_path) {
    if (initialized_) {
        ShowWarning("[BridgeLayer] Already initialized\n");
        return true;
    }
    
    ShowInfo("[BridgeLayer] Initializing Bridge Layer...\n");
    
    // Load configuration
    ConfigManager& config = ConfigManager::instance();
    if (!config.load(config_path)) {
        ShowError("[BridgeLayer] Failed to load configuration from %s\n", config_path.c_str());
        return false;
    }
    
    // Get AI service configuration
    std::string base_url = config.get_string("ai_service", "base_url", "http://192.168.0.100:8000");
    int64_t timeout_connect = config.get_int("ai_service", "timeout_connect", 5000);
    int64_t timeout_read = config.get_int("ai_service", "timeout_read", 30000);
    int64_t max_retries = config.get_int("http_pool", "max_retries", 3);
    
    // Create HTTP client
    http_client_ = std::make_shared<HTTPClient>(base_url, timeout_read, max_retries);
    
    ShowInfo("[BridgeLayer] HTTP Client: %s (timeout=%lldms, retries=%lld)\n",
             base_url.c_str(), timeout_read, max_retries);
    
    // Create Event Dispatcher
    size_t batch_size = config.get_int("event_dispatcher", "batch_size", 50);
    int64_t batch_interval = config.get_int("event_dispatcher", "batch_interval", 1000);
    size_t queue_size = config.get_int("event_dispatcher", "queue_size", 1000);
    
    event_dispatcher_ = std::make_shared<EventDispatcher>(
        http_client_, batch_size, batch_interval, queue_size
    );
    
    if (!event_dispatcher_->start()) {
        ShowError("[BridgeLayer] Failed to start Event Dispatcher\n");
        return false;
    }
    
    ShowInfo("[BridgeLayer] Event Dispatcher started (batch=%zu, interval=%lldms, queue=%zu)\n",
             batch_size, batch_interval, queue_size);
    
    // Create Action Executor
    int64_t poll_interval = config.get_int("action_executor", "poll_interval", 1000);
    size_t max_concurrent = config.get_int("action_executor", "max_concurrent", 5);
    
    action_executor_ = std::make_shared<ActionExecutor>(
        http_client_, poll_interval, max_concurrent
    );
    
    if (!action_executor_->start()) {
        ShowError("[BridgeLayer] Failed to start Action Executor\n");
        event_dispatcher_->stop();
        return false;
    }
    
    ShowInfo("[BridgeLayer] Action Executor started (poll=%lldms, max_concurrent=%zu)\n",
             poll_interval, max_concurrent);
    
    initialized_ = true;
    
    ShowInfo("[BridgeLayer] *** Bridge Layer initialized successfully ***\n");
    return true;
}

void BridgeLayer::shutdown() {
    if (!initialized_) {
        return;
    }
    
    ShowInfo("[BridgeLayer] Shutting down Bridge Layer...\n");
    
    // Stop action executor
    if (action_executor_) {
        action_executor_->stop();
    }
    
    // Stop event dispatcher
    if (event_dispatcher_) {
        event_dispatcher_->stop();
    }
    
    initialized_ = false;
    
    ShowInfo("[BridgeLayer] Bridge Layer shutdown complete\n");
}

bool BridgeLayer::is_initialized() const {
    return initialized_;
}

std::shared_ptr<HTTPClient> BridgeLayer::get_http_client() const {
    return http_client_;
}

std::shared_ptr<EventDispatcher> BridgeLayer::get_event_dispatcher() const {
    return event_dispatcher_;
}

std::shared_ptr<ActionExecutor> BridgeLayer::get_action_executor() const {
    return action_executor_;
}

ConfigManager& BridgeLayer::get_config() const {
    return ConfigManager::instance();
}

// ============================================================================
// Convenience Functions
// ============================================================================

std::string BridgeLayer::http_get(const std::string& url, int64_t timeout_ms) {
    if (!initialized_ || !http_client_) {
        ShowError("[BridgeLayer] Not initialized\n");
        return "";
    }
    
    HTTPResponse response = http_client_->get(url);
    return response.success ? response.body : "";
}

std::string BridgeLayer::http_post(const std::string& url, const std::string& json_body, int64_t timeout_ms) {
    if (!initialized_ || !http_client_) {
        ShowError("[BridgeLayer] Not initialized\n");
        return "";
    }
    
    HTTPResponse response = http_client_->post(url, json_body);
    return response.success ? response.body : "";
}

bool BridgeLayer::queue_event(const GameEvent& event) {
    if (!initialized_ || !event_dispatcher_) {
        ShowError("[BridgeLayer] Not initialized\n");
        return false;
    }
    
    return event_dispatcher_->queue_event(event);
}

bool BridgeLayer::on_player_login(uint32 player_id, const std::string& map_name) {
    GameEvent event = EventFactory::create_player_login(player_id, map_name);
    return queue_event(event);
}

bool BridgeLayer::on_player_logout(uint32 player_id) {
    GameEvent event = EventFactory::create_player_logout(player_id);
    return queue_event(event);
}

bool BridgeLayer::on_monster_kill(uint32 player_id, uint32 monster_id, const std::string& map_name) {
    GameEvent event = EventFactory::create_monster_kill(player_id, monster_id, map_name);
    return queue_event(event);
}

bool BridgeLayer::on_mvp_kill(uint32 player_id, uint32 mvp_id, const std::string& map_name) {
    GameEvent event = EventFactory::create_mvp_kill(player_id, mvp_id, map_name);
    return queue_event(event);
}

bool BridgeLayer::on_daily_reset() {
    GameEvent event = EventFactory::create_daily_reset();
    return queue_event(event);
}

bool BridgeLayer::on_map_change(uint32 player_id, const std::string& from_map, const std::string& to_map) {
    GameEvent event = EventFactory::create_map_change(player_id, from_map, to_map);
    return queue_event(event);
}

bool BridgeLayer::on_npc_interaction(uint32 player_id, uint32 npc_id, const std::string& map_name) {
    GameEvent event = EventFactory::create_npc_interaction(player_id, npc_id, map_name);
    return queue_event(event);
}

bool BridgeLayer::on_quest_complete(uint32 player_id, uint32 quest_id) {
    GameEvent event = EventFactory::create_quest_complete(player_id, quest_id);
    return queue_event(event);
}

std::string BridgeLayer::get_status_json() const {
    json status;
    
    status["initialized"] = initialized_;
    
    if (initialized_) {
        // HTTP Client stats
        if (http_client_) {
            ConnectionPoolStats pool_stats = http_client_->get_pool_stats();
            status["http_client"]["pool"]["total_connections"] = pool_stats.total_connections;
            status["http_client"]["pool"]["active"] = pool_stats.active_connections;
            status["http_client"]["pool"]["idle"] = pool_stats.idle_connections;
            status["http_client"]["pool"]["total_requests"] = pool_stats.total_requests;
            status["http_client"]["pool"]["failed_requests"] = pool_stats.failed_requests;
            status["http_client"]["pool"]["avg_latency_ms"] = pool_stats.avg_latency_ms;
            status["http_client"]["pool"]["reuse_rate"] = pool_stats.pool_reuse_rate;
            
            CircuitState circuit = http_client_->get_circuit_state();
            const char* state_str = "unknown";
            switch (circuit) {
                case CircuitState::CLOSED: state_str = "closed"; break;
                case CircuitState::OPEN: state_str = "open"; break;
                case CircuitState::HALF_OPEN: state_str = "half_open"; break;
            }
            status["http_client"]["circuit_breaker"]["state"] = state_str;
            status["http_client"]["healthy"] = http_client_->is_healthy();
        }
        
        // Event Dispatcher stats
        if (event_dispatcher_) {
            EventQueueStats queue_stats = event_dispatcher_->get_stats();
            status["event_dispatcher"]["current_size"] = queue_stats.current_size;
            status["event_dispatcher"]["total_queued"] = queue_stats.total_queued;
            status["event_dispatcher"]["total_dispatched"] = queue_stats.total_dispatched;
            status["event_dispatcher"]["total_failed"] = queue_stats.total_failed;
            status["event_dispatcher"]["total_batches"] = queue_stats.total_batches;
            status["event_dispatcher"]["avg_batch_size"] = queue_stats.avg_batch_size;
            status["event_dispatcher"]["avg_dispatch_latency_ms"] = queue_stats.avg_dispatch_latency_ms;
            status["event_dispatcher"]["running"] = event_dispatcher_->is_running();
        }
        
        // Action Executor stats
        if (action_executor_) {
            ActionExecutorStats exec_stats = action_executor_->get_stats();
            status["action_executor"]["total_polled"] = exec_stats.total_polled;
            status["action_executor"]["total_executed"] = exec_stats.total_executed;
            status["action_executor"]["total_failed"] = exec_stats.total_failed;
            status["action_executor"]["current_pending"] = exec_stats.current_pending;
            status["action_executor"]["avg_execution_time_ms"] = exec_stats.avg_execution_time_ms;
            status["action_executor"]["last_poll_time_ms"] = exec_stats.last_poll_time_ms;
            status["action_executor"]["running"] = action_executor_->is_running();
        }
    }
    
    return status.dump(2); // Pretty print with 2-space indent
}

bool BridgeLayer::reload_config() {
    ConfigManager& config = ConfigManager::instance();
    return config.reload();
}

// ============================================================================
// C-style wrapper functions
// ============================================================================

extern "C" {

int32 ai_bridge_initialize(const char* config_path) {
    std::string path = config_path ? config_path : "conf/ai_bridge.conf";
    return BridgeLayer::instance().initialize(path) ? 1 : 0;
}

void ai_bridge_shutdown() {
    BridgeLayer::instance().shutdown();
}

int32 ai_bridge_http_get(const char* url, char* output_buffer, size_t buffer_size) {
    if (!url || !output_buffer || buffer_size == 0) {
        return -1;
    }
    
    std::string response = BridgeLayer::instance().http_get(url);
    
    if (response.empty()) {
        return -1;
    }
    
    size_t copy_len = std::min(response.length(), buffer_size - 1);
    memcpy(output_buffer, response.c_str(), copy_len);
    output_buffer[copy_len] = '\0';
    
    return static_cast<int32>(copy_len);
}

int32 ai_bridge_http_post(const char* url, const char* json_body, char* output_buffer, size_t buffer_size) {
    if (!url || !json_body || !output_buffer || buffer_size == 0) {
        return -1;
    }
    
    std::string response = BridgeLayer::instance().http_post(url, json_body);
    
    if (response.empty()) {
        return -1;
    }
    
    size_t copy_len = std::min(response.length(), buffer_size - 1);
    memcpy(output_buffer, response.c_str(), copy_len);
    output_buffer[copy_len] = '\0';
    
    return static_cast<int32>(copy_len);
}

} // extern "C"

} // namespace AIBridge