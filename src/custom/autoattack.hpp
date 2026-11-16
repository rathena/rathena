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
//   AA_BUF_SEC[0..4]          0 = auto (read buff time), >0 = recast every N seconds instead.
//                             Sphere skills ignore it (refill by count); heals / summons use it.
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
constexpr t_tick SPHERE_RETRY_DELAY = 500;  // minimum ms between casts of a sphere slot
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
	std::unordered_map<int32, t_tick> bad_mobs; // mob id -> ignore until tick (stuck on it)

	// Stuck watchdog: what we were doing and when it last made progress
	int32 watch_id = 0;      // target or floor item being watched
	uint32 watch_hp = 0;     // target hp at last progress
	int16 watch_x = 0, watch_y = 0; // our position at last progress
	t_tick watch_tick = 0;
};

constexpr t_tick STUCK_TIMEOUT = 3000;  // ms without progress before giving up on a target/item
constexpr t_tick BAD_MOB_TIME = 10000;  // ms to ignore a monster we got stuck on

/// Returns true when `id` has made no progress (we did not move, its hp did not drop)
/// for STUCK_TIMEOUT. `hp` is 0 for floor items.
inline bool is_stuck( map_session_data* sd, s_state& st, int32 id, uint32 hp, t_tick tick ) {
	if( st.watch_id != id || hp < st.watch_hp || sd->x != st.watch_x || sd->y != st.watch_y ) {
		st.watch_id = id;
		st.watch_hp = hp;
		st.watch_x = sd->x;
		st.watch_y = sd->y;
		st.watch_tick = tick;
		return false;
	}
	st.watch_hp = hp;
	return DIFF_TICK( tick, st.watch_tick ) >= STUCK_TIMEOUT;
}

// ---- warp avoidance -----------------------------------------------------------
// Warps (and other OnTouch NPCs) mark their trigger area with CELL_NPC. We never pick
// destinations whose path crosses such a cell, and stop walking if a path is about to.

/// True if the cells of `wpd` (from x,y, starting at step `from`) touch an OnTouch NPC cell.
inline bool walkpath_hits_npc( int16 m, int16 x, int16 y, const walkpath_data& wpd, int32 from ) {
	for( int32 i = from; i < wpd.path_len; i++ ) {
		x += dirx[wpd.path[i]];
		y += diry[wpd.path[i]];
		if( map_getcell( m, x, y, CELL_CHKNPC ) )
			return true;
	}
	return false;
}

/// True if walking from our position to (x, y) is possible without stepping on a warp.
inline bool safe_path( map_session_data* sd, int16 x, int16 y ) {
	if( map_getcell( sd->m, x, y, CELL_CHKNPC ) )
		return false;
	walkpath_data wpd;
	if( !path_search( &wpd, sd->m, sd->x, sd->y, x, y, 0, CELL_CHKNOREACH ) )
		return false;
	return !walkpath_hits_npc( sd->m, sd->x, sd->y, wpd, 0 );
}

/// True if a target/item is reachable without stepping on a warp.
inline bool safe_path_bl( map_session_data* sd, block_list* bl ) {
	if( sd->x == bl->x && sd->y == bl->y )
		return true;
	int16 x = bl->x, y = bl->y;
	if( !unit_can_reach_bl( sd, bl, MAX_RANGE * 2, 0, &x, &y ) )
		return false;
	return safe_path( sd, x, y );
}

/// Our current walk is about to step on a warp cell.
inline bool walking_into_warp( map_session_data* sd ) {
	if( !unit_is_walking( sd ) )
		return false;
	return walkpath_hits_npc( sd->m, sd->x, sd->y, sd->ud.walkpath, sd->ud.walkpath.path_pos );
}

inline bool is_bad_mob( s_state& st, int32 id, t_tick tick ) {
	auto it = st.bad_mobs.find( id );
	if( it == st.bad_mobs.end() )
		return false;
	if( DIFF_TICK( tick, it->second ) >= 0 ) {
		st.bad_mobs.erase( it );
		return false;
	}
	return true;
}

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

// ---- skill classification ---------------------------------------------------
// skill_db has no "heal" / "sphere" flag, so these groups are explicit lists.
// Add custom skills here (then recompile) to put them in a group.

