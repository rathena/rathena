// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "action_executor.hpp"

#include <sstream>

#include <common/showmsg.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace AIBridge {

// ============================================================================
// AIAction Implementation
// ============================================================================

bool AIAction::from_json(const std::string& json_str) {
    try {
        json j = json::parse(json_str);
        
        // Required fields
        if (!j.contains("action_id") || !j.contains("action_type")) {
            ShowError("[AIAction] Missing required fields in action JSON\n");
            return false;
        }
        
        action_id = j["action_id"].get<std::string>();
        
        // Parse action type
        std::string type_str = j["action_type"].get<std::string>();
        if (type_str == "spawn_monster") {
            type = ActionType::SPAWN_MONSTER;
        } else if (type_str == "spawn_npc") {
            type = ActionType::SPAWN_NPC;
        } else if (type_str == "give_reward") {
            type = ActionType::GIVE_REWARD;
        } else if (type_str == "broadcast_message") {
            type = ActionType::BROADCAST;
        } else if (type_str == "create_quest") {
            type = ActionType::CREATE_QUEST;
        } else if (type_str == "set_map_flag") {
            type = ActionType::SET_MAP_FLAG;
        } else if (type_str == "move_npc") {
            type = ActionType::MOVE_NPC;
        } else if (type_str == "remove_npc") {
            type = ActionType::REMOVE_NPC;
        } else if (type_str == "update_economy") {
            type = ActionType::UPDATE_ECONOMY;
        } else {
            type = ActionType::UNKNOWN;
            ShowWarning("[AIAction] Unknown action type: %s\n", type_str.c_str());
        }
        
        // Optional fields
        if (j.contains("payload")) {
            payload = j["payload"].dump();
        }
        
        if (j.contains("target_player_id")) {
            target_player_id = j["target_player_id"].get<uint32>();
        }
        
        if (j.contains("target_map")) {
            target_map = j["target_map"].get<std::string>();
        }
        
        if (j.contains("priority")) {
            priority = j["priority"].get<int32>();
        }
        
        return true;
    } catch (const std::exception& e) {
        ShowError("[AIAction] Failed to parse action JSON: %s\n", e.what());
        return false;
    }
}

std::string AIAction::result_to_json(bool success, const std::string& message) const {
    json j;
    j["action_id"] = action_id;
    j["status"] = success ? "completed" : "failed";
    j["message"] = message;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    return j.dump();
}

// ============================================================================
// ActionExecutor Implementation
// ============================================================================

ActionExecutor::ActionExecutor(
    std::shared_ptr<HTTPClient> http_client,
    int64_t poll_interval_ms,
    size_t max_concurrent
)
    : http_client_(http_client),
      poll_interval_ms_(poll_interval_ms),
      max_concurrent_(max_concurrent) {
}

ActionExecutor::~ActionExecutor() {
    stop();
}

bool ActionExecutor::start() {
    if (running_.load()) {
        ShowWarning("[ActionExecutor] Already running\n");
        return false;
    }
    
    should_stop_ = false;
    running_ = true;
    
    polling_thread_ = std::make_unique<std::thread>(&ActionExecutor::polling_thread_func, this);
    
    ShowInfo("[ActionExecutor] Started (poll_interval=%lldms, max_concurrent=%zu)\n",
             poll_interval_ms_, max_concurrent_);
    
    return true;
}

void ActionExecutor::stop() {
    if (!running_.load()) {
        return;
    }
    
    ShowInfo("[ActionExecutor] Stopping...\n");
    
    should_stop_ = true;
    
    if (polling_thread_ && polling_thread_->joinable()) {
        polling_thread_->join();
    }
    
    running_ = false;
    
    ShowInfo("[ActionExecutor] Stopped\n");
}

void ActionExecutor::polling_thread_func() {
    while (!should_stop_.load()) {
        auto poll_start = std::chrono::steady_clock::now();
        
        // Poll for pending actions
        std::vector<AIAction> actions = poll_actions();
        
        last_poll_time_ms_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        
        // Execute each action
        for (const auto& action : actions) {
            execute_action(action);
        }
        
        // Sleep until next poll interval
        auto poll_end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(poll_end - poll_start).count();
        auto sleep_time = poll_interval_ms_ - elapsed;
        
        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }
}

