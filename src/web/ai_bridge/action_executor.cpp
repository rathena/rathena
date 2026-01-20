// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "action_executor.hpp"

#include <sstream>

#include <common/showmsg.hpp>
#include <nlohmann/json.hpp>

// Map server includes for action execution
#include "../../map/map.hpp"
#include "../../map/mob.hpp"
#include "../../map/npc.hpp"
#include "../../map/pc.hpp"
#include "../../map/quest.hpp"
#include "../../map/unit.hpp"
#include "../../map/intif.hpp"
#include "../../map/clif.hpp"
#include "../../map/itemdb.hpp"
#include "../../map/log.hpp"

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
        
        std::string map_name = j.value("map", "");
        int32 x = j.value("x", 0);
        int32 y = j.value("y", 0);
        int32 monster_id = j.value("monster_id", 0);
        int32 amount = j.value("amount", 1);
        std::string event = j.value("event", "");
        
        // Validate parameters
        if (map_name.empty() || monster_id == 0) {
            ShowError("[ActionExecutor] Invalid spawn_monster parameters: map=%s, monster_id=%d\n",
                     map_name.c_str(), monster_id);
            return false;
        }
        
        if (amount < 1) amount = 1;
        if (amount > 100) {
            ShowWarning("[ActionExecutor] Spawn amount %d too high, capping at 100\n", amount);
            amount = 100;
        }
        
        // Get map ID from map name
        int16 m = map_mapname2mapid(map_name.c_str());
        if (m < 0) {
            ShowError("[ActionExecutor] Invalid map name: %s\n", map_name.c_str());
            return false;
        }
        
        // Call rAthena's mob_once_spawn
        // Parameters: sd, m, x, y, mobname, mob_id, amount, event, size, ai
        int32 spawned = mob_once_spawn(
            nullptr,                    // sd (no specific player)
            m,                          // map ID
            x, y,                       // coordinates
            "--ai-spawned--",           // mob name (use placeholder)
            monster_id,                 // monster ID
            amount,                     // amount to spawn
            event.empty() ? "" : event.c_str(),  // event on death
            SZ_MEDIUM,                  // size (medium)
            AI_NONE                     // AI type (normal)
        );
        
        if (spawned > 0) {
            ShowInfo("[ActionExecutor] Successfully spawned %d monster(s) (ID:%d) at %s(%d,%d)\n",
                     spawned, monster_id, map_name.c_str(), x, y);
            return true;
        } else {
            ShowError("[ActionExecutor] Failed to spawn monsters (ID:%d) at %s(%d,%d)\n",
                     monster_id, map_name.c_str(), x, y);
            return false;
        }
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse spawn_monster payload: %s\n", e.what());
        return false;
    }
}

