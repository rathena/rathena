// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "unit.hpp"

#include <cstdlib>
#include <cstring>

#include <common/db.hpp>
#include <common/ers.hpp>  // ers_destroy
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/timer.hpp>
#include <common/utils.hpp>

#include "achievement.hpp"
#include "battle.hpp"
#include "battleground.hpp"
#include "channel.hpp"
#include "chat.hpp"
#include "clif.hpp"
#include "duel.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "intif.hpp"
#include "map.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pet.hpp"
#include "storage.hpp"
#include "trade.hpp"

using namespace rathena;

#ifndef MAX_SHADOW_SCAR 
	#define MAX_SHADOW_SCAR 100 /// Max Shadow Scars
#endif

// How many milliseconds need to pass before we calculated the exact position of a unit
// Calculation will only happen on demand and when at least the time defined here has passed
#ifndef MIN_POS_INTERVAL
	#define MIN_POS_INTERVAL 20
#endif

// Minimum delay after which new client-sided commands for slaves are accepted
// Applies to mercenaries and homunculus
#ifndef MIN_DELAY_SLAVE
	#define MIN_DELAY_SLAVE MAX_ASPD_NOPC * 2
#endif

// Time frame during which we will send move packets each cell moved after being hit
// This is needed because damage packets prevent the client from displaying movement for a while
#ifndef MOVE_REFRESH_TIME
	#define MOVE_REFRESH_TIME MAX_WALK_SPEED
#endif

// Directions values
// 1 0 7
// 2 . 6
// 3 4 5
// See also path.cpp walk_choices
const int16 dirx[DIR_MAX]={0,-1,-1,-1,0,1,1,1}; ///lookup to know where will move to x according dir
const int16 diry[DIR_MAX]={1,1,0,-1,-1,-1,0,1}; ///lookup to know where will move to y according dir

//early declaration
static TIMER_FUNC(unit_attack_timer);
static TIMER_FUNC(unit_walktoxy_timer);
int32 unit_unattackable(struct block_list *bl);

/**
 * Get the unit_data related to the bl
 * @param bl : Object to get the unit_data from
 *	valid type are : BL_PC|BL_MOB|BL_PET|BL_NPC|BL_HOM|BL_MER|BL_ELEM
 * @return unit_data of bl or nullptr
 */
struct unit_data* unit_bl2ud(struct block_list *bl)
{
	if( bl == nullptr) return nullptr;
	switch(bl->type){
	case BL_PC: return &((map_session_data*)bl)->ud;
	case BL_MOB: return &((struct mob_data*)bl)->ud;
	case BL_PET: return &((struct pet_data*)bl)->ud;
	case BL_NPC: return &((struct npc_data*)bl)->ud;
	case BL_HOM: return &((struct homun_data*)bl)->ud;
	case BL_MER: return &((s_mercenary_data*)bl)->ud;
	case BL_ELEM: return &((s_elemental_data*)bl)->ud;
	default : return nullptr;
	}
}

/**
 * Updates chase depending on situation:
 * If target in attack range -> attack
 * If target out of sight -> drop target
 * Otherwise update chase path
 * @param bl: Moving bl
 * @param tick: Current tick
 * @param fullcheck: If false, only check for attack, don't drop target or update chase path
 * @return Whether the chase path was updated (true) or current movement can continue (false)
 */
bool unit_update_chase(block_list& bl, t_tick tick, bool fullcheck) {
	unit_data* ud = unit_bl2ud(&bl);

	if (ud == nullptr)
		return true;

	block_list* tbl = nullptr;
	if (ud->target_to != 0)
		tbl = map_id2bl(ud->target_to);

	// Reached destination, start attacking
	if (tbl != nullptr && tbl->type != BL_ITEM && tbl->m == bl.m && ud->walkpath.path_pos > 0 && check_distance_bl(&bl, tbl, ud->chaserange)) {
		int32 stop_flag = USW_FIXPOS|USW_RELEASE_TARGET;

		// Source may die due to reflect damage
		map_freeblock_lock();

		if (ud->state.attack_continue)
			stop_flag = unit_attack(&bl, tbl->id, ud->state.attack_continue);

		// Stop, unless the attack was skipped
		if (stop_flag != USW_NONE) {
			// We need to make sure the walkpath is cleared here so a monster doesn't continue walking in case it unlocked its target
			unit_stop_walking(&bl, stop_flag|USW_FORCE_STOP);
			map_freeblock_unlock();
			return true;
		}
		map_freeblock_unlock();
	}
	// Cancel chase
	else if (tbl == nullptr || (fullcheck && !status_check_visibility(&bl, tbl, (bl.type == BL_MOB)))) {
		// Looted items will have no tbl but target ID is still set, that's why we need to check for the ID here
		if (ud->target_to != 0 && bl.type == BL_MOB) {
			mob_data& md = reinterpret_cast<mob_data&>(bl);
			if (tbl != nullptr) {
				int32 warp = mob_warpchase(&md, tbl);
				// Do warp chase
				if (warp == 1)
					return true;
				// Continue moving to warp
				else if (warp == 2)
					return false;
			}
			// Make sure monsters properly unlock their target, but still continue movement
			mob_unlocktarget(&md, tick);
			return false;
		}

		unit_stop_walking(&bl, USW_FIXPOS|USW_FORCE_STOP|USW_RELEASE_TARGET);
		return true;
	}
	// Update chase path
	else if (fullcheck && ud->walkpath.path_pos > 0 && DIFF_TICK(ud->canmove_tick, tick) <= 0) {
		// We call this only when we know there is no walk delay to prevent it pre-planning a chase
		// If the call here fails, the unit should continue its current path
		if(unit_walktobl(&bl, tbl, ud->chaserange, ud->state.walk_easy | (ud->state.attack_continue ? 2 : 0)))
			return true;
	}

	return false;
}

/**
 * Handles everything that happens when movement to the next cell is initiated
 * @param bl: Moving bl
 * @param sendMove: Whether move packet should be sent or not
 * @param tick: Current tick
 * @return Whether movement was initialized (true) or not (false)
 */
bool unit_walktoxy_nextcell(block_list& bl, bool sendMove, t_tick tick) {
	unit_data* ud = unit_bl2ud(&bl);

	if (ud == nullptr)
		return true;

	// Reached end of walkpath
	if (ud->walkpath.path_pos >= ud->walkpath.path_len) {
		// We need to send the reply to the client even if already at the target cell
		// This allows the client to synchronize the position correctly
		if (sendMove && bl.type == BL_PC)
			clif_walkok(reinterpret_cast<map_session_data&>(bl));
		return false;
	}

	// Monsters first check for a chase skill and if they didn't use one if their target is in range each cell after checking for a chase skill
	if (bl.type == BL_MOB) {
		mob_data& md = reinterpret_cast<mob_data&>(bl);
		// Walk skills are triggered regardless of target due to the idle-walk mob state.
		// But avoid triggering when already reached the end of the walkpath.
		// Monsters use walk/chase skills every second, but we only get here every "speed" ms
		// To make sure we check one skill per second on average, we substract half the speed as ms
		if (!ud->state.force_walk &&
			DIFF_TICK(tick, md.last_skillcheck) > MOB_SKILL_INTERVAL - md.status.speed / 2 &&
			mobskill_use(&md, tick, -1)) {
			if ((ud->skill_id != NPC_SPEEDUP || md.trickcasting == 0) //Stop only when trickcasting expired
				&& ud->skill_id != NPC_EMOTION && ud->skill_id != NPC_EMOTION_ON //NPC_EMOTION doesn't make the monster stop
				&& md.state.skillstate != MSS_WALK) //Walk skills are supposed to be used while walking
			{
				// Skill used, abort walking
				// Fix position as walk has been cancelled.
				clif_fixpos(bl);
				// Movement was initialized, so we need to return true even though it was stopped
				// Monsters only start moving or drop target when they stop using chase skills that stop them
				return true;
			}
			// Resend walk packet for proper Self Destruction display
			sendMove = true;
		}
		if (ud->target_to != 0) {
			// Monsters update their chase path one cell before reaching their final destination
			if (unit_update_chase(bl, tick, (ud->walkpath.path_pos == ud->walkpath.path_len - 1)))
				return true;
		}
	}

	// Get current speed
	int32 speed;
	if (direction_diagonal(ud->walkpath.path[ud->walkpath.path_pos]))
		speed = status_get_speed(&bl) * MOVE_DIAGONAL_COST / MOVE_COST;
	else
		speed = status_get_speed(&bl);

	// Make sure there is no active walktimer
	if (ud->walktimer != INVALID_TIMER) {
		delete_timer(ud->walktimer, unit_walktoxy_timer);
		ud->walktimer = INVALID_TIMER;
	}
	ud->walktimer = add_timer(tick + speed, unit_walktoxy_timer, bl.id, speed);

	// Resend move packet when unit was damaged recently
	if (sendMove || DIFF_TICK(tick, ud->dmg_tick) < MOVE_REFRESH_TIME) {
		clif_move(*ud);
		if (bl.type == BL_PC)
			clif_walkok(reinterpret_cast<map_session_data&>(bl));
	}
	return true;
}

/**
 * Tells a unit to walk to a specific coordinate
 * @param bl: Unit to walk [ALL]
 * @return 1: Success 0: Fail
 */
int32 unit_walktoxy_sub(struct block_list *bl)
{
	nullpo_retr(1, bl);

	unit_data *ud = unit_bl2ud(bl);

	if (ud == nullptr)
		return 0;

	walkpath_data wpd = { 0 };

	if( !path_search(&wpd,bl->m,bl->x,bl->y,ud->to_x,ud->to_y,ud->state.walk_easy,CELL_CHKNOPASS) )
		return 0;

#ifdef OFFICIAL_WALKPATH
	if( bl->type != BL_NPC // If type is a NPC, please disregard.
		&& wpd.path_len > 14 // Official number of walkable cells is 14 if and only if there is an obstacle between. [malufett]
		&& !path_search_long(nullptr, bl->m, bl->x, bl->y, ud->to_x, ud->to_y, CELL_CHKNOPASS) ) // Check if there is an obstacle between
			return 0;
#endif

	ud->walkpath = wpd;

	int32 i;

	// Monsters always target an adjacent tile even if ranged, no need to shorten the path
	if (ud->target_to != 0 && ud->chaserange > 1 && bl->type != BL_MOB) {
		// Generally speaking, the walk path is already to an adjacent tile
		// so we only need to shorten the path if the range is greater than 1.
		// Trim the last part of the path to account for range,
		// but always move at least one cell when requested to move.
		for (i = (ud->chaserange*10)-10; i > 0 && ud->walkpath.path_len>1;) {
			ud->walkpath.path_len--;
			enum directions dir = ud->walkpath.path[ud->walkpath.path_len];
			if (direction_diagonal(dir))
				i -= MOVE_COST * 2; //When chasing, units will target a diamond-shaped area in range [Playtester]
			else
				i -= MOVE_COST;
			ud->to_x -= dirx[dir];
			ud->to_y -= diry[dir];
		}
	}

	ud->state.change_walk_target=0;

	if (bl->type == BL_PC) {
		map_session_data *sd = BL_CAST(BL_PC, bl);
		sd->head_dir = DIR_NORTH;
	}
#if PACKETVER >= 20170726
	// If this is a walking NPC and it will use a player sprite
	else if( bl->type == BL_NPC && pcdb_checkid( status_get_viewdata( bl )->look[LOOK_BASE] ) ){
		// Respawn the NPC as player unit
		unit_refresh( bl, true );
	}
#endif
	// Set mobstate here already as chase skills can be used on the first frame of movement
	// If we don't set it now the monster will always move a full cell before checking
	else if (bl->type == BL_MOB && ud->state.attack_continue) {
		mob_data& md = reinterpret_cast<mob_data&>(*bl);
		mob_setstate(md, MSS_RUSH);
	}

	unit_walktoxy_nextcell(*bl, true, gettick());

	return 1;
}

/**
 * Retrieve the direct master of a bl if one exists.
 * @param bl: char to get his master [HOM|ELEM|PET|MER]
 * @return map_session_data of master or nullptr
 */
TBL_PC* unit_get_master(struct block_list *bl)
{
	if(bl)
		switch(bl->type) {
			case BL_HOM: return (((TBL_HOM *)bl)->master);
			case BL_ELEM: return (((TBL_ELEM *)bl)->master);
			case BL_PET: return (((TBL_PET *)bl)->master);
			case BL_MER: return (((TBL_MER *)bl)->master);
		}
	return nullptr;
}

/**
 * Retrieve a unit's master's teleport timer
 * @param bl: char to get his master's teleport timer [HOM|ELEM|PET|MER]
 * @return timer or nullptr
 */
int32* unit_get_masterteleport_timer(struct block_list *bl)
{
	if(bl)
		switch(bl->type) {
			case BL_HOM: return &(((TBL_HOM *)bl)->masterteleport_timer);
			case BL_ELEM: return &(((TBL_ELEM *)bl)->masterteleport_timer);
			case BL_PET: return &(((TBL_PET *)bl)->masterteleport_timer);
			case BL_MER: return &(((TBL_MER *)bl)->masterteleport_timer);
		}
	return nullptr;
}

/**
 * Warps a unit to its master if the master has gone out of sight (3 second default)
 * Can be any object with a master [MOB|PET|HOM|MER|ELEM]
 * @param tid: Timer
 * @param tick: tick (unused)
 * @param id: Unit to warp
 * @param data: Data transferred from timer call
 * @return 0
 */
TIMER_FUNC(unit_teleport_timer){
	struct block_list *bl = map_id2bl(id);
	int32 *mast_tid = unit_get_masterteleport_timer(bl);

	if(tid == INVALID_TIMER || mast_tid == nullptr)
		return 0;
	else if(*mast_tid != tid || bl == nullptr)
		return 0;
	else {
		map_session_data* msd = unit_get_master( bl );

		if( msd != nullptr && !check_distance_bl( msd, bl, static_cast<int32>( data ) ) ){
			*mast_tid = INVALID_TIMER;
			unit_warp(bl, msd->m, msd->x, msd->y, CLR_TELEPORT );
		} else // No timer needed
			*mast_tid = INVALID_TIMER;
	}
	return 0;
}

/**
 * Checks if a slave unit is outside their max distance from master
 * If so, starts a timer (default: 3 seconds) which will teleport the unit back to master
 * @param sbl: Object with a master [MOB|PET|HOM|MER|ELEM]
 * @return 0
 */
int32 unit_check_start_teleport_timer(struct block_list *sbl)
{
	TBL_PC *msd = nullptr;
	int32 max_dist = 0;

	switch(sbl->type) {
		case BL_HOM:	
		case BL_ELEM:	
		case BL_PET:	
		case BL_MER:	
			msd = unit_get_master(sbl);
			break;
		default:
			return 0;
	}

	switch(sbl->type) {
		case BL_HOM:	max_dist = AREA_SIZE;			break;
		case BL_ELEM:	max_dist = MAX_ELEDISTANCE;		break;
		case BL_PET:	max_dist = AREA_SIZE;			break;
		case BL_MER:	max_dist = MAX_MER_DISTANCE;	break;
	}
	// If there is a master and it's a valid type
	if(msd && max_dist) {
		int32 *msd_tid = unit_get_masterteleport_timer(sbl);

		if(msd_tid == nullptr)
			return 0;
		if (!check_distance_bl(msd, sbl, max_dist)) {
			if(*msd_tid == INVALID_TIMER || *msd_tid == 0)
				*msd_tid = add_timer(gettick()+3000,unit_teleport_timer,sbl->id,max_dist);
		} else {
			if(*msd_tid && *msd_tid != INVALID_TIMER)
				delete_timer(*msd_tid,unit_teleport_timer);
			*msd_tid = INVALID_TIMER; // Cancel recall
		}
	}
	return 0;
}

/**
 * Triggered on full step if stepaction is true and executes remembered action.
 * @param tid: Timer ID
 * @param tick: Unused
 * @param id: ID of bl to do the action
 * @param data: Not used
 * @return 1: Success 0: Fail (No valid bl)
 */
TIMER_FUNC(unit_step_timer){
	struct block_list *bl;
	struct unit_data *ud;
	int32 target_id;

	bl = map_id2bl(id);

	if (!bl || bl->prev == nullptr)
		return 0;

	ud = unit_bl2ud(bl);

	if(!ud)
		return 0;

	if(ud->steptimer != tid) {
		ShowError("unit_step_timer mismatch %d != %d\n",ud->steptimer,tid);
		return 0;
	}

	ud->steptimer = INVALID_TIMER;

	if(!ud->stepaction)
		return 0;

	//Set to false here because if an error occurs, it should not be executed again
	ud->stepaction = false;

	if(!ud->target_to)
		return 0;

	//Flush target_to as it might contain map coordinates which should not be used by other functions
	target_id = ud->target_to;
	ud->target_to = 0;

	//If stepaction is set then we remembered a client request that should be executed on the next step
	//Execute request now if target is in attack range
	if(ud->stepskill_id && skill_get_inf(ud->stepskill_id) & INF_GROUND_SKILL) {
		//Execute ground skill
		struct map_data *md = &map[bl->m];			
		unit_skilluse_pos(bl, target_id%md->xs, target_id/md->xs, ud->stepskill_id, ud->stepskill_lv);
	} else {
		//If a player has target_id set and target is in range, attempt attack
		struct block_list *tbl = map_id2bl(target_id);
		if (tbl == nullptr || !status_check_visibility(bl, tbl, false)) {
			return 0;
		}
		if(ud->stepskill_id == 0) {
			//Execute normal attack
			unit_attack(bl, tbl->id, (ud->state.attack_continue) + 2);
		} else {
			//Execute non-ground skill
			unit_skilluse_id(bl, tbl->id, ud->stepskill_id, ud->stepskill_lv);
		}
	}

	return 1;
}

int32 unit_walktoxy_ontouch(struct block_list *bl, va_list ap)
{
	struct npc_data *nd;

	nullpo_ret(bl);
	nullpo_ret(nd = va_arg(ap,struct npc_data *));

	switch( bl->type ) {
	case BL_PC:
		TBL_PC *sd = (TBL_PC*)bl;

		if (sd == nullptr)
			return 1;
		if (sd->state.block_action & PCBLOCK_NPCCLICK)
			return 1;

		// Remove NPCs that are no longer within the OnTouch area
		for (size_t i = 0; i < sd->areanpc.size(); i++) {
			struct npc_data *nd = map_id2nd(sd->areanpc[i]);

			if (!nd || nd->subtype != NPCTYPE_SCRIPT || !(nd->m == bl->m && bl->x >= nd->x - nd->u.scr.xs && bl->x <= nd->x + nd->u.scr.xs && bl->y >= nd->y - nd->u.scr.ys && bl->y <= nd->y + nd->u.scr.ys))
				rathena::util::erase_at(sd->areanpc, i);
		}
		npc_touchnext_areanpc(sd, false);

		if (map_getcell(bl->m, bl->x, bl->y, CELL_CHKNPC) == 0)
			return 1;

		npc_touch_areanpc(sd, bl->m, bl->x, bl->y, nd);
		break;
	// case BL_MOB:	// unsupported
	}
	
	return 0;
}

