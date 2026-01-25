// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mob_ml_encoder.hpp"

#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/timer.hpp>
#include <common/random.hpp>

#include "pc.hpp"
#include "skill.hpp"
#include "guild.hpp"

using namespace rathena;

/**
 * Encode complete 64-dimensional state vector from mob_data
 */
std::vector<float> MobMLEncoder::encode_state(const struct mob_data* md) {
	nullpo_retr(std::vector<float>(), md);
	
	// Initialize 64-dimensional vector
	std::vector<float> state(STATE_DIM, 0.0f);
	
	// Encode each awareness category
	encode_self_awareness(md, state, 0);    // dims 0-11
	encode_environment(md, state, 12);       // dims 12-21
	encode_social(md, state, 22);            // dims 22-29
	encode_tactical(md, state, 30);          // dims 30-41
	encode_temporal(md, state, 42);          // dims 42-51
	encode_goals(md, state, 52);             // dims 52-63
	
	return state;
}

/**
 * Self-Awareness Encoding (dims 0-11: 12 dimensions)
 * 
 * Encodes monster's awareness of its own state:
 * 0:  HP ratio (current HP / max HP)
 * 1:  SP ratio (current SP / max SP)
 * 2:  Level (normalized 0-150)
 * 3:  Mobility (can move, speed)
 * 4:  Combat power (ATK normalized)
 * 5:  Skill readiness (skills available)
 * 6:  Status effects (negative status count)
 * 7:  Equipment quality (N/A for monsters, set to 0)
 * 8:  Archetype ID (0-5 for 6 archetypes)
 * 9:  Role confidence (based on mode flags)
 * 10: Performance score (kill/death ratio estimate)
 * 11: Learning progress (N/A for C++, set to 0)
 */
void MobMLEncoder::encode_self_awareness(const struct mob_data* md, std::vector<float>& state, size_t offset) {
	// 0: HP ratio
	state[offset + 0] = (md->status.max_hp > 0) ? 
		(float)md->status.hp / (float)md->status.max_hp : 0.0f;
	
	// 1: SP ratio
	state[offset + 1] = (md->status.max_sp > 0) ? 
		(float)md->status.sp / (float)md->status.max_sp : 0.0f;
	
	// 2: Level (normalized to 0-1, max level 150)
	state[offset + 2] = normalize((float)md->level, 1.0f, 150.0f);
	
	// 3: Mobility (can move + speed factor)
	float can_move = status_has_mode(&md->status, MD_CANMOVE) ? 1.0f : 0.0f;
	float speed_factor = normalize(md->status.speed, MIN_WALK_SPEED, MAX_WALK_SPEED);
	state[offset + 3] = can_move * (1.0f - speed_factor);  // Higher speed = higher mobility
	
	// 4: Combat power (ATK normalized)
	float atk = (float)(md->status.rhw.atk + md->status.rhw.atk2) / 2.0f;
	state[offset + 4] = normalize(atk, 0.0f, 10000.0f);  // Max ~10K ATK
	
	// 5: Skill readiness (ratio of skills ready vs total skills)
	float skills_ready = 0.0f;
	float total_skills = 0.0f;
	t_tick tick = gettick();
	
	if (!md->db->skill.empty()) {
		total_skills = (float)md->db->skill.size();
		for (size_t i = 0; i < md->db->skill.size() && i < MAX_MOBSKILL; i++) {
			if (md->skilldelay[i] == 0 || DIFF_TICK(tick, md->skilldelay[i]) >= 0) {
				skills_ready += 1.0f;
			}
		}
		state[offset + 5] = skills_ready / total_skills;
	} else {
		state[offset + 5] = 0.0f;
	}
	
	// 6: Status effects (count negative status effects, normalized)
	int32 neg_status = 0;
	if (md->sc.opt1 && md->sc.opt1 != OPT1_STONEWAIT && md->sc.opt1 != OPT1_BURNING) neg_status++;
	if (md->sc.getSCE(SC_FREEZE)) neg_status++;
	if (md->sc.getSCE(SC_STUN)) neg_status++;
	if (md->sc.getSCE(SC_SLEEP)) neg_status++;
	if (md->sc.getSCE(SC_STONE)) neg_status++;
	if (md->sc.getSCE(SC_CURSE)) neg_status++;
	if (md->sc.getSCE(SC_BLIND)) neg_status++;
	if (md->sc.getSCE(SC_POISON)) neg_status++;
	if (md->sc.getSCE(SC_SILENCE)) neg_status++;
	if (md->sc.getSCE(SC_CONFUSION)) neg_status++;
	state[offset + 6] = normalize((float)neg_status, 0.0f, 10.0f);
	
	// 7: Equipment quality (N/A for monsters, always 0)
	state[offset + 7] = 0.0f;
	
	// 8: Archetype ID (determine from mode and stats)
	// 0=aggressive, 1=defensive, 2=support, 3=mage, 4=tank, 5=ranged
	float archetype_id = 0.0f;  // Default aggressive
	
	if (status_has_mode(&md->status, MD_STATUSIMMUNE) || md->status.max_hp > 100000) {
		archetype_id = 4.0f;  // Tank (boss/high HP)
	} else if (md->status.rhw.range > 3) {
		archetype_id = 5.0f;  // Ranged
	} else if (md->status.int_ > md->status.str && md->status.rhw.atk < 500) {
		archetype_id = 3.0f;  // Mage (high INT, low ATK)
	} else if (md->status.def > 50 || md->status.mdef > 30) {
		archetype_id = 1.0f;  // Defensive (high DEF)
	} else if (!md->db->skill.empty()) {
		// Check for support skills
		bool has_support = false;
		for (const auto& skill : md->db->skill) {
			if (skill->skill_id == AL_HEAL || skill->skill_id == AL_BLESSING || 
			    skill->skill_id == AL_INCAGI) {
				has_support = true;
				break;
			}
		}
		if (has_support) archetype_id = 2.0f;  // Support
	}
	
	state[offset + 8] = normalize(archetype_id, 0.0f, 5.0f);
	
	// 9: Role confidence (based on how well stats match archetype)
	state[offset + 9] = 0.75f;  // Default confidence
	
	// 10: Performance score (simplified: based on current HP vs initial)
	state[offset + 10] = state[offset + 0];  // Use HP ratio as proxy
	
	// 11: Learning progress (N/A for C++, set to 0)
	state[offset + 11] = 0.0f;
}

