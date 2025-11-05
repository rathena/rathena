// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PLAYER_STATISTICS_HPP
#define PLAYER_STATISTICS_HPP

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>
#include <common/timer.hpp>

class map_session_data;
struct item;
struct mob_data;

/**
 * Item usage types for tracking
 */
enum e_item_usage_type : uint8 {
	ITEM_USE_OTHER = 0,     // Other items
	ITEM_USE_HEALING = 1,   // Healing items (HP/SP recovery)
	ITEM_USE_BUFF = 2,      // Buff items (temporary status effects)
};

/**
 * Teleport types for tracking
 */
enum e_teleport_type : uint8 {
	TELEPORT_SKILL = 0,         // Teleport skill
	TELEPORT_FLY_WING = 1,      // Fly Wing item
	TELEPORT_BUTTERFLY_WING = 2, // Butterfly Wing item
	TELEPORT_WARP_PORTAL = 3,   // Warp Portal skill
};

/**
 * Skill usage types for tracking
 */
enum e_skill_usage_type : uint8 {
	SKILL_USE_OTHER = 0,      // Other skills
	SKILL_USE_OFFENSIVE = 1,  // Offensive/Attack skills
	SKILL_USE_SUPPORT = 2,    // Support/Buff skills
};

/**
 * Player Statistics Structure
 * Stores real-time player activity statistics in memory
 */
struct player_statistics {
	uint32 char_id;
	uint32 account_id;

	// Online Time Tracking
	uint64 online_time;           // Total online time in seconds
	time_t session_start;         // Current session start timestamp
	time_t last_login;            // Last login timestamp
	uint32 login_count;           // Total number of logins

	// Item Usage Statistics
	uint64 item_used_count;       // Total items used/consumed
	uint32 item_used_healing;     // Healing items used
	uint32 item_used_buff;        // Buff items used
	uint32 item_used_other;       // Other items used

	// Movement & Teleport Statistics
	uint32 teleport_count;        // Teleport skill uses
	uint32 warp_portal_count;     // Warp portal uses
	uint32 butterfly_wing_count;  // Butterfly Wing uses
	uint64 movement_count;        // Total cell movements
	uint64 movement_distance;     // Total distance in cells

	// Healing & SP Recovery Statistics
	uint32 heal_skill_count;      // Heal skill uses
	uint64 heal_amount_total;     // Total HP healed
	uint32 sp_recovery_skill_count; // SP recovery skill uses
	uint64 sp_recovery_amount;    // Total SP recovered

	// Item Pickup Statistics
	uint64 item_picked_count;     // Total items picked up
	uint32 item_picked_from_mob;  // Items from monsters
	uint32 item_picked_from_ground; // Items from ground
	uint64 zeny_picked;           // Total zeny picked

	// Monster Kill Statistics
	uint64 mob_kill_count;        // Total monsters killed
	uint32 mvp_kill_count;        // MVP monsters killed
	uint32 boss_kill_count;       // Boss monsters killed
	uint32 mini_boss_kill_count;  // Mini-boss monsters killed

	// Skill Usage Statistics
	uint64 skill_used_count;      // Total skills used
	uint32 skill_offensive_count; // Offensive skills
	uint32 skill_support_count;   // Support skills
	uint32 skill_passive_triggered; // Passive skill triggers

	// Additional Statistics
	uint32 death_count;           // Total deaths
	uint64 damage_dealt;          // Total damage dealt
	uint64 damage_received;       // Total damage received
	uint32 chat_message_count;    // Chat messages sent

	// Internal flags
	bool dirty;                   // Needs to be saved to database
	bool loaded;                  // Data has been loaded from database
};

/**
 * Configuration structure for player statistics
 */
struct player_statistics_config {
	bool enabled;                 // Enable statistics tracking
	uint32 autosave_interval;     // Auto-save interval in milliseconds
	bool detailed_tracking;       // Enable detailed statistics (JSON data)
	bool daily_tracking;          // Enable daily statistics tracking
};

// Global configuration
extern struct player_statistics_config player_statistics_conf;

// ============================================================================
// System Functions
// ============================================================================

/**
 * Initialize the player statistics system
 */
void player_statistics_init(void);

/**
 * Finalize the player statistics system
 */
void player_statistics_final(void);

/**
 * Load configuration from char_athena.conf
 */
void player_statistics_load_config(void);

// ============================================================================
// Data Management Functions
// ============================================================================

/**
 * Load player statistics from database
 * @param char_id: Character ID
 * @return Player statistics structure or nullptr if not found
 */
struct player_statistics* player_statistics_load(uint32 char_id);

/**
 * Save player statistics to database
 * @param stats: Player statistics structure
 * @param force: Force save even if not dirty
 */
void player_statistics_save(struct player_statistics* stats, bool force = false);