/**
 * Defines when to refresh the walking character to object and restart the timer if applicable
 * Also checks for speed update, target location, and slave teleport timers
 * @param tid: Timer ID
 * @param tick: Current tick to decide next timer update
 * @param data: Data used in timer calls
 * @return 0 or unit_walktoxy_sub() or unit_walktoxy()
 */
static TIMER_FUNC(unit_walktoxy_timer)
{
	block_list *bl = map_id2bl(id);

	if(bl == nullptr)
		return 0;
	
	unit_data *ud = unit_bl2ud(bl);

	if(ud == nullptr)
		return 0;

	if(ud->walktimer != tid) {
		ShowError("unit_walk_timer mismatch %d != %d\n",ud->walktimer,tid);
		return 0;
	}

	ud->walktimer = INVALID_TIMER;
	// As movement to next cell finished, set sub-cell position to center
	ud->sx = 8;
	ud->sy = 8;

	if (bl->prev == nullptr)
		return 0; // Stop moved because it is missing from the block_list

	if(ud->walkpath.path_pos>=ud->walkpath.path_len)
		return 0;

	enum directions dir = ud->walkpath.path[ud->walkpath.path_pos];

	if( dir <= DIR_CENTER || dir >= DIR_MAX ){
		return 0;
	}

	unit_setdir(bl, dir, false);

	int32 dx = dirx[dir];
	int32 dy = diry[dir];

	map_session_data *sd = nullptr;
	mob_data *md = nullptr;
	npc_data *nd = nullptr;

	// Get icewall walk block depending on Status Immune mode (players can't be trapped)
	unsigned char icewall_walk_block = 0;

	switch(bl->type) { // avoid useless cast, we can only be 1 type
		case BL_PC: sd = BL_CAST(BL_PC, bl); break;
		case BL_MOB:
			md = BL_CAST(BL_MOB, bl);

			if (status_has_mode(&md->status,MD_STATUSIMMUNE))
				icewall_walk_block = battle_config.boss_icewall_walk_block;
			else
				icewall_walk_block = battle_config.mob_icewall_walk_block;
			break;
		case BL_NPC: nd = BL_CAST(BL_NPC, bl); break;
	}

	int32 x = bl->x;
	int32 y = bl->y;

	//Monsters will walk into an icewall from the west and south if they already started walking
	if(map_getcell(bl->m,x+dx,y+dy,CELL_CHKNOPASS) 
		&& (icewall_walk_block == 0 || dx < 0 || dy < 0 || !map_getcell(bl->m,x+dx,y+dy,CELL_CHKICEWALL)))
		return unit_walktoxy_sub(bl);

	//Monsters can only leave icewalls to the west and south
	//But if movement fails more than icewall_walk_block times, they can ignore this rule
	if(md && !ud->state.force_walk && md->walktoxy_fail_count < icewall_walk_block && map_getcell(bl->m,x,y,CELL_CHKICEWALL) && (dx > 0 || dy > 0)) {
		//Needs to be done here so that rudeattack skills are invoked
		md->walktoxy_fail_count++;
		clif_fixpos( *bl );
		// Monsters in this situation will unlock target
		mob_unlocktarget(md, tick);
		return 0;
	}

	// Refresh view for all those we lose sight
	map_foreachinmovearea(clif_outsight, bl, AREA_SIZE, dx, dy, sd?BL_ALL:BL_PC, bl);

	x += dx;
	y += dy;
	map_moveblock(bl, x, y, tick);

	if (bl->x != x || bl->y != y || ud->walktimer != INVALID_TIMER)
		return 0; // map_moveblock has altered the object beyond what we expected (moved/warped it)

	ud->walktimer = CLIF_WALK_TIMER; // Arbitrary non-INVALID_TIMER value to make the clif code send walking packets
	map_foreachinmovearea(clif_insight, bl, AREA_SIZE, -dx, -dy, sd?BL_ALL:BL_PC, bl);
	ud->walktimer = INVALID_TIMER;

	if (bl->x == ud->to_x && bl->y == ud->to_y) {
#if PACKETVER >= 20170726
		// If this was a walking NPC and it used a player sprite
		if( bl->type == BL_NPC && pcdb_checkid( status_get_viewdata( bl )->look[LOOK_BASE] ) ){
			// Respawn the NPC as NPC unit
			unit_refresh( bl, false );
		}
#endif

		// Remove any possible escape states present for mobs once they stopped moving.
		if (md != nullptr) {
			md->state.can_escape = 0;
		}

		ud->state.force_walk = false;

		if (ud->walk_done_event[0]){
			char walk_done_event[EVENT_NAME_LENGTH];

			// Copying is required in case someone uses unitwalkto inside the event code
			safestrncpy(walk_done_event, ud->walk_done_event, EVENT_NAME_LENGTH);

			//Clear the event
			ud->walk_done_event[0] = 0;

			ud->state.walk_script = true;

			// Execute the event
			npc_event_do_id(walk_done_event,bl->id);

			ud->state.walk_script = false;

			// Check if the unit was killed
			if( status_isdead(*bl) ){
				struct mob_data* md = BL_CAST(BL_MOB, bl);

				if( md && !md->spawn ){
					unit_free(bl, CLR_OUTSIGHT);
				}

				return 0;
			}

		}
	}

	switch(bl->type) {
		case BL_PC:
			if( !sd->npc_ontouch_.empty() )
				npc_touchnext_areanpc(sd,false);
			if(map_getcell(bl->m,x,y,CELL_CHKNPC)) {
				npc_touch_area_allnpc(sd,bl->m,x,y);
				if (bl->prev == nullptr) // Script could have warped char, abort remaining of the function.
					return 0;
			} else
				sd->areanpc.clear();
			pc_cell_basilica(sd);
			break;
		case BL_MOB:
			//Movement was successful, reset walktoxy_fail_count
			md->walktoxy_fail_count = 0;
			if( map_getcell(bl->m,x,y,CELL_CHKNPC) ) {
				if( npc_touch_areanpc2(md) )
					return 0; // Warped
			} else
				md->areanpc_id = 0;
			break;
		case BL_NPC:
			if (nd->is_invisible)
				break;

			int32 xs = -1, ys = -1;
			switch (nd->subtype) {
			case NPCTYPE_SCRIPT:
				xs = nd->u.scr.xs;
				ys = nd->u.scr.ys;
				break;
			case NPCTYPE_WARP:
				xs = nd->u.warp.xs;
				ys = nd->u.warp.ys;
				break;
			}
			if (xs > -1 && ys > -1)
				map_foreachinmap(unit_walktoxy_ontouch, nd->m, BL_PC, nd);
			break;
	}

	int32 speed;

	//If stepaction is set then we remembered a client request that should be executed on the next step
	if (ud->stepaction && ud->target_to) {
		//Delete old stepaction even if not executed yet, the latest command is what counts
		if(ud->steptimer != INVALID_TIMER) {
			delete_timer(ud->steptimer, unit_step_timer);
			ud->steptimer = INVALID_TIMER;
		}
		//Delay stepactions by half a step (so they are executed at full step)
		if( direction_diagonal( ud->walkpath.path[ud->walkpath.path_pos] ) )
			speed = status_get_speed(bl)*MOVE_DIAGONAL_COST/MOVE_COST/2;
		else
			speed = status_get_speed(bl)/2;
		ud->steptimer = add_timer(tick+speed, unit_step_timer, bl->id, 0);
	}

	if(ud->state.change_walk_target) {
		if(unit_walktoxy_sub(bl)) {
			return 1;	
		} else {
			clif_fixpos( *bl );
			return 0;
		}
	}

	ud->walkpath.path_pos++;

	if(unit_walktoxy_nextcell(*bl, false, tick)) {
		// Nothing else needs to be done
	} else if(ud->state.running) { // Keep trying to run.
		if (!(unit_run(bl, nullptr, SC_RUN) || unit_run(bl, sd, SC_WUGDASH)) )
			ud->state.running = 0;
	} else {
		if (!ud->stepaction && ud->target_to > 0) {
			// Update target trajectory.
			if(unit_update_chase(*bl, tick, true))
				return 0;
		}

		// Stopped walking. Update to_x and to_y to current location
		ud->to_x = bl->x;
		ud->to_y = bl->y;

		if (bl->type != BL_NPC	// walking npc ignores cell stack limit
			&& !ud->state.ignore_cell_stack_limit
			&& battle_config.official_cell_stack_limit > 0
			&& map_count_oncell(bl->m, x, y, BL_CHAR|BL_NPC, 1) > battle_config.official_cell_stack_limit) {

			//Walked on occupied cell, call unit_walktoxy again
			if(unit_walktoxy(bl, x, y, 8)) {
				//Execute step timer on next step instead
				if (ud->steptimer != INVALID_TIMER) {
					//Execute step timer on next step instead
					delete_timer(ud->steptimer, unit_step_timer);
					ud->steptimer = INVALID_TIMER;
				}
				return 1;
			}
		}
	}

	return 0;
}

/**
 * Delays an xy timer
 * @param tid: Timer ID
 * @param tick: Unused
 * @param id: ID of bl to delay timer on
 * @param data: Data used in timer calls
 * @return 1: Success 0: Fail (No valid bl)
 */
TIMER_FUNC(unit_delay_walktoxy_timer){
	struct block_list *bl = map_id2bl(id);

	if (!bl || bl->prev == nullptr)
		return 0;

	unit_walktoxy(bl, (int16)((data>>16)&0xffff), (int16)(data&0xffff), 0);

	return 1;
}

/**
 * Delays an walk-to-bl timer
 * @param tid: Timer ID
 * @param tick: Unused
 * @param id: ID of bl to delay timer on
 * @param data: Data used in timer calls (target bl)
 * @return 1: Success 0: Fail (No valid bl or target)
 */
TIMER_FUNC(unit_delay_walktobl_timer){
	block_list* bl = map_id2bl( id );
	block_list* tbl = map_id2bl( static_cast<int32>( data ) );

	if(!bl || bl->prev == nullptr || tbl == nullptr)
		return 0;
	else {
		struct unit_data* ud = unit_bl2ud(bl);
		unit_walktobl(bl, tbl, 0, 0);
		ud->target_to = 0;
	}

	return 1;
}

/**
 * Begins the function of walking a unit to an x,y location
 * This is where the path searches and unit can_move checks are done
 * @param bl: Object to send to x,y coordinate
 * @param x: X coordinate where the object will be walking to
 * @param y: Y coordinate where the object will be walking to
 * @param flag: Parameter to decide how to walk
 *	&1: Easy walk (fail if CELL_CHKNOPASS is in direct path)
 *	&2: Force walking (override can_move)
 *	&4: Delay walking for can_move
 *	&8: Search for an unoccupied cell and cancel if none available
 * @return 1: Success 0: Fail or unit_walktoxy_sub()
 */
int32 unit_walktoxy( struct block_list *bl, int16 x, int16 y, unsigned char flag)
{
	nullpo_ret(bl);

	unit_data* ud = unit_bl2ud(bl);

	if (ud == nullptr)
		return 0;

	if ((flag&8) && !map_nearby_freecell(bl->m, x, y, BL_CHAR|BL_NPC, 1)) //This might change x and y
		return 0;

	walkpath_data wpd = { 0 };

	if (!path_search(&wpd, bl->m, bl->x, bl->y, x, y, flag&1, CELL_CHKNOPASS)) // Count walk path cells
		return 0;

	// NPCs do not need to fulfill the following checks
	if( bl->type != BL_NPC ){
		if( wpd.path_len > battle_config.max_walk_path ){
			return 0;
		}

#ifdef OFFICIAL_WALKPATH
		// Official number of walkable cells is 14 if and only if there is an obstacle between.
		if( wpd.path_len > 14 && !path_search_long( nullptr, bl->m, bl->x, bl->y, x, y, CELL_CHKNOPASS ) ){
			return 0;
		}
#endif
	}

	if (flag&4) {
		unit_unattackable(bl);
		unit_stop_attack(bl);

		if(DIFF_TICK(ud->canmove_tick, gettick()) > 0 && DIFF_TICK(ud->canmove_tick, gettick()) < 2000) { // Delay walking command. [Skotlex]
			add_timer(ud->canmove_tick+1, unit_delay_walktoxy_timer, bl->id, (x<<16)|(y&0xFFFF));
			return 1;
		}
	}

	if(!(flag&2) && (!status_bl_has_mode(bl,MD_CANMOVE) || !unit_can_move(bl)))
		return 0;

	ud->state.walk_easy = flag&1;
	ud->to_x = x;
	ud->to_y = y;
	unit_stop_attack(bl); //Sets target to 0

	status_change* sc = status_get_sc(bl);
	if (sc && sc->getSCE(SC_CONFUSION)) // Randomize the target position
		map_random_dir(bl, &ud->to_x, &ud->to_y);

	if(ud->walktimer != INVALID_TIMER) {
		// When you come to the center of the grid because the change of destination while you're walking right now
		// Call a function from a timer unit_walktoxy_sub
		ud->state.change_walk_target = 1;
		return 1;
	}

	TBL_PC *sd = BL_CAST(BL_PC, bl);

	// Start timer to recall summon
	if( sd != nullptr ){
		if (sd->md != nullptr)
			unit_check_start_teleport_timer(sd->md);
		if (sd->ed != nullptr)
			unit_check_start_teleport_timer(sd->ed);
		if (sd->hd != nullptr)
			unit_check_start_teleport_timer(sd->hd);
		if (sd->pd != nullptr)
			unit_check_start_teleport_timer(sd->pd);
	}

	return unit_walktoxy_sub(bl);
}

/**
 * Timer to walking a unit to another unit's location
 * Calls unit_walktoxy_sub once determined the unit can move
 * @param tid: Object's timer ID
 * @param id: Object's ID
 * @param data: Data passed through timer function (target)
 * @return 0
 */
static TIMER_FUNC(unit_walktobl_sub){
	struct block_list *bl = map_id2bl(id);
	struct unit_data *ud = bl?unit_bl2ud(bl):nullptr;

	if (ud != nullptr && ud->walktimer == INVALID_TIMER && ud->target_to != 0 && ud->target_to == data) {
		if (DIFF_TICK(ud->canmove_tick, tick) > 0) // Keep waiting?
			add_timer(ud->canmove_tick+1, unit_walktobl_sub, id, data);
		else if (unit_can_move(bl))
			unit_walktoxy_sub(bl);
	}

	return 0;
}

/**
 * Tells a unit to walk to a target's location (chase)
 * @param bl: Object that is walking to target
 * @param tbl: Target object
 * @param range: How close to get to target (or attack range if flag&2)
 * @param flag: Extra behaviour
 *	&1: Use easy path seek (obstacles will not be walked around)
 *	&2: Start attacking upon arrival within range, otherwise just walk to target
 * @return 1: Started walking or set timer 0: Failed
 */
int32 unit_walktobl(struct block_list *bl, struct block_list *tbl, int32 range, unsigned char flag)
{
	nullpo_ret(bl);
	nullpo_ret(tbl);

	unit_data *ud  = unit_bl2ud(bl);

	if(ud == nullptr)
		return 0;

	if (!status_bl_has_mode(bl,MD_CANMOVE))
		return 0;

	if (!unit_can_reach_bl(bl, tbl, distance_bl(bl, tbl)+1, flag&1, &ud->to_x, &ud->to_y)) {
		ud->to_x = bl->x;
		ud->to_y = bl->y;
		ud->target_to = 0;

		return 0;
	} else if (range == 0) {
		//Should walk on the same cell as target (for looters)
		ud->to_x = tbl->x;
		ud->to_y = tbl->y;
		//Because of the change of target position the easy walkpath could fail
		//Note: Easy walking is no longer used by default, but we keep this to prevent endless loops [Playtester]
		flag &= ~1;
	}

	ud->state.walk_easy = flag&1;
	ud->target_to = tbl->id;
	ud->chaserange = range; // Note that if flag&2, this SHOULD be attack-range
	ud->state.attack_continue = flag&2?1:0; // Chase to attack.
	unit_stop_attack(bl); //Sets target to 0

	status_change *sc = status_get_sc(bl);
	if (sc && sc->getSCE(SC_CONFUSION)) // Randomize the target position
		map_random_dir(bl, &ud->to_x, &ud->to_y);

	if(ud->walktimer != INVALID_TIMER) {
		ud->state.change_walk_target = 1;

		// New target, make sure a monster is still in chase state
		if (bl->type == BL_MOB && ud->state.attack_continue) {
			mob_data& md = reinterpret_cast<mob_data&>(*bl);
			mob_setstate(md, MSS_RUSH);
		}

		return 1;
	}

	if(DIFF_TICK(ud->canmove_tick, gettick()) > 0) { // Can't move, wait a bit before invoking the movement.
		add_timer(ud->canmove_tick+1, unit_walktobl_sub, bl->id, ud->target_to);
		return 1;
	}

	if(!unit_can_move(bl))
		return 0;

	if (unit_walktoxy_sub(bl))
		return 1;

	return 0;
}

/**
 * Called by unit_run when an object is hit.
 * @param sd Required only when using SC_WUGDASH
 */
void unit_run_hit(struct block_list *bl, status_change *sc, map_session_data *sd, enum sc_type type)
{
	int32 lv = sc->getSCE(type)->val1;

	// If you can't run forward, you must be next to a wall, so bounce back. [Skotlex]
	if (type == SC_RUN)
		clif_status_change(bl, EFST_TING, 1, 0, 0, 0, 0);

	// Set running to 0 beforehand so status_change_end knows not to enable spurt [Kevin]
	unit_bl2ud(bl)->state.running = 0;
	status_change_end(bl, type);

	if (type == SC_RUN) {
		skill_blown(bl, bl, skill_get_blewcount(TK_RUN, lv), unit_getdir(bl), BLOWN_NONE);
		clif_status_change(bl, EFST_TING, 0, 0, 0, 0, 0);
	} else if (sd) {
		clif_fixpos( *bl );
		skill_castend_damage_id(bl, sd, RA_WUGDASH, lv, gettick(), SD_LEVEL);
	}
	return;
}

/**
 * Set a unit to run, checking for obstacles
 * @param bl: Object that is running
 * @param sd: Required only when using SC_WUGDASH
 * @return true: Success (Finished running) false: Fail (Hit an object/Couldn't run)
 */