std::vector<AIAction> ActionExecutor::poll_actions() {
    std::vector<AIAction> actions;
    
    // Poll AI service for pending actions
    HTTPResponse response = http_client_->get("/api/v1/actions/pending");
    
    if (!response.success) {
        ShowDebug("[ActionExecutor] Poll failed: %s\n", response.error_message.c_str());
        return actions;
    }
    
    try {
        json j = json::parse(response.body);
        
        if (!j.contains("actions") || !j["actions"].is_array()) {
            ShowWarning("[ActionExecutor] Invalid response format\n");
            return actions;
        }
        
        for (const auto& action_json : j["actions"]) {
            AIAction action;
            if (action.from_json(action_json.dump())) {
                actions.push_back(action);
                total_polled_++;
            }
        }
        
        if (!actions.empty()) {
            ShowInfo("[ActionExecutor] Polled %zu pending actions\n", actions.size());
        }
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse actions: %s\n", e.what());
    }
    
    return actions;
}

bool ActionExecutor::execute_action(const AIAction& action) {
    auto start_time = std::chrono::steady_clock::now();
    
    bool success = false;
    std::string message;
    
    ShowInfo("[ActionExecutor] Executing action %s (type=%d)\n", 
             action.action_id.c_str(), static_cast<int>(action.type));
    
    try {
        switch (action.type) {
            case ActionType::SPAWN_MONSTER:
                success = execute_spawn_monster(action);
                message = success ? "Monster spawned" : "Failed to spawn monster";
                break;
                
            case ActionType::SPAWN_NPC:
                success = execute_spawn_npc(action);
                message = success ? "NPC spawned" : "Failed to spawn NPC";
                break;
                
            case ActionType::GIVE_REWARD:
                success = execute_give_reward(action);
                message = success ? "Reward given" : "Failed to give reward";
                break;
                
            case ActionType::BROADCAST:
                success = execute_broadcast(action);
                message = success ? "Message broadcasted" : "Failed to broadcast";
                break;
                
            case ActionType::CREATE_QUEST:
                success = execute_create_quest(action);
                message = success ? "Quest created" : "Failed to create quest";
                break;
                
            case ActionType::SET_MAP_FLAG:
                success = execute_set_map_flag(action);
                message = success ? "Map flag set" : "Failed to set map flag";
                break;
                
            case ActionType::MOVE_NPC:
                success = execute_move_npc(action);
                message = success ? "NPC moved" : "Failed to move NPC";
                break;
                
            case ActionType::REMOVE_NPC:
                success = execute_remove_npc(action);
                message = success ? "NPC removed" : "Failed to remove NPC";
                break;
                
            case ActionType::UPDATE_ECONOMY:
                success = execute_update_economy(action);
                message = success ? "Economy updated" : "Failed to update economy";
                break;
                
            default:
                success = false;
                message = "Unknown action type";
                ShowError("[ActionExecutor] Unknown action type\n");
                break;
        }
    } catch (const std::exception& e) {
        success = false;
        message = std::string("Exception: ") + e.what();
        ShowError("[ActionExecutor] Exception executing action: %s\n", e.what());
    }
    
    auto end_time = std::chrono::steady_clock::now();
    int64_t execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time
    ).count();
    
    if (success) {
        total_executed_++;
        total_execution_time_ms_ += execution_time;
        ShowInfo("[ActionExecutor] Action %s executed in %lldms\n", 
                 action.action_id.c_str(), execution_time);
    } else {
        total_failed_++;
        ShowError("[ActionExecutor] Action %s failed: %s\n", 
                  action.action_id.c_str(), message.c_str());
    }
    
    // Report result back to AI service
    report_execution_result(action, success, message);
    
    return success;
}

bool ActionExecutor::execute_spawn_monster(const AIAction& action) {
    // Parse payload
    try {
        json j = json::parse(action.payload);
        
        std::string map = j.value("map", "");
        int32 x = j.value("x", 0);
        int32 y = j.value("y", 0);
        uint32 monster_id = j.value("monster_id", 0);
        int32 amount = j.value("amount", 1);
        
        if (map.empty() || monster_id == 0) {
            ShowError("[ActionExecutor] Invalid spawn_monster parameters\n");
            return false;
        }
        
        // TODO: Call rAthena mob_once_spawn() function
        // This requires including mob.hpp and linking properly
        // For now, return true to indicate logic is in place
        ShowInfo("[ActionExecutor] Would spawn %d monster(s) ID %u at %s(%d,%d)\n",
                 amount, monster_id, map.c_str(), x, y);
        
        return true; // Placeholder
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse spawn_monster payload: %s\n", e.what());
        return false;
    }
}