bool ActionExecutor::execute_spawn_npc(const AIAction& action) {
    // Parse payload
    try {
        json j = json::parse(action.payload);
        
        std::string map_name = j.value("map", "");
        int32 x = j.value("x", 0);
        int32 y = j.value("y", 0);
        std::string npc_name = j.value("npc_name", "");
        int32 sprite_id = j.value("sprite", 111);  // Default sprite: HIDDEN_NPC
        int32 facing = j.value("facing", 0);
        std::string script = j.value("script", "");
        
        // Validate basic parameters
        if (map_name.empty() || npc_name.empty()) {
            ShowError("[ActionExecutor] Invalid spawn_npc parameters: map=%s, name=%s\n",
                     map_name.c_str(), npc_name.c_str());
            return false;
        }
        
        // Validate map exists
        int16 m = map_mapname2mapid(map_name.c_str());
        if (m < 0) {
            ShowError("[ActionExecutor] Invalid map for NPC spawn: %s\n", map_name.c_str());
            return false;
        }
        
        ShowWarning("[ActionExecutor] spawn_npc: Dynamic NPC creation is complex and limited\n");
        ShowWarning("[ActionExecutor] Requested NPC: name='%s', map=%s(%d,%d), sprite=%d\n",
                   npc_name.c_str(), map_name.c_str(), x, y, sprite_id);
        
        if (!script.empty()) {
            ShowWarning("[ActionExecutor] Script provided (%zu chars) - runtime script parsing not supported\n",
                       script.length());
        }
        
        // NOTE: Dynamic NPC creation in rAthena is extremely complex because:
        //
        // 1. NPC Creation Process:
        //    - Requires parsing NPC script syntax (npc_parse_script)
        //    - Script compilation into bytecode
        //    - Registration in global NPC database
        //    - Adding to map's NPC list
        //    - Client notification
        //
        // 2. Script Limitations:
        //    - rAthena's script engine expects scripts during server startup
        //    - Runtime script compilation requires script_state setup
        //    - Variable scope and NPC-specific data structures
        //    - OnInit and other event triggers
        //
        // 3. Memory Management:
        //    - NPCs created at runtime need proper cleanup
        //    - Script data persistence across server restarts
        //    - Potential memory leaks if not managed correctly
        //
        // 4. Alternative Approaches:
        //    a) Use duplicate() script command with existing NPC templates
        //    b) Pre-define NPC templates and enable/disable them
        //    c) Use monster_summmon with talk sprites for simple NPCs
        //    d) Create NPCs via script files and reload NPC system
        //
        // RECOMMENDED IMPLEMENTATION:
        // Instead of runtime NPC creation, use a pool of pre-defined NPCs that can be:
        // - Enabled/disabled dynamically
        // - Moved to different locations
        // - Have their dialogue changed through NPC variables
        //
        // Example workaround using existing NPCs:
        // 1. Create template NPCs in script files (hidden initially)
        // 2. Use enablenpc/disablenpc to show/hide
        // 3. Use movenpc to position
        // 4. Use NPC variables for dynamic dialogue
        
        ShowWarning("[ActionExecutor] For dynamic NPCs, use script commands instead:\n");
        ShowWarning("[ActionExecutor]   enablenpc/disablenpc - Show/hide existing NPCs\n");
        ShowWarning("[ActionExecutor]   movenpc - Relocate NPCs\n");
        ShowWarning("[ActionExecutor]   npcwalk - Make NPCs move\n");
        ShowWarning("[ActionExecutor]   setnpcdisp - Change NPC sprite\n");
        
        return false; // Not implemented - requires major NPC system refactoring
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
        std::string player_name = j.value("player_name", "");
        uint32 item_id = j.value("item_id", 0);
        int32 quantity = j.value("quantity", 1);
        int32 zeny = j.value("zeny", 0);
        int64 base_exp = j.value("base_exp", 0);
        int64 job_exp = j.value("job_exp", 0);
        
        // Get player session data
        map_session_data* sd = nullptr;
        
        if (player_id > 0) {
            sd = map_id2sd(player_id);
        } else if (!player_name.empty()) {
            sd = map_nick2sd(player_name.c_str(), false);
        }
        
        if (sd == nullptr) {
            ShowError("[ActionExecutor] Player not found (ID:%u, Name:%s)\n",
                     player_id, player_name.c_str());
            return false;
        }
        
        bool success = true;
        
        // Give item if specified
        if (item_id > 0 && quantity > 0) {
            struct item item_tmp;
            memset(&item_tmp, 0, sizeof(item_tmp));
            item_tmp.nameid = item_id;
            item_tmp.identify = 1;
            
            enum e_additem_result result = pc_additem(sd, &item_tmp, quantity, LOG_TYPE_SCRIPT);
            if (result == ADDITEM_SUCCESS) {
                ShowInfo("[ActionExecutor] Gave %d x Item ID %u to player %s (ID:%u)\n",
                        quantity, item_id, sd->status.name, sd->status.account_id);
            } else {
                ShowError("[ActionExecutor] Failed to give item %u to player %s (result:%d)\n",
                         item_id, sd->status.name, result);
                success = false;
            }
        }
        
        // Give zeny if specified
        if (zeny > 0) {
            if (pc_getzeny(sd, zeny, LOG_TYPE_SCRIPT, 0) == 0) {
                ShowInfo("[ActionExecutor] Gave %d zeny to player %s (ID:%u)\n",
                        zeny, sd->status.name, sd->status.account_id);
            } else {
                ShowError("[ActionExecutor] Failed to give %d zeny to player %s\n",
                         zeny, sd->status.name);
                success = false;
            }
        }
        
        // Give experience if specified
        if (base_exp > 0 || job_exp > 0) {
            pc_gainexp(sd, nullptr, base_exp, job_exp, 0);
            ShowInfo("[ActionExecutor] Gave %lld base exp, %lld job exp to player %s (ID:%u)\n",
                    (long long)base_exp, (long long)job_exp, sd->status.name, sd->status.account_id);
        }
        
        if (!item_id && !zeny && !base_exp && !job_exp) {
            ShowWarning("[ActionExecutor] No reward specified in give_reward action\n");
            return false;
        }
        
        return success;
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
        std::string type = j.value("type", "all");  // all, map, guild, party
        std::string target_map = j.value("map", "");
        
        if (message.empty()) {
            ShowError("[ActionExecutor] Empty broadcast message\n");
            return false;
        }
        
        // Validate message length (prevent spam)
        if (message.length() > 255) {
            ShowWarning("[ActionExecutor] Broadcast message too long (%zu chars), truncating\n",
                       message.length());
            message = message.substr(0, 255);
        }
        
        // Add broadcast prefix if not already present
        std::string broadcast_msg = message;
        if (broadcast_msg.find("[Broadcast]") == std::string::npos) {
            broadcast_msg = "[Broadcast] " + message;
        }
        
        if (type == "all" || type == "server") {
            // Server-wide broadcast via inter-server
            int32 result = intif_broadcast(broadcast_msg.c_str(), broadcast_msg.length() + 1, BC_ALL);
            if (result == 0) {
                ShowInfo("[ActionExecutor] Server-wide broadcast sent: %s\n", message.c_str());
                return true;
            } else {
                ShowError("[ActionExecutor] Failed to send server-wide broadcast\n");
                return false;
            }
        } else if (type == "map" && !target_map.empty()) {
            // Map-specific broadcast
            int16 m = map_mapname2mapid(target_map.c_str());
            if (m < 0) {
                ShowError("[ActionExecutor] Invalid map for broadcast: %s\n", target_map.c_str());
                return false;
            }
            
            // Use clif_broadcast to send to all players on the map
            clif_broadcast(nullptr, broadcast_msg.c_str(), broadcast_msg.length() + 1, BC_ALL, ALL_SAMEMAP);
            ShowInfo("[ActionExecutor] Map broadcast sent to %s: %s\n", target_map.c_str(), message.c_str());
            return true;
        } else {
            // Default to server-wide if type not recognized
            int32 result = intif_broadcast(broadcast_msg.c_str(), broadcast_msg.length() + 1, BC_ALL);
            if (result == 0) {
                ShowInfo("[ActionExecutor] Broadcast sent (type:%s): %s\n", type.c_str(), message.c_str());
                return true;
            } else {
                ShowError("[ActionExecutor] Failed to send broadcast\n");
                return false;
            }
        }
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse broadcast payload: %s\n", e.what());
        return false;
    }
}