bool unit_run(struct block_list *bl, map_session_data *sd, enum sc_type type)
{
	status_change *sc;
	int16 to_x, to_y, dir_x, dir_y;
	int32 i;

	nullpo_retr(false, bl);

	sc = status_get_sc(bl);

	if (!(sc && sc->getSCE(type)))
		return false;

	if (!unit_can_move(bl)) {
		status_change_end(bl, type);
		return false;
	}

	dir_x = dirx[sc->getSCE(type)->val2];
	dir_y = diry[sc->getSCE(type)->val2];

	// Determine destination cell
	to_x = bl->x;
	to_y = bl->y;

	// Search for available path
	for(i = 0; i < AREA_SIZE; i++) {
		if(!map_getcell(bl->m, to_x + dir_x, to_y + dir_y, CELL_CHKPASS))
			break;

		// If sprinting and there's a PC/Mob/NPC, block the path [Kevin]
		if(map_count_oncell(bl->m, to_x + dir_x, to_y + dir_y, BL_PC|BL_MOB|BL_NPC, 0))
			break;

		to_x += dir_x;
		to_y += dir_y;
	}

	// Can't run forward.
	if( (to_x == bl->x && to_y == bl->y) || (to_x == (bl->x + 1) || to_y == (bl->y + 1)) || (to_x == (bl->x - 1) || to_y == (bl->y - 1))) {
		unit_run_hit(bl, sc, sd, type);
		return false;
	}

	if (unit_walktoxy(bl, to_x, to_y, 1))
		return true;

	// There must be an obstacle nearby. Attempt walking one cell at a time.
	do {
		to_x -= dir_x;
		to_y -= dir_y;
	} while (--i > 0 && !unit_walktoxy(bl, to_x, to_y, 1));

	if (i == 0) {
		unit_run_hit(bl, sc, sd, type);
		return false;
	}

	return true;
}

/**
 * Returns duration of an object's current walkpath
 * @param bl: Object that is moving
 * @return Duration of the walkpath
 */
t_tick unit_get_walkpath_time(struct block_list& bl)
{
	t_tick time = 0;
	uint16 speed = status_get_speed(&bl);
	struct unit_data* ud = unit_bl2ud(&bl);

	// The next walk start time is calculated.
	for (uint8 i = 0; i < ud->walkpath.path_len; i++) {
		if (direction_diagonal(ud->walkpath.path[i]))
			time += speed * MOVE_DIAGONAL_COST / MOVE_COST;
		else
			time += speed;
	}

	return time;
}

/**
 * Makes unit attempt to run away from target in a straight line or using hard paths
 * @param bl: Object that is running away from target
 * @param target: Target
 * @param dist: How far bl should run
 * @param flag: unit_walktoxy flag (&1 = straight line escape)
 * @return The duration the unit will run (0 on fail)
 */
t_tick unit_escape(struct block_list *bl, struct block_list *target, int16 dist, uint8 flag)
{
	uint8 dir = map_calc_dir(target, bl->x, bl->y);

	if (flag&1) {
		// Straight line escape
		// Keep moving until we hit an unreachable cell
		for (int16 i = 1; i <= dist; i++) {
			if (map_getcell(bl->m, bl->x + i*dirx[dir], bl->y + i*diry[dir], CELL_CHKNOREACH))
				dist = i - 1;
		}
	} else {
		// Find the furthest reachable cell (then find a walkpath to it)
		while( dist > 0 && map_getcell(bl->m, bl->x + dist*dirx[dir], bl->y + dist*diry[dir], CELL_CHKNOREACH) )
			dist--;
	}

	if (dist > 0 && unit_walktoxy(bl, bl->x + dist * dirx[dir], bl->y + dist * diry[dir], flag))
		return unit_get_walkpath_time(*bl);

	return 0;
}

/**
 * Instant warps a unit to x,y coordinate
 * @param bl: Object to instant warp
 * @param dst_x: X coordinate to warp to
 * @param dst_y: Y coordinate to warp to
 * @param easy: 
 *		0: Hard path check (attempt to go around obstacle)
 *		1: Easy path check (no obstacle on movement path)
 *		2: Long path check (no obstacle on line from start to destination)
 * @param checkpath: Whether or not to do a cell and path check for NOPASS and NOREACH
 * @return True: Success False: Fail
 */
bool unit_movepos(struct block_list *bl, int16 dst_x, int16 dst_y, int32 easy, bool checkpath)
{
	int16 dx,dy;
	struct unit_data        *ud = nullptr;
	map_session_data *sd = nullptr;

	nullpo_retr(false,bl);

	sd = BL_CAST(BL_PC, bl);
	ud = unit_bl2ud(bl);

	if(ud == nullptr)
		return false;

	unit_stop_walking( bl, USW_FIXPOS );
	unit_stop_attack(bl);

	if( checkpath && (map_getcell(bl->m,dst_x,dst_y,CELL_CHKNOPASS) || !path_search(nullptr,bl->m,bl->x,bl->y,dst_x,dst_y,easy,CELL_CHKNOREACH)) )
		return false; // Unreachable

	ud->to_x = dst_x;
	ud->to_y = dst_y;

	unit_setdir(bl, map_calc_dir(bl, dst_x, dst_y), false);

	dx = dst_x - bl->x;
	dy = dst_y - bl->y;

	map_foreachinmovearea(clif_outsight, bl, AREA_SIZE, dx, dy, (sd ? BL_ALL : BL_PC), bl);

	map_moveblock(bl, dst_x, dst_y, gettick());

	ud->walktimer = CLIF_WALK_TIMER; // Arbitrary non-INVALID_TIMER value to make the clif code send walking packets
	map_foreachinmovearea(clif_insight, bl, AREA_SIZE, -dx, -dy, (sd ? BL_ALL : BL_PC), bl);
	ud->walktimer = INVALID_TIMER;

	if(sd) {
		if( !sd->npc_ontouch_.empty() )
			npc_touchnext_areanpc(sd,false);

		if(map_getcell(bl->m,bl->x,bl->y,CELL_CHKNPC)) {
			npc_touch_area_allnpc(sd, bl->m, bl->x, bl->y);

			if (bl->prev == nullptr) // Script could have warped char, abort remaining of the function.
				return false;
		} else
			sd->areanpc.clear();

		if( sd->status.pet_id > 0 && sd->pd && sd->pd->pet.intimate > PET_INTIMATE_NONE ) {
			// Check if pet needs to be teleported. [Skotlex]
			int32 flag = 0;
			struct block_list* pbl = sd->pd;

			if( !checkpath && !path_search(nullptr,pbl->m,pbl->x,pbl->y,dst_x,dst_y,0,CELL_CHKNOPASS) )
				flag = 1;
			else if (!check_distance_bl(sd, pbl, AREA_SIZE)) // Too far, teleport.
				flag = 2;

			if( flag ) {
				unit_movepos(pbl,sd->x,sd->y, 0, 0);
				clif_slide(*pbl,pbl->x,pbl->y);
			}
		}
	}

	return true;
}

/**
 * Sets direction of a unit
 * @param bl: Object to set direction
 * @param dir: Direction (0-7)
 * @param send_update: Update the client area of direction (default: true)
 * @return True on success or False on failure
 */
bool unit_setdir(block_list *bl, uint8 dir, bool send_update)
{
	nullpo_ret(bl);

	unit_data *ud = unit_bl2ud(bl);

	if (ud == nullptr)
		return false;

	ud->dir = dir;

	if (bl->type == BL_PC) {
		map_session_data *sd = BL_CAST(BL_PC, bl);

		sd->head_dir = DIR_NORTH;
		sd->status.body_direction = ud->dir;
	}

	if (send_update)
		clif_changed_dir(*bl, AREA);

	return true;
}

/**
 * Gets direction of a unit
 * @param bl: Object to get direction
 * @return direction (0-7)
 */
uint8 unit_getdir(struct block_list *bl)
{
	struct unit_data *ud;

	nullpo_ret(bl);

	ud = unit_bl2ud(bl);

	if (!ud)
		return 0;

	return ud->dir;
}

/**
 * Pushes a unit in a direction by a given amount of cells
 * There is no path check, only map cell restrictions are respected
 * @param bl: Object to push
 * @param dx: Destination cell X
 * @param dy: Destination cell Y
 * @param count: How many cells to push bl
 * @param flag: See skill.hpp::e_skill_blown
 * @return count (can be modified due to map cell restrictions)
 */
int32 unit_blown(struct block_list* bl, int32 dx, int32 dy, int32 count, enum e_skill_blown flag)
{
	if(count) {
		map_session_data* sd;
		struct skill_unit* su = nullptr;
		int32 nx, ny, result;

		sd = BL_CAST(BL_PC, bl);
		su = BL_CAST(BL_SKILL, bl);

		result = path_blownpos(bl->m, bl->x, bl->y, dx, dy, count);

		nx = result>>16;
		ny = result&0xffff;

		if(!su)
			unit_stop_walking( bl, USW_NONE );

		if( sd ) {
			unit_stop_stepaction(bl); //Stop stepaction when knocked back
			sd->ud.to_x = nx;
			sd->ud.to_y = ny;
		}

		dx = nx-bl->x;
		dy = ny-bl->y;

		if(dx || dy) {
			map_foreachinmovearea(clif_outsight, bl, AREA_SIZE, dx, dy, bl->type == BL_PC ? BL_ALL : BL_PC, bl);

			if(su) {
				if (su->group && skill_get_unit_flag(su->group->skill_id, UF_KNOCKBACKGROUP))
					skill_unit_move_unit_group(su->group, bl->m, dx, dy);
				else
					skill_unit_move_unit(bl, nx, ny);
			} else
				map_moveblock(bl, nx, ny, gettick());

			map_foreachinmovearea(clif_insight, bl, AREA_SIZE, -dx, -dy, bl->type == BL_PC ? BL_ALL : BL_PC, bl);

			if(!(flag&BLOWN_DONT_SEND_PACKET))
				clif_blown(bl);

			if(sd) {
				if(!sd->npc_ontouch_.empty())
					npc_touchnext_areanpc(sd, false);

				if(map_getcell(bl->m, bl->x, bl->y, CELL_CHKNPC))
					npc_touch_area_allnpc(sd, bl->m, bl->x, bl->y);
				else
					sd->areanpc.clear();
			}
		}

		count = distance(dx, dy);
	}

	return count;  // Return amount of knocked back cells
}

/**
 * Checks if unit can be knocked back / stopped by skills.
 * @param bl: Object to check
 * @param flag
 *		0x1 - Offensive (not set: self skill, e.g. Backslide)
 *		0x2 - Knockback type (not set: Stop type, e.g. Ankle Snare)
 *		0x4 - Boss attack
 *		0x8 - Ignore target player 'special_state.no_knockback'
 * @return reason for immunity
 *		UB_KNOCKABLE - can be knocked back / stopped
 *		UB_NO_KNOCKBACK_MAP - at WOE/BG map
 *		UB_MD_KNOCKBACK_IMMUNE - target is MD_KNOCKBACK_IMMUNE
 *		UB_TARGET_BASILICA - target is in Basilica area (Pre-Renewal)
 *		UB_TARGET_NO_KNOCKBACK - target has 'special_state.no_knockback'
 *		UB_TARGET_TRAP - target is trap that cannot be knocked back
 */
enum e_unit_blown unit_blown_immune(struct block_list* bl, uint8 flag)
{
	if ((flag&0x1)
		&& (map_flag_gvg2(bl->m) || map_getmapflag(bl->m, MF_BATTLEGROUND))
		&& ((flag&0x2) || !(battle_config.skill_trap_type&0x1)))
		return UB_NO_KNOCKBACK_MAP; // No knocking back in WoE / BG

	switch (bl->type) {
		case BL_MOB:
			// Immune can't be knocked back
			if (((flag&0x1) && status_bl_has_mode(bl,MD_KNOCKBACKIMMUNE))
				&& ((flag&0x2) || !(battle_config.skill_trap_type&0x2)))
				return UB_MD_KNOCKBACK_IMMUNE;
			break;
		case BL_PC: {
				map_session_data *sd = BL_CAST(BL_PC, bl);

#ifndef RENEWAL
				// Basilica caster can't be knocked-back by normal monsters.
				if( !(flag&0x4) && sd->sc.getSCE(SC_BASILICA) && sd->sc.getSCE(SC_BASILICA)->val4 == sd->id)
					return UB_TARGET_BASILICA;
#endif
				// Target has special_state.no_knockback (equip)
				if( (flag&(0x1|0x2)) && !(flag&0x8) && sd->special_state.no_knockback )
					return UB_TARGET_NO_KNOCKBACK;
			}
			break;
		case BL_SKILL: {
				struct skill_unit* su = (struct skill_unit *)bl;
				// Trap cannot be knocked back
				if (su && su->group && skill_get_unit_flag(su->group->skill_id, UF_NOKNOCKBACK))
					return UB_TARGET_TRAP;
			}
			break;
	}

	//Object can be knocked back / stopped
	return UB_KNOCKABLE;
}

/**
 * Warps a unit to a map/position
 * pc_setpos is used for player warping
 * This function checks for "no warp" map flags, so it's safe to call without doing nowarpto/nowarp checks
 * @param bl: Object to warp
 * @param m: Map ID from bl structure (NOT index)
 * @param x: Destination cell X
 * @param y: Destination cell Y
 * @param type: Clear type used in clif_clearunit_area()
 * @return Success(0); Failed(1); Error(2); unit_remove_map() Failed(3); map_addblock Failed(4)
 */
int32 unit_warp(struct block_list *bl,int16 m,int16 x,int16 y,clr_type type)
{
	struct unit_data *ud;

	nullpo_ret(bl);

	ud = unit_bl2ud(bl);

	if(bl->prev==nullptr || !ud)
		return 1;

	if (type == CLR_DEAD)
		// Type 1 is invalid, since you shouldn't warp a bl with the "death"
		// animation, it messes up with unit_remove_map! [Skotlex]
		return 1;

	if( m < 0 )
		m = bl->m;

	switch (bl->type) {
		case BL_MOB:
			if (map_getmapflag(bl->m, MF_MONSTER_NOTELEPORT) && ((TBL_MOB*)bl)->master_id == 0)
				return 1;

			if (m != bl->m && map_getmapflag(m, MF_NOBRANCH) && battle_config.mob_warp&4 && !(((TBL_MOB *)bl)->master_id))
				return 1;
			break;
		case BL_PC:
			if (map_getmapflag(bl->m, MF_NOTELEPORT))
				return 1;
			break;
	}

	if (x < 0 || y < 0) { // Random map position.
		if (!map_search_freecell(nullptr, m, &x, &y, -1, -1, 1)) {
			ShowWarning("unit_warp failed. Unit Id:%d/Type:%d, target position map %d (%s) at [%d,%d]\n", bl->id, bl->type, m, map[m].name, x, y);
			return 2;

		}
	} else if ( bl->type != BL_NPC && map_getcell(m,x,y,CELL_CHKNOREACH)) { // Invalid target cell
		ShowWarning("unit_warp: Specified non-walkable target cell: %d (%s) at [%d,%d]\n", m, map[m].name, x,y);

		if (!map_search_freecell(nullptr, m, &x, &y, 4, 4, 1)) { // Can't find a nearby cell
			ShowWarning("unit_warp failed. Unit Id:%d/Type:%d, target position map %d (%s) at [%d,%d]\n", bl->id, bl->type, m, map[m].name, x, y);
			return 2;
		}
	}

	if (bl->type == BL_PC) // Use pc_setpos
		return pc_setpos((TBL_PC*)bl, map_id2index(m), x, y, type);

	if (!unit_remove_map(bl, type))
		return 3;

	if (bl->m != m && battle_config.clear_unit_onwarp &&
		battle_config.clear_unit_onwarp&bl->type)
		skill_clear_unitgroup(bl);

	bl->x = ud->to_x = x;
	bl->y = ud->to_y = y;
	bl->m = m;

	switch (bl->type) {
		case BL_NPC:
		{
			TBL_NPC* nd = reinterpret_cast<npc_data*>(bl);
			map_addnpc(m, nd);
			npc_setcells(nd);
			break;
		}
		case BL_MOB:
		{
			TBL_MOB* md = reinterpret_cast<mob_data*>(bl);
			// If slaves are set to stick with their master they should drop target if recalled at range
			if (battle_config.slave_stick_with_master && md->target_id != 0) {
				block_list* tbl = map_id2bl(md->target_id);
				if (tbl == nullptr || !check_distance_bl(bl, tbl, AREA_SIZE)) {
					md->target_id = 0;
				}
			}
			break;
		}
	}

	if(map_addblock(bl))
		return 4; //error on adding bl to map

	clif_spawn(bl);
	skill_unit_move(bl,gettick(),1);

	if (!battle_config.slave_stick_with_master && bl->type == BL_MOB && mob_countslave(bl) > 0)
		mob_warpslave(bl,MOB_SLAVEDISTANCE);

	return 0;
}

/**
 * Calculates the exact coordinates of a bl considering the walktimer
 * This is needed because we only update X/Y when finishing movement to the next cell
 * Officially, however, the coordinates update when crossing the border to the next cell
 * The coordinates are stored in unit_data.pos together with the tick based on which they were calculated
 * Access to these coordinates is only allowed through corresponding unit_data class functions
 * This function makes sure calculation only happens when it's needed to save performance
 * @param tick: Tick based on which we calculate the coordinates
 */
void unit_data::update_pos(t_tick tick)
{
	// Check if coordinates are still up-to-date
	if (DIFF_TICK(tick, this->pos.tick) < MIN_POS_INTERVAL)
		return;

	if (this->bl == nullptr)
		return;

	// Set initial coordinates
	this->pos.x = this->bl->x;
	this->pos.y = this->bl->y;
	this->pos.sx = 8;
	this->pos.sy = 8;

	// Remember time at which we did the last calculation
	this->pos.tick = tick;

	if (this->walkpath.path_pos >= this->walkpath.path_len)
		return;

	if (this->walktimer == INVALID_TIMER)
		return;

	const TimerData* td = get_timer(this->walktimer);

	if (td == nullptr)
		return;

	// Get how much percent we traversed on the timer
	double cell_percent = 1.0 - ((double)DIFF_TICK(td->tick, tick) / (double)td->data);

	if (cell_percent > 0.0 && cell_percent < 1.0) {
		// Set subcell coordinates according to timer
		// This gives a value between 8 and 39
		this->pos.sx = static_cast<uint8>(24.0 + dirx[this->walkpath.path[this->walkpath.path_pos]] * 16.0 * cell_percent);
		this->pos.sy = static_cast<uint8>(24.0 + diry[this->walkpath.path[this->walkpath.path_pos]] * 16.0 * cell_percent);
		// 16-31 reflect sub position 0-15 on the current cell
		// 8-15 reflect sub position 8-15 at -1 main coordinate
		// 32-39 reflect sub position 0-7 at +1 main coordinate
		if (this->pos.sx < 16 || this->pos.sy < 16 || this->pos.sx > 31 || this->pos.sy > 31) {
			if (this->pos.sx < 16) this->pos.x--;
			if (this->pos.sy < 16) this->pos.y--;
			if (this->pos.sx > 31) this->pos.x++;
			if (this->pos.sy > 31) this->pos.y++;
		}
		this->pos.sx %= 16;
		this->pos.sy %= 16;
	}
	else if (cell_percent >= 1.0) {
		// Assume exactly one cell moved
		this->pos.x += dirx[this->walkpath.path[this->walkpath.path_pos]];
		this->pos.y += diry[this->walkpath.path[this->walkpath.path_pos]];
	}
}