bool ActionExecutor::execute_spawn_npc(const AIAction& action) {
    // Parse payload
    try {
        json j = json::parse(action.payload);
        
        std::string map = j.value("map", "");
        int32 x = j.value("x", 0);
        int32 y = j.value("y", 0);
        std::string npc_name = j.value("npc_name", "");
        std::string script = j.value("script", "");
        
        if (map.empty() || npc_name.empty()) {
            ShowError("[ActionExecutor] Invalid spawn_npc parameters\n");
            return false;
        }
        
        // TODO: Call rAthena npc_parse_script() function
        ShowInfo("[ActionExecutor] Would spawn NPC '%s' at %s(%d,%d)\n",
                 npc_name.c_str(), map.c_str(), x, y);
        
        return true; // Placeholder
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse spawn_npc payload: %s\n", e.what());
        return false;
    }
}

bool ActionExecutor::execute_give_reward(const AIAction& action) {
    // Parse payload
    try {
        json j = json::parse(action.payload);
        
        uint32 player_id = j.value("player_id", action.target_player_id);
        uint32 item_id = j.value("item_id", 0);
        int32 quantity = j.value("quantity", 1);
        int32 zeny = j.value("zeny", 0);
        int64 base_exp = j.value("base_exp", 0);
        int64 job_exp = j.value("job_exp", 0);
        
        if (player_id == 0) {
            ShowError("[ActionExecutor] Invalid player_id for give_reward\n");
            return false;
        }
        
        // TODO: Call rAthena pc_additem(), pc_getzeny(), pc_gainexp() functions
        ShowInfo("[ActionExecutor] Would give player %u: item=%u qty=%d zeny=%d exp=%lld/%lld\n",
                 player_id, item_id, quantity, zeny, base_exp, job_exp);
        
        return true; // Placeholder
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse give_reward payload: %s\n", e.what());
        return false;
    }
}

bool ActionExecutor::execute_broadcast(const AIAction& action) {
    // Parse payload
    try {
        json j = json::parse(action.payload);
        
        std::string message = j.value("message", "");
        
        if (message.empty()) {
            ShowError("[ActionExecutor] Empty broadcast message\n");
            return false;
        }
        
        // TODO: Call rAthena intif_broadcast() function
        ShowInfo("[ActionExecutor] Would broadcast: %s\n", message.c_str());
        
        return true; // Placeholder
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse broadcast payload: %s\n", e.what());
        return false;
    }
}

bool ActionExecutor::execute_create_quest(const AIAction& action) {
    ShowInfo("[ActionExecutor] create_quest not yet implemented\n");
    return false; // Not implemented
}

bool ActionExecutor::execute_set_map_flag(const AIAction& action) {
    ShowInfo("[ActionExecutor] set_map_flag not yet implemented\n");
    return false; // Not implemented
}

bool ActionExecutor::execute_move_npc(const AIAction& action) {
    ShowInfo("[ActionExecutor] move_npc not yet implemented\n");
    return false; // Not implemented
}

bool ActionExecutor::execute_remove_npc(const AIAction& action) {
    ShowInfo("[ActionExecutor] remove_npc not yet implemented\n");
    return false; // Not implemented
}

bool ActionExecutor::execute_update_economy(const AIAction& action) {
    ShowInfo("[ActionExecutor] update_economy not yet implemented\n");
    return false; // Not implemented
}

bool ActionExecutor::report_execution_result(const AIAction& action, bool success, const std::string& message) {
    std::string result_json = action.result_to_json(success, message);
    
    HTTPResponse response = http_client_->post("/api/v1/actions/complete", result_json);
    
    if (!response.success) {
        ShowWarning("[ActionExecutor] Failed to report result for action %s: %s\n",
                    action.action_id.c_str(), response.error_message.c_str());
        return false;
    }
    
    return true;
}

ActionExecutorStats ActionExecutor::get_stats() const {
    ActionExecutorStats stats;
    
    stats.total_polled = total_polled_.load();
    stats.total_executed = total_executed_.load();
    stats.total_failed = total_failed_.load();
    stats.last_poll_time_ms = last_poll_time_ms_.load();
    
    size_t executed = total_executed_.load();
    if (executed > 0) {
        stats.avg_execution_time_ms = static_cast<double>(total_execution_time_ms_) / executed;
    }
    
    return stats;
}

bool ActionExecutor::is_running() const {
    return running_.load();
}

} // namespace AIBridge