bool ActionExecutor::execute_create_quest(const AIAction& action) {
    // Parse payload
    try {
        json j = json::parse(action.payload);
        
        int32 quest_id = j.value("quest_id", 0);
        uint32 player_id = j.value("player_id", action.target_player_id);
        std::string player_name = j.value("player_name", "");
        
        if (quest_id <= 0) {
            ShowError("[ActionExecutor] Invalid quest_id: %d\n", quest_id);
            return false;
        }
        
        // Get player session data
        map_session_data* sd = nullptr;
        
        if (player_id > 0) {
            sd = map_id2sd(player_id);
        } else if (!player_name.empty()) {
            sd = map_nick2sd(player_name.c_str(), false);
        }
        
        if (sd == nullptr) {
            ShowError("[ActionExecutor] Player not found for create_quest (ID:%u, Name:%s)\n",
                     player_id, player_name.c_str());
            return false;
        }
        
        // Add quest to player
        int32 result = quest_add(sd, quest_id);
        
        if (result == 0) {
            ShowInfo("[ActionExecutor] Successfully added quest %d to player %s (ID:%u)\n",
                    quest_id, sd->status.name, sd->status.account_id);
            return true;
        } else if (result == -1) {
            ShowWarning("[ActionExecutor] Quest %d already exists for player %s\n",
                       quest_id, sd->status.name);
            return false;
        } else {
            ShowError("[ActionExecutor] Failed to add quest %d to player %s (error:%d)\n",
                     quest_id, sd->status.name, result);
            return false;
        }
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse create_quest payload: %s\n", e.what());
        return false;
    }
}

