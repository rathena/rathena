// TranscendedRO - Auto Attack system
//
// Self-contained: included only from src/custom/atcommand.inc and src/custom/script.inc,
// no changes to core files. All functions are inline so the header can be included
// from more than one translation unit; runtime state lives in one function-local static.
//
// Settings are permanent character variables written by the NPC menu
// (npc/custom/transcendedro_autoattack.txt) and re-read every tick, so changes apply live.
//
//   AA_NORMAL                 1 = normal attack enabled (always used when no attack skill is set)
//   AA_ATK_SK[0..4]/AA_ATK_LV attack skill slots (lv 0 = highest learned)
//   AA_BUF_SK[0..4]/AA_BUF_LV buff skill slots, checked in order 1 > 5; recast when the
//                             buff's real remaining time is almost over
//   AA_BUF_SEC[0..4]          0 = auto (read buff time), >0 = recast every N seconds instead
//   AA_HP_IT[0..4]/AA_HP_PCT  HP items, used when HP% < pct (first slot the player owns)
//   AA_SP_IT[0..4]/AA_SP_PCT  SP items, same for SP
//   AA_OT_IT[0..4]/AA_OT_MS   other items, used every N ms
//   AA_TPDELAY                seconds without a monster before teleporting (3/5/7/10)
//   AA_TPITEM                 1 = teleport with Fly Wing (601 / 12887)
//   AA_TPSKILL                1 = teleport with AL_TELEPORT
//   AA_AVOID[0..29]           monster ids to teleport away from
//   AA_AVOIDBOSS              1 = teleport away from MVP / mini boss
//   AA_PICKUP                 1 = walk to and pick up floor items
//   AA_GREED                  1 = use BS_GREED when items are close
//   AA_RANGE                  search range (hidden, default 9)
//
// Loot (VIP) settings AA_LOOTALL / AA_LOOTTYPE / AA_LOOTID[] / AA_LOOTON[] are applied to
// the engine's own autoloot fields by apply_loot() (script command aa_applyloot).

#ifndef CUSTOM_AUTOATTACK_HPP
#define CUSTOM_AUTOATTACK_HPP

#include <algorithm>
#include <array>
#include <unordered_map>
#include <unordered_set>

#include <common/timer.hpp>

#include "../map/battle.hpp"
#include "../map/clif.hpp"
#include "../map/itemdb.hpp"
#include "../map/map.hpp"
#include "../map/mob.hpp"
#include "../map/path.hpp"
#include "../map/pc.hpp"
#include "../map/script.hpp"
#include "../map/skill.hpp"
#include "../map/status.hpp"
#include "../map/unit.hpp"

