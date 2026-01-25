// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mob_ml_executor.hpp"

#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/random.hpp>

#include "battle.hpp"
#include "skill.hpp"
#include "unit.hpp"
#include "path.hpp"

using namespace rathena;

/**
 * Execute ML decision
 */
bool MobMLExecutor::execute(struct mob_data* md, const MobMLGateway::MLDecision& decision) {
	nullpo_retr(false, md);
	
	// Should not execute TRADITIONAL_AI action
	if (decision.action == MobMLGateway::MLAction::TRADITIONAL_AI) {
		return false;
	}
	
	// Execute action based on type
	switch (decision.action) {
		case MobMLGateway::MLAction::IDLE:
			return execute_idle(md);
			
		case MobMLGateway::MLAction::ATTACK:
			return execute_attack(md);
			
		case MobMLGateway::MLAction::MOVE_CLOSER:
			return execute_move_closer(md);
			
		case MobMLGateway::MLAction::MOVE_AWAY:
			return execute_move_away(md);
			
		case MobMLGateway::MLAction::MOVE_RANDOM:
			return execute_move_random(md);
			
		case MobMLGateway::MLAction::SKILL_1:
			return execute_skill(md, 0);
			
		case MobMLGateway::MLAction::SKILL_2:
			return execute_skill(md, 1);
			
		case MobMLGateway::MLAction::SKILL_3:
			return execute_skill(md, 2);
			
		case MobMLGateway::MLAction::CHANGE_TARGET:
			return execute_change_target(md);
			
		case MobMLGateway::MLAction::FLEE:
			return execute_flee(md);
			
		default:
			ShowWarning("[ML-EXECUTOR] Unknown action %d for mob %d\n", 
			            (int)decision.action, md->id);
			return false;
	}
	
	return false;
}

/**
 * Execute IDLE action
 * Stop all actions and wait
 */
bool MobMLExecutor::execute_idle(struct mob_data* md) {
	// Stop walking
	if (md->ud.walktimer != INVALID_TIMER) {
		unit_stop_walking(md, USW_FIXPOS);
	}
	
	// Stop attacking
	if (md->ud.attacktimer != INVALID_TIMER) {
		unit_stop_attack(md);
	}
	
	// Set to idle state
	mob_setstate(*md, MSS_IDLE);
	
	return true;
}

/**
 * Execute ATTACK action
 * Attack current target or find new one
 */
bool MobMLExecutor::execute_attack(struct mob_data* md) {
	// Check if we have a valid target
	block_list* target = nullptr;
	
	if (md->target_id > 0) {
		target = map_id2bl(md->target_id);
	}
	
	// If no target or invalid target, try to find one
	if (!target || target->prev == nullptr) {
		// Find nearest enemy
		int32 view_range = md->db->range2;
		target = battle_getenemy(md, DEFAULT_ENEMY_TYPE(md), view_range);
		
		if (target) {
			md->target_id = target->id;
		} else {
			// No target found, cannot attack
			return false;
		}
	}
	
	// Check if in range
	if (!battle_check_range(md, target, md->status.rhw.range)) {
		// Not in range, move closer first
		return execute_move_closer(md);
	}
	
	// Execute attack
	mob_setstate(*md, MSS_BERSERK);
	unit_attack(md, target->id, 1);
	
	return true;
}

/**
 * Execute MOVE_CLOSER action
 * Move towards current target
 */
bool MobMLExecutor::execute_move_closer(struct mob_data* md) {
	// Check if we have a target
	block_list* target = nullptr;
	
	if (md->target_id > 0) {
		target = map_id2bl(md->target_id);
	}
	
	if (!target || target->prev == nullptr) {
		// No target, cannot move closer
		return false;
	}
	
	// Check if can move
	if (!status_has_mode(&md->status, MD_CANMOVE) || !unit_can_move(md)) {
		return false;
	}
	
	// Move towards target
	mob_setstate(*md, MSS_RUSH);
	bool success = unit_walktobl(md, target, md->status.rhw.range, 1);
	
	return success;
}

/**
 * Execute MOVE_AWAY action
 * Retreat from current target or danger
 */
bool MobMLExecutor::execute_move_away(struct mob_data* md) {
	// Check if can move
	if (!status_has_mode(&md->status, MD_CANMOVE) || !unit_can_move(md)) {
		return false;
	}
	
	// Determine what to move away from
	block_list* threat = nullptr;
	
	if (md->target_id > 0) {
		threat = map_id2bl(md->target_id);
	}
	
	if (!threat && md->attacked_id > 0) {
		threat = map_id2bl(md->attacked_id);
	}
	
	if (!threat) {
		// No specific threat, just move somewhere
		return execute_move_random(md);
	}
	
	// Calculate retreat position (opposite direction from threat)
	int32 dx = md->x - threat->x;
	int32 dy = md->y - threat->y;
	
	// Normalize and extend
	float len = sqrtf((float)(dx*dx + dy*dy));
	if (len < 1.0f) len = 1.0f;
	
	int32 retreat_dist = 5;  // Move 5 cells away
	int16 dest_x = md->x + (int16)((float)dx / len * retreat_dist);
	int16 dest_y = md->y + (int16)((float)dy / len * retreat_dist);
	
	// Ensure destination is on map
	struct map_data* mapdata = map_getmapdata(md->m);
	dest_x = (dest_x < 0) ? 0 : (dest_x >= mapdata->xs) ? mapdata->xs - 1 : dest_x;
	dest_y = (dest_y < 0) ? 0 : (dest_y >= mapdata->ys) ? mapdata->ys - 1 : dest_y;
	
	// Check if destination is walkable
	if (!map_getcell(md->m, dest_x, dest_y, CELL_CHKPASS)) {
		// Try nearby cells
		for (int32 i = 0; i < 8; i++) {
			int16 alt_x = dest_x + ((i % 3) - 1);
			int16 alt_y = dest_y + ((i / 3) - 1);
			
			if (map_getcell(md->m, alt_x, alt_y, CELL_CHKPASS)) {
				dest_x = alt_x;
				dest_y = alt_y;
				break;
			}
		}
	}
	
	return unit_walktoxy(md, dest_x, dest_y, 0) == 0;
}