/// How a Skill Buff slot decides when to recast.
enum e_buff_mode : int32 {
	BUFF_NONE = 0,     // not a buff
	BUFF_STATUS = 1,   // watch the skill's status and its real remaining time
	BUFF_SPHERE = 2,   // keep spirit spheres / coins / charms / soul energy / elemental balls full
	BUFF_INTERVAL = 3, // no status to watch: recast every AA_BUF_SEC seconds (heals, summons)
};

/// Spirit spheres, coins, charms, soul energy, Warlock balls: recast while below max.
constexpr uint16 SPHERE_SKILLS[] = {
	MO_CALLSPIRITS, CH_SOULCOLLECT, GS_GLITTERING, RL_RICHS_COIN,
	KO_KAHU_ENTEN, KO_HYOUHU_HUBUKI, KO_KAZEHU_SEIRAN, KO_DOHU_KOUKAI,
	SOA_SOUL_GATHERING,
	WL_SUMMONFB, WL_SUMMONBL, WL_SUMMONWB, WL_SUMMONSTONE,
};

/// Heals and summons: no status, recast on a fixed interval set by the player.
constexpr uint16 INTERVAL_SKILLS[] = {
	// heal
	NV_FIRSTAID, AL_HEAL, AB_HIGHNESSHEAL, AB_CHEAL, CD_DILECTIO_HEAL,
	AM_POTIONPITCHER, CR_SLIMPITCHER, SU_TUNABELLY,
	// summon
	SO_SUMMON_AGNI, SO_SUMMON_AQUA, SO_SUMMON_VENTUS, SO_SUMMON_TERA,
	AM_CANNIBALIZE, AM_SPHEREMINE, KO_ZANZOU, NC_SILVERSNIPER, NC_MAGICDECOY,
	MT_M_MACHINE, SH_KI_SUL_WATER_SPRAYING,
};

/// Toggle / stealth skills that would break auto attack (can't attack while hidden, running, ...).
constexpr uint16 NO_AUTO_SKILLS[] = {
	TF_HIDING, AS_CLOAKING, GC_CLOAKINGEXCEED, NV_TRICKDEAD, ST_CHASEWALK,
	RA_CAMOUFLAGE, KO_YAMIKUMO, TK_RUN, RA_WUGDASH, SC_INVISIBILITY,
};

/// Skills that switch OFF when cast again but lack the Toggleable flag in skill_db.
constexpr uint16 TOGGLE_SKILLS[] = {
	SM_AUTOBERSERK, NC_STEALTHFIELD, LG_FORCEOFVANGUARD, SC_REPRODUCE, CG_LONGINGFREEDOM,
};

inline bool is_toggle( uint16 skill_id );

/// Status that shows the skill is active on the caster (usually the skill_db Status).
inline sc_type watched_status( uint16 skill_id ) {
	if( skill_id == NC_STEALTHFIELD )
		return SC_STEALTHFIELD_MASTER; // caster gets the MASTER status, allies get SC_STEALTHFIELD
	return skill_get_sc( skill_id );
}

template <size_t N>
inline bool in_list( const uint16 ( &list )[N], uint16 skill_id ) {
	return std::find( std::begin( list ), std::end( list ), skill_id ) != std::end( list );
}

inline bool is_toggle( uint16 skill_id ) {
	return skill_get_inf2( skill_id, INF2_TOGGLEABLE ) || in_list( TOGGLE_SKILLS, skill_id );
}

inline e_buff_mode buff_mode( uint16 skill_id ) {
	std::shared_ptr<s_skill_db> skill = skill_db.find( skill_id );
	if( skill_id == 0 || skill == nullptr || skill->inf == INF_PASSIVE_SKILL )
		return BUFF_NONE;
	if( in_list( NO_AUTO_SKILLS, skill_id ) )
		return BUFF_NONE;
	if( in_list( SPHERE_SKILLS, skill_id ) )
		return BUFF_SPHERE;
	if( in_list( INTERVAL_SKILLS, skill_id ) )
		return BUFF_INTERVAL;
	if( ( skill->inf & ( INF_SELF_SKILL | INF_SUPPORT_SKILL ) ) && skill->sc > SC_NONE && skill->sc < SC_MAX )
		return BUFF_STATUS;
	return BUFF_NONE;
}