namespace autoattack {

constexpr t_tick TICK_INTERVAL = 300;      // ms between decisions
constexpr t_tick IDLE_WALK_DELAY = 1500;    // ms without a target before wandering
constexpr t_tick BUFF_RETRY_DELAY = 3000;   // minimum ms between casts of the same buff slot
constexpr t_tick BUFF_REFRESH_LEAD = 1500;  // recast a buff when this many ms are left on it
constexpr t_tick AVOID_TP_DELAY = 1000;     // minimum ms between "run away" teleports
constexpr int32 DEFAULT_RANGE = 9;
constexpr int32 MAX_RANGE = 14;
constexpr int32 SLOTS = 5;
constexpr int32 MAX_AVOID = 30;
constexpr int32 LOOT_RANGE = 9;
constexpr t_itemid FLY_WINGS[] = { 601, 12887 };

struct s_state {
	int32 timer = INVALID_TIMER;
	int32 target_id = 0;
	int32 loot_id = 0;
	t_tick last_target_tick = 0;
	t_tick last_avoid_tp = 0;
	std::array<t_tick, SLOTS> buff_last{};
	std::array<t_tick, SLOTS> other_last{};
	std::unordered_set<int32> bad_items; // floor items we failed to take
};

// char_id -> state
inline std::unordered_map<uint32, s_state>& states() {
	static std::unordered_map<uint32, s_state> instance;
	return instance;
}

inline int64 setting( map_session_data* sd, const char* var, uint32 index = 0 ) {
	return pc_readglobalreg( sd, reference_uid( add_str( var ), index ) );
}

inline bool is_active( map_session_data* sd ) {
	return sd != nullptr && states().count( sd->status.char_id ) > 0;
}

inline void message( map_session_data* sd, const char* msg ) {
	if( sd->fd > 0 )
		clif_displaymessage( sd->fd, msg );
}

// ---- skill helpers (also used by the NPC menu) ------------------------------

/// Bitmask: 1 = usable as attack skill, 2 = usable as buff skill, 0 = neither (passive / unknown)
inline int32 skill_category( uint16 skill_id ) {
	std::shared_ptr<s_skill_db> skill = skill_db.find( skill_id );
	if( skill_id == 0 || skill == nullptr )
		return 0;

	int32 inf = skill->inf;
	if( inf == INF_PASSIVE_SKILL ) // passive: never castable
		return 0;

	bool nodamage = skill->nk[NK_NODAMAGE];

	// Attack: deals damage, or is cast on an enemy (e.g. Provoke, Lex Divina)
	if( !nodamage || ( inf & INF_ATTACK_SKILL ) )
		return 1;

	// Buff: cast on self/ally and gives a status we can watch for recasting
	// (excludes Heal, Teleport, Warp Portal, Resurrection, ground utility like Ice Wall)
	if( ( inf & ( INF_SELF_SKILL | INF_SUPPORT_SKILL ) ) && skill->sc > SC_NONE && skill->sc < SC_MAX )
		return 2;

	return 0;
}

/// Level to use for a slot: configured level capped to learned level, 0 if not usable.
inline uint16 slot_level( map_session_data* sd, uint16 skill_id, int64 wanted ) {
	if( skill_id == 0 || !skill_db.find( skill_id ) )
		return 0;
	uint16 learned = pc_checkskill( sd, skill_id );
	if( learned == 0 )
		return 0;
	if( wanted <= 0 || wanted > learned )
		return learned;
	return static_cast<uint16>( wanted );
}

inline bool skill_ready( map_session_data* sd, uint16 skill_id, uint16 skill_lv ) {
	if( sd->scd.count( skill_id ) > 0 ) // cooldown
		return false;
	return sd->battle_status.sp >= static_cast<uint32>( std::max( 0, skill_get_sp( skill_id, skill_lv ) ) );
}

// ---- start / stop -----------------------------------------------------------

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
	st = s_state{};
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

// ---- items ------------------------------------------------------------------

inline bool use_item_id( map_session_data* sd, t_itemid nameid ) {
	if( nameid == 0 )
		return false;
	int16 idx = pc_search_inventory( sd, nameid );
	return idx >= 0 && pc_useitem( sd, idx ) != 0;
}

/// HP/SP slots: the first slot whose threshold is reached and whose item the player owns.
inline void use_recovery( map_session_data* sd, const char* item_var, const char* pct_var, uint32 cur, uint32 max ) {
	if( max == 0 )
		return;
	int64 now_pct = static_cast<int64>( cur ) * 100 / max;

	for( int32 i = 0; i < SLOTS; i++ ) {
		t_itemid nameid = static_cast<t_itemid>( setting( sd, item_var, i ) );
		int64 pct = setting( sd, pct_var, i );

		if( nameid == 0 || pct <= 0 || now_pct >= pct )
			continue;
		if( use_item_id( sd, nameid ) )
			return;
	}
}

inline void use_other_items( map_session_data* sd, s_state& st, t_tick tick ) {
	for( int32 i = 0; i < SLOTS; i++ ) {
		t_itemid nameid = static_cast<t_itemid>( setting( sd, "AA_OT_IT", i ) );
		int64 interval = setting( sd, "AA_OT_MS", i );

		if( nameid == 0 || interval <= 0 )
			continue;
		if( st.other_last[i] != 0 && DIFF_TICK( tick, st.other_last[i] ) < interval )
			continue;
		if( use_item_id( sd, nameid ) )
			st.other_last[i] = tick;
	}
}

// ---- teleport ---------------------------------------------------------------

/// Teleports using the enabled method(s). Skill first (cheaper), then Fly Wing.
inline bool teleport( map_session_data* sd ) {
	if( map_getmapflag( sd->m, MF_NOTELEPORT ) )
		return false;

	if( setting( sd, "AA_TPSKILL" ) && pc_checkskill( sd, AL_TELEPORT ) > 0 && skill_ready( sd, AL_TELEPORT, 1 ) ) {
		status_zap( sd, 0, skill_get_sp( AL_TELEPORT, 1 ) );
		clif_skill_nodamage( sd, *sd, AL_TELEPORT, 1 );
		pc_randomwarp( sd, CLR_TELEPORT );
		return true;
	}

	if( setting( sd, "AA_TPITEM" ) ) {
		for( t_itemid wing : FLY_WINGS ) {
			if( use_item_id( sd, wing ) )
				return true;
		}
	}
	return false;
}

inline bool is_avoided( map_session_data* sd, mob_data* md ) {
	if( setting( sd, "AA_AVOIDBOSS" ) && md->get_bosstype() != BOSSTYPE_NONE )
		return true;

	for( int32 i = 0; i < MAX_AVOID; i++ ) {
		int64 id = setting( sd, "AA_AVOID", i );
		if( id != 0 && id == md->mob_id )
			return true;
	}
	return false;
}

inline int32 avoid_sub( block_list* bl, va_list ap ) {
	map_session_data* sd = va_arg( ap, map_session_data* );
	mob_data* md = reinterpret_cast<mob_data*>( bl );

	if( status_isdead( *bl ) || md->special_state.ai != AI_NONE )
		return 0;
	return is_avoided( sd, md ) ? 1 : 0;
}

// ---- target search ----------------------------------------------------------

inline int32 search_range( map_session_data* sd ) {
	int32 range = static_cast<int32>( setting( sd, "AA_RANGE" ) );
	if( range <= 0 )
		range = DEFAULT_RANGE;
	return cap_value( range, 1, MAX_RANGE );
}

inline int32 search_sub( block_list* bl, va_list ap ) {
	map_session_data* sd = va_arg( ap, map_session_data* );
	int32* best_id = va_arg( ap, int32* );
	int32* best_dist = va_arg( ap, int32* );

	mob_data* md = reinterpret_cast<mob_data*>( bl );

	if( status_isdead( *bl ) )
		return 0;
	if( md->special_state.ai != AI_NONE ) // summoned / player-owned monsters
		return 0;
	if( is_avoided( sd, md ) )
		return 0;
	if( battle_check_target( sd, bl, BCT_ENEMY ) <= 0 )
		return 0;
	// Don't steal monsters that are fighting another player
	if( md->target_id != 0 && md->target_id != sd->id ) {
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
	int32 range = search_range( sd );
	int32 best_id = 0;
	int32 best_dist = range + 1;
	map_foreachinallrange( search_sub, sd, range, BL_MOB, sd, &best_id, &best_dist );
	return best_id ? map_id2bl( best_id ) : nullptr;
}

// ---- loot -------------------------------------------------------------------

inline int32 item_sub( block_list* bl, va_list ap ) {
	map_session_data* sd = va_arg( ap, map_session_data* );
	s_state* st = va_arg( ap, s_state* );
	int32* best_id = va_arg( ap, int32* );
	int32* best_dist = va_arg( ap, int32* );

	if( st->bad_items.count( bl->id ) )
		return 0;
	int32 dist = distance_bl( sd, bl );
	if( dist >= *best_dist )
		return 0;
	if( dist > 1 && !unit_can_reach_bl( sd, bl, LOOT_RANGE * 2, 0, nullptr, nullptr ) )
		return 0;

	*best_id = bl->id;
	*best_dist = dist;
	return 1;
}

inline int32 count_items_sub( block_list* bl, va_list ap ) {
	s_state* st = va_arg( ap, s_state* );
	return st->bad_items.count( bl->id ) ? 0 : 1;
}

/// Returns true if this tick was spent on looting.
inline bool loot( map_session_data* sd, s_state& st ) {
	bool pickup = setting( sd, "AA_PICKUP" ) != 0;
	bool greed = setting( sd, "AA_GREED" ) != 0 && pc_checkskill( sd, BS_GREED ) > 0;

	if( !pickup && !greed )
		return false;

	if( st.bad_items.size() > 500 )
		st.bad_items.clear();

	// Greed grabs everything within 2 cells
	if( greed && skill_ready( sd, BS_GREED, 1 )
		&& map_foreachinallrange( count_items_sub, sd, 2, BL_ITEM, &st ) > 0
		&& unit_skilluse_id( sd, sd->id, BS_GREED, 1 ) ) {
		return true;
	}

	if( !pickup )
		return false;

	int32 best_id = 0;
	int32 best_dist = LOOT_RANGE + 1;
	map_foreachinallrange( item_sub, sd, LOOT_RANGE, BL_ITEM, sd, &st, &best_id, &best_dist );

	block_list* bl = best_id ? map_id2bl( best_id ) : nullptr;
	if( bl == nullptr || bl->type != BL_ITEM )
		return false;

	if( check_distance_bl( sd, bl, 1 ) ) {
		if( !pc_takeitem( sd, reinterpret_cast<flooritem_data*>( bl ) ) )
			st.bad_items.insert( bl->id ); // not ours (yet) / overweight
		return true;
	}

	if( !unit_is_walking( sd ) || st.loot_id != bl->id ) {
		st.loot_id = bl->id;
		if( !unit_walktoxy( sd, bl->x, bl->y, 0 ) )
			st.bad_items.insert( bl->id );
	}
	return true;
}

/// Engine autoloot fields from the AA_LOOT* variables. `allowed` = player may use VIP loot.
inline void apply_loot( map_session_data* sd, bool allowed ) {
	sd->state.autoloot = 0;
	sd->state.autoloottype = 0;
	sd->state.autolooting = 0;
	for( int32 i = 0; i < AUTOLOOTITEM_SIZE; i++ )
		sd->state.autolootid[i] = 0;

	if( !allowed )
		return;

	if( setting( sd, "AA_LOOTALL" ) )
		sd->state.autoloot = 10000; // 100%

	sd->state.autoloottype = static_cast<decltype( sd->state.autoloottype )>( setting( sd, "AA_LOOTTYPE" ) );

	int32 n = 0;
	for( int32 i = 0; i < AUTOLOOTITEM_SIZE; i++ ) {
		t_itemid nameid = static_cast<t_itemid>( setting( sd, "AA_LOOTID", i ) );
		if( nameid != 0 && setting( sd, "AA_LOOTON", i ) && item_db.exists( nameid ) )
			sd->state.autolootid[n++] = nameid;
	}
	// pc_isautolooting() skips the autolootid list entirely unless this flag is set
	// (@alootid sets it; without it the list only worked if @alootid was used earlier).
	sd->state.autolooting = ( n > 0 ) ? 1 : 0;
}

// ---- combat -----------------------------------------------------------------

/// Remaining ms of a status on the player: 0 = not active, -1 = active without end (permanent).
inline t_tick status_remaining( map_session_data* sd, sc_type sc, t_tick tick ) {
	if( sc <= SC_NONE || sc >= SC_MAX )
		return 0;
	status_change_entry* sce = sd->sc.getSCE( sc );
	if( sce == nullptr )
		return 0;
	const TimerData* td = get_timer( sce->timer );
	if( td == nullptr )
		return -1;
	return std::max<t_tick>( 1, DIFF_TICK( td->tick, tick ) );
}

/// Walks buff slots in order 1 > 5 and casts the first one that needs refreshing.
/// Auto mode reads the real remaining time of the buff's status (incl. duration bonuses)
/// and recasts when it is about to run out; AA_BUF_SEC[slot] > 0 overrides with a fixed interval.
/// Returns true if a cast started.
inline bool use_buffs( map_session_data* sd, s_state& st, t_tick tick ) {
	for( int32 i = 0; i < SLOTS; i++ ) {
		uint16 skill_id = static_cast<uint16>( setting( sd, "AA_BUF_SK", i ) );
		uint16 skill_lv = slot_level( sd, skill_id, setting( sd, "AA_BUF_LV", i ) );

		if( skill_lv == 0 )
			continue;
		if( st.buff_last[i] != 0 && DIFF_TICK( tick, st.buff_last[i] ) < BUFF_RETRY_DELAY )
			continue;

		bool need;
		int64 manual_sec = setting( sd, "AA_BUF_SEC", i );
		sc_type sc = skill_get_sc( skill_id );

		if( manual_sec > 0 ) {
			// Manual: fixed interval since our last cast
			need = st.buff_last[i] == 0 || DIFF_TICK( tick, st.buff_last[i] ) >= manual_sec * 1000;
		} else if( sc > SC_NONE && sc < SC_MAX ) {
			t_tick left = status_remaining( sd, sc, tick );
			need = left == 0 || ( left > 0 && left <= BUFF_REFRESH_LEAD );
		} else {
			// No status to watch: use the duration from skill_db, or cast once if it has none
			int32 duration = skill_get_time( skill_id, skill_lv );
			need = st.buff_last[i] == 0 || ( duration > 0 && DIFF_TICK( tick, st.buff_last[i] ) >= duration );
		}

		if( !need || !skill_ready( sd, skill_id, skill_lv ) )
			continue;

		bool ok;
		if( skill_get_inf( skill_id ) & INF_GROUND_SKILL )
			ok = unit_skilluse_pos( sd, sd->x, sd->y, skill_id, skill_lv ) != 0;
		else
			ok = unit_skilluse_id( sd, sd->id, skill_id, skill_lv ) != 0;

		if( ok ) {
			st.buff_last[i] = tick;
			return true;
		}
	}
	return false;
}

/// Tries the attack skill slots in order. Returns true if the tick was used (cast or walk).
inline bool use_attack_skills( map_session_data* sd, block_list* target, bool* has_skill ) {
	*has_skill = false;

	for( int32 i = 0; i < SLOTS; i++ ) {
		uint16 skill_id = static_cast<uint16>( setting( sd, "AA_ATK_SK", i ) );
		uint16 skill_lv = slot_level( sd, skill_id, setting( sd, "AA_ATK_LV", i ) );

		if( skill_lv == 0 )
			continue;
		*has_skill = true;

		if( !skill_ready( sd, skill_id, skill_lv ) )
			continue;

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

		bool ok;
		if( inf & INF_GROUND_SKILL )
			ok = unit_skilluse_pos( sd, target->x, target->y, skill_id, skill_lv ) != 0;
		else if( inf & INF_SELF_SKILL )
			ok = unit_skilluse_id( sd, sd->id, skill_id, skill_lv ) != 0;
		else
			ok = unit_skilluse_id( sd, target->id, skill_id, skill_lv ) != 0;

		if( ok )
			return true;
	}
	return false;
}

inline void wander( map_session_data* sd ) {
	if( unit_is_walking( sd ) )
		return;

	int16 x = sd->x, y = sd->y;
	if( map_search_freecell( sd, sd->m, &x, &y, 10, 10, 1 ) )
		unit_walktoxy( sd, x, y, 0 );
}

inline void after_teleport( s_state& st, t_tick tick ) {
	st.target_id = 0;
	st.loot_id = 0;
	st.last_target_tick = tick;
}

inline void think( map_session_data* sd, s_state& st, t_tick tick ) {
	const char* reason = nullptr;
	if( !can_run( sd, &reason ) ) {
		stop( sd, reason );
		return;
	}

	// 1. Items (allowed while casting / walking)
	use_recovery( sd, "AA_HP_IT", "AA_HP_PCT", sd->battle_status.hp, sd->battle_status.max_hp );
	use_recovery( sd, "AA_SP_IT", "AA_SP_PCT", sd->battle_status.sp, sd->battle_status.max_sp );
	use_other_items( sd, st, tick );

	// 2. Run away from avoided monsters / bosses
	if( DIFF_TICK( tick, st.last_avoid_tp ) >= AVOID_TP_DELAY
		&& map_foreachinallrange( avoid_sub, sd, AREA_SIZE, BL_MOB, sd ) > 0 ) {
		st.last_avoid_tp = tick;
		if( teleport( sd ) ) {
			after_teleport( st, tick );
			return;
		}
	}

	// Busy casting / stunned etc. - wait for next tick
	if( pc_cant_act( sd ) || sd->ud.skilltimer != INVALID_TIMER )
		return;

	// 3. Buffs
	if( use_buffs( sd, st, tick ) )
		return;

	// 4. Keep fighting the current target, otherwise loot before looking for the next one
	block_list* target = valid_target( sd, st.target_id );
	if( target == nullptr ) {
		st.target_id = 0;
		if( loot( sd, st ) ) {
			st.last_target_tick = tick;
			return;
		}
		target = find_target( sd );
		st.target_id = target ? target->id : 0;
	}

	// 5. Nothing to fight: teleport after the configured delay, otherwise wander
	if( target == nullptr ) {
		t_tick idle = DIFF_TICK( tick, st.last_target_tick );
		int64 delay = setting( sd, "AA_TPDELAY" );
		if( delay <= 0 )
			delay = 3;

		if( idle >= delay * 1000 && ( setting( sd, "AA_TPITEM" ) || setting( sd, "AA_TPSKILL" ) ) ) {
			st.last_target_tick = tick; // retry after another delay if teleport failed
			if( teleport( sd ) ) {
				after_teleport( st, tick );
				return;
			}
		}
		if( idle >= IDLE_WALK_DELAY )
			wander( sd );
		return;
	}

	st.last_target_tick = tick;

	// 6. Attack: skills first, normal attack as fallback (or always when no skill is set)
	bool has_skill = false;
	if( use_attack_skills( sd, target, &has_skill ) )
		return;

	if( !has_skill || setting( sd, "AA_NORMAL" ) ) {
		if( sd->ud.target != target->id || sd->ud.attacktimer == INVALID_TIMER )
			unit_attack( sd, target->id, 1 );
	}
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