/**
 * Execute MOVE_RANDOM action
 * Random walk
 */
bool MobMLExecutor::execute_move_random(struct mob_data* md) {
	// Use existing random walk function
	t_tick tick = gettick();
	return mob_randomwalk(md, tick) != 0;
}

/**
 * Execute SKILL action
 * Use monster skill from skill database
 */
bool MobMLExecutor::execute_skill(struct mob_data* md, int skill_slot) {
	t_tick tick = gettick();
	
	// Check if monster has skills
	if (md->db->skill.empty()) {
		return false;
	}
	
	// Get skill at slot (if available)
	size_t skill_index = (size_t)skill_slot;
	if (skill_index >= md->db->skill.size()) {
		// Skill slot doesn't exist, try first available skill
		skill_index = 0;
	}
	
	// Check if skill is ready (not on cooldown)
	if (skill_index < MAX_MOBSKILL) {
		if (md->skilldelay[skill_index] > 0 && 
		    DIFF_TICK(tick, md->skilldelay[skill_index]) < 0) {
			// Skill on cooldown
			return false;
		}
	}
	
	// Try to use skill
	md->skill_idx = (int8)skill_index;
	
	// Use mobskill_use to execute the skill
	// This handles all the complexity of skill targeting, range checking, etc.
	bool skill_used = mobskill_use(md, tick, -1);
	
	if (!skill_used) {
		md->skill_idx = -1;
	}
	
	return skill_used;
}

/**
 * Execute CHANGE_TARGET action
 * Find and switch to a different target
 */
bool MobMLExecutor::execute_change_target(struct mob_data* md) {
	// Find alternative target
	block_list* new_target = find_alternative_target(md);
	
	if (!new_target) {
		return false;
	}
	
	// Switch target
	md->target_id = new_target->id;
	
	// Set to rush mode
	mob_setstate(*md, MSS_RUSH);
	
	return true;
}

/**
 * Execute FLEE action
 * Full retreat to spawn point
 */
bool MobMLExecutor::execute_flee(struct mob_data* md) {
	// Check if can move
	if (!status_has_mode(&md->status, MD_CANMOVE) || !unit_can_move(md)) {
		return false;
	}
	
	// Check if has spawn point
	if (!md->spawn) {
		return execute_move_random(md);
	}
	
	// Clear target (stop chasing)
	t_tick tick = gettick();
	mob_unlocktarget(md, tick);
	
	// Move to spawn point
	int16 spawn_x = md->centerX;
	int16 spawn_y = md->centerY;
	
	mob_setstate(*md, MSS_WALK);
	
	return unit_walktoxy(md, spawn_x, spawn_y, 0) == 0;
}

/**
 * Helper: Find alternative target
 * Searches for different enemy than current target
 */
block_list* MobMLExecutor::find_alternative_target(struct mob_data* md) {
	int32 view_range = md->db->range2;
	int32 current_target_id = md->target_id;
	
	// Use battle_getenemy to find nearest enemy
	block_list* new_target = battle_getenemy(md, DEFAULT_ENEMY_TYPE(md), view_range);
	
	// If found the same target, try to find another
	if (new_target && new_target->id == current_target_id) {
		// Try searching in different range
		new_target = battle_getenemy(md, DEFAULT_ENEMY_TYPE(md), view_range / 2);
		
		if (new_target && new_target->id == current_target_id) {
			// Still same target, give up
			return nullptr;
		}
	}
	
	return new_target;
}

/**
 * Helper: Move towards position
 */
bool MobMLExecutor::move_towards(struct mob_data* md, int16 x, int16 y, int32 range) {
	return unit_walktoxy(md, x, y, range) == 0;
}

/**
 * Helper: Move away from position
 */
bool MobMLExecutor::move_away_from(struct mob_data* md, int16 x, int16 y, int32 distance) {
	// Calculate opposite direction
	int32 dx = md->x - x;
	int32 dy = md->y - y;
	
	// Normalize
	float len = sqrtf((float)(dx*dx + dy*dy));
	if (len < 1.0f) len = 1.0f;
	
	// Calculate destination
	int16 dest_x = md->x + (int16)((float)dx / len * distance);
	int16 dest_y = md->y + (int16)((float)dy / len * distance);
	
	return unit_walktoxy(md, dest_x, dest_y, 0) == 0;
}