/**
 * Create new player statistics record
 * @param char_id: Character ID
 * @param account_id: Account ID
 * @return Newly created player statistics structure
 */
struct player_statistics* player_statistics_create(uint32 char_id, uint32 account_id);

/**
 * Delete player statistics record
 * @param char_id: Character ID
 */
void player_statistics_delete(uint32 char_id);

/**
 * Free player statistics structure from memory
 * @param stats: Player statistics structure
 */
void player_statistics_free(struct player_statistics* stats);

// ============================================================================
// Tracking Functions
// ============================================================================

/**
 * Track player login
 * @param sd: Player session data
 */
void player_statistics_track_login(struct map_session_data* sd);

/**
 * Track player logout and update online time
 * @param sd: Player session data
 */
void player_statistics_track_logout(struct map_session_data* sd);

/**
 * Track item usage
 * @param sd: Player session data
 * @param it: Item structure
 * @param type: Item usage type (healing, buff, other)
 */
void player_statistics_track_item_use(struct map_session_data* sd, struct item* it, e_item_usage_type type);

/**
 * Track teleport usage
 * @param sd: Player session data
 * @param type: Teleport type (skill, fly wing, butterfly wing, warp portal)
 */
void player_statistics_track_teleport(struct map_session_data* sd, e_teleport_type type);

/**
 * Track player movement
 * @param sd: Player session data
 * @param x0: Starting X coordinate
 * @param y0: Starting Y coordinate
 * @param x1: Ending X coordinate
 * @param y1: Ending Y coordinate
 */
void player_statistics_track_movement(struct map_session_data* sd, int x0, int y0, int x1, int y1);

/**
 * Track HP/SP healing
 * @param sd: Player session data
 * @param hp: HP healed (0 if none)
 * @param sp: SP recovered (0 if none)
 */
void player_statistics_track_heal(struct map_session_data* sd, int hp, int sp);

/**
 * Track item pickup
 * @param sd: Player session data
 * @param it: Item structure
 * @param amount: Amount picked up
 * @param from_mob: Whether item was dropped by monster
 */
void player_statistics_track_item_pickup(struct map_session_data* sd, struct item* it, int amount, bool from_mob);

/**
 * Track monster kill
 * @param sd: Player session data
 * @param md: Monster data
 */
void player_statistics_track_mob_kill(struct map_session_data* sd, struct mob_data* md);

/**
 * Track skill usage
 * @param sd: Player session data
 * @param skill_id: Skill ID
 * @param skill_lv: Skill level
 * @param type: Skill usage type (offensive, support, other)
 */
void player_statistics_track_skill_use(struct map_session_data* sd, uint16 skill_id, uint16 skill_lv, e_skill_usage_type type);

/**
 * Track player death
 * @param sd: Player session data
 */
void player_statistics_track_death(struct map_session_data* sd);

/**
 * Track damage dealt or received
 * @param sd: Player session data
 * @param damage: Amount of damage
 * @param dealt: True if damage was dealt, false if received
 */
void player_statistics_track_damage(struct map_session_data* sd, int64 damage, bool dealt);

/**
 * Track chat message sent
 * @param sd: Player session data
 */
void player_statistics_track_chat(struct map_session_data* sd);

// ============================================================================
// Update Functions
// ============================================================================

/**
 * Update online time for current session
 * @param sd: Player session data
 */
void player_statistics_update_online_time(struct map_session_data* sd);

/**
 * Auto-save timer callback
 * Periodically saves all dirty player statistics
 * @param tid: Timer ID
 * @param tick: Current tick
 * @param id: Not used
 * @param data: Not used
 * @return 0 on success
 */
int player_statistics_autosave_timer(int tid, int64 tick, int id, intptr_t data);

// ============================================================================
// Query Functions
// ============================================================================

/**
 * Get player's total online time
 * @param sd: Player session data
 * @return Total online time in seconds
 */
uint64 player_statistics_get_online_time(struct map_session_data* sd);

/**
 * Get specific statistic value
 * @param char_id: Character ID
 * @param stat_name: Statistic field name
 * @return Statistic value
 */
uint64 player_statistics_get_stat(uint32 char_id, const char* stat_name);

// ============================================================================
// Daily Statistics Functions
// ============================================================================

/**
 * Update daily statistics
 * @param sd: Player session data
 */
void player_statistics_update_daily(struct map_session_data* sd);

/**
 * Reset daily statistics (called at midnight)
 * @param tid: Timer ID
 * @param tick: Current tick
 * @param id: Not used
 * @param data: Not used
 * @return 0 on success
 */
int player_statistics_daily_reset_timer(int tid, int64 tick, int id, intptr_t data);

/**
 * Cleanup old daily statistics records
 * @param days: Number of days to keep (default: 90)
 */
void player_statistics_cleanup_daily(uint32 days = 90);

#endif /* PLAYER_STATISTICS_HPP */