/**
 * Helper function to get the exact X coordinate
 * This ensures that the coordinate is calculated when needed
 * @param tick: Tick based on which we calculate the coordinate
 * @return The exact X coordinate
 */
int16 unit_data::getx(t_tick tick) {
	// Make sure exact coordinates are up-to-date
	this->update_pos(tick);
	return this->pos.x;
}

/**
 * Helper function to get the exact Y coordinate
 * This ensures that the coordinate is calculated when needed
 * @param tick: Tick based on which we calculate the coordinate
 * @return The exact Y coordinate
 */
int16 unit_data::gety(t_tick tick) {
	// Make sure exact coordinates are up-to-date
	this->update_pos(tick);
	return this->pos.y;
}

/**
 * Helper function to get exact coordinates
 * This ensures that the coordinates are calculated when needed
 * @param x: Will be set to the exact X value of the bl
 * @param y: Will be set to the exact Y value of the bl
 * @param sx: Will be set to the exact subcell X value of the bl
 * @param sy: Will be set to the exact subcell Y value of the bl
 * @param tick: Tick based on which we calculate the coordinate
 * @return The exact Y coordinate
 */
void unit_data::getpos(int16 &x, int16 &y, uint8 &sx, uint8 &sy, t_tick tick) {
	// Make sure exact coordinates are up-to-date
	this->update_pos(tick);
	x = this->pos.x;
	y = this->pos.y;
	sx = this->pos.sx;
	sy = this->pos.sy;
}

/**
 * Updates the walkpath of a unit to end after 0.5-1.5 cells moved
 * Sends required packet for proper display on the client using subcoordinates
 * @param bl: Object to stop walking
 */
void unit_stop_walking_soon(struct block_list& bl, t_tick tick)
{
	struct unit_data* ud = unit_bl2ud(&bl);

	if (ud == nullptr)
		return;

	int16 ox = bl.x, oy = bl.y; // Remember original x and y coordinates
	int16 path_remain = 1; // Remaining path to walk
	bool shortened = false;

	if (ud->walkpath.path_pos + 1 >= ud->walkpath.path_len) {
		// Less than 1 cell left to walk so no need to shorten the path
		// Since we don't need to resend the move packet, we don't need to calculate the exact coordinates
		path_remain = ud->walkpath.path_len - ud->walkpath.path_pos;
	}
	else {
		// Set coordinates to exact coordinates
		ud->getpos(bl.x, bl.y, ud->sx, ud->sy, tick);

		// If x or y already changed, we need to move one more cell
		if (ox != bl.x || oy != bl.y)
			path_remain = 2;

		// Shorten walkpath
		if (ud->walkpath.path_pos + path_remain < ud->walkpath.path_len) {
			ud->walkpath.path_len = ud->walkpath.path_pos + path_remain;
			shortened = true;
		}
	}

	// Make sure to_x and to_y match the walk path even if not shortened in case they were modified
	ud->to_x = ox;
	ud->to_y = oy;
	for (int32 i = 0; i < path_remain; i++) {
		ud->to_x += dirx[ud->walkpath.path[ud->walkpath.path_pos + i]];
		ud->to_y += diry[ud->walkpath.path[ud->walkpath.path_pos + i]];
	}
	// To prevent sending a pointless walk command
	ud->state.change_walk_target = 0;

	// Send movement packet with calculated coordinates and subcoordinates
	// Only need to send if walkpath was shortened
	if (shortened)
		clif_move(*ud);

	// Reset coordinates
	bl.x = ox;
	bl.y = oy;
	ud->sx = 8;
	ud->sy = 8;
}

/**
 * Stops a unit from walking
 * @param bl: Object to stop walking
 * @param type: Options, see e_unit_stop_walking
 * @return Success(true); Failed(false);
 */
bool unit_stop_walking( block_list* bl, int32 type, t_tick canmove_delay ){
	const struct TimerData* td = nullptr;
	t_tick tick;

	if( bl == nullptr ){
		return false;
	}

	unit_data* ud = unit_bl2ud(bl);
	if (ud == nullptr)
		return false;

	// Need to release chase target even if already not walking
	if (type&USW_RELEASE_TARGET)
		ud->target_to = 0;

	if (!(type&USW_FORCE_STOP) && ud->walktimer == INVALID_TIMER)
		return false;

	// NOTE: We are using timer data after deleting it because we know the
	// delete_timer function does not mess with it. If the function's
	// behaviour changes in the future, this code could break!
	if (ud->walktimer != INVALID_TIMER) {
		td = get_timer(ud->walktimer);
		delete_timer(ud->walktimer, unit_walktoxy_timer);
		ud->walktimer = INVALID_TIMER;
	}
	ud->state.change_walk_target = 0;
	tick = gettick();

	if( (type&USW_MOVE_ONCE && !ud->walkpath.path_pos) // Force moving at least one cell.
	||  (type&USW_MOVE_FULL_CELL && td && DIFF_TICK(td->tick, tick) <= td->data/2) // Enough time has passed to cover half-cell
	) {
		ud->walkpath.path_len = ud->walkpath.path_pos+1;
		unit_walktoxy_timer(INVALID_TIMER, tick, bl->id, ud->walkpath.path_pos);
	}

	if (type&USW_FIXPOS) {
		// Stop on cell center
		ud->sx = 8;
		ud->sy = 8;
		clif_fixpos(*bl);
	}

	ud->walkpath.path_len = 0;
	ud->walkpath.path_pos = 0;
	ud->to_x = bl->x;
	ud->to_y = bl->y;

	if( canmove_delay > 0 ){
		ud->canmove_tick = gettick() + canmove_delay;
	}

	// Re-added, the check in unit_set_walkdelay means dmg during running won't fall through to this place in code [Kevin]
	if (ud->state.running) {
		status_change_end(bl, SC_RUN);
		status_change_end(bl, SC_WUGDASH);
	}

	return true;
}

/**
 * Initiates a skill use by a unit
 * @param src: Source object initiating skill use
 * @param target_id: Target ID (bl->id)
 * @param skill_id: Skill ID
 * @param skill_lv: Skill Level
 * @return unit_skilluse_id2()
 */
int32 unit_skilluse_id(struct block_list *src, int32 target_id, uint16 skill_id, uint16 skill_lv)
{
	return unit_skilluse_id2(
		src, target_id, skill_id, skill_lv,
		skill_castfix(src, skill_id, skill_lv),
		skill_get_castcancel(skill_id)
	);
}

/**
 * Checks if a unit is walking
 * @param bl: Object to check walk status
 * @return Walking(1); Not Walking(0)
 */
int32 unit_is_walking(struct block_list *bl)
{
	struct unit_data *ud = unit_bl2ud(bl);

	nullpo_ret(bl);

	if(!ud)
		return 0;

	return (ud->walktimer != INVALID_TIMER);
}

/** 
 * Checks if a unit is able to move based on status changes
 * View the StatusChangeStateTable in status.cpp for a list of statuses
 * Some statuses are still checked here due too specific variables
 * @author [Skotlex]
 * @param bl: Object to check
 * @return True - can move; False - can't move
 */
bool unit_can_move(struct block_list *bl) {
	map_session_data *sd;
	struct unit_data *ud;
	status_change *sc;

	nullpo_ret(bl);

	ud = unit_bl2ud(bl);
	sc = status_get_sc(bl);
	sd = BL_CAST(BL_PC, bl);

	if (!ud)
		return false;

	if (ud->skilltimer != INVALID_TIMER && ud->skill_id != LG_EXEEDBREAK && (!sd || !pc_checkskill(sd, SA_FREECAST) || skill_get_inf2(ud->skill_id, INF2_ISGUILD)))
		return false; // Prevent moving while casting

	if (DIFF_TICK(ud->canmove_tick, gettick()) > 0)
		return false;

	if ((sd && (pc_issit(sd) || sd->state.vending || sd->state.buyingstore || (sd->state.block_action & PCBLOCK_MOVE) || sd->state.mail_writing)) || ud->state.blockedmove)
		return false; // Can't move

	// Status changes that block movement
	if (sc && sc->cant.move)
		return false;

	// Icewall walk block special trapped monster mode
	if(bl->type == BL_MOB) {
		struct mob_data *md = BL_CAST(BL_MOB, bl);
		if(md && ((status_has_mode(&md->status,MD_STATUSIMMUNE) && battle_config.boss_icewall_walk_block == 1 && map_getcell(bl->m,bl->x,bl->y,CELL_CHKICEWALL))
			|| (!status_has_mode(&md->status,MD_STATUSIMMUNE) && battle_config.mob_icewall_walk_block == 1 && map_getcell(bl->m,bl->x,bl->y,CELL_CHKICEWALL)))) {
			md->walktoxy_fail_count = 1; //Make sure rudeattacked skills are invoked
			return false;
		}
	}

	return true;
}

/**
 * Resumes running (RA_WUGDASH or TK_RUN) after a walk delay
 * @param tid: Timer ID
 * @param id: Object ID
 * @param data: Data passed through timer function (unit_data)
 * @return 0
 */
TIMER_FUNC(unit_resume_running){
	struct unit_data *ud = (struct unit_data *)data;
	TBL_PC *sd = map_id2sd(id);

	if (sd && pc_isridingwug(sd))
		clif_skill_nodamage(ud->bl,*ud->bl,RA_WUGDASH,ud->skill_lv,sc_start4(ud->bl,ud->bl,SC_WUGDASH,100,ud->skill_lv,unit_getdir(ud->bl),0,0,0));
	else
		clif_skill_nodamage(ud->bl,*ud->bl,TK_RUN,ud->skill_lv,sc_start4(ud->bl,ud->bl,SC_RUN,100,ud->skill_lv,unit_getdir(ud->bl),0,0,0));

	if (sd)
		clif_walkok(*sd);

	return 0;
}

/**
 * Sets the delays that prevent attacks and skill usage considering the bl type
 * Makes sure that delays are not decreased in case they are already higher
 * Will also invoke bl type specific delay functions when required
 * @param bl Object to apply attack delay to
 * @param tick Current tick
 * @param event The event that resulted in calling this function
 */
void unit_set_attackdelay(block_list& bl, t_tick tick, e_delay_event event)
{
	unit_data* ud = unit_bl2ud(&bl);

	if (ud == nullptr)
		return;

	t_tick attack_delay = 0;
	t_tick act_delay = 0;

	switch (bl.type) {
		case BL_PC:
			switch (event) {
				case DELAY_EVENT_CASTBEGIN_ID:
				case DELAY_EVENT_CASTBEGIN_POS:
					if (reinterpret_cast<map_session_data*>(&bl)->skillitem == ud->skill_id) {
						// Skills used from items don't seem to give any attack or act delay
						return;
					}
					[[fallthrough]];
				case DELAY_EVENT_ATTACK:
				case DELAY_EVENT_PARRY:
					// Officially for players it just remembers the last attack time here and applies the delays during the comparison
					// But we pre-calculate the delays instead and store them in attackabletime and canact_tick
					attack_delay = status_get_adelay(&bl);
					// A fixed delay is added here which is equal to the minimum attack motion you can get
					// This ensures that at max ASPD attackabletime and canact_tick are equal
					act_delay = status_get_amotion(&bl) + (pc_maxaspd(reinterpret_cast<map_session_data*>(&bl)) / AMOTION_DIVIDER_PC);
					break;
			}
			break;
		case BL_MOB:
			switch (event) {
				case DELAY_EVENT_ATTACK:
				case DELAY_EVENT_CASTEND:
				case DELAY_EVENT_CASTCANCEL:
					// This represents setting of attack delay (recharge time) that happens for non-PCs
					attack_delay = status_get_adelay(&bl);
					break;
				case DELAY_EVENT_CASTBEGIN_ID:
				case DELAY_EVENT_CASTBEGIN_POS:
					// When monsters use skills, they only get delays on cast end and cast cancel
					break;
			}
			// Set monster-specific delays (inactive AI time, monster skill delays)
			mob_set_delay(reinterpret_cast<mob_data&>(bl), tick, event);
			break;
		case BL_HOM:
			switch (event) {
				case DELAY_EVENT_ATTACK:
					// This represents setting of attack delay (recharge time) that happens for non-PCs
					attack_delay = status_get_adelay(&bl);
					break;
				case DELAY_EVENT_CASTBEGIN_ID:
				case DELAY_EVENT_CASTBEGIN_POS:
					// For non-PCs that can be controlled from the client, there is a security delay of 200ms
					// However to prevent tricks to use skills faster, we have a config to use amotion instead
					if (battle_config.amotion_min_skill_delay == 1)
						act_delay = status_get_amotion(&bl) + MAX_ASPD_NOPC;
					else
						act_delay = MIN_DELAY_SLAVE;
					break;
			}
			break;
		case BL_MER:
			switch (event) {
				case DELAY_EVENT_ATTACK:
					// This represents setting of attack delay (recharge time) that happens for non-PCs
					attack_delay = status_get_adelay(&bl);
					break;
				case DELAY_EVENT_CASTBEGIN_ID:
					// For non-PCs that can be controlled from the client, there is a security delay of 200ms
					// However to prevent tricks to use skills faster, we have a config to use amotion instead
					if (battle_config.amotion_min_skill_delay == 1)
						act_delay = status_get_amotion(&bl) + MAX_ASPD_NOPC;
					else
						act_delay = MIN_DELAY_SLAVE;
					break;
				case DELAY_EVENT_CASTBEGIN_POS:
					// For ground skills, mercenaries work similar to players
					attack_delay = status_get_adelay(&bl);
					act_delay = status_get_amotion(&bl) + MAX_ASPD_NOPC;
					break;
			}
			break;
		default:
			// Fallback to original behavior as unit type is not fully integrated yet
			switch (event) {
				case DELAY_EVENT_ATTACK:
					attack_delay = status_get_adelay(&bl);
					break;
				case DELAY_EVENT_CASTBEGIN_ID:
				case DELAY_EVENT_CASTBEGIN_POS:
					act_delay = status_get_amotion(&bl);
					break;
			}
			break;
	}

	// When setting delays, we need to make sure not to decrease them in case they've been set by another source already
	if (attack_delay > 0)
		ud->attackabletime = i64max(tick + attack_delay, ud->attackabletime);
	if (act_delay > 0)
		ud->canact_tick = i64max(tick + act_delay, ud->canact_tick);
}

/**
 * Updates skill delays according to cast time and minimum delay, and applies security casttime
 * @param bl Object to apply update delay for
 * @param tick Current tick
 * @param event The event that resulted in calling this function
 */
void unit_set_castdelay(unit_data& ud, t_tick tick, int32 casttime) {
	// Use casttime or minimum delay, whatever is longer
	t_tick cast_delay = i64max(casttime, battle_config.min_skill_delay_limit);

	// Only apply the cast delay, if it is longer than the act delay (set by unit_set_attackdelay)
	ud.canact_tick = i64max(ud.canact_tick, tick + cast_delay);

	// Security delay that will be removed at castend again
	ud.canact_tick += SECURITY_CASTTIME;
}

/**
 * Applies a walk delay to a unit
 * @param bl: Object to apply walk delay to
 * @param tick: Current tick
 * @param delay: Amount of time to set walk delay
 * @param type: Type of delay
 *	0: Damage induced delay; Do not change previous delay
 *	1: Skill induced delay; Walk delay can only be increased, not decreased
 * @param skill_id: ID of skill that dealt damage (type 0 only)
 * @return Success(1); Fail(0);
 */
int32 unit_set_walkdelay(struct block_list *bl, t_tick tick, t_tick delay, int32 type, uint16 skill_id)
{
	struct unit_data *ud = unit_bl2ud(bl);

	if (delay <= 0 || !ud)
		return 0;

	if (type) {
		//Bosses can ignore skill induced walkdelay (but not damage induced)
		if(bl->type == BL_MOB && status_has_mode(status_get_status_data(*bl),MD_STATUSIMMUNE))
			return 0;
		//Make sure walk delay is not decreased
		if (DIFF_TICK(ud->canmove_tick, tick+delay) > 0)
			return 0;
	} else {
		if (bl->type == BL_MOB) {
			mob_data& md = *reinterpret_cast<mob_data*>(bl);

			// Mob needs to escape, don't stop it
			if (md.state.can_escape == 1)
				return 0;
		}
		// Trapped or legacy walk delay system disabled
		if (!unit_can_move(bl) || !(bl->type&battle_config.damage_walk_delay)) {
			// Stop on the closest cell center
			unit_stop_walking( bl, USW_MOVE_FULL_CELL );
			return 0;
		}

		switch (skill_id) {
			case MG_FIREWALL:
			case PR_SANCTUARY:
			case NJ_KAENSIN:
				// When using legacy walk delay, these skills should just stop the target
				delay = 1;
				break;
		}

		//Immune to being stopped for double the flinch time
		if (DIFF_TICK(ud->canmove_tick, tick-delay) > 0)
			return 0;
	}

	ud->canmove_tick = tick + delay;

	if (ud->walktimer != INVALID_TIMER) { // Stop walking, if chasing, readjust timers.
		if (delay == 1) // Minimal delay (walk-delay) disabled. Just stop walking.
			unit_stop_walking( bl, USW_NONE );
		else {
			// Resume running after can move again [Kevin]
			if(ud->state.running)
				add_timer(ud->canmove_tick, unit_resume_running, bl->id, (intptr_t)ud);
			else {
				unit_stop_walking( bl, USW_MOVE_FULL_CELL );

				if(ud->target_to != 0)
					add_timer(ud->canmove_tick+1, unit_walktobl_sub, bl->id, ud->target_to);
			}
		}
	}

	return 1;
}

