// TranscendedRO - Auto Attack system
//
// Self-contained: included only from src/custom/atcommand.inc and src/custom/script.inc,
// no changes to core files. All functions are inline so the header can be included
// from more than one translation unit; state lives in a single function-local static.
//
// Settings are permanent character variables, written by the NPC menu
// (npc/custom/transcendedro_autoattack.txt) and re-read on every tick, so changes apply live:
//   AA_SKILL     skill id to use (0 = normal attack)
//   AA_SKILLLV   skill level (0 or above learned level = highest learned level)
//   AA_HPITEM    item id to use for HP (0 = off)
//   AA_HPPCT     use HP item when HP% is below this
//   AA_SPITEM    item id to use for SP (0 = off)
//   AA_SPPCT     use SP item when SP% is below this
//   AA_RANGE     monster search range in cells (default 9)
//   AA_TELE      1 = random teleport when no monster found for a while
//   AA_NOBOSS    1 = ignore MVP / mini boss
//   AA_NOKS      1 = ignore monsters already fighting another player

#ifndef CUSTOM_AUTOATTACK_HPP
#define CUSTOM_AUTOATTACK_HPP

#include <algorithm>
#include <unordered_map>

#include <common/timer.hpp>

#include "../map/battle.hpp"
#include "../map/clif.hpp"
#include "../map/map.hpp"
#include "../map/mob.hpp"
#include "../map/path.hpp"
#include "../map/pc.hpp"
#include "../map/skill.hpp"
#include "../map/status.hpp"
#include "../map/unit.hpp"