/**
 * Environmental Awareness Encoding (dims 12-21: 10 dimensions)
 * 
 * 12: Terrain type (walkable, water, cliff)
 * 13: Obstacles nearby (unwalkable cells in 3-cell radius)
 * 14: Items nearby (loot on ground)
 * 15: Hazards (traps, AoE skills)
 * 16: Player count (in view range)
 * 17: Monster count (in view range)
 * 18: Distance to spawn (normalized)
 * 19: Map danger level (based on map flags)
 * 20: Weather/time (day/night)
 * 21: Area control (friendly vs enemy territory)
 */
void MobMLEncoder::encode_environment(const struct mob_data* md, std::vector<float>& state, size_t offset) {
	// 12: Terrain type (current cell)
	int32 cell_type = map_getcell(md->m, md->x, md->y, CELL_GETTYPE);
	state[offset + 0] = (float)cell_type / 3.0f;  // Normalize cell types
	
	// 13: Obstacles nearby (count unwalkable cells in 3-cell radius)
	int32 obstacles = 0;
	for (int32 dx = -3; dx <= 3; dx++) {
		for (int32 dy = -3; dy <= 3; dy++) {
			if (dx == 0 && dy == 0) continue;
			if (!map_getcell(md->m, md->x + dx, md->y + dy, CELL_CHKPASS)) {
				obstacles++;
			}
		}
	}
	state[offset + 1] = normalize((float)obstacles, 0.0f, 49.0f);  // Max 7×7-1 = 48
	
	// 14: Items nearby (loot within 5 cells)
	int32 items_nearby = count_entities_in_range(md, 5, BL_ITEM);
	state[offset + 2] = normalize((float)items_nearby, 0.0f, 10.0f);
	
	// 15: Hazards (skill units nearby)
	int32 hazards = count_entities_in_range(md, 5, BL_SKILL);
	state[offset + 3] = normalize((float)hazards, 0.0f, 10.0f);
	
	// 16: Player count (in view range)
	int32 view_range = md->db->range2;
	int32 players = count_entities_in_range(md, view_range, BL_PC);
	state[offset + 4] = normalize((float)players, 0.0f, 20.0f);
	
	// 17: Monster count (in view range)
	int32 monsters = count_entities_in_range(md, view_range, BL_MOB);
	state[offset + 5] = normalize((float)monsters, 0.0f, 30.0f);
	
	// 18: Distance to spawn point
	float dist_spawn = 0.0f;
	if (md->spawn) {
		int32 dx = md->x - md->centerX;
		int32 dy = md->y - md->centerY;
		dist_spawn = sqrtf((float)(dx*dx + dy*dy));
	}
	state[offset + 6] = normalize(dist_spawn, 0.0f, 50.0f);
	
	// 19: Map danger level (based on map flags)
	struct map_data* mapdata = map_getmapdata(md->m);
	float danger = 0.0f;
	if (mapdata->getMapFlag(MF_PVP)) danger += 0.3f;
	if (mapdata->getMapFlag(MF_GVG)) danger += 0.5f;
	if (map_flag_gvg2(md->m)) danger += 0.4f;
	if (mapdata->getMapFlag(MF_BATTLEGROUND)) danger += 0.4f;
	state[offset + 7] = clamp01(danger);
	
	// 20: Weather/time (night = 0.0, day = 1.0)
	state[offset + 8] = night_flag ? 0.0f : 1.0f;
	
	// 21: Area control (ratio of allies to enemies in range)
	int32 allies = count_entities_in_range(md, 10, BL_MOB);
	int32 enemies = players;  // From offset + 4
	float control = (allies + enemies > 0) ? (float)allies / (float)(allies + enemies) : 0.5f;
	state[offset + 9] = control;
}