/**
 * Performs checks for a unit using a skill and executes after cast time completion
 * @param src: Object using skill
 * @param target_id: Target ID (bl->id)
 * @param skill_id: Skill ID
 * @param skill_lv: Skill Level
 * @param casttime: Initial cast time before cast time reductions
 * @param castcancel: Whether or not the skill can be cancelled by interruption (hit)
 * @return Success(1); Fail(0);
 */
int32 unit_skilluse_id2(struct block_list *src, int32 target_id, uint16 skill_id, uint16 skill_lv, int32 casttime, int32 castcancel, bool ignore_range)
{
	struct unit_data *ud;
	status_change *sc;
	map_session_data *sd = nullptr;
	struct block_list * target = nullptr;
	t_tick tick = gettick();
	int32 combo = 0, range;

	nullpo_ret(src);

	if(status_isdead(*src))
		return 0; // Do not continue source is dead

	sd = BL_CAST(BL_PC, src);
	ud = unit_bl2ud(src);

	if(ud == nullptr)
		return 0;

	if (ud && ud->state.blockedskill)
		return 0;

	sc = status_get_sc(src);

	if (sc != nullptr && sc->empty())
		sc = nullptr; // Unneeded

	int32 inf = skill_get_inf(skill_id);
	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	if (!skill)
		return 0;

	// temp: used to signal combo-skills right now.
	if (sc && sc->getSCE(SC_COMBO) &&
		skill_is_combo(skill_id) &&
		(sc->getSCE(SC_COMBO)->val1 == skill_id ||
		(sd?skill_check_condition_castbegin(*sd,skill_id,skill_lv):0) )) {
		if (skill_is_combo(skill_id) == 2 && target_id == src->id && ud->target > 0)
			target_id = ud->target;
		else if (sc->getSCE(SC_COMBO)->val2)
			target_id = sc->getSCE(SC_COMBO)->val2;
		else if (target_id == src->id || ud->target > 0)
			target_id = ud->target;

		if (inf&INF_SELF_SKILL && skill->nk[NK_NODAMAGE])// exploit fix
			target_id = src->id;

		combo = 1;
	} else if (target_id == src->id && inf&INF_SELF_SKILL && skill->inf2[INF2_NOTARGETSELF]) {
		target_id = ud->target; // Auto-select target. [Skotlex]
		combo = 1;
	}

	if (sd) {
		// Target_id checking.
		if(skill_isNotOk(skill_id, *sd))
			return 0;

		switch(skill_id) { // Check for skills that auto-select target
			case MO_CHAINCOMBO:
				if (sc && sc->getSCE(SC_BLADESTOP)) {
					if ((target=map_id2bl(sc->getSCE(SC_BLADESTOP)->val4)) == nullptr)
						return 0;
				}
				break;
			case GC_WEAPONCRUSH:
				if (sc && sc->getSCE(SC_WEAPONBLOCK_ON)) {
					if ((target = map_id2bl(sc->getSCE(SC_WEAPONBLOCK_ON)->val1)) == nullptr)
						return 0;
					combo = 1;
				}
				break;
			case RL_QD_SHOT:
				if (sc && sc->getSCE(SC_QD_SHOT_READY)) {
					if ((target = map_id2bl(sc->getSCE(SC_QD_SHOT_READY)->val1)) == nullptr)
						return 0;
					combo = 1;
				}
				break;
			case WE_MALE:
			case WE_FEMALE:
				if (!sd->status.partner_id)
					return 0;

				target = (struct block_list*)map_charid2sd(sd->status.partner_id);

				if (!target) {
					clif_skill_fail( *sd, skill_id );
					return 0;
				}
				break;
		}

		if (target)
			target_id = target->id;
	} else if (src->type == BL_HOM) {
		switch(skill_id) { // Homun-auto-target skills.
			case HLIF_HEAL:
			case HLIF_AVOID:
			case HAMI_DEFENCE:
			case HAMI_CASTLE:
				target = battle_get_master(src);

				if (!target)
					return 0;

				target_id = target->id;
				break;
			case MH_SONIC_CRAW:
			case MH_TINDER_BREAKER: {
				int32 skill_id2 = ((skill_id==MH_SONIC_CRAW)?MH_MIDNIGHT_FRENZY:MH_EQC);

				if(sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == skill_id2) { // It's a combo
					target_id = sc->getSCE(SC_COMBO)->val2;
					combo = 1;
					casttime = -1;
				}
				break;
			}
		}
	}

	if( !target ) // Choose default target
		target = map_id2bl(target_id);

	if( !target || src->m != target->m || !src->prev || !target->prev )
		return 0;

	if( battle_config.ksprotection && sd && mob_ksprotected(src, target) )
		return 0;

	// Normally not needed because clif.cpp checks for it, but the at/char/script commands don't! [Skotlex]
	if(ud->skilltimer != INVALID_TIMER && skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST)
		return 0;

	if(skill->inf2[INF2_NOTARGETSELF] && src->id == target_id)
		return 0;

	if(!status_check_skilluse(src, target, skill_id, 0))
		return 0;

	// Fail if the targetted skill is near NPC [Cydh]
	if(skill->inf2[INF2_DISABLENEARNPC] && !ignore_range && skill_isNotOk_npcRange(src,skill_id,skill_lv,target->x,target->y)) {
		if (sd)
			clif_skill_fail( *sd, skill_id );

		return 0;
	}

	status_data* tstatus = status_get_status_data(*target);

	// Record the status of the previous skill)
	if(sd) {
		switch(skill_id) {
			case SA_CASTCANCEL:
				if(ud->skill_id != skill_id) {
					sd->skill_id_old = ud->skill_id;
					sd->skill_lv_old = ud->skill_lv;
				}
				break;
			case BD_ENCORE:
				// Prevent using the dance skill if you no longer have the skill in your tree.
				if(!sd->skill_id_dance || pc_checkskill(sd,sd->skill_id_dance)<=0) {
					clif_skill_fail( *sd, skill_id );
					return 0;
				}

				sd->skill_id_old = skill_id;
				break;
			case BA_PANGVOICE:
			case DC_WINKCHARM:
				if (status_get_class_(target) == CLASS_BOSS) {
					clif_skill_fail(*sd, skill_id, USESKILL_FAIL_TOTARGET);
					return 0;
				}
				break;
			case WL_WHITEIMPRISON:
				if( battle_check_target(src,target,BCT_SELF|BCT_ENEMY) < 0 ) {
					clif_skill_fail( *sd, skill_id, USESKILL_FAIL_TOTARGET );
					return 0;
				}
				break;
			case MG_FIREBOLT:
			case MG_LIGHTNINGBOLT:
			case MG_COLDBOLT:
				sd->skill_id_old = skill_id;
				sd->skill_lv_old = skill_lv;
				break;
			case CR_DEVOTION:
				if (target->type == BL_PC) {
					uint8 i = 0, count = min(skill_lv, MAX_DEVOTION);

					ARR_FIND(0, count, i, sd->devotion[i] == target_id);
					if (i == count) {
						ARR_FIND(0, count, i, sd->devotion[i] == 0);
						if (i == count) { // No free slots, skill Fail
							clif_skill_fail( *sd, skill_id );
							return 0;
						}
					}
				}
				break;
			case RL_C_MARKER: {
					uint8 i = 0;

					ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, sd->c_marker[i] == target_id);
					if (i == MAX_SKILL_CRIMSON_MARKER) {
						ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, sd->c_marker[i] == 0);
						if (i == MAX_SKILL_CRIMSON_MARKER) { // No free slots, skill Fail
							clif_skill_fail( *sd, skill_id );
							return 0;
						}
					}
				}
				break;
			case DK_SERVANT_W_SIGN: {
					uint8 i = 0, count = min(skill_lv, MAX_SERVANT_SIGN);

					ARR_FIND( 0, count, i, sd->servant_sign[i] == target_id );

					// Already targetted
					if( i < count ){
						break;
					}

					ARR_FIND( 0, count, i, sd->servant_sign[i] == 0 );

					// No free slots
					if( i == count ){
						clif_skill_fail( *sd, skill_id );
						return 0;
					}
				}
				break;
			case TR_RETROSPECTION:
				// Prevent using the song skill if you no longer have the skill in your tree.
				if (!sd->skill_id_song || pc_checkskill(sd, sd->skill_id_song) <= 0) {
					clif_skill_fail( *sd, skill_id );
					return 0;
				}

				sd->skill_id_old = skill_id;
				break;
		}

		if (!skill_check_condition_castbegin(*sd, skill_id, skill_lv))
			return 0;
	}

	if( src->type == BL_MOB ) {
		switch( skill_id ) {
			case NPC_SUMMONSLAVE:
			case NPC_SUMMONMONSTER:
			case NPC_DEATHSUMMON:
			case AL_TELEPORT:
				if( ((TBL_MOB*)src)->master_id && ((TBL_MOB*)src)->special_state.ai )
					return 0;
		}
	}

	range = skill_get_range2(src, skill_id, skill_lv, true); // Skill cast distance from database

	// New action request received, delete previous action request if not executed yet
	if(ud->stepaction || ud->steptimer != INVALID_TIMER)
		unit_stop_stepaction(src);
	// Remember the skill request from the client while walking to the next cell
	if(src->type == BL_PC && ud->walktimer != INVALID_TIMER && (!battle_check_range(src, target, range-1) || ignore_range)) {
		ud->stepaction = true;
		ud->target_to = target_id;
		ud->stepskill_id = skill_id;
		ud->stepskill_lv = skill_lv;
		return 0; // Attacking will be handled by unit_walktoxy_timer in this case
	}

	// Check range when not using skill on yourself or is a combo-skill during attack
	// (these are supposed to always have the same range as your attack)
	if( src->type != BL_NPC && !ignore_range && src->id != target_id && (!combo || ud->attacktimer == INVALID_TIMER) ) {
		if( skill_get_state(ud->skill_id) == ST_MOVE_ENABLE ) {
			if( !unit_can_reach_bl(src, target, range + 1, 1, nullptr, nullptr) )
				return 0; // Walk-path check failed.
		} else if( src->type == BL_MER && skill_id == MA_REMOVETRAP ) {
			if( !battle_check_range(battle_get_master(src), target, range + 1) )
				return 0; // Aegis calc remove trap based on Master position, ignoring mercenary O.O
		} else if( !battle_check_range(src, target, range) )
			return 0; // Arrow-path check failed.
	}

	if (!combo) // Stop attack on non-combo skills [Skotlex]
		unit_stop_attack(src);

	ud->state.skillcastcancel = castcancel;

	// Combo: Used to signal force cast now.
	combo = 0;

	switch(skill_id) {
		case ALL_RESURRECTION:
			if(battle_check_undead(tstatus->race,tstatus->def_ele))
				combo = 1;
			else if (!status_isdead(*target))
				return 0; // Can't cast on non-dead characters.
		break;
#ifndef RENEWAL
		case MO_FINGEROFFENSIVE:
			if(sd)
				casttime += casttime * min(skill_lv, sd->spiritball);
		break;
#endif
		case MO_EXTREMITYFIST:
			if (sc && sc->getSCE(SC_COMBO) &&
			   (sc->getSCE(SC_COMBO)->val1 == MO_COMBOFINISH ||
				sc->getSCE(SC_COMBO)->val1 == CH_CHAINCRUSH))
				casttime = -1;
			combo = 1;
		break;
		case SR_GATEOFHELL:
		case SR_TIGERCANNON:
			if (sc && sc->getSCE(SC_COMBO) &&
			   sc->getSCE(SC_COMBO)->val1 == SR_FALLENEMPIRE)
				casttime = -1;
			combo = 1;
		break;
		case SA_SPELLBREAKER:
			combo = 1;
		break;
#ifndef RENEWAL_CAST
		case ST_CHASEWALK:
			if (sc && sc->getSCE(SC_CHASEWALK))
				casttime = -1;
		break;
#endif
		case TK_RUN:
			if (sc && sc->getSCE(SC_RUN))
				casttime = -1;
		break;
#ifndef RENEWAL
		case HP_BASILICA:
			if( sc && sc->getSCE(SC_BASILICA) )
				casttime = -1; // No Casting time on basilica cancel
		break;
#endif
#ifndef RENEWAL_CAST
		case KN_CHARGEATK:
		{
			int32 k = static_cast<int32>(distance_math_bl(src, target));
			k = cap_value((k - 1) / 3, 0, 2); //Range 0-3: 500ms, Range 4-6: 1000ms, Range 7+: 1500ms
			casttime += casttime * k;
		}
		break;
#endif
		case GD_EMERGENCYCALL: // Emergency Call double cast when the user has learned Leap [Daegaladh]
			if (sd && (pc_checkskill(sd,TK_HIGHJUMP) || pc_checkskill(sd,SU_LOPE) >= 3))
				casttime *= 2;
			break;
		case RA_WUGDASH:
			if (sc && sc->getSCE(SC_WUGDASH))
				casttime = -1;
			break;
		case DK_SERVANT_W_PHANTOM: { // Stops servants from being consumed on unmarked targets.
				status_change *tsc = status_get_sc(target);

				// Only allow to attack if the enemy has a sign mark given by the caster.
				if( tsc == nullptr || tsc->getSCE(SC_SERVANT_SIGN) == nullptr || tsc->getSCE(SC_SERVANT_SIGN)->val1 != src->id ){
					if (sd)
						clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
					return 0;
				}
			}
			break;
		case EL_WIND_SLASH:
		case EL_HURRICANE:
		case EL_TYPOON_MIS:
		case EL_STONE_HAMMER:
		case EL_ROCK_CRUSHER:
		case EL_STONE_RAIN:
		case EL_ICE_NEEDLE:
		case EL_WATER_SCREW:
		case EL_TIDAL_WEAPON:
			if( src->type == BL_ELEM ) {
				sd = BL_CAST(BL_PC, battle_get_master(src));
				if( sd && sd->skill_id_old == SO_EL_ACTION ) {
					casttime = -1;
					sd->skill_id_old = 0;
				}
			}
			break;
	}

	// Moved here to prevent Suffragium from ending if skill fails
#ifndef RENEWAL_CAST
	casttime = skill_castfix_sc(src, casttime, skill_get_castnodex(skill_id));
#else
	casttime = skill_vfcastfix(src, casttime, skill_id, skill_lv);
#endif

	// Need TK_RUN or WUGDASH handler to be done before that, see bugreport:6026
	if(!ud->state.running){
		// Even though this is not how official works but this will do the trick. bugreport:6829
		unit_stop_walking( src, USW_FIXPOS );
	}

	// SC_MAGICPOWER needs to switch states at start of cast
#ifndef RENEWAL
	skill_toggle_magicpower(src, skill_id);
#endif

	// In official this is triggered even if no cast time.
	clif_skillcasting(src, src->id, target_id, 0,0, skill_id, skill_lv, skill_get_ele(skill_id, skill_lv), casttime);

	if (sd != nullptr && target->type == BL_MOB
#ifndef RENEWAL
		&& (casttime > 0 || combo > 0)
#endif
	) {
		TBL_MOB *md = (TBL_MOB*)target;

		mobskill_event(md, src, tick, -1); // Cast targetted skill event.

		if ((status_has_mode(tstatus,MD_CASTSENSORIDLE) || status_has_mode(tstatus,MD_CASTSENSORCHASE)) && battle_check_target(target, src, BCT_ENEMY) > 0 && !ignore_range) {
			switch (md->state.skillstate) {
				case MSS_RUSH:
				case MSS_FOLLOW:
					if (!status_has_mode(tstatus,MD_CASTSENSORCHASE))
						break;

					md->target_id = src->id;
					md->state.aggressive = status_has_mode(tstatus,MD_ANGRY)?1:0;
					break;
				case MSS_IDLE:
				case MSS_WALK:
					if (!status_has_mode(tstatus,MD_CASTSENSORIDLE))
						break;

					md->target_id = src->id;
					md->state.aggressive = status_has_mode(tstatus,MD_ANGRY)?1:0;
					break;
			}
		}
	}

	if( casttime <= 0 )
		ud->state.skillcastcancel = 0;

	if( sd ) {
		switch( skill_id ) {
			case CG_ARROWVULCAN:
				sd->canequip_tick = tick + casttime;
				break;
		}
	}

	ud->skilltarget  = target_id;
	ud->skillx       = 0;
	ud->skilly       = 0;
	ud->skill_id      = skill_id;
	ud->skill_lv      = skill_lv;

	// Set attack and act delays
	// Please note that the call below relies on ud->skill_id being set!
	unit_set_attackdelay(*src, tick, DELAY_EVENT_CASTBEGIN_ID);
	// Apply cast time and general delays
	unit_set_castdelay(*ud, tick, (skill_get_cast(skill_id, skill_lv) != 0) ? casttime : 0);

	if( sc ) {
		// These 3 status do not stack, so it's efficient to use if-else
 		if( sc->getSCE(SC_CLOAKING) && !(sc->getSCE(SC_CLOAKING)->val4&4) && skill_id != AS_CLOAKING && skill_id != SHC_SHADOW_STAB) {
			status_change_end(src, SC_CLOAKING);

			if (!src->prev)
				return 0; // Warped away!
		} else if( sc->getSCE(SC_CLOAKINGEXCEED) && !(sc->getSCE(SC_CLOAKINGEXCEED)->val4&4) && skill_id != GC_CLOAKINGEXCEED && skill_id != SHC_SHADOW_STAB  && skill_id != SHC_SAVAGE_IMPACT ) {
			status_change_end(src,SC_CLOAKINGEXCEED);

			if (!src->prev)
				return 0;
		} else if (sc->getSCE(SC_NEWMOON) && skill_id != SJ_NEWMOONKICK) {
			status_change_end(src, SC_NEWMOON);
			if (!src->prev)
				return 0; // Warped away!
		}
	}


	if( casttime > 0 ) {
		ud->skilltimer = add_timer( tick+casttime, skill_castend_id, src->id, 0 );

		if( sd && (pc_checkskill(sd,SA_FREECAST) > 0 || skill_id == LG_EXEEDBREAK) )
			status_calc_bl(sd, { SCB_SPEED, SCB_ASPD });
	} else
		skill_castend_id(ud->skilltimer,tick,src->id,0);

	if( sd && battle_config.prevent_logout_trigger&PLT_SKILL )
		sd->canlog_tick = gettick();

	return 1;
}

/**
 * Initiates a placement (ground/non-targeted) skill
 * @param src: Object using skill
 * @param skill_x: X coordinate where skill is being casted (center)
 * @param skill_y: Y coordinate where skill is being casted (center)
 * @param skill_id: Skill ID
 * @param skill_lv: Skill Level
 * @return unit_skilluse_pos2()
 */
int32 unit_skilluse_pos(struct block_list *src, int16 skill_x, int16 skill_y, uint16 skill_id, uint16 skill_lv)
{
	return unit_skilluse_pos2(
		src, skill_x, skill_y, skill_id, skill_lv,
		skill_castfix(src, skill_id, skill_lv),
		skill_get_castcancel(skill_id)
	);
}

