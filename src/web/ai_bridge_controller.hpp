// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef AI_BRIDGE_CONTROLLER_HPP
#define AI_BRIDGE_CONTROLLER_HPP

#include <common/cbasetypes.hpp>

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

	// Read AI Bridge configuration from web_athena.conf
	void read_config(const char* w1, const char* w2);

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

/**
 * POST /ai/quest/generate
 * Generate a dynamic quest through the AI service
 * Request body: JSON with player_id, quest_type, difficulty, context
 * Response: JSON with generated quest details
 */
HANDLER_FUNC(ai_quest_generate);

/**
 * POST /ai/quest/{quest_id}/progress
 * Update quest progress
 * URL parameter: quest_id
 * Request body: JSON with player_id, progress_data
 * Response: JSON with updated quest status
 */
HANDLER_FUNC(ai_quest_progress);

/**
 * GET /ai/quest/{quest_id}
 * Get quest details
 * URL parameter: quest_id
 * Response: JSON with quest information
 */
HANDLER_FUNC(ai_quest_get);

/**
 * POST /ai/quest/{quest_id}/complete
 * Mark quest as complete
 * URL parameter: quest_id
 * Request body: JSON with player_id, completion_data
 * Response: JSON with rewards and completion status
 */
HANDLER_FUNC(ai_quest_complete);

/**
 * POST /ai/chat/command
 * Process free-form chat command
 * Request body: JSON with player_id, command_text, context
 * Response: JSON with AI-generated response
 */
HANDLER_FUNC(ai_chat_command);

/**
 * GET /ai/chat/history
 * Get chat history
 * Query parameters: player_id, limit, offset
 * Response: JSON with chat history
 */
HANDLER_FUNC(ai_chat_history);

/**
 * GET /ai/npc/{npc_id}/state
 * Get NPC state
 * URL parameter: npc_id
 * Response: JSON with NPC state data
 */
HANDLER_FUNC(ai_npc_state_get);

/**
 * PUT /ai/npc/{npc_id}/state
 * Update NPC state
 * URL parameter: npc_id
 * Request body: JSON with state updates
 * Response: JSON with updated state
 */
HANDLER_FUNC(ai_npc_state_update);

/**
 * DELETE /ai/npc/{npc_id}
 * Delete NPC from AI service
 * URL parameter: npc_id
 * Response: JSON with deletion status
 */
HANDLER_FUNC(ai_npc_delete);

/**
 * GET /ai/npc/{npc_id}/memory
 * Get NPC memory/relationships
 * URL parameter: npc_id
 * Query parameters: memory_type, limit
 * Response: JSON with memory data
 */
HANDLER_FUNC(ai_npc_memory_get);

/**
 * POST /ai/npc/{npc_id}/memory
 * Add memory to NPC
 * URL parameter: npc_id
 * Request body: JSON with memory data
 * Response: JSON with memory addition status
 */
HANDLER_FUNC(ai_npc_memory_add);

/**
 * POST /api/batch/npcs/update
 * Batch update multiple NPCs
 * Request body: JSON array with NPC updates
 * Response: JSON with batch operation results
 */
HANDLER_FUNC(ai_batch_npcs_update);

/**
 * POST /api/batch/players/interact
 * Batch process multiple player interactions
 * Request body: JSON array with interactions
 * Response: JSON with batch operation results
 */
HANDLER_FUNC(ai_batch_interactions);

/**
 * POST /ai/npc/{npc_id}/execute-action
 * Execute an NPC action and translate to Bridge Layer commands
 * Path parameter: npc_id - NPC identifier
 * Request body: JSON with action_type and action_data
 * Response: JSON with execution result and bridge commands
 */
HANDLER_FUNC(ai_npc_execute_action);

/**
 * GET /ai/chat/status
 * Get chat command interface status
 * Response: JSON with chat system configuration and status
 */
HANDLER_FUNC(ai_chat_status);

/**
 * GET /ai/economy/state
 * Get current economic state
 * Response: JSON with market prices, trends, and economic indicators
 */
HANDLER_FUNC(ai_economy_state);

/**
 * POST /ai/economy/price/update
 * Update item price in the economy
 * Request body: JSON with item_id, new_price, reason
 * Response: JSON with update status
 */
HANDLER_FUNC(ai_economy_price_update);

/**
 * POST /ai/economy/market/analyze
 * Request market analysis from AI
 * Request body: JSON with item_ids, category, time_range_days
 * Response: JSON with market analysis and recommendations
 */
HANDLER_FUNC(ai_economy_market_analyze);

/**
 * GET /ai/faction/list
 * Get list of all factions
 * Response: JSON array of faction data
 */
HANDLER_FUNC(ai_faction_list);

/**
 * POST /ai/faction/create
 * Create a new faction
 * Request body: JSON with faction_id, name, description, alignment
 * Response: JSON with creation status
 */
HANDLER_FUNC(ai_faction_create);

/**
 * POST /ai/faction/reputation/update
 * Update player reputation with a faction
 * Request body: JSON with player_id, faction_id, change, reason
 * Response: JSON with updated reputation
 */
HANDLER_FUNC(ai_faction_reputation_update);

/**
 * GET /ai/faction/reputation/:player_id
 * Get player's reputation with all factions
 * URL parameter: player_id
 * Response: JSON with reputation data for all factions
 */
HANDLER_FUNC(ai_faction_reputation_get);

#endif