bool ActionExecutor::execute_set_map_flag(const AIAction& action) {
    // Parse payload
    try {
        json j = json::parse(action.payload);
        
        std::string map_name = j.value("map", "");
        std::string flag_name = j.value("flag", "");
        bool flag_value = j.value("value", true);
        
        if (map_name.empty() || flag_name.empty()) {
            ShowError("[ActionExecutor] Invalid set_map_flag parameters: map=%s, flag=%s\n",
                     map_name.c_str(), flag_name.c_str());
            return false;
        }
        
        // Get map ID
        int16 m = map_mapname2mapid(map_name.c_str());
        if (m < 0) {
            ShowError("[ActionExecutor] Invalid map name: %s\n", map_name.c_str());
            return false;
        }
        
        ShowWarning("[ActionExecutor] set_map_flag: Direct map flag manipulation not implemented\n");
        ShowWarning("[ActionExecutor] Requested: map=%s, flag=%s, value=%d\n",
                   map_name.c_str(), flag_name.c_str(), flag_value);
        ShowWarning("[ActionExecutor] Use script commands or map configuration instead\n");
        
        // NOTE: Direct mapflag manipulation requires access to internal map structures
        // and is typically done through map configuration files or script commands.
        // Implementing this safely requires adding new map API functions.
        
        return false; // Not fully implemented - requires map API extension
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse set_map_flag payload: %s\n", e.what());
        return false;
    }
}

bool ActionExecutor::execute_move_npc(const AIAction& action) {
    // Parse payload
    try {
        json j = json::parse(action.payload);
        
        std::string npc_name = j.value("npc_name", "");
        int32 npc_id = j.value("npc_id", 0);
        int16 target_x = j.value("x", 0);
        int16 target_y = j.value("y", 0);
        
        // Get NPC data
        struct npc_data* nd = nullptr;
        
        if (!npc_name.empty()) {
            nd = npc_name2id(npc_name.c_str());
        } else if (npc_id > 0) {
            nd = map_id2nd(npc_id);
        }
        
        if (nd == nullptr) {
            ShowError("[ActionExecutor] NPC not found (name:%s, id:%d)\n",
                     npc_name.c_str(), npc_id);
            return false;
        }
        
        // Validate coordinates
        if (target_x < 0 || target_y < 0) {
            ShowError("[ActionExecutor] Invalid coordinates for move_npc: (%d,%d)\n",
                     target_x, target_y);
            return false;
        }
        
        // Use unit_walktoxy to move NPC
        int result = unit_walktoxy((struct block_list *)nd, target_x, target_y, 0);
        
        if (result == 0) {
            ShowInfo("[ActionExecutor] NPC '%s' (ID:%d) walking to (%d,%d)\n",
                    nd->exname, nd->id, target_x, target_y);
            return true;
        } else {
            ShowError("[ActionExecutor] Failed to move NPC '%s' to (%d,%d) - result: %d\n",
                     nd->exname, target_x, target_y, result);
            return false;
        }
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse move_npc payload: %s\n", e.what());
        return false;
    }
}