/**
 * Performs checks for a unit using a skill and executes after cast time completion
 * @param src: Object using skill
 * @param skill_x: X coordinate where skill is being casted (center)
 * @param skill_y: Y coordinate where skill is being casted (center)
 * @param skill_id: Skill ID
 * @param skill_lv: Skill Level
 * @param casttime: Initial cast time before cast time reductions
 * @param castcancel: Whether or not the skill can be cancelled by interuption (hit)
 * @return Success(1); Fail(0);
 */
int32 unit_skilluse_pos2( struct block_list *src, int16 skill_x, int16 skill_y, uint16 skill_id, uint16 skill_lv, int32 casttime, int32 castcancel, bool ignore_range)
{
	map_session_data *sd = nullptr;
	struct unit_data *ud = nullptr;
	status_change *sc;
	struct block_list bl;
	t_tick tick = gettick();
	int32 range;

	nullpo_ret(src);

	if (!src->prev)
		return 0; // Not on the map

	if(status_isdead(*src))
		return 0;

	sd = BL_CAST(BL_PC, src);
	ud = unit_bl2ud(src);

	if(ud == nullptr)
		return 0;

	if (ud && ud->state.blockedskill)
		return 0;

	if(ud->skilltimer != INVALID_TIMER) // Normally not needed since clif.cpp checks for it, but at/char/script commands don't! [Skotlex]
		return 0;

	sc = status_get_sc(src);

	if (sc != nullptr && sc->empty())
		sc = nullptr;

	if (!skill_db.find(skill_id))
		return 0;

	if( sd ) {
		if( skill_isNotOk(skill_id, *sd) || !skill_check_condition_castbegin(*sd, skill_id, skill_lv) )
			return 0;
		if (skill_id == MG_FIREWALL && !skill_pos_maxcount_check(src, skill_x, skill_y, skill_id, skill_lv, BL_PC, true))
			return 0; // Special check for Firewall only
	}

	if( (skill_id >= SC_MANHOLE && skill_id <= SC_FEINTBOMB) && map_getcell(src->m, skill_x, skill_y, CELL_CHKMAELSTROM) ) {
		if (sd)
			clif_skill_fail( *sd, skill_id );
		return 0;
	}

	if (!status_check_skilluse(src, nullptr, skill_id, 0))
		return 0;

	// Fail if the targetted skill is near NPC [Cydh]
	if(skill_get_inf2(skill_id, INF2_DISABLENEARNPC) && !ignore_range && skill_isNotOk_npcRange(src,skill_id,skill_lv,skill_x,skill_y)) {
		if (sd)
			clif_skill_fail( *sd, skill_id );

		return 0;
	}

	// Check range and obstacle
	bl.type = BL_NUL;
	bl.m = src->m;
	bl.x = skill_x;
	bl.y = skill_y;

	if (src->type == BL_NPC) // NPC-objects can override cast distance
		range = AREA_SIZE; // Maximum visible distance before NPC goes out of sight
	else
		range = skill_get_range2(src, skill_id, skill_lv, true); // Skill cast distance from database

	// New action request received, delete previous action request if not executed yet
	if(ud->stepaction || ud->steptimer != INVALID_TIMER)
		unit_stop_stepaction(src);
	// Remember the skill request from the client while walking to the next cell
	if(src->type == BL_PC && ud->walktimer != INVALID_TIMER && (!battle_check_range(src, &bl, range-1) || ignore_range)) {
		struct map_data *md = &map[src->m];
		// Convert coordinates to target_to so we can use it as target later
		ud->stepaction = true;
		ud->target_to = (skill_x + skill_y*md->xs);
		ud->stepskill_id = skill_id;
		ud->stepskill_lv = skill_lv;
		return 0; // Attacking will be handled by unit_walktoxy_timer in this case
	}

	if (!ignore_range) {
		if( skill_get_state(ud->skill_id) == ST_MOVE_ENABLE ) {
			if( !unit_can_reach_bl(src, &bl, range + 1, 1, nullptr, nullptr) )
				return 0; // Walk-path check failed.
		}else if( !battle_check_range(src, &bl, range) )
			return 0; // Arrow-path check failed.
	}
	unit_stop_attack(src);

	// Moved here to prevent Suffragium from ending if skill fails
#ifndef RENEWAL_CAST
	casttime = skill_castfix_sc(src, casttime, skill_get_castnodex(skill_id));
#else
	casttime = skill_vfcastfix(src, casttime, skill_id, skill_lv );
#endif

	ud->state.skillcastcancel = castcancel&&casttime>0?1:0;

// 	if( sd )
// 	{
// 		switch( skill_id )
// 		{
// 		case ????:
// 			sd->canequip_tick = tick + casttime;
// 		}
// 	}

	ud->skill_id    = skill_id;
	ud->skill_lv    = skill_lv;
	ud->skillx      = skill_x;
	ud->skilly      = skill_y;
	ud->skilltarget = 0;

	// Set attack and act delays
	// Please note that the call below relies on ud->skill_id being set!
	unit_set_attackdelay(*src, tick, DELAY_EVENT_CASTBEGIN_POS);
	// Apply cast time and general delays
	unit_set_castdelay(*ud, tick, (skill_get_cast(skill_id, skill_lv) != 0) ? casttime : 0);

	if( sc ) {
		// These 3 status do not stack, so it's efficient to use if-else
		if (sc->getSCE(SC_CLOAKING) && !(sc->getSCE(SC_CLOAKING)->val4&4)) {
			status_change_end(src, SC_CLOAKING);

			if (!src->prev)
				return 0; // Warped away!
		} else if (sc->getSCE(SC_CLOAKINGEXCEED) && !(sc->getSCE(SC_CLOAKINGEXCEED)->val4&4)) {
			status_change_end(src, SC_CLOAKINGEXCEED);

			if (!src->prev)
				return 0;
		} else if (sc->getSCE(SC_NEWMOON)) {
			status_change_end(src, SC_NEWMOON);

			if (!src->prev)
				return 0;
		}
	}

	unit_stop_walking( src, USW_FIXPOS );

	// SC_MAGICPOWER needs to switch states at start of cast
#ifndef RENEWAL
	skill_toggle_magicpower(src, skill_id);
#endif

	// In official this is triggered even if no cast time.
	clif_skillcasting(src, src->id, 0, skill_x, skill_y, skill_id, skill_lv, skill_get_ele(skill_id, skill_lv), casttime);

	if( casttime > 0 ) {
		ud->skilltimer = add_timer( tick+casttime, skill_castend_pos, src->id, 0 );

		if( (sd && pc_checkskill(sd,SA_FREECAST) > 0) || skill_id == LG_EXEEDBREAK)
			status_calc_bl(sd, { SCB_SPEED, SCB_ASPD });
	} else {
		ud->skilltimer = INVALID_TIMER;
		skill_castend_pos(ud->skilltimer,tick,src->id,0);
	}

	if( sd && battle_config.prevent_logout_trigger&PLT_SKILL )
		sd->canlog_tick = gettick();

	return 1;
}

/**
 * Update a unit's attack target
 * @param ud: Unit data
 * @param target_id: Target ID (bl->id)
 * @return 0
 */
int32 unit_set_target(struct unit_data* ud, int32 target_id)
{
	nullpo_ret(ud);

	if( ud->target != target_id ) {
		struct unit_data * ux;
		struct block_list* target;
	
		if (ud->target && (target = map_id2bl(ud->target)) && (ux = unit_bl2ud(target)) && ux->target_count > 0)
			ux->target_count--;

		if (target_id && (target = map_id2bl(target_id)) && (ux = unit_bl2ud(target)) && ux->target_count < 255)
			ux->target_count++;
	}

	ud->target = target_id;

	return 0;
}

/**
 * Helper function used in foreach calls to stop auto-attack timers
 * @param bl: Block object
 * @param ap: func* with va_list values
 *   Parameter: '0' - everyone, 'id' - only those attacking someone with that id
 * @return 1 on success or 0 otherwise
 */
int32 unit_stopattack(struct block_list *bl, va_list ap)
{
	struct unit_data *ud = unit_bl2ud(bl);
	int32 id = va_arg(ap, int32);

	if (ud && ud->attacktimer != INVALID_TIMER && (!id || id == ud->target)) {
		unit_stop_attack(bl);
		return 1;
	}

	return 0;
}

/**
 * Stop a unit's attacks
 * @param bl: Object to stop
 */
void unit_stop_attack(struct block_list *bl)
{
	struct unit_data *ud;
	nullpo_retv(bl);
	ud = unit_bl2ud(bl);
	nullpo_retv(ud);

	//Clear target
	unit_set_target(ud, 0);

	if(ud->attacktimer == INVALID_TIMER)
		return;

	//Clear timer
	delete_timer(ud->attacktimer, unit_attack_timer);
	ud->attacktimer = INVALID_TIMER;
}

/**
 * Stop a unit's step action
 * @param bl: Object to stop
 */
void unit_stop_stepaction(struct block_list *bl)
{
	struct unit_data *ud;
	nullpo_retv(bl);
	ud = unit_bl2ud(bl);
	nullpo_retv(ud);

	//Clear remembered step action
	ud->stepaction = false;
	ud->target_to = 0;
	ud->stepskill_id = 0;
	ud->stepskill_lv = 0;

	if(ud->steptimer == INVALID_TIMER)
		return;

	//Clear timer
	delete_timer(ud->steptimer, unit_step_timer);
	ud->steptimer = INVALID_TIMER;
}

/**
 * Removes a unit's target due to being unattackable
 * @param bl: Object to unlock target
 * @return 0
 */
int32 unit_unattackable(struct block_list *bl)
{
	struct unit_data *ud = unit_bl2ud(bl);

	if (ud) {
		ud->state.attack_continue = 0;
		ud->state.step_attack = 0;
		ud->target_to = 0;
		unit_set_target(ud, 0);
	}

	if(bl->type == BL_MOB)
		mob_unlocktarget((struct mob_data*)bl, gettick());
	else if(bl->type == BL_PET)
		pet_unlocktarget((struct pet_data*)bl);

	return 0;
}

/**
 * Requests a unit to attack a target
 * @param src: Object initiating attack
 * @param target_id: Target ID (bl->id)
 * @param continuous: 
 *		0x1 - Whether or not the attack is ongoing
 *		0x2 - Whether function was called from unit_step_timer or not
 * @return How the unit should stop; see e_unit_stop_walking
 */
int32 unit_attack(struct block_list *src,int32 target_id,int32 continuous)
{
	struct block_list *target;
	int32 range;

	unit_data* ud = unit_bl2ud(src);
	if (ud == nullptr)
		return USW_NONE;

	mob_data* md = BL_CAST(BL_MOB, src);

	// Check for special monster random target mode, function might overwrite the original target
	if (md != nullptr && !mob_randomtarget(*md, target_id))
		return USW_NONE; // Continue walking

	int32 stop_flag = (USW_FIXPOS|USW_RELEASE_TARGET);

	target = map_id2bl(target_id);
	if( target == nullptr || status_isdead(*target) ) {
		unit_unattackable(src);
		return stop_flag;
	}

	if( src->type == BL_PC &&
		target->type == BL_NPC ) {
		// Monster npcs [Valaris]
		npc_click((TBL_PC*)src,(TBL_NPC*)target);
		return stop_flag;
	}

	if( !unit_can_attack(src, target_id) ) {
		unit_stop_attack(src);
		return USW_NONE; // Continue walking
	}

	if( battle_check_target(src,target,BCT_ENEMY) <= 0 || !status_check_skilluse(src, target, 0, 0) ) {
		unit_unattackable(src);
		return stop_flag;
	}

	ud->state.attack_continue = (continuous&1)?1:0;
	ud->state.step_attack = (continuous&2)?1:0;
	unit_set_target(ud, target_id);

	range = status_get_range(src);

	if (continuous) // If you're to attack continously, set to auto-chase character
		ud->chaserange = range;

	// Just change target/type. [Skotlex]
	if(ud->attacktimer != INVALID_TIMER)
		return stop_flag;

	// New action request received, delete previous action request if not executed yet
	if(ud->stepaction || ud->steptimer != INVALID_TIMER)
		unit_stop_stepaction(src);
	// Remember the attack request from the client while walking to the next cell
	if(src->type == BL_PC && ud->walktimer != INVALID_TIMER && !battle_check_range(src, target, range-1)) {
		ud->stepaction = true;
		ud->target_to = ud->target;
		ud->stepskill_id = 0;
		ud->stepskill_lv = 0;
		// Attacking will be handled by unit_walktoxy_timer in this case
		return USW_NONE;
	}

	// Source may die due to reflect damage
	map_freeblock_lock();

	if(DIFF_TICK(ud->attackabletime, gettick()) > 0) // Do attack next time it is possible. [Skotlex]
		ud->attacktimer=add_timer(ud->attackabletime,unit_attack_timer,src->id,0);
	else { // Attack NOW.
		// We need to send fixpos before the attack so that we don't cancel the attack animation
		clif_fixpos(*src);
		unit_attack_timer(INVALID_TIMER, gettick(), src->id, 0);
		stop_flag &= ~USW_FIXPOS;
	}

	// Monster state is set regardless of whether the attack is executed now or later
	// The check is here because unit_attack can be called from both the monster AI and the walking logic
	if (md != nullptr)
		mob_setstate(*md, MSS_BERSERK);

	map_freeblock_unlock();
	return stop_flag;
}

/** 
 * Cancels an ongoing combo, resets attackable time, and restarts the
 * attack timer to resume attack after amotion time
 * @author [Skotlex]
 * @param bl: Object to cancel combo
 * @return Success(1); Fail(0);
 */
int32 unit_cancel_combo(struct block_list *bl)
{
	struct unit_data  *ud;

	if (!status_change_end(bl, SC_COMBO))
		return 0; // Combo wasn't active.

	ud = unit_bl2ud(bl);
	nullpo_ret(ud);

	ud->attackabletime = gettick() + status_get_amotion(bl);

	if (ud->attacktimer == INVALID_TIMER)
		return 1; // Nothing more to do.

	delete_timer(ud->attacktimer, unit_attack_timer);
	ud->attacktimer=add_timer(ud->attackabletime,unit_attack_timer,bl->id,0);

	return 1;
}

/**
 * Does a path_search to check if a position can be reached
 * @param bl: Object to check path
 * @param x: X coordinate that will be path searched
 * @param y: Y coordinate that will be path searched
 * @param easy: Easy(1) or Hard(0) path check (hard attempts to go around obstacles)
 * @return true or false
 */
bool unit_can_reach_pos(struct block_list *bl,int32 x,int32 y, int32 easy)
{
	nullpo_retr(false, bl);

	if (bl->x == x && bl->y == y) // Same place
		return true;

	return path_search(nullptr,bl->m,bl->x,bl->y,x,y,easy,CELL_CHKNOREACH);
}

/**
 * Does a path_search to check if a unit can be reached
 * @param bl: Object to check path
 * @param tbl: Target to be checked for available path
 * @param range: The number of cells away from bl that the path should be checked
 * @param easy: Easy(1) or Hard(0) path check (hard attempts to go around obstacles)
 * @param x: Pointer storing a valid X coordinate around tbl that can be reached
 * @param y: Pointer storing a valid Y coordinate around tbl that can be reached
 * @return true or false
 */
bool unit_can_reach_bl(struct block_list *bl,struct block_list *tbl, int32 range, int32 easy, int16 *x, int16 *y)
{
	struct walkpath_data wpd;
	int16 dx, dy;

	nullpo_retr(false, bl);
	nullpo_retr(false, tbl);

	if( bl->m != tbl->m)
		return false;

	if( bl->x == tbl->x && bl->y == tbl->y )
		return true;

	if(range > 0 && !check_distance_bl(bl, tbl, range))
		return false;

	// It judges whether it can adjoin or not.
	dx = tbl->x - bl->x;
	dy = tbl->y - bl->y;
	dx = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
	dy = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);

	if (map_getcell(tbl->m,tbl->x-dx,tbl->y-dy,CELL_CHKNOPASS)) { // Look for a suitable cell to place in.
		int32 i;

		for(i = 0; i < 8 && map_getcell(tbl->m,tbl->x-dirx[i],tbl->y-diry[i],CELL_CHKNOPASS); i++);

		if (i == 8)
			return false; // No valid cells.

		dx = dirx[i];
		dy = diry[i];
	}

	if (x)
		*x = tbl->x-dx;

	if (y)
		*y = tbl->y-dy;

	if (!path_search(&wpd,bl->m,bl->x,bl->y,tbl->x-dx,tbl->y-dy,easy,CELL_CHKNOREACH))
		return false;

#ifdef OFFICIAL_WALKPATH
	if( bl->type != BL_NPC // If type is a NPC, please disregard.
		&& wpd.path_len > 14 // Official number of walkable cells is 14 if and only if there is an obstacle between. [malufett]
		&& !path_search_long(nullptr, bl->m, bl->x, bl->y, tbl->x-dx, tbl->y-dy, CELL_CHKNOPASS) ) // Check if there is an obstacle between
			return false;
#endif

	return true;
}

/**
 * Calculates position of Pet/Mercenary/Homunculus/Elemental
 * @param bl: Object to calculate position
 * @param tx: X coordinate to go to
 * @param ty: Y coordinate to go to
 * @param dir: Direction which to be 2 cells from master's position
 * @return Success(0); Fail(1);
 */
int32 unit_calc_pos(struct block_list *bl, int32 tx, int32 ty, uint8 dir)
{
	int32 dx, dy, x, y;
	struct unit_data *ud = unit_bl2ud(bl);

	nullpo_ret(ud);

	if(dir >= DIR_MAX || dir <= DIR_CENTER)
		return 1;

	ud->to_x = tx;
	ud->to_y = ty;

	// 2 cells from Master Position
	dx = -dirx[dir] * 2;
	dy = -diry[dir] * 2;
	x = tx + dx;
	y = ty + dy;

	if( !unit_can_reach_pos(bl, x, y, 0) ) {
		if( dx > 0 )
			x--;
		else if( dx < 0 )
			x++;

		if( dy > 0 )
			y--;
		else if( dy < 0 )
			y++;

		if( !unit_can_reach_pos(bl, x, y, 0) ) {
			int32 i;

			for( i = 0; i < 12; i++ ) {
				int32 k = rnd_value<int32>(DIR_NORTH, DIR_NORTHEAST); // Pick a Random Dir

				dx = -dirx[k] * 2;
				dy = -diry[k] * 2;
				x = tx + dx;
				y = ty + dy;

				if( unit_can_reach_pos(bl, x, y, 0) )
					break;
				else {
					if( dx > 0 )
						x--;
					else if( dx < 0 )
						x++;

					if( dy > 0 )
						y--;
					else if( dy < 0 )
						y++;

					if( unit_can_reach_pos(bl, x, y, 0) )
						break;
				}
			}

			if( i == 12 ) {
				x = tx; y = tx; // Exactly Master Position

				if( !unit_can_reach_pos(bl, x, y, 0) )
					return 1;
			}
		}
	}

	ud->to_x = x;
	ud->to_y = y;

	return 0;
}

