/**
 * @file ai_integration.cpp
 * @brief AI integration for map server
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the integration between the map server and the AI module.
 */

#include "ai_integration.hpp"

#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

#include "../common/cbasetypes.hpp"
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"

#include "map.hpp"
#include "pc.hpp"
#include "npc.hpp"
#include "mob.hpp"
#include "script.hpp"
#include "skill.hpp"
#include "battle.hpp"
#include "clif.hpp"
#include "atcommand.hpp"

#include "../ai/common/ai_request.hpp"
#include "../ai/common/ai_response.hpp"

using json = nlohmann::json;

// Timer ID for AI updates
static int ai_update_timer_id = INVALID_TIMER;

// AI update interval (in milliseconds)
#define AI_UPDATE_INTERVAL 1000

/**
 * Initialize the AI module for the map server
 * @return True if initialization was successful, false otherwise
 */
bool ai_init() {
    ShowInfo("AI Module: Initializing integration with map server...\n");
    
    // Initialize the AI module
    if (!AIModule::getInstance().initialize()) {
        ShowError("AI Module: Failed to initialize\n");
        return false;
    }
    
    // Register script commands
    ai_register_script_commands();
    
    // Start the update timer
    ai_update_timer_id = add_timer_interval(gettick(), [](int tid, unsigned int tick, int id, intptr_t data) -> int {
        ai_update(tick);
        return AI_UPDATE_INTERVAL;
    }, 0, 0, AI_UPDATE_INTERVAL);
    
    ShowInfo("AI Module: Integration initialized successfully\n");
    return true;
}

/**
 * Finalize the AI module for the map server
 */
void ai_final() {
    ShowInfo("AI Module: Finalizing integration with map server...\n");
    
    // Stop the update timer
    if (ai_update_timer_id != INVALID_TIMER) {
        delete_timer(ai_update_timer_id, nullptr);
        ai_update_timer_id = INVALID_TIMER;
    }
    
    // Finalize the AI module
    AIModule::getInstance().finalize();
    
    ShowInfo("AI Module: Integration finalized\n");
}

/**
 * Process an AI request from a script
 * @param agent_name Agent name
 * @param request_type Request type
 * @param request_data Request data
 * @return Response data
 */
const char* ai_process_script_request(const char* agent_name, const char* request_type, const char* request_data) {
    if (!agent_name || !request_type || !request_data) {
        ShowError("AI Module: Invalid parameters for ai_process_script_request\n");
        return "";
    }
    
    // Get the agent
    auto agent = AIModule::getInstance().getAgent(agent_name);
    if (!agent) {
        ShowError("AI Module: Agent '%s' not found\n", agent_name);
        return "";
    }
    
    // Create the request
    AIRequest request(request_type, request_data);
    
    // Process the request
    AIResponse response = agent->processRequest(request);
    
    // Return the response content
    static char response_buffer[4096];
    safestrncpy(response_buffer, response.getContent().c_str(), sizeof(response_buffer));
    return response_buffer;
}

/**
 * Handle a player event
 * @param event_name Event name
 * @param char_id Character ID
 * @param params Event parameters
 */
void ai_handle_player_event(const char* event_name, int char_id, const char* params) {
    if (!event_name || !params) {
        ShowError("AI Module: Invalid parameters for ai_handle_player_event\n");
        return;
    }
    
    // Parse parameters
    std::unordered_map<std::string, std::string> param_map;
    try {
        json j = json::parse(params);
        for (auto& [key, value] : j.items()) {
            param_map[key] = value.get<std::string>();
        }
    } catch (const std::exception& e) {
        ShowError("AI Module: Failed to parse parameters for ai_handle_player_event: %s\n", e.what());
        return;
    }
    
    // Notify all agents
    for (const auto& agent_name : {"world_evolution", "legend_bloodlines", "cross_class_synthesis", 
                                  "quest_generation", "economic_ecosystem", "social_dynamics", 
                                  "combat_mechanics", "housing_system", "time_manipulation", 
                                  "npc_intelligence", "guild_evolution", "dimensional_warfare"}) {
        auto agent = AIModule::getInstance().getAgent(agent_name);
        if (agent && agent->isEnabled()) {
            agent->handlePlayerEvent(event_name, char_id, param_map);
        }
    }
}

/**
 * Handle a world event
 * @param event_name Event name
 * @param params Event parameters
 */
void ai_handle_world_event(const char* event_name, const char* params) {
    if (!event_name || !params) {
        ShowError("AI Module: Invalid parameters for ai_handle_world_event\n");
        return;
    }
    
    // Parse parameters
    std::unordered_map<std::string, std::string> param_map;
    try {
        json j = json::parse(params);
        for (auto& [key, value] : j.items()) {
            param_map[key] = value.get<std::string>();
        }
    } catch (const std::exception& e) {
        ShowError("AI Module: Failed to parse parameters for ai_handle_world_event: %s\n", e.what());
        return;
    }
    
    // Notify all agents
    for (const auto& agent_name : {"world_evolution", "legend_bloodlines", "cross_class_synthesis", 
                                  "quest_generation", "economic_ecosystem", "social_dynamics", 
                                  "combat_mechanics", "housing_system", "time_manipulation", 
                                  "npc_intelligence", "guild_evolution", "dimensional_warfare"}) {
        auto agent = AIModule::getInstance().getAgent(agent_name);
        if (agent && agent->isEnabled()) {
            agent->handleWorldEvent(event_name, param_map);
        }
    }
}

/**
 * Update the AI module
 * @param tick Current tick value
 */
void ai_update(unsigned int tick) {
    // Update the AI module
    AIModule::getInstance().update(tick);
}

/**
 * Script command: AI_Request
 * Usage: AI_Request("<agent_name>", "<request_type>", "<request_data>");
 * Example: AI_Request("npc_intelligence", "generate_dialogue", "{\"npc_id\":123,\"char_id\":456,\"context\":\"greeting\"}");
 */
BUILDIN(AI_Request) {
    const char* agent_name = script_getstr(st, 2);
    const char* request_type = script_getstr(st, 3);
    const char* request_data = script_getstr(st, 4);
    
    const char* response = ai_process_script_request(agent_name, request_type, request_data);
    script_pushstr(st->stack, C_STR, response);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * Script command: AI_PlayerEvent
 * Usage: AI_PlayerEvent("<event_name>", <char_id>, "<params>");
 * Example: AI_PlayerEvent("level_up", getcharid(0), "{\"old_level\":10,\"new_level\":11}");
 */
BUILDIN(AI_PlayerEvent) {
    const char* event_name = script_getstr(st, 2);
    int char_id = script_getnum(st, 3);
    const char* params = script_getstr(st, 4);
    
    ai_handle_player_event(event_name, char_id, params);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * Script command: AI_WorldEvent
 * Usage: AI_WorldEvent("<event_name>", "<params>");
 * Example: AI_WorldEvent("server_event_start", "{\"event_name\":\"Christmas\",\"duration\":86400}");
 */
BUILDIN(AI_WorldEvent) {
    const char* event_name = script_getstr(st, 2);
    const char* params = script_getstr(st, 3);
    
    ai_handle_world_event(event_name, params);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * Register AI-related script commands
 */
void ai_register_script_commands() {
    add_scriptcmd("AI_Request", AI_Request);
    add_scriptcmd("AI_PlayerEvent", AI_PlayerEvent);
    add_scriptcmd("AI_WorldEvent", AI_WorldEvent);
}