/**
 * Social Awareness Encoding (dims 22-29: 8 dimensions)
 * 
 * 22: Pack size (number of nearby allied monsters)
 * 23: Pack cohesion (avg distance to pack members)
 * 24: Ally HP average
 * 25: Leader present (is there a master/leader)
 * 26: Formation quality (pack position variance)
 * 27: Communication load (N/A, set to 0)
 * 28: Ally skills available (count ally skills ready)
 * 29: Trust level (N/A, set to 0.5)
 */
void MobMLEncoder::encode_social(const struct mob_data* md, std::vector<float>& state, size_t offset) {
	// 22: Pack size (nearby same-type monsters)
	int32 pack_size = 0;
	int32 pack_member_count = 0;
	float total_dist = 0.0f;
	float total_hp = 0.0f;
	
	// Count allies within 15 cells
	struct count_allies_data {
		const mob_data* self_md;
		int32* pack_count;
		float* total_distance;
		float* total_hp_ratio;
	};
	
	// Simplified: count nearby monsters of same mob_id as pack
	for (int32 dx = -15; dx <= 15; dx++) {
		for (int32 dy = -15; dy <= 15; dy++) {
			if (dx == 0 && dy == 0) continue;
			
			int32 x = md->x + dx;
			int32 y = md->y + dy;
			
			// Check if cell is valid
			if (x < 0 || y < 0) continue;
			
			// This is simplified - in real implementation would use map_foreachinrange
			// but that requires callback structure
		}
	}
	
	// Simplified pack detection: use master_id as indicator
	if (md->master_id > 0) {
		pack_size = 1;  // Has master, part of pack
		pack_member_count = mob_countslave(const_cast<block_list*>(reinterpret_cast<const block_list*>(md)));
	}
	
	state[offset + 0] = normalize((float)pack_size, 0.0f, 10.0f);
	
	// 23: Pack cohesion (simplified: 1.0 if has master, 0.0 otherwise)
	state[offset + 1] = (md->master_id > 0) ? 0.8f : 0.0f;
	
	// 24: Ally HP average (simplified: self HP if no pack)
	state[offset + 2] = state[0];  // Use self HP ratio
	
	// 25: Leader present
	state[offset + 3] = (md->master_id > 0) ? 1.0f : 0.0f;
	
	// 26: Formation quality (simplified)
	state[offset + 4] = (pack_member_count > 0) ? 0.7f : 0.0f;
	
	// 27: Communication load (N/A)
	state[offset + 5] = 0.0f;
	
	// 28: Ally skills available (simplified)
	state[offset + 6] = state[5];  // Reuse self skill readiness
	
	// 29: Trust level (default)
	state[offset + 7] = 0.5f;
}

