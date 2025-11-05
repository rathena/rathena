// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "player_statistics.hpp"

#include <cstdlib>
#include <cstring>
#include <ctime>

#include <common/cbasetypes.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>

#include "clif.hpp"
#include "itemdb.hpp"
#include "map.hpp"
#include "mob.hpp"
#include "pc.hpp"

// Global configuration
struct player_statistics_config player_statistics_conf = {
	true,    // enabled
	300000,  // autosave_interval (5 minutes in milliseconds)
	false,   // detailed_tracking
	true     // daily_tracking
};

// Auto-save timer
static int statistics_autosave_timer = INVALID_TIMER;

/**
 * Initialize the player statistics system
 */
void player_statistics_init(void) {
	ShowStatus("Initializing Player Statistics System...\n");

	// Load configuration
	player_statistics_load_config();

	if (!player_statistics_conf.enabled) {
		ShowInfo("Player Statistics System is disabled.\n");
		return;
	}

	// Register auto-save timer
	statistics_autosave_timer = add_timer_interval(gettick() + player_statistics_conf.autosave_interval,
		player_statistics_autosave_timer, 0, 0, player_statistics_conf.autosave_interval);

	ShowStatus("Player Statistics System initialized.\n");
}

/**
 * Finalize the player statistics system
 */
void player_statistics_final(void) {
	if (!player_statistics_conf.enabled)
		return;

	ShowStatus("Finalizing Player Statistics System...\n");

	// Remove auto-save timer
	if (statistics_autosave_timer != INVALID_TIMER) {
		delete_timer(statistics_autosave_timer, player_statistics_autosave_timer);
		statistics_autosave_timer = INVALID_TIMER;
	}

	ShowStatus("Player Statistics System finalized.\n");
}

/**
 * Load configuration from char_athena.conf
 */
void player_statistics_load_config(void) {
	// TODO: Load from configuration file
	// For now, use default values
	ShowInfo("Player Statistics configuration loaded (using defaults).\n");
}

/**
 * Load player statistics from database
 * @param char_id: Character ID
 * @return Player statistics structure or nullptr if not found
 */
struct player_statistics* player_statistics_load(uint32 char_id) {
	if (!player_statistics_conf.enabled)
		return nullptr;

	// TODO: Implement database loading
	// For now, return nullptr to indicate creation is needed
	return nullptr;
}

/**
 * Save player statistics to database
 * @param stats: Player statistics structure
 * @param force: Force save even if not dirty
 */
void player_statistics_save(struct player_statistics* stats, bool force) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(stats);

	if (!force && !stats->dirty)
		return;

	// TODO: Implement database saving
	// For now, just mark as clean
	stats->dirty = false;

	ShowInfo("Saved player statistics for char_id: %d\n", stats->char_id);
}

/**
 * Create new player statistics record
 * @param char_id: Character ID
 * @param account_id: Account ID
 * @return Newly created player statistics structure
 */
struct player_statistics* player_statistics_create(uint32 char_id, uint32 account_id) {
	if (!player_statistics_conf.enabled)
		return nullptr;

	struct player_statistics* stats;

	CREATE(stats, struct player_statistics, 1);
	memset(stats, 0, sizeof(struct player_statistics));

	stats->char_id = char_id;
	stats->account_id = account_id;
	stats->dirty = true;
	stats->loaded = true;

	// TODO: Insert into database

	ShowInfo("Created player statistics for char_id: %d\n", char_id);
	return stats;
}

/**
 * Delete player statistics record
 * @param char_id: Character ID
 */
void player_statistics_delete(uint32 char_id) {
	if (!player_statistics_conf.enabled)
		return;

	// TODO: Implement database deletion
	ShowInfo("Deleted player statistics for char_id: %d\n", char_id);
}

/**
 * Free player statistics structure from memory
 * @param stats: Player statistics structure
 */
void player_statistics_free(struct player_statistics* stats) {
	if (stats) {
		aFree(stats);
	}
}

// ============================================================================
// Tracking Functions
// ============================================================================

/**
 * Track player login
 * @param sd: Player session data
 */
void player_statistics_track_login(struct map_session_data* sd) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	sd->statistics->session_start = time(nullptr);
	sd->statistics->last_login = sd->statistics->session_start;
	sd->statistics->login_count++;
	sd->statistics->dirty = true;
}

/**
 * Track player logout and update online time
 * @param sd: Player session data
 */