/**
 * Function timer to continuously attack
 * @param src: Object to continuously attack
 * @param tid: Timer ID
 * @param tick: Current tick
 * @return Attackable(1); Unattackable(0);
 */
static int32 unit_attack_timer_sub(struct block_list* src, int32 tid, t_tick tick)
{
	struct block_list *target;
	struct unit_data *ud;
	map_session_data *sd = nullptr;
	struct mob_data *md = nullptr;
	int32 range;

	if( (ud = unit_bl2ud(src)) == nullptr )
		return 0;

	if( ud->attacktimer != tid ) {
		ShowError("unit_attack_timer %d != %d\n",ud->attacktimer,tid);
		return 0;
	}

	sd = BL_CAST(BL_PC, src);
	md = BL_CAST(BL_MOB, src);

	// Make sure attacktimer is removed before doing anything else
	ud->attacktimer = INVALID_TIMER;

	// Note: Officially there is no such thing as an attack timer. All actions are driven by the client or the AI.
	// We use the continuous attack timers to have accurate attack timings that don't depend on the AI interval.
	// However, for a clean implementation we still should channel through the whole AI code so the same rules
	// apply as usual and we don't need to code extra rules. Currently we resolved this only for monsters.
	// We don't want this to trigger on direct calls of the timer function as that should just execute the attack.
	if (md != nullptr && tid != INVALID_TIMER)
		return mob_ai_sub_hard_attacktimer(*md, tick);

	target = map_id2bl(ud->target);

	if( src == nullptr || src->prev == nullptr || target==nullptr || target->prev == nullptr )
		return 0;

	if( status_isdead(*src) || status_isdead(*target) ||
			battle_check_target(src,target,BCT_ENEMY) <= 0 || !status_check_skilluse(src, target, 0, 0)
#ifdef OFFICIAL_WALKPATH
	   || !path_search_long(nullptr, src->m, src->x, src->y, target->x, target->y, CELL_CHKWALL)
#endif
	   || !unit_can_attack(src, target->id) )
		return 0; // Can't attack under these conditions

	if( ud->skilltimer != INVALID_TIMER && !(sd && pc_checkskill(sd,SA_FREECAST) > 0) )
		return 0; // Can't attack while casting

	if( !battle_config.sdelay_attack_enable && DIFF_TICK(ud->canact_tick,tick) > 0 && !(sd && pc_checkskill(sd,SA_FREECAST) > 0) ) {
		// Attacking when under cast delay has restrictions:
		if( tid == INVALID_TIMER ) { // Requested attack.
			if(sd)
				clif_skill_fail( *sd, 1, USESKILL_FAIL_SKILLINTERVAL );

			return 0;
		}

		// Otherwise, we are in a combo-attack, delay this until your canact time is over. [Skotlex]
		if( ud->state.attack_continue ) {
			if( DIFF_TICK(ud->canact_tick, ud->attackabletime) > 0 )
				ud->attackabletime = ud->canact_tick;

			ud->attacktimer=add_timer(ud->attackabletime,unit_attack_timer,src->id,0);
		}

		return 1;
	}

	status_data* sstatus = status_get_status_data(*src);
	range = sstatus->rhw.range;

	if( (unit_is_walking(target) || ud->state.step_attack)
		&& (target->type == BL_PC || !map_getcell(target->m,target->x,target->y,CELL_CHKICEWALL)) )
		range++; // Extra range when chasing (does not apply to mobs locked in an icewall)

	if(sd && !check_distance_client_bl(src,target,range)) {
		// Player tries to attack but target is too far, notify client
		clif_movetoattack( *sd, *target );
		return 1;
	} else if(md && !check_distance_bl(src,target,range)) {
		// Monster: Chase if required
		unit_walktobl(src,target,ud->chaserange,ud->state.walk_easy|2);
		return 1;
	}

	if( !battle_check_range(src,target,range) ) {
		// Within range, but no direct line of attack
		// This code can usually only be reached if OFFICIAL_WALKPATH is disabled
		if (ud->state.attack_continue && ud->chaserange > 1) {
			ud->chaserange = std::max(1, ud->chaserange - 2);

			// Walk closer / around the obstacle and start attacking once you are in range
			return unit_walktobl(src,target,ud->chaserange,ud->state.walk_easy|2);
		}
		// Can't attack even though next to the target? Giving up here.
		return 0;
	}

	// Sync packet only for players.
	// Non-players use the sync packet on the walk timer. [Skotlex]
	if (tid == INVALID_TIMER && sd)
		clif_fixpos( *src );

	map_freeblock_lock();

	if( DIFF_TICK(ud->attackabletime,tick) <= 0 ) {
		if (battle_config.attack_direction_change && (src->type&battle_config.attack_direction_change))
			unit_setdir(src, map_calc_dir(src, target->x, target->y), false);

		if(ud->walktimer != INVALID_TIMER)
			unit_stop_walking( src, USW_FIXPOS );

		if(md) {
			if (status_has_mode(sstatus,MD_ASSIST) && DIFF_TICK(tick, md->last_linktime) >= MIN_MOBLINKTIME) { 
				// Link monsters nearby [Skotlex]
				md->last_linktime = tick;
				map_foreachinshootrange(mob_linksearch, src, battle_config.assist_range, BL_MOB, md->mob_id, target->id, tick);
			}
		}

		if (src->type == BL_PET && pet_attackskill((TBL_PET*)src, target->id)) {
			map_freeblock_unlock();
			return 1;
		}

		ud->attacktarget_lv = battle_weapon_attack(src,target,tick,0);

		if(sd && sd->status.pet_id > 0 && sd->pd && battle_config.pet_attack_support)
			pet_target_check(sd->pd,target,0);

		/**
		 * Applied when you're unable to attack (e.g. out of ammo)
		 * We should stop here otherwise timer keeps on and this happens endlessly
		 */
		if (ud->attacktarget_lv == ATK_NONE) {
			map_freeblock_unlock();
			return 1;
		}

		unit_set_attackdelay(*src, tick, DELAY_EVENT_ATTACK);

		// Only reset skill_id here if no skilltimer is currently ongoing
		if (ud->skilltimer == INVALID_TIMER)
			ud->skill_id = 0;

		// You can't move during your attack motion
		if (src->type&battle_config.attack_walk_delay)
			unit_set_walkdelay(src, tick, sstatus->amotion, 1);
	}

	if (ud->state.attack_continue && !status_isdead(*src)) {
		if (src->type == BL_PC && battle_config.idletime_option&IDLE_ATTACK)
			((TBL_PC*)src)->idletime = last_tick;

		ud->attacktimer = add_timer(ud->attackabletime,unit_attack_timer,src->id,0);
	}

	if( sd && battle_config.prevent_logout_trigger&PLT_ATTACK )
		sd->canlog_tick = gettick();

	map_freeblock_unlock();
	return 1;
}

/**
 * Timer function to cancel attacking if unit has become unattackable
 * @param tid: Timer ID
 * @param tick: Current tick
 * @param id: Object to cancel attack if applicable
 * @param data: Data passed from timer call
 * @return 0
 */
static TIMER_FUNC(unit_attack_timer){
	struct block_list *bl;

	bl = map_id2bl(id);

	if (bl != nullptr) {
		// Execute attack
		if (unit_attack_timer_sub(bl, tid, tick) == 0)
			unit_unattackable(bl);
	}

	return 0;
}

/**
 * Determines whether a player can attack based on status changes
 *  Why not use status_check_skilluse?
 *  "src MAY be null to indicate we shouldn't check it, this is a ground-based skill attack."
 *  Even ground-based attacks should be blocked by these statuses
 * Called from unit_attack and unit_attack_timer_sub
 * @retval true Can attack
 **/
bool unit_can_attack(struct block_list *bl, int32 target_id) {
	nullpo_retr(false, bl);

	if (bl->type == BL_PC) {
		map_session_data *sd = ((TBL_PC *)bl);

		if (sd && ((sd->state.block_action & PCBLOCK_ATTACK) || pc_isridingwug(sd)))
			return false;
	}

	status_change *sc;

	if (!(sc = status_get_sc(bl)))
		return true;

	if (sc->cant.attack || (sc->getSCE(SC_VOICEOFSIREN) && sc->getSCE(SC_VOICEOFSIREN)->val2 == target_id))
		return false;

	if (bl->type != BL_PC && sc->hasSCE(SC_WINKCHARM) && sc->getSCE(SC_WINKCHARM)->val2 == target_id)
		return false;

	return true;
}

/**
 * Cancels a skill's cast
 * @param bl: Object to cancel cast
 * @param type: Cancel check flag
 *	&1: Cancel skill stored in sd->skill_id_old instead
 *	&2: Cancel only if skill is cancellable
 * @return Success(1); Fail(0);
 */
int32 unit_skillcastcancel(struct block_list *bl, char type)
{
	map_session_data *sd = nullptr;
	struct unit_data *ud = unit_bl2ud( bl);
	t_tick tick = gettick();
	int32 ret = 0, skill_id;

	nullpo_ret(bl);

	if (!ud || ud->skilltimer == INVALID_TIMER)
		return 0; // Nothing to cancel.

	sd = BL_CAST(BL_PC, bl);

	if (type&2) { // See if it can be cancelled.
		if (!ud->state.skillcastcancel)
			return 0;

		if (sd && (sd->special_state.no_castcancel2 ||
			((sd->sc.getSCE(SC_UNLIMITEDHUMMINGVOICE) || sd->special_state.no_castcancel) && !map_flag_gvg2(bl->m) && !map_getmapflag(bl->m, MF_BATTLEGROUND)))) // fixed flags being read the wrong way around [blackhole89]
			return 0;
	}

	ud->canact_tick = tick;

	if(type&1 && sd)
		skill_id = sd->skill_id_old;
	else
		skill_id = ud->skill_id;

	if (skill_get_inf(skill_id) & INF_GROUND_SKILL)
		ret=delete_timer( ud->skilltimer, skill_castend_pos );
	else
		ret=delete_timer( ud->skilltimer, skill_castend_id );

	if(ret < 0)
		ShowError("delete timer error : skill_id : %d\n",ret);

	ud->skilltimer = INVALID_TIMER;

	if( sd && (pc_checkskill(sd,SA_FREECAST) > 0 || skill_id == LG_EXEEDBREAK) )
		status_calc_bl(sd, { SCB_SPEED, SCB_ASPD });

	if( sd ) {
		switch( skill_id ) {
			case CG_ARROWVULCAN:
				sd->canequip_tick = tick;
				break;
		}
	}

	unit_set_attackdelay(*bl, tick, DELAY_EVENT_CASTCANCEL);

	if (bl->type == BL_MOB) {
		mob_data& md = reinterpret_cast<mob_data&>(*bl);
		md.skill_idx = -1;
	}

	clif_skillcastcancel( *bl );

	return 1;
}

/**
 * Initialized data on a unit
 * @param bl: Object to initialize data on
 */
void unit_dataset(struct block_list *bl)
{
	struct unit_data *ud;

	nullpo_retv(ud = unit_bl2ud(bl));

	memset( ud, 0, sizeof( struct unit_data) );
	ud->bl             = bl;
	ud->walktimer      = INVALID_TIMER;
	ud->skilltimer     = INVALID_TIMER;
	ud->attacktimer    = INVALID_TIMER;
	ud->steptimer      = INVALID_TIMER;
	t_tick tick = gettick();
	ud->attackabletime = tick;
	ud->canact_tick = tick;
	ud->canmove_tick = tick;
	ud->endure_tick = tick;
	ud->dmg_tick = 0;
	ud->sx = 8;
	ud->sy = 8;
}

/**
 * Returns the remaining max amount of skill units per object for a specific skill
 * @param ud: Unit data
 * @param skill_id: Skill to search for
 * @param maxcount: Maximum amount of placeable units
 */
void unit_skillunit_maxcount(unit_data& ud, uint16 skill_id, int& maxcount) {
	for (const auto su : ud.skillunits) {
		if (su->skill_id == skill_id && --maxcount == 0 )
			break;
	}
}

/**
 * Gets the number of units attacking another unit
 * @param bl: Object to check amount of targets
 * @return number of targets or 0
 */
int32 unit_counttargeted(struct block_list* bl)
{
	struct unit_data* ud;

	if( bl && (ud = unit_bl2ud(bl)) )
		return ud->target_count;

	return 0;
}

/**
 * Makes 'bl' that attacking 'src' switch to attack 'target'
 * @param bl
 * @param ap
 * @param src Current target
 * @param target New target
 **/
int32 unit_changetarget(block_list *bl, va_list ap) {
	if (bl == nullptr)
		return 1;
	unit_data *ud = unit_bl2ud(bl);
	block_list *src = va_arg(ap, block_list *);
	block_list *target = va_arg(ap, block_list *);

	if (ud == nullptr || src == nullptr || target == nullptr || ud->target == target->id)
		return 1;
	if (ud->target <= 0 && ud->target_to <= 0)
		return 1;
	if (ud->target != src->id && ud->target_to != src->id)
		return 1;

	unit_changetarget_sub(*ud, *target);

	//unit_attack(bl, target->id, ud->state.attack_continue);
	return 0;
}

/**
 * Changes the target of a unit
 * @param ud: Unit data
 * @param target: New target data
 **/
void unit_changetarget_sub(unit_data& ud, block_list& target) {
	if (status_isdead(target))
		return;

	if (ud.bl->type == BL_MOB)
		reinterpret_cast<mob_data*>(ud.bl)->target_id = target.id;
	if (ud.target_to > 0)
		ud.target_to = target.id;
	if (ud.skilltarget > 0)
		ud.skilltarget = target.id;
	unit_set_target(&ud, target.id);
}

/**
 * Removes a bl/ud from the map
 * On kill specifics are not performed here, check status_damage()
 * @param bl: Object to remove from map
 * @param clrtype: How bl is being removed
 *	0: Assume bl is being warped
 *	1: Death, appropriate cleanup performed
 * @param file, line, func: Call information for debug purposes
 * @return Success(1); Couldn't be removed or bl was free'd(0)
 */