/**
 * Tactical Awareness Encoding (dims 30-41: 12 dimensions)
 * 
 * 30: Enemy count (in attack range)
 * 31: Nearest enemy distance
 * 32: Threat level (DPS estimate of enemies)
 * 33: DPS received (recent damage rate)
 * 34: Enemy HP ratio (target HP if exists)
 * 35: Enemy class (target type)
 * 36: Enemy skill threat
 * 37: Positioning quality (distance to ideal position)
 * 38: Escape routes (number of walkable paths)
 * 39: Advantage score (relative power)
 * 40: Surprise factor (did we initiate)
 * 41: Strategic context (current state)
 */
void MobMLEncoder::encode_tactical(const struct mob_data* md, std::vector<float>& state, size_t offset) {
	// 30: Enemy count (players + enemy mobs in range)
	int32 view_range = md->db->range2;
	int32 enemy_count = count_entities_in_range(md, view_range, BL_PC);
	state[offset + 0] = normalize((float)enemy_count, 0.0f, 10.0f);
	
	// 31-36: Target-related features
	block_list* target = nullptr;
	if (md->target_id > 0) {
		target = map_id2bl(md->target_id);
	}
	
	if (target && target->prev) {
		// 31: Distance to target
		float dist = (float)distance_bl(md, target);
		state[offset + 1] = normalize(dist, 0.0f, (float)view_range);
		
		// 32: Threat level of target
		state[offset + 2] = calculate_threat_level(md, target);
		
		// 33: DPS received (estimate from recent damage, simplified)
		state[offset + 3] = (md->status.hp < md->status.max_hp) ? 0.3f : 0.0f;
		
		// 34: Enemy HP ratio
		int32 target_hp = status_get_hp(target);
		int32 target_max_hp = status_get_max_hp(target);
		state[offset + 4] = (target_max_hp > 0) ? 
			(float)target_hp / (float)target_max_hp : 0.0f;
		
		// 35: Enemy class (PC=1.0, MOB=0.5, other=0.0)
		if (target->type == BL_PC) {
			state[offset + 5] = 1.0f;
		} else if (target->type == BL_MOB) {
			state[offset + 5] = 0.5f;
		} else {
			state[offset + 5] = 0.0f;
		}
		
		// 36: Enemy skill threat (estimate based on target type)
		if (target->type == BL_PC) {
			map_session_data* sd = (map_session_data*)target;
			state[offset + 6] = normalize((float)sd->status.base_level, 1.0f, 150.0f);
		} else {
			state[offset + 6] = 0.3f;
		}
	} else {
		// No target
		state[offset + 1] = 1.0f;  // Max distance (no target)
		state[offset + 2] = 0.0f;  // No threat
		state[offset + 3] = 0.0f;  // No DPS
		state[offset + 4] = 0.0f;  // No target HP
		state[offset + 5] = 0.0f;  // No class
		state[offset + 6] = 0.0f;  // No skill threat
	}
	
	// 37: Positioning quality (distance to ideal position, simplified)
	float pos_quality = 1.0f;
	if (md->spawn) {
		int32 dx = md->x - md->centerX;
		int32 dy = md->y - md->centerY;
		float dist_from_center = sqrtf((float)(dx*dx + dy*dy));
		pos_quality = 1.0f - normalize(dist_from_center, 0.0f, 20.0f);
	}
	state[offset + 7] = clamp01(pos_quality);
	
	// 38: Escape routes (count walkable adjacent cells)
	int32 escape_routes = 0;
	for (int32 dx = -1; dx <= 1; dx++) {
		for (int32 dy = -1; dy <= 1; dy++) {
			if (dx == 0 && dy == 0) continue;
			if (map_getcell(md->m, md->x + dx, md->y + dy, CELL_CHKPASS)) {
				escape_routes++;
			}
		}
	}
	state[offset + 8] = normalize((float)escape_routes, 0.0f, 8.0f);
	
	// 39: Advantage score (self HP vs target HP, simplified)
	if (target && target->prev) {
		int32 target_hp = status_get_hp(target);
		int32 target_max_hp = status_get_max_hp(target);
		float target_ratio = (target_max_hp > 0) ? (float)target_hp / (float)target_max_hp : 0.0f;
		float self_ratio = state[0];  // Self HP ratio
		state[offset + 9] = (self_ratio > target_ratio) ? 0.7f : 0.3f;
	} else {
		state[offset + 9] = 0.5f;  // Neutral
	}
	
	// 40: Surprise factor (did monster initiate combat)
	state[offset + 10] = (md->state.aggressive) ? 1.0f : 0.0f;
	
	// 41: Strategic context (current skill state as proxy)
	float context = 0.0f;
	switch (md->state.skillstate) {
		case MSS_IDLE:    context = 0.0f; break;
		case MSS_WALK:    context = 0.2f; break;
		case MSS_LOOT:    context = 0.3f; break;
		case MSS_RUSH:    context = 0.6f; break;
		case MSS_FOLLOW:  context = 0.5f; break;
		case MSS_BERSERK: context = 0.9f; break;
		case MSS_ANGRY:   context = 0.8f; break;
		default:          context = 0.5f; break;
	}
	state[offset + 11] = context;
}