void player_statistics_track_logout(struct map_session_data* sd) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	// Update online time
	player_statistics_update_online_time(sd);
	sd->statistics->session_start = 0;
	sd->statistics->dirty = true;
}

/**
 * Track item usage
 * @param sd: Player session data
 * @param it: Item structure
 * @param type: Item usage type (healing, buff, other)
 */
void player_statistics_track_item_use(struct map_session_data* sd, struct item* it, e_item_usage_type type) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);
	nullpo_retv(it);

	sd->statistics->item_used_count++;

	switch (type) {
		case ITEM_USE_HEALING:
			sd->statistics->item_used_healing++;
			break;
		case ITEM_USE_BUFF:
			sd->statistics->item_used_buff++;
			break;
		default:
			sd->statistics->item_used_other++;
			break;
	}

	sd->statistics->dirty = true;
}

/**
 * Track teleport usage
 * @param sd: Player session data
 * @param type: Teleport type (skill, fly wing, butterfly wing, warp portal)
 */
void player_statistics_track_teleport(struct map_session_data* sd, e_teleport_type type) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	switch (type) {
		case TELEPORT_SKILL:
			sd->statistics->teleport_count++;
			break;
		case TELEPORT_FLY_WING:
			// Can also count as teleport
			sd->statistics->teleport_count++;
			break;
		case TELEPORT_BUTTERFLY_WING:
			sd->statistics->butterfly_wing_count++;
			break;
		case TELEPORT_WARP_PORTAL:
			sd->statistics->warp_portal_count++;
			break;
	}

	sd->statistics->dirty = true;
}

/**
 * Track player movement
 * @param sd: Player session data
 * @param x0: Starting X coordinate
 * @param y0: Starting Y coordinate
 * @param x1: Ending X coordinate
 * @param y1: Ending Y coordinate
 */
void player_statistics_track_movement(struct map_session_data* sd, int x0, int y0, int x1, int y1) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	sd->statistics->movement_count++;

	// Calculate distance
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int distance = (dx > dy) ? dx : dy; // Manhattan distance approximation

	sd->statistics->movement_distance += distance;
	sd->statistics->dirty = true;
}

/**
 * Track HP/SP healing
 * @param sd: Player session data
 * @param hp: HP healed (0 if none)
 * @param sp: SP recovered (0 if none)
 */
void player_statistics_track_heal(struct map_session_data* sd, int hp, int sp) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	if (hp > 0) {
		sd->statistics->heal_skill_count++;
		sd->statistics->heal_amount_total += hp;
	}

	if (sp > 0) {
		sd->statistics->sp_recovery_skill_count++;
		sd->statistics->sp_recovery_amount += sp;
	}

	if (hp > 0 || sp > 0)
		sd->statistics->dirty = true;
}

/**
 * Track item pickup
 * @param sd: Player session data
 * @param it: Item structure
 * @param amount: Amount picked up
 * @param from_mob: Whether item was dropped by monster
 */
void player_statistics_track_item_pickup(struct map_session_data* sd, struct item* it, int amount, bool from_mob) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);
	nullpo_retv(it);

	sd->statistics->item_picked_count += amount;

	if (from_mob)
		sd->statistics->item_picked_from_mob += amount;
	else
		sd->statistics->item_picked_from_ground += amount;

	sd->statistics->dirty = true;
}

/**
 * Track monster kill
 * @param sd: Player session data
 * @param md: Monster data
 */
void player_statistics_track_mob_kill(struct map_session_data* sd, struct mob_data* md) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);
	nullpo_retv(md);

	sd->statistics->mob_kill_count++;

	// Check mob type
	if (md->db->mexp > 0) {
		// MVP monster
		sd->statistics->mvp_kill_count++;
	}
	// TODO: Add boss and mini-boss detection based on mob flags

	sd->statistics->dirty = true;
}

/**
 * Track skill usage
 * @param sd: Player session data
 * @param skill_id: Skill ID
 * @param skill_lv: Skill level
 * @param type: Skill usage type (offensive, support, other)
 */
void player_statistics_track_skill_use(struct map_session_data* sd, uint16 skill_id, uint16 skill_lv, e_skill_usage_type type) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	sd->statistics->skill_used_count++;

	switch (type) {
		case SKILL_USE_OFFENSIVE:
			sd->statistics->skill_offensive_count++;
			break;
		case SKILL_USE_SUPPORT:
			sd->statistics->skill_support_count++;
			break;
		default:
			break;
	}

	sd->statistics->dirty = true;
}