namespace autoattack {

constexpr t_tick TICK_INTERVAL = 300;       // ms between decisions
constexpr t_tick IDLE_TELEPORT_DELAY = 3000; // ms without a target before teleporting
constexpr t_tick IDLE_WALK_DELAY = 1500;     // ms without a target before wandering
constexpr int32 DEFAULT_RANGE = 9;
constexpr int32 MAX_RANGE = 14;

struct s_state {
	int32 timer = INVALID_TIMER;
	int32 account_id = 0;
	int32 target_id = 0;
	t_tick last_target_tick = 0;
};

// char_id -> state
inline std::unordered_map<uint32, s_state>& states() {
	static std::unordered_map<uint32, s_state> instance;
	return instance;
}

inline int64 setting( map_session_data* sd, const char* var ) {
	return pc_readglobalreg( sd, add_str( var ) );
}

inline bool is_active( map_session_data* sd ) {
	return sd != nullptr && states().count( sd->status.char_id ) > 0;
}

/// Can auto attack run on the player's current map / state? Writes a reason on failure.
inline bool can_run( map_session_data* sd, const char** reason ) {
	int16 m = sd->m;

	if( pc_isdead( sd ) ) {
		*reason = "you are dead";
		return false;
	}
	if( map_getmapflag( m, MF_TOWN ) ) {
		*reason = "not allowed in town";
		return false;
	}
	if( map_flag_vs( m ) || map_flag_gvg2( m ) || map_getmapflag( m, MF_BATTLEGROUND ) ) {
		*reason = "not allowed on PvP/GvG/Battleground maps";
		return false;
	}
	if( sd->state.vending || sd->state.buyingstore || sd->state.autotrade || sd->chatID ) {
		*reason = "you are busy (vending/chat)";
		return false;
	}
	return true;
}

inline void message( map_session_data* sd, const char* msg ) {
	if( sd->fd > 0 )
		clif_displaymessage( sd->fd, msg );
}

inline TIMER_FUNC( tick_timer );

inline bool start( map_session_data* sd ) {
	if( sd == nullptr )
		return false;
	if( is_active( sd ) )
		return true;

	const char* reason = nullptr;
	if( !can_run( sd, &reason ) ) {
		char buf[CHAT_SIZE_MAX];
		safesnprintf( buf, sizeof( buf ), "[Auto Attack] Cannot start: %s.", reason );
		message( sd, buf );
		return false;
	}

	s_state& st = states()[sd->status.char_id];
	st.account_id = sd->status.account_id;
	st.target_id = 0;
	st.last_target_tick = gettick();
	st.timer = add_timer( gettick() + TICK_INTERVAL, tick_timer, sd->status.account_id, (intptr_t)sd->status.char_id );

	message( sd, "[Auto Attack] Started." );
	return true;
}

/// Stops auto attack. `reason` is shown to the player when not null.
inline void stop( map_session_data* sd, const char* reason = nullptr ) {
	if( !is_active( sd ) )
		return;

	auto it = states().find( sd->status.char_id );
	if( it->second.timer != INVALID_TIMER )
		delete_timer( it->second.timer, tick_timer );
	states().erase( it );

	unit_stop_attack( sd );

	char buf[CHAT_SIZE_MAX];
	if( reason != nullptr )
		safesnprintf( buf, sizeof( buf ), "[Auto Attack] Stopped: %s.", reason );
	else
		safesnprintf( buf, sizeof( buf ), "[Auto Attack] Stopped." );
	message( sd, buf );
}

// ---- target search ----------------------------------------------------------

inline int32 search_sub( block_list* bl, va_list ap ) {
	map_session_data* sd = va_arg( ap, map_session_data* );
	int32* best_id = va_arg( ap, int32* );
	int32* best_dist = va_arg( ap, int32* );
	bool noboss = va_arg( ap, int32 ) != 0;
	bool noks = va_arg( ap, int32 ) != 0;

	mob_data* md = reinterpret_cast<mob_data*>( bl );

	if( status_isdead( *bl ) )
		return 0;
	if( md->special_state.ai != AI_NONE ) // summoned / player-owned monsters
		return 0;
	if( noboss && md->get_bosstype() != BOSSTYPE_NONE )
		return 0;
	if( battle_check_target( sd, bl, BCT_ENEMY ) <= 0 )
		return 0;
	if( noks && md->target_id != 0 && md->target_id != sd->id ) {
		block_list* other = map_id2bl( md->target_id );
		if( other != nullptr && other->type == BL_PC )
			return 0;
	}

	int32 dist = distance_bl( sd, bl );
	if( dist >= *best_dist )
		return 0;
	if( !unit_can_reach_bl( sd, bl, MAX_RANGE * 2, 0, nullptr, nullptr ) )
		return 0;

	*best_id = bl->id;
	*best_dist = dist;
	return 1;
}

inline block_list* valid_target( map_session_data* sd, int32 id ) {
	if( id == 0 )
		return nullptr;
	block_list* bl = map_id2bl( id );
	if( bl == nullptr || bl->type != BL_MOB || bl->m != sd->m || status_isdead( *bl ) )
		return nullptr;
	if( battle_check_target( sd, bl, BCT_ENEMY ) <= 0 )
		return nullptr;
	if( !check_distance_bl( sd, bl, MAX_RANGE * 2 ) )
		return nullptr;
	return bl;
}

inline block_list* find_target( map_session_data* sd ) {
	int32 range = static_cast<int32>( setting( sd, "AA_RANGE" ) );
	if( range <= 0 )
		range = DEFAULT_RANGE;
	range = cap_value( range, 1, MAX_RANGE );

	int32 best_id = 0;
	int32 best_dist = range + 1;
	map_foreachinallrange( search_sub, sd, range, BL_MOB, sd, &best_id, &best_dist,
		static_cast<int32>( setting( sd, "AA_NOBOSS" ) ), static_cast<int32>( setting( sd, "AA_NOKS" ) ) );

	return best_id ? map_id2bl( best_id ) : nullptr;
}

// ---- actions ----------------------------------------------------------------

inline void use_potion( map_session_data* sd, const char* item_var, const char* pct_var, uint32 cur, uint32 max ) {
	t_itemid nameid = static_cast<t_itemid>( setting( sd, item_var ) );
	int64 pct = setting( sd, pct_var );

	if( nameid == 0 || pct <= 0 || max == 0 )
		return;
	if( static_cast<int64>( cur ) * 100 / max >= pct )
		return;

	int16 idx = pc_search_inventory( sd, nameid );
	if( idx >= 0 )
		pc_useitem( sd, idx );
}

/// Try to use the configured skill on target. Returns true if a skill was started (or we walked toward it).
inline bool use_skill( map_session_data* sd, block_list* target ) {
	uint16 skill_id = static_cast<uint16>( setting( sd, "AA_SKILL" ) );
	if( skill_id == 0 )
		return false;

	uint16 learned = pc_checkskill( sd, skill_id );
	if( learned == 0 )
		return false;

	uint16 skill_lv = static_cast<uint16>( setting( sd, "AA_SKILLLV" ) );
	if( skill_lv == 0 || skill_lv > learned )
		skill_lv = learned;

	int32 inf = skill_get_inf( skill_id );
	int32 range;

	if( inf & INF_SELF_SKILL )
		range = std::max( 1, skill_get_splash( skill_id, skill_lv ) );
	else
		range = skill_get_range2( sd, skill_id, skill_lv, true );

	if( !battle_check_range( sd, target, range ) ) {
		if( !unit_is_walking( sd ) )
			unit_walktobl( sd, target, range, 0 );
		return true;
	}

	if( inf & INF_GROUND_SKILL )
		return unit_skilluse_pos( sd, target->x, target->y, skill_id, skill_lv ) != 0;
	if( inf & INF_SELF_SKILL )
		return unit_skilluse_id( sd, sd->id, skill_id, skill_lv ) != 0;
	return unit_skilluse_id( sd, target->id, skill_id, skill_lv ) != 0;
}

inline void wander( map_session_data* sd ) {
	if( unit_is_walking( sd ) )
		return;

	int16 x = sd->x, y = sd->y;
	if( map_search_freecell( sd, sd->m, &x, &y, 10, 10, 1 ) )
		unit_walktoxy( sd, x, y, 0 );
}

inline void think( map_session_data* sd, s_state& st, t_tick tick ) {
	const char* reason = nullptr;
	if( !can_run( sd, &reason ) ) {
		stop( sd, reason );
		return;
	}

	use_potion( sd, "AA_HPITEM", "AA_HPPCT", sd->battle_status.hp, sd->battle_status.max_hp );
	use_potion( sd, "AA_SPITEM", "AA_SPPCT", sd->battle_status.sp, sd->battle_status.max_sp );

	// Busy casting / stunned etc. - wait for next tick
	if( pc_cant_act( sd ) || sd->ud.skilltimer != INVALID_TIMER )
		return;

	block_list* target = valid_target( sd, st.target_id );
	if( target == nullptr ) {
		target = find_target( sd );
		st.target_id = target ? target->id : 0;
	}

	if( target == nullptr ) {
		t_tick idle = DIFF_TICK( tick, st.last_target_tick );

		if( setting( sd, "AA_TELE" ) && idle >= IDLE_TELEPORT_DELAY && !map_getmapflag( sd->m, MF_NOTELEPORT ) ) {
			st.last_target_tick = tick;
			pc_randomwarp( sd, CLR_TELEPORT );
		} else if( idle >= IDLE_WALK_DELAY ) {
			wander( sd );
		}
		return;
	}

	st.last_target_tick = tick;

	if( use_skill( sd, target ) )
		return;

	// Normal attack (unit_attack walks to the target by itself)
	if( sd->ud.target != target->id || sd->ud.attacktimer == INVALID_TIMER )
		unit_attack( sd, target->id, 1 );
}

inline TIMER_FUNC( tick_timer ) {
	uint32 char_id = static_cast<uint32>( data );
	auto it = states().find( char_id );

	if( it == states().end() || it->second.timer != tid )
		return 0;

	map_session_data* sd = map_id2sd( id );

	// Logged out or switched character: drop state silently
	if( sd == nullptr || sd->status.char_id != char_id ) {
		states().erase( it );
		return 0;
	}

	it->second.timer = INVALID_TIMER;
	think( sd, it->second, tick );

	// think() may have stopped us
	it = states().find( char_id );
	if( it != states().end() )
		it->second.timer = add_timer( tick + TICK_INTERVAL, tick_timer, id, data );

	return 0;
}

} // namespace autoattack

#endif /* CUSTOM_AUTOATTACK_HPP */