/**
 * Temporal Awareness Encoding (dims 42-51: 10 dimensions)
 * 
 * 42: Recent damage history (HP lost in last 10s)
 * 43: Skill cooldowns (average cooldown progress)
 * 44: Combat duration (time since first combat)
 * 45: Action success rate (simplified)
 * 46: Pattern recognition (N/A, set to 0)
 * 47: Time since spawn
 * 48: Recent deaths (N/A for individual, set to 0)
 * 49: Healing received (simplified)
 * 50: State changes (mode changes, simplified)
 * 51: Flow state (combat momentum)
 */
void MobMLEncoder::encode_temporal(const struct mob_data* md, std::vector<float>& state, size_t offset) {
	t_tick tick = gettick();
	
	// 42: Recent damage (1.0 - current HP ratio indicates damage taken)
	state[offset + 0] = 1.0f - state[0];
	
	// 43: Skill cooldowns (average progress)
	float cd_progress = 0.0f;
	int32 skill_count = 0;
	
	if (!md->db->skill.empty()) {
		for (size_t i = 0; i < md->db->skill.size() && i < MAX_MOBSKILL; i++) {
			if (md->db->skill[i]->delay > 0) {
				skill_count++;
				if (md->skilldelay[i] == 0) {
					cd_progress += 1.0f;  // Ready
				} else {
					t_tick remaining = DIFF_TICK(md->skilldelay[i], tick);
					if (remaining > 0) {
						float progress = 1.0f - ((float)remaining / (float)md->db->skill[i]->delay);
						cd_progress += clamp01(progress);
					} else {
						cd_progress += 1.0f;  // Ready
					}
				}
			}
		}
		state[offset + 1] = (skill_count > 0) ? cd_progress / (float)skill_count : 1.0f;
	} else {
		state[offset + 1] = 1.0f;  // No skills, always ready
	}
	
	// 44: Combat duration (time since last PC near time)
	float combat_duration = 0.0f;
	if (md->last_pcneartime > 0) {
		t_tick combat_time = DIFF_TICK(tick, md->last_pcneartime);
		combat_duration = normalize((float)combat_time, 0.0f, 30000.0f);  // Max 30 seconds
	}
	state[offset + 2] = combat_duration;
	
	// 45: Action success rate (simplified: use HP ratio as proxy)
	state[offset + 3] = state[0];
	
	// 46: Pattern recognition (N/A)
	state[offset + 4] = 0.0f;
	
	// 47: Time since spawn
	float time_alive = 0.0f;
	if (md->spawn) {
		// Use next_thinktime as proxy for how long monster has existed
		time_alive = normalize((float)DIFF_TICK(tick, md->next_thinktime), 0.0f, 300000.0f);  // Max 5 min
	}
	state[offset + 5] = clamp01(time_alive);
	
	// 48: Recent deaths (N/A)
	state[offset + 6] = 0.0f;
	
	// 49: Healing received (1.0 if HP increased, simplified)
	state[offset + 7] = (md->status.hp == md->status.max_hp) ? 1.0f : 0.0f;
	
	// 50: State changes (track aggressive/provoked states)
	float state_changes = 0.0f;
	if (md->state.aggressive) state_changes += 0.5f;
	if (md->state.provoke_flag) state_changes += 0.5f;
	state[offset + 8] = state_changes;
	
	// 51: Flow state (combat momentum: attacking = high, idle = low)
	float flow = 0.0f;
	if (md->ud.attacktimer != INVALID_TIMER) flow = 0.9f;
	else if (md->ud.walktimer != INVALID_TIMER) flow = 0.5f;
	else if (md->ud.skilltimer != INVALID_TIMER) flow = 1.0f;
	else flow = 0.1f;
	state[offset + 9] = flow;
}

