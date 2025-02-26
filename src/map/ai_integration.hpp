/**
 * @file ai_integration.hpp
 * @brief AI integration for map server
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the integration between the map server and the AI module.
 */

#ifndef AI_INTEGRATION_HPP
#define AI_INTEGRATION_HPP

#include "../ai/ai_module.hpp"

/**
 * Initialize the AI module for the map server
 * @return True if initialization was successful, false otherwise
 */
bool ai_init();

/**
 * Finalize the AI module for the map server
 */
void ai_final();

/**
 * Process an AI request from a script
 * @param agent_name Agent name
 * @param request_type Request type
 * @param request_data Request data
 * @return Response data
 */
const char* ai_process_script_request(const char* agent_name, const char* request_type, const char* request_data);

/**
 * Handle a player event
 * @param event_name Event name
 * @param char_id Character ID
 * @param params Event parameters
 */
void ai_handle_player_event(const char* event_name, int char_id, const char* params);

/**
 * Handle a world event
 * @param event_name Event name
 * @param params Event parameters
 */
void ai_handle_world_event(const char* event_name, const char* params);

/**
 * Update the AI module
 * @param tick Current tick value
 */
void ai_update(unsigned int tick);

/**
 * Register AI-related script commands
 */
void ai_register_script_commands();

#endif // AI_INTEGRATION_HPP