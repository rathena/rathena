// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
/**
 * @file script_ai_bridge.hpp
 * @brief NPC script commands for AI Bridge Layer
 * 
 * Provides script commands:
 * - httpget(url$) - HTTP GET request
 * - httppost(url$, json_body$) - HTTP POST request
 * 
 * These commands are callable from NPC scripts to interact with AI service
 */

#ifndef SCRIPT_AI_BRIDGE_HPP
#define SCRIPT_AI_BRIDGE_HPP

#include <common/cbasetypes.hpp>

struct script_state;

// Script command declarations
// These will be registered with BUILDIN_DEF in script.cpp

/**
 * @brief HTTP GET script command
 * 
 * Usage: .@response$ = httpget("http://192.168.0.100:8000/api/v1/health");
 * 
 * @param st Script state
 * @return SCRIPT_CMD_SUCCESS or SCRIPT_CMD_FAILURE
 */
BUILDIN_FUNC(httpget);

/**
 * @brief HTTP POST script command
 * 
 * Usage: .@response$ = httppost("http://192.168.0.100:8000/api/v1/events/dispatch", 
 *                               "{\"event_type\":\"player_login\"}");
 * 
 * @param st Script state
 * @return SCRIPT_CMD_SUCCESS or SCRIPT_CMD_FAILURE
 */
BUILDIN_FUNC(httppost);

/**
 * @brief Get Bridge Layer status
 * 
 * Usage: .@status$ = ai_bridge_status();
 * Returns JSON with system status
 * 
 * @param st Script state
 * @return SCRIPT_CMD_SUCCESS
 */
BUILDIN_FUNC(ai_bridge_status);

/**
 * @brief Reload Bridge Layer configuration
 * 
 * Usage: ai_bridge_reload();
 * 
 * @param st Script state
 * @return SCRIPT_CMD_SUCCESS or SCRIPT_CMD_FAILURE
 */
BUILDIN_FUNC(ai_bridge_reload);

// Initialize script commands (called during server startup)
void script_ai_bridge_init();

// Cleanup (called during server shutdown)
void script_ai_bridge_final();

#endif // SCRIPT_AI_BRIDGE_HPP