int32 unit_remove_map_(struct block_list *bl, clr_type clrtype, const char* file, int32 line, const char* func)
{
	struct unit_data *ud = unit_bl2ud(bl);
	status_change *sc = status_get_sc(bl);

	nullpo_ret(ud);

	if(bl->prev == nullptr)
		return 0; // Already removed?

	map_freeblock_lock();

	if (ud->walktimer != INVALID_TIMER)
		unit_stop_walking(bl,USW_RELEASE_TARGET);

	if (clrtype == CLR_DEAD)
		ud->state.blockedmove = true;

	if (ud->skilltimer != INVALID_TIMER)
		unit_skillcastcancel(bl,0);

	//Clear target even if there is no timer
	if (ud->target || ud->attacktimer != INVALID_TIMER)
		unit_stop_attack(bl);

	//Clear stepaction even if there is no timer
	if (ud->stepaction || ud->steptimer != INVALID_TIMER)
		unit_stop_stepaction(bl);

	// Do not reset can-act delay. [Skotlex]
	ud->attackabletime = ud->canmove_tick /*= ud->canact_tick*/ = gettick();

	if(sc != nullptr && !sc->empty() ) { // map-change/warp dispells.
		status_db.removeByStatusFlag(bl, { SCF_REMOVEONCHANGEMAP });

		// Ensure the bl is a PC; if so, we'll handle the removal of cloaking and cloaking exceed later
		if ( bl->type != BL_PC ) {
			status_change_end(bl, SC_CLOAKING);
			status_change_end(bl, SC_CLOAKINGEXCEED);
		}
		if (sc->getSCE(SC_GOSPEL) && sc->getSCE(SC_GOSPEL)->val4 == BCT_SELF)
			status_change_end(bl, SC_GOSPEL);
		if (sc->getSCE(SC_PROVOKE) && sc->getSCE(SC_PROVOKE)->val4 == 1)
			status_change_end(bl, SC_PROVOKE); //End infinite provoke to prevent exploit
	}

	switch( bl->type ) {
		case BL_PC: {
			map_session_data *sd = (map_session_data*)bl;

			if(sd->shadowform_id) { // If shadow target has leave the map
			    struct block_list *d_bl = map_id2bl(sd->shadowform_id);

			    if( d_bl )
				    status_change_end(d_bl,SC__SHADOWFORM);
			}

			// Leave/reject all invitations.
			if(sd->chatID)
				chat_leavechat(sd,0);

			if(sd->trade_partner.id > 0)
				trade_tradecancel(sd);

			searchstore_close(*sd);

			if (sd->menuskill_id != AL_TELEPORT) { //bugreport:8027
				if (sd->state.storage_flag == 1)
					storage_storage_quit(sd,0);
				else if (sd->state.storage_flag == 2)
					storage_guild_storage_quit(sd, 0);
				else if (sd->state.storage_flag == 3)
					storage_premiumStorage_quit(sd);

				sd->state.storage_flag = 0; //Force close it when being warped.
			}

			if(sd->party_invite > 0)
				party_reply_invite( *sd, sd->party_invite, 0 );

			if(sd->guild_invite > 0)
				guild_reply_invite( *sd, sd->guild_invite, 0 );

			if(sd->guild_alliance > 0)
				guild_reply_reqalliance(sd,sd->guild_alliance_account,0);

			if(sd->menuskill_id)
				sd->menuskill_id = sd->menuskill_val = 0;

			if( !sd->npc_ontouch_.empty() )
				npc_touchnext_areanpc(sd,true);

			// Check if warping and not changing the map.
			if ( sd->state.warping && !sd->state.changemap ) {
				status_change_end(bl, SC_CLOAKING);
				status_change_end(bl, SC_CLOAKINGEXCEED);
			}

			sd->npc_shopid = 0;
			sd->adopt_invite = 0;

			if(sd->pvp_timer != INVALID_TIMER) {
				delete_timer(sd->pvp_timer,pc_calc_pvprank_timer);
				sd->pvp_timer = INVALID_TIMER;
				sd->pvp_rank = 0;
			}

			if(sd->duel_group > 0)
				duel_leave(sd->duel_group, sd);

			if(pc_issit(sd) && pc_setstand(sd, false))
				skill_sit(sd, false);

			party_send_dot_remove(sd);// minimap dot fix [Kevin]
			guild_send_dot_remove(sd);
			bg_send_dot_remove(sd);

			if( map[bl->m].users <= 0 || sd->state.debug_remove_map ) {
				// This is only place where map users is decreased, if the mobs were removed
				// too soon then this function was executed too many times [FlavioJS]
				if( sd->debug_file == nullptr || !(sd->state.debug_remove_map) ) {
					sd->debug_file = "";
					sd->debug_line = 0;
					sd->debug_func = "";
				}

				ShowDebug("unit_remove_map: unexpected state when removing player AID/CID:%d/%d"
					" (active=%d connect_new=%d rewarp=%d changemap=%d debug_remove_map=%d)"
					" from map=%s (users=%d)."
					" Previous call from %s:%d(%s), current call from %s:%d(%s)."
					" Please report this!!!\n",
					sd->status.account_id, sd->status.char_id,
					sd->state.active, sd->state.connect_new, sd->state.rewarp, sd->state.changemap, sd->state.debug_remove_map,
					map[bl->m].name, map[bl->m].users,
					sd->debug_file, sd->debug_line, sd->debug_func, file, line, func);
			}
			else if (--map[bl->m].users == 0 && battle_config.dynamic_mobs)
				map_removemobs(bl->m);

			if( !pc_isinvisible(sd) ) // Decrement the number of active pvp players on the map
				--map[bl->m].users_pvp;

			if( sd->state.hpmeter_visible ) {
				map[bl->m].hpmeter_visible--;
				sd->state.hpmeter_visible = 0;
			}

			sd->state.debug_remove_map = 1; // Temporary state to track double remove_map's [FlavioJS]
			sd->debug_file = file;
			sd->debug_line = line;
			sd->debug_func = func;
			break;
		}
		case BL_MOB: {
			struct mob_data *md = (struct mob_data*)bl;

			// Drop previous target mob_slave_keep_target: no.
			if (!battle_config.mob_slave_keep_target)
				md->target_id=0;

			// When a monster is removed from map, its spotted log is cleared
			for (int32 i = 0; i < DAMAGELOG_SIZE; i++)
				md->spotted_log[i] = 0;

			md->attacked_id=0;
			mob_setstate(*md, MSS_IDLE);
			break;
		}
		case BL_PET: {
			struct pet_data *pd = (struct pet_data*)bl;

			if( pd->pet.intimate <= PET_INTIMATE_NONE && !(pd->master && !pd->master->state.active) ) {
				// If logging out, this is deleted on unit_free
				clif_clearunit_area( *bl, clrtype );
				map_delblock(bl);
				unit_free(bl,CLR_OUTSIGHT);
				map_freeblock_unlock();

				return 0;
			}
			break;
		}
		case BL_HOM: {
			struct homun_data *hd = (struct homun_data *)bl;

			ud->canact_tick = ud->canmove_tick; // It appears HOM do reset the can-act tick.

			if( !hd->homunculus.intimacy && !(hd->master && !hd->master->state.active) ) {
				// If logging out, this is deleted on unit_free
				clif_clearunit_area( *bl, clrtype );
				map_delblock(bl);
				unit_free(bl,CLR_OUTSIGHT);
				map_freeblock_unlock();

				return 0;
			}
			break;
		}
		case BL_MER: {
			s_mercenary_data *md = (s_mercenary_data *)bl;

			ud->canact_tick = ud->canmove_tick;

			if( mercenary_get_lifetime(md) <= 0 && !(md->master && !md->master->state.active) ) {
				clif_clearunit_area( *bl, clrtype );
				map_delblock(bl);
				unit_free(bl,CLR_OUTSIGHT);
				map_freeblock_unlock();

				return 0;
			}
			break;
		}
		case BL_ELEM: {
			s_elemental_data *ed = (s_elemental_data *)bl;

			ud->canact_tick = ud->canmove_tick;

			if( elemental_get_lifetime(ed) <= 0 && !(ed->master && !ed->master->state.active) ) {
				clif_clearunit_area( *bl, clrtype );
				map_delblock(bl);
				unit_free(bl,CLR_OUTSIGHT);
				map_freeblock_unlock();

				return 0;
			}
			break;
		}
		case BL_NPC:
			if (npc_remove_map( (TBL_NPC*)bl ) != 0)
				return 0;
			break;
		default:
			break;// do nothing
	}

	if (bl->type&(BL_CHAR|BL_PET)) {
		skill_unit_move(bl,gettick(),4);
		skill_cleartimerskill(bl);
	}

	switch (bl->type) {
		case BL_NPC:
			// already handled by npc_remove_map
			break;
		case BL_MOB:
			// /BL_MOB is handled by mob_dead unless the monster is not dead.
			if (status_isdead(*bl)) {
				map_delblock(bl);
				break;
			}
			[[fallthrough]];
		default:
			clif_clearunit_area( *bl, clrtype );
			map_delblock(bl);
			break;
	}

	map_freeblock_unlock();

	return 1;
}

/**
 * Refresh the area with a change in display of a unit.
 * @bl: Object to update
 */
void unit_refresh(struct block_list *bl, bool walking) {
	nullpo_retv(bl);

	if (bl->m < 0)
		return;

	struct map_data *mapdata = map_getmapdata(bl->m);

	// Using CLR_TRICKDEAD because other flags show effects
	// Probably need to use another flag or other way to refresh it
	if (mapdata->users) {
		clif_clearunit_area( *bl, CLR_TRICKDEAD ); // Fade out
		clif_spawn(bl,walking); // Fade in
	}
}

/**
 * Removes units of a master when the master is removed from map
 * @param sd: Player
 * @param clrtype: How bl is being removed
 *	0: Assume bl is being warped
 *	1: Death, appropriate cleanup performed
 */
void unit_remove_map_pc(map_session_data *sd, clr_type clrtype)
{
	unit_remove_map(sd,clrtype);

	//CLR_RESPAWN is the warp from logging out, CLR_TELEPORT is the warp from teleporting, but pets/homunc need to just 'vanish' instead of showing the warping animation.
	if (clrtype == CLR_RESPAWN || clrtype == CLR_TELEPORT)
		clrtype = CLR_OUTSIGHT;

	if(sd->pd)
		unit_remove_map(sd->pd, clrtype);

	if(hom_is_active(sd->hd))
		unit_remove_map(sd->hd, clrtype);

	if(sd->md)
		unit_remove_map(sd->md, clrtype);

	if(sd->ed)
		unit_remove_map(sd->ed, clrtype);
}

/**
 * Frees units of a player when is removed from map
 * Also free his pets/homon/mercenary/elemental/etc if he have any
 * @param sd: Player
 */
void unit_free_pc(map_session_data *sd)
{
	if (sd->pd)
		unit_free(sd->pd,CLR_OUTSIGHT);

	if (sd->hd)
		unit_free(sd->hd,CLR_OUTSIGHT);

	if (sd->md)
		unit_free(sd->md,CLR_OUTSIGHT);

	if (sd->ed)
		unit_free(sd->ed,CLR_OUTSIGHT);

	unit_free(sd,CLR_TELEPORT);
}

/**
 * Frees all related resources to the unit
 * @param bl: Object being removed from map
 * @param clrtype: How bl is being removed
 *	0: Assume bl is being warped
 *	1: Death, appropriate cleanup performed
 * @return 0
 */
int32 unit_free(struct block_list *bl, clr_type clrtype)
{
	struct unit_data *ud = unit_bl2ud( bl );

	nullpo_ret(ud);

	map_freeblock_lock();

	if( bl->prev )	// Players are supposed to logout with a "warp" effect.
		unit_remove_map(bl, clrtype);

	switch( bl->type ) {
		case BL_PC: {
			map_session_data *sd = (map_session_data*)bl;
			int32 i;

			if( status_isdead(*bl) )
				pc_setrestartvalue(sd,2);

			pc_delinvincibletimer(sd);

			pc_delautobonus(*sd, sd->autobonus, false);
			pc_delautobonus(*sd, sd->autobonus2, false);
			pc_delautobonus(*sd, sd->autobonus3, false);

			if( sd->followtimer != INVALID_TIMER )
				pc_stop_following(sd);

			if( sd->duel_invite > 0 )
				duel_reject(sd->duel_invite, sd);

			channel_pcquit(sd,0xF); // Leave all chan
			skill_blockpc_clear(*sd); // Clear all skill cooldown related

			// Notify friends that this char logged out.
			if( battle_config.friend_auto_add ){
				for( const s_friend& my_friend : sd->status.friends ){
					// Cancel early
					if( my_friend.char_id == 0 ){
						break;
					}

					if( map_session_data* tsd = map_charid2sd( my_friend.char_id ); tsd != nullptr ){
						for( const s_friend& their_friend : tsd->status.friends ){
							// Cancel early
							if( their_friend.char_id == 0 ){
								break;
							}

							if( their_friend.account_id != sd->status.account_id ){
								continue;
							}

							if( their_friend.char_id != sd->status.char_id ){
								continue;
							}

							clif_friendslist_toggle( *tsd, their_friend, false );
							break;
						}
					}
				}
			}else{
				map_foreachpc( clif_friendslist_toggle_sub, sd->status.account_id, sd->status.char_id, static_cast<int32>( false ) );
			}
			party_send_logout(sd);
			guild_send_memberinfoshort(sd,0);
			pc_cleareventtimer(sd);
			pc_inventory_rental_clear(sd);
			pc_delspiritball(sd, sd->spiritball, 1);
			pc_delspiritcharm(sd, sd->spiritcharm, sd->spiritcharm_type);
			mob_removeslaves(sd);

			if( sd->st && sd->st->state != RUN ) {// free attached scripts that are waiting
				script_free_state(sd->st);
				sd->st = nullptr;
				sd->npc_id = 0;
			}

			if( !sd->npc_id_dynamic.empty() ){
				for (const auto &it : sd->npc_id_dynamic) {
					struct npc_data* nd = map_id2nd( it );

					if( nd != nullptr ){
						// Erase the owner first to prevent loops from npc_unload
						nd->dynamicnpc.owner_char_id = 0;

						// Delete the NPC
						npc_unload( nd, true );
					}
				}
				// Update NPC event database
				npc_read_event_script();

				sd->npc_id_dynamic.clear();
			}

			sd->combos.clear();

			if( sd->sc_display_count ) { /* [Ind] */
				for( i = 0; i < sd->sc_display_count; i++ )
					ers_free(pc_sc_display_ers, sd->sc_display[i]);

				sd->sc_display_count = 0;
				aFree(sd->sc_display);
				sd->sc_display = nullptr;
			}

			if( sd->quest_log != nullptr ) {
				aFree(sd->quest_log);
				sd->quest_log = nullptr;
				sd->num_quests = sd->avail_quests = 0;
			}

			sd->qi_display.clear();

#if PACKETVER_MAIN_NUM >= 20150507 || PACKETVER_RE_NUM >= 20150429 || defined(PACKETVER_ZERO)
			sd->hatEffects.clear();
#endif

			if (sd->achievement_data.achievements)
				achievement_free(sd);

			// Clearing...
			if (sd->bonus_script.head)
				pc_bonus_script_clear(sd, BSF_REM_ALL);

			skill_clear_unitgroup(bl);
			status_change_clear(bl,1);
			break;
		}
		case BL_PET: {
			struct pet_data *pd = (struct pet_data*)bl;
			map_session_data *sd = pd->master;

			pet_delautobonus(*sd, pd->autobonus, false);
			pet_delautobonus(*sd, pd->autobonus2, false);
			pet_delautobonus(*sd, pd->autobonus3, false);

			pet_hungry_timer_delete(pd);
			pet_clear_support_bonuses(sd);

			if( pd->pet.intimate > PET_INTIMATE_NONE )
				intif_save_petdata(pd->pet.account_id,&pd->pet);
			else { // Remove pet.
				intif_delete_petdata(pd->pet.pet_id);

				if (sd)
					sd->status.pet_id = 0;
			}

			if( sd )
				sd->pd = nullptr;
			pd->master = nullptr;

			skill_clear_unitgroup(bl);
			status_change_clear(bl,1);
			break;
		}
		case BL_MOB: {
			struct mob_data *md = (struct mob_data*)bl;

			mob_free_dynamic_viewdata( md );

			if( md->spawn_timer != INVALID_TIMER ) {
				delete_timer(md->spawn_timer,mob_delayspawn);
				md->spawn_timer = INVALID_TIMER;
			}

			if( md->deletetimer != INVALID_TIMER ) {
				delete_timer(md->deletetimer,mob_timer_delete);
				md->deletetimer = INVALID_TIMER;
			}

			if (md->lootitems) {
				aFree(md->lootitems);
				md->lootitems = nullptr;
			}

			if( md->guardian_data ) {
				std::shared_ptr<guild_castle> gc = md->guardian_data->castle;

				if( md->guardian_data->number >= 0 && md->guardian_data->number < MAX_GUARDIANS )
					gc->guardian[md->guardian_data->number].id = 0;
				else {
					int32 i;

					ARR_FIND(0, gc->temp_guardians_max, i, gc->temp_guardians[i] == md->id);
					if( i < gc->temp_guardians_max )
						gc->temp_guardians[i] = 0;
				}

				md->guardian_data->~guardian_data();
				aFree(md->guardian_data);
				md->guardian_data = nullptr;
			}

			if( md->spawn ) {
				md->spawn->active--;

				if( !md->spawn->state.dynamic ) { // Permanently remove the mob
					if( --md->spawn->num == 0 ) { // Last freed mob is responsible for deallocating the group's spawn data.
						aFree(md->spawn);
						md->spawn = nullptr;
					}
				}
			}

			if( md->base_status) {
				aFree(md->base_status);
				md->base_status = nullptr;
			}

			skill_clear_unitgroup(bl);
			status_change_clear(bl,1);

			if( mob_is_clone(md->mob_id) )
				mob_clone_delete(md);

			if( md->tomb_nid )
				mvptomb_destroy(md);
			break;
		}
		case BL_HOM:
		{
			struct homun_data *hd = (TBL_HOM*)bl;
			map_session_data *sd = hd->master;

			hom_hungry_timer_delete(hd);

			if( hd->homunculus.intimacy > 0 )
				hom_save(hd);
			else {
				intif_homunculus_requestdelete(hd->homunculus.hom_id);

				if( sd )
					sd->status.hom_id = 0;

#ifdef RENEWAL
				status_change_end(sd, SC_HOMUN_TIME);
#endif
			}

			skill_blockhomun_clear(*hd); // Clear all skill cooldown related

			if( sd )
				sd->hd = nullptr;
			hd->master = nullptr;

			skill_clear_unitgroup(bl);
			status_change_clear(bl,1);
			break;
		}
		case BL_MER: {
			s_mercenary_data *md = (TBL_MER*)bl;
			map_session_data *sd = md->master;

			if( mercenary_get_lifetime(md) > 0 )
				mercenary_save(md);
			else {
				intif_mercenary_delete(md->mercenary.mercenary_id);

				if( sd )
					sd->status.mer_id = 0;
			}

			if( sd )
				sd->md = nullptr;

			skill_blockmerc_clear(*md); // Clear all skill cooldown related
			mercenary_contract_stop(md);
			md->master = nullptr;

			skill_clear_unitgroup(bl);
			status_change_clear(bl,1);
			break;
		}
		case BL_ELEM: {
			s_elemental_data *ed = (TBL_ELEM*)bl;
			map_session_data *sd = ed->master;

			if( elemental_get_lifetime(ed) > 0 )
				elemental_save(ed);
			else {
				intif_elemental_delete(ed->elemental.elemental_id);

				if( sd )
					sd->status.ele_id = 0;
			}

			if( sd )
				sd->ed = nullptr;

			elemental_summon_stop(ed);
			ed->master = nullptr;

			skill_clear_unitgroup(bl);
			status_change_clear(bl,1);
			break;
		}
	}

	map_deliddb(bl);

	if( bl->type != BL_PC ) // Players are handled by map_quit
		map_freeblock(bl);

	map_freeblock_unlock();

	return 0;
}

static TIMER_FUNC(unit_shadowscar_timer) {
	block_list *bl = map_id2bl(id);

	if (bl == nullptr)
		return 1;

	unit_data *ud = unit_bl2ud(bl);

	if (ud == nullptr)
		return 1;

	std::vector<int32>::iterator it = ud->shadow_scar_timer.begin();

	while (it != ud->shadow_scar_timer.end()) {
		if (*it == tid) {
			ud->shadow_scar_timer.erase(it);
			break;
		}

		it++;
	}

	if (ud->shadow_scar_timer.empty())
		status_change_end( bl, SC_SHADOW_SCAR );

	return 0;
}

/**
 * Adds a Shadow Scar to unit for 'interval' ms.
 * @param ud: Unit data
 * @param interval: Duration
 */
void unit_addshadowscar(unit_data &ud, int32 interval) {
	if (ud.shadow_scar_timer.size() >= MAX_SHADOW_SCAR) {
		ShowWarning("unit_addshadowscar: Unit %s (%d) has reached the maximum amount of Shadow Scars (%d).\n", status_get_name(*ud.bl), ud.bl->id, MAX_SHADOW_SCAR);
		return;
	}

	ud.shadow_scar_timer.push_back(add_timer(gettick() + interval, unit_shadowscar_timer, ud.bl->id, 0));

	status_change *sc = status_get_sc(ud.bl);

	if (sc != nullptr) {
		if (sc->getSCE(SC_SHADOW_SCAR) != nullptr) {
			sc->getSCE(SC_SHADOW_SCAR)->val1 = static_cast<int32>(ud.shadow_scar_timer.size());
		} else {
			sc_start(ud.bl, ud.bl, SC_SHADOW_SCAR, 100, 1, INFINITE_TICK);
		}

		clif_enchantingshadow_spirit(ud);
	}
}

/**
 * Initialization function for unit on map start
 * called in map::do_init
 */
void do_init_unit(void){
	add_timer_func_list(unit_attack_timer,  "unit_attack_timer");
	add_timer_func_list(unit_walktoxy_timer,"unit_walktoxy_timer");
	add_timer_func_list(unit_walktobl_sub, "unit_walktobl_sub");
	add_timer_func_list(unit_delay_walktoxy_timer,"unit_delay_walktoxy_timer");
	add_timer_func_list(unit_delay_walktobl_timer,"unit_delay_walktobl_timer");
	add_timer_func_list(unit_teleport_timer,"unit_teleport_timer");
	add_timer_func_list(unit_step_timer,"unit_step_timer");
	add_timer_func_list(unit_shadowscar_timer, "unit_shadowscar_timer");
}

/**
 * Unit module destructor, (thing to do before closing the module)
 * called in map::do_final
 * @return 0
 */
void do_final_unit(void){
	// Nothing to do
}