/**
 * Track player death
 * @param sd: Player session data
 */
void player_statistics_track_death(struct map_session_data* sd) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	sd->statistics->death_count++;
	sd->statistics->dirty = true;
}

/**
 * Track damage dealt or received
 * @param sd: Player session data
 * @param damage: Amount of damage
 * @param dealt: True if damage was dealt, false if received
 */
void player_statistics_track_damage(struct map_session_data* sd, int64 damage, bool dealt) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	if (dealt)
		sd->statistics->damage_dealt += damage;
	else
		sd->statistics->damage_received += damage;

	sd->statistics->dirty = true;
}

/**
 * Track chat message sent
 * @param sd: Player session data
 */
void player_statistics_track_chat(struct map_session_data* sd) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	sd->statistics->chat_message_count++;
	sd->statistics->dirty = true;
}

// ============================================================================
// Update Functions
// ============================================================================

/**
 * Update online time for current session
 * @param sd: Player session data
 */
void player_statistics_update_online_time(struct map_session_data* sd) {
	if (!player_statistics_conf.enabled)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	if (sd->statistics->session_start > 0) {
		time_t now = time(nullptr);
		time_t session_duration = now - sd->statistics->session_start;

		if (session_duration > 0) {
			sd->statistics->online_time += session_duration;
			sd->statistics->session_start = now; // Reset for next update
			sd->statistics->dirty = true;
		}
	}
}

/**
 * Auto-save timer callback
 * Periodically saves all dirty player statistics
 */
int player_statistics_autosave_timer(int tid, int64 tick, int id, intptr_t data) {
	if (!player_statistics_conf.enabled)
		return 0;

	int saved_count = 0;

	// Iterate through all online players
	map_foreachpc([&saved_count](struct map_session_data* sd) -> int {
		if (sd && sd->statistics) {
			// Update online time before saving
			player_statistics_update_online_time(sd);

			if (sd->statistics->dirty) {
				player_statistics_save(sd->statistics, false);
				saved_count++;
			}
		}
		return 0;
	});

	if (saved_count > 0) {
		ShowInfo("Auto-saved %d player statistics.\n", saved_count);
	}

	return 0;
}

// ============================================================================
// Query Functions
// ============================================================================

/**
 * Get player's total online time
 * @param sd: Player session data
 * @return Total online time in seconds
 */
uint64 player_statistics_get_online_time(struct map_session_data* sd) {
	if (!player_statistics_conf.enabled || !sd || !sd->statistics)
		return 0;

	// Update current session time
	uint64 total_time = sd->statistics->online_time;

	if (sd->statistics->session_start > 0) {
		time_t now = time(nullptr);
		total_time += (now - sd->statistics->session_start);
	}

	return total_time;
}

/**
 * Get specific statistic value
 * @param char_id: Character ID
 * @param stat_name: Statistic field name
 * @return Statistic value
 */
uint64 player_statistics_get_stat(uint32 char_id, const char* stat_name) {
	if (!player_statistics_conf.enabled)
		return 0;

	// TODO: Implement stat lookup by name
	// For now, return 0
	return 0;
}

// ============================================================================
// Daily Statistics Functions
// ============================================================================

/**
 * Update daily statistics
 * @param sd: Player session data
 */
void player_statistics_update_daily(struct map_session_data* sd) {
	if (!player_statistics_conf.enabled || !player_statistics_conf.daily_tracking)
		return;

	nullpo_retv(sd);
	nullpo_retv(sd->statistics);

	// TODO: Implement daily statistics update
}

/**
 * Reset daily statistics (called at midnight)
 */
int player_statistics_daily_reset_timer(int tid, int64 tick, int id, intptr_t data) {
	if (!player_statistics_conf.enabled || !player_statistics_conf.daily_tracking)
		return 0;

	// TODO: Implement daily reset
	ShowInfo("Daily statistics reset.\n");

	return 0;
}

/**
 * Cleanup old daily statistics records
 * @param days: Number of days to keep (default: 90)
 */
void player_statistics_cleanup_daily(uint32 days) {
	if (!player_statistics_conf.enabled || !player_statistics_conf.daily_tracking)
		return;

	// TODO: Implement cleanup of old daily records
	ShowInfo("Cleaned up daily statistics older than %d days.\n", days);
}
