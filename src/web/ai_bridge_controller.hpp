// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef AI_BRIDGE_CONTROLLER_HPP
#define AI_BRIDGE_CONTROLLER_HPP

#include "http.hpp"

// AI Service Configuration
namespace AIBridge {
	// AI Service endpoint configuration
	extern std::string ai_service_url;
	extern uint16 ai_service_port;
	extern std::string ai_service_api_key;
	extern bool ai_service_enabled;
	
	// Initialize AI Bridge configuration
	void initialize();
	
	// Helper function to make HTTP requests to AI service
	bool make_ai_request(
		const std::string& endpoint,
		const std::string& method,
		const std::string& body,
		std::string& response_body,
		int& status_code
	);
}

// AI Bridge API Endpoints
// These endpoints act as a bridge between rAthena and the AI Service

/**
 * POST /ai/npc/register
 * Register an NPC with the AI service
 * Request body: JSON with npc_id, name, class, level, position, personality
 * Response: JSON with registration status and assigned agent_id
 */
HANDLER_FUNC(ai_npc_register);

/**
 * POST /ai/npc/event
 * Send a game event to the AI service for processing
 * Request body: JSON with npc_id, event_type, event_data, timestamp
 * Response: JSON with event_id and processing status
 */
HANDLER_FUNC(ai_npc_event);

/**
 * GET /ai/npc/{id}/action
 * Get the next action for an NPC from the AI service
 * URL parameter: id (NPC ID)
 * Response: JSON with action type, action data, and execution parameters
 */
HANDLER_FUNC(ai_npc_action);

/**
 * POST /ai/world/state
 * Update world state in the AI service
 * Request body: JSON with state_type, state_data, timestamp
 * Response: JSON with update status
 */
HANDLER_FUNC(ai_world_state_update);

/**
 * GET /ai/world/state
 * Get current world state from the AI service
 * Query parameters: state_type (optional, e.g., "economy", "politics")
 * Response: JSON with requested world state data
 */
HANDLER_FUNC(ai_world_state_get);

/**
 * POST /ai/player/interaction
 * Handle player-NPC interaction through the AI service
 * Request body: JSON with npc_id, player_id, interaction_type, context
 * Response: JSON with NPC response (dialogue, action, etc.)
 */
HANDLER_FUNC(ai_player_interaction);

#endif

