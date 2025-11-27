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
// Note: BUILDIN_FUNC macro expands to: int32 buildin_<name>(struct script_state* st)
// We use explicit declarations here to avoid header dependency on script.hpp

/**
 * @brief HTTP GET script command
 *
 * Usage: .@response$ = httpget("http://192.168.0.100:8000/api/v1/health");
 *
 * @param st Script state
 * @return SCRIPT_CMD_SUCCESS or SCRIPT_CMD_FAILURE
 */
int32 buildin_httpget(struct script_state* st);

/**
 * @brief HTTP POST script command
 *
 * Usage: .@response$ = httppost("http://192.168.0.100:8000/api/v1/events/dispatch",
 *                               "{\"event_type\":\"player_login\"}");
 *
 * @param st Script state
 * @return SCRIPT_CMD_SUCCESS or SCRIPT_CMD_FAILURE
 */
int32 buildin_httppost(struct script_state* st);

/**
 * @brief Get Bridge Layer status
 *
 * Usage: .@status$ = ai_bridge_status();
 * Returns JSON with system status
 *
 * @param st Script state
 * @return SCRIPT_CMD_SUCCESS
 */
int32 buildin_ai_bridge_status(struct script_state* st);

/**
 * @brief Reload Bridge Layer configuration
 *
 * Usage: ai_bridge_reload();
 *
 * @param st Script state
 * @return SCRIPT_CMD_SUCCESS or SCRIPT_CMD_FAILURE
 */
int32 buildin_ai_bridge_reload(struct script_state* st);

// Initialize script commands (called during server startup)
void script_ai_bridge_init();

// Cleanup (called during server shutdown)
void script_ai_bridge_final();

#endif // SCRIPT_AI_BRIDGE_HPP