/// Bitmask: 1 = usable as attack skill, 2 = usable as buff skill, 0 = neither (passive / unknown)
inline int32 skill_category( uint16 skill_id ) {
	std::shared_ptr<s_skill_db> skill = skill_db.find( skill_id );
	if( skill_id == 0 || skill == nullptr )
		return 0;

	int32 inf = skill->inf;
	if( inf == INF_PASSIVE_SKILL ) // passive: never castable
		return 0;

	// Attack: deals damage, or is cast on an enemy (e.g. Provoke, Lex Divina)
	if( !skill->nk[NK_NODAMAGE] || ( inf & INF_ATTACK_SKILL ) )
		return 1;

	// Buff: status buffs (incl. toggles), sphere skills, heals / summons on interval.
	// Traps, cures, teleport/warp, crafting etc. are in no group.
	if( buff_mode( skill_id ) != BUFF_NONE )
		return 2;

	return 0;
}

/// BUFF_SPHERE: true while the skill's resource is below its maximum.
inline bool sphere_needed( map_session_data* sd, uint16 skill_id, uint16 skill_lv ) {
	int32 dragon = 0;
	if( status_change_entry* rd = sd->sc.getSCE( SC_RAISINGDRAGON ); rd != nullptr )
		dragon = rd->val1;

	switch( skill_id ) {
	case MO_CALLSPIRITS:
		return sd->spiritball < skill_lv + dragon;
	case CH_SOULCOLLECT:
		return sd->spiritball < 5 + dragon;
	case GS_GLITTERING:
	case RL_RICHS_COIN:
		return sd->spiritball < 10;
	case KO_KAHU_ENTEN:
	case KO_HYOUHU_HUBUKI:
	case KO_KAZEHU_SEIRAN:
	case KO_DOHU_KOUKAI:
		return sd->spiritcharm_type != skill_get_ele( skill_id, skill_lv ) || sd->spiritcharm < MAX_SPIRITCHARM;
	case SOA_SOUL_GATHERING:
		// Only castable while Soul Collect is on
		return sd->sc.getSCE( SC_SOULCOLLECT ) != nullptr
			&& sd->soulball < 5 + pc_checkskill( sd, SP_SOULENERGY ) * 3;
	case WL_SUMMONFB:
	case WL_SUMMONBL:
	case WL_SUMMONWB:
	case WL_SUMMONSTONE:
		for( int32 i = SC_SPHERE_1; i <= SC_SPHERE_5; i++ )
			if( sd->sc.getSCE( static_cast<sc_type>( i ) ) == nullptr )
				return true; // a free ball slot
		return false;
	}
	return false;
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
	s_state* st = va_arg( ap, s_state* );
	t_tick tick = va_arg( ap, t_tick );

	mob_data* md = reinterpret_cast<mob_data*>( bl );

	if( status_isdead( *bl ) )
		return 0;
	if( is_bad_mob( *st, bl->id, tick ) )
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

	// Monsters already attacking us come first (score lowered by 100), then the nearest
	int32 dist = distance_bl( sd, bl );
	if( md->target_id == sd->id )
		dist -= 100;
	if( dist >= *best_dist )
		return 0;
	if( !safe_path_bl( sd, bl ) ) // unreachable, or only through a warp
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

inline block_list* find_target( map_session_data* sd, s_state& st, t_tick tick ) {
	if( st.bad_mobs.size() > 200 )
		st.bad_mobs.clear();

	int32 range = search_range( sd );
	int32 best_id = 0;
	int32 best_dist = range + 1;
	map_foreachinallrange( search_sub, sd, range, BL_MOB, sd, &best_id, &best_dist, &st, tick );
	return best_id ? map_id2bl( best_id ) : nullptr;
}

inline int32 aggressor_sub( block_list* bl, va_list ap ) {
	map_session_data* sd = va_arg( ap, map_session_data* );
	mob_data* md = reinterpret_cast<mob_data*>( bl );
	return ( !status_isdead( *bl ) && md->target_id == sd->id ) ? 1 : 0;
}

/// True when a living monster nearby is targeting us.
inline bool under_attack( map_session_data* sd ) {
	return map_foreachinallrange( aggressor_sub, sd, AREA_SIZE, BL_MOB, sd ) > 0;
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
	if( dist > 1 && !safe_path( sd, bl->x, bl->y ) ) // unreachable, or only through a warp
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
inline bool loot( map_session_data* sd, s_state& st, t_tick tick ) {
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

	// Could not get closer for a while (blocked path / kept getting interrupted): skip it
	if( is_stuck( sd, st, bl->id, 0, tick ) ) {
		st.bad_items.insert( bl->id );
		return false;
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

		e_buff_mode mode = buff_mode( skill_id );
		if( skill_lv == 0 || mode == BUFF_NONE )
			continue;

		// Spheres refill one cast at a time, so retry quickly; others wait a bit after a cast
		t_tick retry = ( mode == BUFF_SPHERE ) ? SPHERE_RETRY_DELAY : BUFF_RETRY_DELAY;
		if( st.buff_last[i] != 0 && DIFF_TICK( tick, st.buff_last[i] ) < retry )
			continue;

		bool need = false;
		int64 manual_sec = setting( sd, "AA_BUF_SEC", i );
		sc_type sc = watched_status( skill_id );

		switch( mode ) {
		case BUFF_SPHERE:
			need = sphere_needed( sd, skill_id, skill_lv );
			break;
		case BUFF_STATUS:
			if( is_toggle( skill_id ) ) {
				// Toggle: casting again would switch it OFF, so only cast when it is off
				need = status_remaining( sd, sc, tick ) == 0;
			} else if( manual_sec > 0 ) {
				need = st.buff_last[i] == 0 || DIFF_TICK( tick, st.buff_last[i] ) >= manual_sec * 1000;
			} else {
				t_tick left = status_remaining( sd, sc, tick );
				need = left == 0 || ( left > 0 && left <= BUFF_REFRESH_LEAD );
			}
			break;
		case BUFF_INTERVAL: {
			// Interval set in the menu; fall back to the skill_db duration, then 10s
			t_tick every = manual_sec * 1000;
			if( every <= 0 )
				every = skill_get_time( skill_id, skill_lv );
			if( every <= 0 )
				every = 10000;
			need = st.buff_last[i] == 0 || DIFF_TICK( tick, st.buff_last[i] ) >= every;
			break;
		}
		default:
			break;
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

	// A few random spots; take the first one we can reach without crossing a warp
	for( int32 i = 0; i < 8; i++ ) {
		int16 x = sd->x, y = sd->y;
		if( !map_search_freecell( sd, sd->m, &x, &y, 10, 10, 1 ) )
			return;
		if( safe_path( sd, x, y ) ) {
			unit_walktoxy( sd, x, y, 0 );
			return;
		}
	}
}

inline void after_teleport( s_state& st, t_tick tick ) {
	st.target_id = 0;
	st.loot_id = 0;
	st.watch_id = 0;
	st.bad_mobs.clear();
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

	// Never walk into a warp: the engine may re-path a chase through one, so check the live path
	if( walking_into_warp( sd ) ) {
		unit_stop_walking( sd, USW_FIXPOS | USW_RELEASE_TARGET );
		unit_stop_attack( sd );
		if( st.target_id != 0 )
			st.bad_mobs[st.target_id] = tick + BAD_MOB_TIME;
		if( st.loot_id != 0 )
			st.bad_items.insert( st.loot_id );
		st.target_id = 0;
		st.loot_id = 0;
		st.watch_id = 0;
		return;
	}

	// Busy casting / stunned etc. - wait for next tick (not counted as being stuck)
	if( pc_cant_act( sd ) || sd->ud.skilltimer != INVALID_TIMER ) {
		st.watch_tick = tick;
		return;
	}

	// 3. Buffs
	if( use_buffs( sd, st, tick ) ) {
		st.watch_tick = tick;
		return;
	}

	// 4. Keep fighting the current target, otherwise loot before looking for the next one.
	//    Looting waits while monsters are attacking us.
	block_list* target = valid_target( sd, st.target_id );
	if( target != nullptr && is_bad_mob( st, target->id, tick ) )
		target = nullptr;
	if( target == nullptr ) {
		st.target_id = 0;
		if( !under_attack( sd ) && loot( sd, st, tick ) ) {
			st.last_target_tick = tick;
			return;
		}
		target = find_target( sd, st, tick );
		st.target_id = target ? target->id : 0;
	}

	// Stuck on this target (can't reach it, not moving, its hp not dropping): drop it for a while
	if( target != nullptr && is_stuck( sd, st, target->id, status_get_hp( target ), tick ) ) {
		st.bad_mobs[target->id] = tick + BAD_MOB_TIME;
		st.target_id = 0;
		st.watch_id = 0;
		unit_stop_attack( sd );
		unit_stop_walking( sd, USW_FIXPOS | USW_RELEASE_TARGET );
		return;
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
		// Don't re-issue while already walking toward / swinging at this target (caused jitter)
		bool busy = sd->ud.target == target->id && ( sd->ud.attacktimer != INVALID_TIMER || unit_is_walking( sd ) );
		if( !busy )
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