/**
 * Goal-Oriented Awareness Encoding (dims 52-63: 12 dimensions)
 * 
 * 52: Primary goal (current state/objective)
 * 53: Secondary goal
 * 54: Goal progress (toward objective)
 * 55: Priority weights (goal importance)
 * 56: Resource needs (HP/SP needs)
 * 57: Risk tolerance (based on mode)
 * 58: Cooperation desire (assist mode)
 * 59: Exploration tendency (random walk vs focused)
 * 60: Aggression level (aggressive mode)
 * 61: Defense priority (defensive posture)
 * 62: Support priority (helping allies)
 * 63: Adaptation rate (N/A, set to 0.5)
 */
void MobMLEncoder::encode_goals(const struct mob_data* md, std::vector<float>& state, size_t offset) {
	// 52: Primary goal (based on current skill state)
	float primary_goal = 0.0f;
	switch (md->state.skillstate) {
		case MSS_IDLE:    primary_goal = 0.0f; break;  // Wander/patrol
		case MSS_WALK:    primary_goal = 0.1f; break;  // Move
		case MSS_LOOT:    primary_goal = 0.2f; break;  // Collect items
		case MSS_RUSH:    primary_goal = 0.7f; break;  // Chase enemy
		case MSS_FOLLOW:  primary_goal = 0.6f; break;  // Follow target
		case MSS_BERSERK: primary_goal = 1.0f; break;  // Attack
		case MSS_ANGRY:   primary_goal = 0.9f; break;  // Retaliate
		case MSS_DEAD:    primary_goal = 0.0f; break;
		default:          primary_goal = 0.5f; break;
	}
	state[offset + 0] = primary_goal;
	
	// 53: Secondary goal (based on looter mode)
	float secondary_goal = 0.0f;
	if (status_has_mode(&md->status, MD_LOOTER)) {
		secondary_goal = 0.3f;  // Loot when possible
	}
	if (status_has_mode(&md->status, MD_ASSIST)) {
		secondary_goal += 0.3f;  // Help allies
	}
	state[offset + 1] = clamp01(secondary_goal);
	
	// 54: Goal progress (distance to target vs max range)
	float goal_progress = 0.5f;
	if (md->target_id > 0) {
		block_list* target = map_id2bl(md->target_id);
		if (target && target->prev) {
			int32 dist = distance_bl(md, target);
			int32 max_range = md->db->range3;
			goal_progress = 1.0f - normalize((float)dist, 0.0f, (float)max_range);
		}
	}
	state[offset + 2] = clamp01(goal_progress);
	
	// 55: Priority weights (HP urgency)
	float hp_urgency = 1.0f - state[0];  // Lower HP = higher urgency
	state[offset + 3] = hp_urgency;
	
	// 56: Resource needs (SP need)
	float sp_need = 1.0f - state[1];  // Lower SP = higher need
	state[offset + 4] = sp_need;
	
	// 57: Risk tolerance (based on mode and HP)
	float risk_tolerance = 0.5f;
	if (status_has_mode(&md->status, MD_AGGRESSIVE)) risk_tolerance += 0.3f;
	if (status_has_mode(&md->status, MD_STATUSIMMUNE)) risk_tolerance += 0.2f;  // Boss
	if (state[0] < 0.3f) risk_tolerance -= 0.4f;  // Low HP = low risk tolerance
	state[offset + 5] = clamp01(risk_tolerance);
	
	// 58: Cooperation desire (assist mode)
	state[offset + 6] = status_has_mode(&md->status, MD_ASSIST) ? 0.8f : 0.2f;
	
	// 59: Exploration tendency (random walk vs focused)
	state[offset + 7] = status_has_mode(&md->status, MD_NORANDOMWALK) ? 0.2f : 0.7f;
	
	// 60: Aggression level
	float aggression = 0.0f;
	if (status_has_mode(&md->status, MD_AGGRESSIVE)) aggression = 0.9f;
	else if (status_has_mode(&md->status, MD_ANGRY)) aggression = 0.7f;
	else aggression = 0.3f;
	state[offset + 8] = aggression;
	
	// 61: Defense priority (inverse of aggression)
	state[offset + 9] = 1.0f - aggression;
	
	// 62: Support priority (based on archetype)
	float support = 0.0f;
	if (status_has_mode(&md->status, MD_ASSIST)) support = 0.7f;
	// Check for healing skills
	if (!md->db->skill.empty()) {
		for (const auto& skill : md->db->skill) {
			if (skill->skill_id == AL_HEAL || skill->skill_id == AL_BLESSING) {
				support = 0.9f;
				break;
			}
		}
	}
	state[offset + 10] = support;
	
	// 63: Adaptation rate (fixed at 0.5 for C++)
	state[offset + 11] = 0.5f;
}