bool ActionExecutor::execute_remove_npc(const AIAction& action) {
    // Parse payload
    try {
        json j = json::parse(action.payload);
        
        std::string npc_name = j.value("npc_name", "");
        int32 npc_id = j.value("npc_id", 0);
        
        // Get NPC data
        struct npc_data* nd = nullptr;
        
        if (!npc_name.empty()) {
            nd = npc_name2id(npc_name.c_str());
        } else if (npc_id > 0) {
            nd = map_id2nd(npc_id);
        }
        
        if (nd == nullptr) {
            ShowError("[ActionExecutor] NPC not found for removal (name:%s, id:%d)\n",
                     npc_name.c_str(), npc_id);
            return false;
        }
        
        // Store NPC info for logging
        std::string npc_display_name = nd->exname;
        int32 npc_bl_id = nd->id;
        
        ShowWarning("[ActionExecutor] remove_npc: NPC removal requires special handling\n");
        ShowWarning("[ActionExecutor] Requested removal of NPC '%s' (ID:%d)\n",
                   npc_display_name.c_str(), npc_bl_id);
        ShowWarning("[ActionExecutor] Dynamic NPC removal can cause crashes if not done carefully\n");
        ShowWarning("[ActionExecutor] Use npc_unload() script command or restart server instead\n");
        
        // NOTE: Direct NPC removal using npc_unload() can cause crashes if:
        // - NPC is currently executing a script
        // - Players are interacting with the NPC
        // - NPC has active timers or events
        //
        // Safe NPC removal requires:
        // 1. Checking if NPC is in use
        // 2. Canceling all active scripts/timers
        // 3. Removing from all maps and databases
        // 4. Cleaning up memory
        //
        // This is complex and error-prone to do at runtime.
        // Recommendation: Use script-based NPC management or configuration files.
        
        return false; // Not implemented - too risky without proper cleanup
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse remove_npc payload: %s\n", e.what());
        return false;
    }
}

bool ActionExecutor::execute_update_economy(const AIAction& action) {
    // Parse payload
    try {
        json j = json::parse(action.payload);
        
        std::string shop_name = j.value("shop_name", "");
        int32 shop_id = j.value("shop_id", 0);
        int32 item_id = j.value("item_id", 0);
        int32 new_price = j.value("price", -1);
        int32 stock_amount = j.value("stock", -1);
        
        if (shop_name.empty() && shop_id == 0) {
            ShowError("[ActionExecutor] No shop specified for update_economy\n");
            return false;
        }
        
        if (item_id <= 0) {
            ShowError("[ActionExecutor] Invalid item_id for update_economy: %d\n", item_id);
            return false;
        }
        
        ShowWarning("[ActionExecutor] update_economy: Dynamic shop modification not implemented\n");
        ShowWarning("[ActionExecutor] Requested: shop=%s/%d, item=%d, price=%d, stock=%d\n",
                   shop_name.c_str(), shop_id, item_id, new_price, stock_amount);
        ShowWarning("[ActionExecutor] Shop data is typically static from configuration\n");
        ShowWarning("[ActionExecutor] Use script-based shops (openshop/sellitem) for dynamic economy\n");
        
        // NOTE: Implementing dynamic shop updates requires:
        // 1. Finding the shop NPC (npc_name2id or iteration)
        // 2. Accessing shop_data structure (may be private/protected)
        // 3. Modifying item prices in the shop's item list
        // 4. Updating stock if applicable (requires vending/shop tracking)
        // 5. Notifying clients of price changes
        //
        // rAthena's shop system is designed for static configuration.
        // Dynamic shops are better handled through:
        // - Script-based vending systems
        // - Cash shop management via web interface
        // - Market board/auction house implementations
        //
        // Recommendation: Implement AI-driven pricing through script commands
        // that NPCs can execute, rather than direct shop data manipulation.
        
        return false; // Not implemented - requires shop system redesign
    } catch (const std::exception& e) {
        ShowError("[ActionExecutor] Failed to parse update_economy payload: %s\n", e.what());
        return false;
    }
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