/**
 * Helper: Count entities of type in range
 */
int32 MobMLEncoder::count_entities_in_range(const struct mob_data* md, int32 range, int32 bl_type) {
	// Use simplified grid scan (map_foreachinrange would require callback setup)
	int32 count = 0;
	
	for (int32 dx = -range; dx <= range; dx++) {
		for (int32 dy = -range; dy <= range; dy++) {
			if (dx == 0 && dy == 0) continue;
			
			int32 x = md->x + dx;
			int32 y = md->y + dy;
			
			// Check distance
			if (dx*dx + dy*dy > range*range) continue;
			
			// This is simplified - proper implementation would use map_foreachinrange
			// For now, return estimated count based on map users
			if (bl_type == BL_PC) {
				struct map_data* mapdata = map_getmapdata(md->m);
				// Estimate: divide map users by 100 (rough approximation)
				return (mapdata->users > 0) ? 
					(int32)((float)mapdata->users * range / 100.0f) : 0;
			}
		}
	}
	
	return count;
}

/**
 * Helper: Get nearest entity (simplified)
 */
block_list* MobMLEncoder::get_nearest_entity(const struct mob_data* md, int32 range, int32 bl_type) {
	// This would normally use map_foreachinrange with callback
	// For now, check target if available
	if (md->target_id > 0) {
		block_list* bl = map_id2bl(md->target_id);
		if (bl && bl->type == bl_type) {
			return bl;
		}
	}
	
	return nullptr;
}

/**
 * Helper: Calculate threat level of entity
 */
float MobMLEncoder::calculate_threat_level(const struct mob_data* md, const block_list* target) {
	if (!target) return 0.0f;
	
	float threat = 0.0f;
	
	if (target->type == BL_PC) {
		map_session_data* sd = (map_session_data*)target;
		
		// Level-based threat
		float level_threat = normalize((float)sd->status.base_level, 1.0f, 150.0f);
		threat += level_threat * 0.5f;
		
		// Equipment-based threat (rough estimate from base status)
		const status_data* target_status = status_get_status_data(*target);
		int32 atk = target_status ? (target_status->rhw.atk + target_status->rhw.atk2) / 2 : 0;
		float atk_threat = normalize((float)atk, 0.0f, 5000.0f);
		threat += atk_threat * 0.3f;
		
		// HP-based threat (more HP = more dangerous)
		int32 max_hp = status_get_max_hp(target);
		float hp_threat = normalize((float)max_hp, 0.0f, 100000.0f);
		threat += hp_threat * 0.2f;
		
	} else if (target->type == BL_MOB) {
		mob_data* target_md = (mob_data*)target;
		
		// Level-based threat
		float level_threat = normalize((float)target_md->level, 1.0f, 150.0f);
		threat += level_threat * 0.4f;
		
		// Boss threat
		if (status_has_mode(&target_md->status, MD_STATUSIMMUNE)) {
			threat += 0.4f;
		} else {
			threat += 0.2f;
		}
		
		// ATK-based threat
		int32 atk = target_md->status.rhw.atk + target_md->status.rhw.atk2;
		float atk_threat = normalize((float)atk, 0.0f, 10000.0f);
		threat += atk_threat * 0.2f;
	}
	
	return clamp01(threat);
}
