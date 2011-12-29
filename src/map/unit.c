// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/showmsg.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "unit.h"
#include "map.h"
#include "path.h"
#include "pc.h"
#include "mob.h"
#include "pet.h"
#include "homunculus.h"
#include "instance.h"
#include "mercenary.h"
#include "skill.h"
#include "clif.h"
#include "duel.h"
#include "npc.h"
#include "guild.h"
#include "status.h"
#include "battle.h"
#include "battleground.h"
#include "chat.h"
#include "trade.h"
#include "vending.h"
#include "party.h"
#include "intif.h"
#include "chrif.h"
#include "script.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const short dirx[8]={0,-1,-1,-1,0,1,1,1};
const short diry[8]={1,1,0,-1,-1,-1,0,1};

struct unit_data* unit_bl2ud(struct block_list *bl)
{
	if( bl == NULL) return NULL;
	if( bl->type == BL_PC)  return &((struct map_session_data*)bl)->ud;
	if( bl->type == BL_MOB) return &((struct mob_data*)bl)->ud;
	if( bl->type == BL_PET) return &((struct pet_data*)bl)->ud;
	if( bl->type == BL_NPC) return &((struct npc_data*)bl)->ud;
	if( bl->type == BL_HOM) return &((struct homun_data*)bl)->ud;
	if( bl->type == BL_MER) return &((struct mercenary_data*)bl)->ud;
	return NULL;
}

static int unit_attack_timer(int tid, unsigned int tick, int id, intptr_t data);
static int unit_walktoxy_timer(int tid, unsigned int tick, int id, intptr_t data);

int unit_walktoxy_sub(struct block_list *bl)
{
	int i;
	struct walkpath_data wpd;
	struct unit_data *ud = NULL;

	nullpo_retr(1, bl);
	ud = unit_bl2ud(bl);
	if(ud == NULL) return 0;

	if( !path_search(&wpd,bl->m,bl->x,bl->y,ud->to_x,ud->to_y,ud->state.walk_easy,CELL_CHKNOPASS) )
		return 0;

	memcpy(&ud->walkpath,&wpd,sizeof(wpd));
	
	if (ud->target && ud->chaserange>1) {
		//Generally speaking, the walk path is already to an adjacent tile
		//so we only need to shorten the path if the range is greater than 1.
		int dir;
		//Trim the last part of the path to account for range,
		//but always move at least one cell when requested to move.
		for (i = ud->chaserange*10; i > 0 && ud->walkpath.path_len>1;) {
		   ud->walkpath.path_len--;
			dir = ud->walkpath.path[ud->walkpath.path_len];
			if(dir&1)
				i-=14;
			else
				i-=10;
			ud->to_x -= dirx[dir];
			ud->to_y -= diry[dir];
		}
	}

	ud->state.change_walk_target=0;

	if (bl->type == BL_PC) {
		((TBL_PC *)bl)->head_dir = 0;
		clif_walkok((TBL_PC*)bl);
	}
	clif_move(ud);

	if(ud->walkpath.path_pos>=ud->walkpath.path_len)
		i = -1;
	else if(ud->walkpath.path[ud->walkpath.path_pos]&1)
		i = status_get_speed(bl)*14/10;
	else
		i = status_get_speed(bl);
	if( i > 0)
		ud->walktimer = add_timer(gettick()+i,unit_walktoxy_timer,bl->id,i);
	return 1;
}

static int unit_walktoxy_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	int i;
	int x,y,dx,dy;
	uint8 dir;
	struct block_list       *bl;
	struct map_session_data *sd;
	struct mob_data         *md;
	struct unit_data        *ud;
	struct mercenary_data   *mrd;

	bl = map_id2bl(id);
	if(bl == NULL)
		return 0;
	sd = BL_CAST(BL_PC, bl);
	md = BL_CAST(BL_MOB, bl);
	mrd = BL_CAST(BL_MER, bl);
	ud = unit_bl2ud(bl);
	
	if(ud == NULL) return 0;

	if(ud->walktimer != tid){
		ShowError("unit_walk_timer mismatch %d != %d\n",ud->walktimer,tid);
		return 0;
	}
	ud->walktimer = INVALID_TIMER;
	if( bl->prev == NULL ) return 0; // block_list から抜けているので移動停止する

	if(ud->walkpath.path_pos>=ud->walkpath.path_len)
		return 0;

	if(ud->walkpath.path[ud->walkpath.path_pos]>=8)
		return 1;
	x = bl->x;
	y = bl->y;

	dir = ud->walkpath.path[ud->walkpath.path_pos];
	ud->dir = dir;

	dx = dirx[(int)dir];
	dy = diry[(int)dir];

	if(map_getcell(bl->m,x+dx,y+dy,CELL_CHKNOPASS))
		return unit_walktoxy_sub(bl);
	
	// バシリカ判定

	map_foreachinmovearea(clif_outsight, bl, AREA_SIZE, dx, dy, sd?BL_ALL:BL_PC, bl);

	x += dx;
	y += dy;
	map_moveblock(bl, x, y, tick);
	ud->walk_count++; //walked cell counter, to be used for walk-triggered skills. [Skotlex]

	if (bl->x != x || bl->y != y || ud->walktimer != INVALID_TIMER)
		return 0; //map_moveblock has altered the object beyond what we expected (moved/warped it)

	ud->walktimer = -2; // arbitrary non-INVALID_TIMER value to make the clif code send walking packets
	map_foreachinmovearea(clif_insight, bl, AREA_SIZE, -dx, -dy, sd?BL_ALL:BL_PC, bl);
	ud->walktimer = INVALID_TIMER;
	
	if(sd) {
		if( sd->touching_id )
			npc_touchnext_areanpc(sd,false);
		if(map_getcell(bl->m,x,y,CELL_CHKNPC)) {
			npc_touch_areanpc(sd,bl->m,x,y);
			if (bl->prev == NULL) //Script could have warped char, abort remaining of the function.
				return 0;
		} else
			sd->areanpc_id=0;

		if( sd->md && !check_distance_bl(&sd->bl, &sd->md->bl, MAX_MER_DISTANCE) )
		{// mercenary is too far from the master so warp the master's position
			unit_warp( &sd->md->bl, sd->bl.m, sd->bl.x, sd->bl.y, CLR_TELEPORT );
		}

		if (sd->state.gmaster_flag &&
			(battle_config.guild_aura&((agit_flag || agit2_flag)?2:1)) &&
			(battle_config.guild_aura&(map_flag_gvg2(bl->m)?8:4))
		)
		{ //Guild Aura: Likely needs to be recoded, this method seems inefficient.
			struct guild *g = sd->state.gmaster_flag;
			int skill, strvit= 0, agidex = 0;
			if ((skill = guild_checkskill(g, GD_LEADERSHIP)) > 0) strvit |= (skill&0xFFFF)<<16;
			if ((skill = guild_checkskill(g, GD_GLORYWOUNDS)) > 0) strvit |= (skill&0xFFFF);
			if ((skill = guild_checkskill(g, GD_SOULCOLD)) > 0) agidex |= (skill&0xFFFF)<<16;
			if ((skill = guild_checkskill(g, GD_HAWKEYES)) > 0) agidex |= skill&0xFFFF;
			if (strvit || agidex)
			{// replaced redundant foreachinrange call with smaller and much more efficient iteration
				for( i = 0; i < g->max_member; i++ )
				{
					if( g->member[i].online && g->member[i].sd && sd->bl.m == g->member[i].sd->bl.m && check_distance_bl(&sd->bl, &g->member[i].sd->bl, 2) )
					{// perform the aura on the member as appropriate
						skill_guildaura_sub(g->member[i].sd, sd->bl.id, strvit, agidex);
					}
				}
			}
		}
	} else if (md) {
		if( map_getcell(bl->m,x,y,CELL_CHKNPC) ) {
			if( npc_touch_areanpc2(md) ) return 0; // Warped
		} else
			md->areanpc_id = 0;
		if (md->min_chase > md->db->range3) md->min_chase--;
		//Walk skills are triggered regardless of target due to the idle-walk mob state.
		//But avoid triggering on stop-walk calls.
		if(tid != INVALID_TIMER &&
			!(ud->walk_count%WALK_SKILL_INTERVAL) &&
			mobskill_use(md, tick, -1))
	  	{
			if (!(ud->skillid == NPC_SELFDESTRUCTION && ud->skilltimer != INVALID_TIMER))
			{	//Skill used, abort walking
				clif_fixpos(bl); //Fix position as walk has been cancelled.
				return 0;
			}
			//Resend walk packet for proper Self Destruction display.
			clif_move(ud);
		}
	}
	else if( mrd && mrd->master && !check_distance_bl(&mrd->master->bl, bl, MAX_MER_DISTANCE) )
	{// mercenary is too far from the master so warp the master's position
		unit_warp( bl, mrd->master->bl.id, mrd->master->bl.x, mrd->master->bl.y, CLR_TELEPORT );
	}

	if(tid == INVALID_TIMER) //A directly invoked timer is from battle_stop_walking, therefore the rest is irrelevant.
		return 0;
		
	if(ud->state.change_walk_target)
		return unit_walktoxy_sub(bl);

	ud->walkpath.path_pos++;
	if(ud->walkpath.path_pos>=ud->walkpath.path_len)
		i = -1;
	else if(ud->walkpath.path[ud->walkpath.path_pos]&1)
		i = status_get_speed(bl)*14/10;
	else
		i = status_get_speed(bl);

	if(i > 0) {
		ud->walktimer = add_timer(tick+i,unit_walktoxy_timer,id,i);
		if( md )
			clif_move(ud);
	} else if(ud->state.running) {
		//Keep trying to run.
		if (!unit_run(bl))
			ud->state.running = 0;
	}
	else if (ud->target) {
		//Update target trajectory.
		struct block_list *tbl = map_id2bl(ud->target);
		if (!tbl || !status_check_visibility(bl, tbl)) {	//Cancel chase.
			ud->to_x = bl->x;
			ud->to_y = bl->y;
			if (tbl && bl->type == BL_MOB) //See if the mob can do a warp chase.
				mob_warpchase((TBL_MOB*)bl, tbl);
			return 0;
		}
		if (tbl->m == bl->m && check_distance_bl(bl, tbl, ud->chaserange))
		{	//Reached destination.
			if (ud->state.attack_continue)
			{	//Aegis uses one before every attack, we should
				//only need this one for syncing purposes. [Skotlex]
				clif_fixpos(bl);
				unit_attack(bl, tbl->id, ud->state.attack_continue);
			}
		} else { //Update chase-path
			unit_walktobl(bl, tbl, ud->chaserange, ud->state.walk_easy|(ud->state.attack_continue?2:0));
			return 0;
		}
	}
	else {	//Stopped walking. Update to_x and to_y to current location [Skotlex]
		ud->to_x = bl->x;
		ud->to_y = bl->y;
	}
	return 0;
}

static int unit_delay_walktoxy_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list *bl = map_id2bl(id);

	if (!bl || bl->prev == NULL)
		return 0;
	unit_walktoxy(bl, (short)((data>>16)&0xffff), (short)(data&0xffff), 0);
	return 1;
}

//flag parameter:
//&1 -> 1/0 = easy/hard
//&2 -> force walking
//&4 -> Delay walking if the reason you can't walk is the canwalk delay
int unit_walktoxy( struct block_list *bl, short x, short y, int flag)
{
	struct unit_data* ud = NULL;
	struct status_change* sc = NULL;

	nullpo_ret(bl);
	
	ud = unit_bl2ud(bl);
	
	if( ud == NULL) return 0;

	if (flag&4 && DIFF_TICK(ud->canmove_tick, gettick()) > 0 &&
		DIFF_TICK(ud->canmove_tick, gettick()) < 2000)
  	{	// Delay walking command. [Skotlex]
		add_timer(ud->canmove_tick+1, unit_delay_walktoxy_timer, bl->id, (x<<16)|(y&0xFFFF));
		return 1;
	}

	if(!(flag&2) && (!(status_get_mode(bl)&MD_CANMOVE) || !unit_can_move(bl)))
		return 0;
	
	ud->state.walk_easy = flag&1;
	ud->target = 0;
	ud->to_x = x;
	ud->to_y = y;
	
	sc = status_get_sc(bl);
	if (sc && sc->data[SC_CONFUSION]) //Randomize the target position
		map_random_dir(bl, &ud->to_x, &ud->to_y);

	if(ud->walktimer != INVALID_TIMER) {
		// 現在歩いている最中の目的地変更なのでマス目の中心に来た時に
		// timer関数からunit_walktoxy_subを呼ぶようにする
		ud->state.change_walk_target = 1;
		return 1;
	}

	if(ud->attacktimer != INVALID_TIMER) {
		delete_timer( ud->attacktimer, unit_attack_timer );
		ud->attacktimer = INVALID_TIMER;
	}

	return unit_walktoxy_sub(bl);
}

//To set Mob's CHASE/FOLLOW states (shouldn't be done if there's no path to reach)
static inline void set_mobstate(struct block_list* bl, int flag)
{
	struct mob_data* md = BL_CAST(BL_MOB,bl);

	if( md && flag )
		md->state.skillstate = md->state.aggressive ? MSS_FOLLOW : MSS_RUSH;
}

static int unit_walktobl_sub(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list *bl = map_id2bl(id);
	struct unit_data *ud = bl?unit_bl2ud(bl):NULL;

	if (ud && ud->walktimer == INVALID_TIMER && ud->target == data)
	{
		if (DIFF_TICK(ud->canmove_tick, tick) > 0) //Keep waiting?
			add_timer(ud->canmove_tick+1, unit_walktobl_sub, id, data);
		else if (unit_can_move(bl))
		{
			if (unit_walktoxy_sub(bl))
				set_mobstate(bl, ud->state.attack_continue);
		}
	}
	return 0;	
}

// Chases a tbl. If the flag&1, use hard-path seek,
// if flag&2, start attacking upon arrival within range, otherwise just walk to that character.
int unit_walktobl(struct block_list *bl, struct block_list *tbl, int range, int flag)
{
	struct unit_data        *ud = NULL;
	struct status_change		*sc = NULL;
	nullpo_ret(bl);
	nullpo_ret(tbl);
	
	ud = unit_bl2ud(bl);
	if( ud == NULL) return 0;

	if (!(status_get_mode(bl)&MD_CANMOVE))
		return 0;
	
	if (!unit_can_reach_bl(bl, tbl, distance_bl(bl, tbl)+1, flag&1, &ud->to_x, &ud->to_y)) {
		ud->to_x = bl->x;
		ud->to_y = bl->y;
		return 0;
	}

	ud->state.walk_easy = flag&1;
	ud->target = tbl->id;
	ud->chaserange = range; //Note that if flag&2, this SHOULD be attack-range
	ud->state.attack_continue = flag&2?1:0; //Chase to attack.

	sc = status_get_sc(bl);
	if (sc && sc->data[SC_CONFUSION]) //Randomize the target position
		map_random_dir(bl, &ud->to_x, &ud->to_y);

	if(ud->walktimer != INVALID_TIMER) {
		ud->state.change_walk_target = 1;
		set_mobstate(bl, flag&2);
		return 1;
	}

	if(DIFF_TICK(ud->canmove_tick, gettick()) > 0)
	{	//Can't move, wait a bit before invoking the movement.
		add_timer(ud->canmove_tick+1, unit_walktobl_sub, bl->id, ud->target);
		return 1;
	}

	if(!unit_can_move(bl))
		return 0;

	if(ud->attacktimer != INVALID_TIMER) {
		delete_timer( ud->attacktimer, unit_attack_timer );
		ud->attacktimer = INVALID_TIMER;
	}

	if (unit_walktoxy_sub(bl)) {
		set_mobstate(bl, flag&2);
		return 1;
	}
	return 0;
}
#undef set_mobstate

int unit_run(struct block_list *bl)
{
	struct status_change *sc = status_get_sc(bl);
	short to_x,to_y,dir_x,dir_y;
	int lv;
	int i;

	if (!(sc && sc->data[SC_RUN]))
		return 0;
	
	if (!unit_can_move(bl)) {
		status_change_end(bl, SC_RUN, INVALID_TIMER);
		return 0;
	}
	
	lv = sc->data[SC_RUN]->val1;
	dir_x = dirx[sc->data[SC_RUN]->val2];
	dir_y = diry[sc->data[SC_RUN]->val2];

	// determine destination cell
	to_x = bl->x;
	to_y = bl->y;
	for(i=0;i<AREA_SIZE;i++)
	{
		if(!map_getcell(bl->m,to_x+dir_x,to_y+dir_y,CELL_CHKPASS))
			break;

		//if sprinting and there's a PC/Mob/NPC, block the path [Kevin]
		if(sc->data[SC_RUN] && map_count_oncell(bl->m, to_x+dir_x, to_y+dir_y, BL_PC|BL_MOB|BL_NPC))
			break;
			
		to_x += dir_x;
		to_y += dir_y;
	}

	if( (to_x == bl->x && to_y == bl->y ) || (to_x == (bl->x+1) || to_y == (bl->y+1)) || (to_x == (bl->x-1) || to_y == (bl->y-1))) {
		//If you can't run forward, you must be next to a wall, so bounce back. [Skotlex]
		clif_status_change(bl, SI_BUMP, 1, 0, 0, 0, 0);

		//Set running to 0 beforehand so status_change_end knows not to enable spurt [Kevin]
		unit_bl2ud(bl)->state.running = 0;
		status_change_end(bl, SC_RUN, INVALID_TIMER);

		skill_blown(bl,bl,skill_get_blewcount(TK_RUN,lv),unit_getdir(bl),0);
		clif_fixpos(bl); //Why is a clif_slide (skill_blown) AND a fixpos needed? Ask Aegis.
		clif_status_change(bl, SI_BUMP, 0, 0, 0, 0, 0);
		return 0;
	}
	if (unit_walktoxy(bl, to_x, to_y, 1))
		return 1;
	//There must be an obstacle nearby. Attempt walking one cell at a time.
	do {
		to_x -= dir_x;
		to_y -= dir_y;
	} while (--i > 0 && !unit_walktoxy(bl, to_x, to_y, 1));
	if (i==0) {
		// copy-paste from above
		clif_status_change(bl, SI_BUMP, 1, 0, 0, 0, 0);

		//Set running to 0 beforehand so status_change_end knows not to enable spurt [Kevin]
		unit_bl2ud(bl)->state.running = 0;
		status_change_end(bl, SC_RUN, INVALID_TIMER);

		skill_blown(bl,bl,skill_get_blewcount(TK_RUN,lv),unit_getdir(bl),0);
		clif_fixpos(bl);
		clif_status_change(bl, SI_BUMP, 0, 0, 0, 0, 0);
		return 0;
	}
	return 1;
}

//Exclusive function to Wug Dash state. [Jobbie/3CeAM]
int unit_wugdash(struct block_list *bl, struct map_session_data *sd) {
	struct status_change *sc = status_get_sc(bl);
	short to_x,to_y,dir_x,dir_y;
	int lv;
	int i;
	if (!(sc && sc->data[SC_WUGDASH]))
		return 0;
	
	nullpo_ret(sd);
	nullpo_ret(bl);

	if (!unit_can_move(bl)) {
		status_change_end(bl,SC_WUGDASH,-1);
		return 0;
	}
	
	lv = sc->data[SC_WUGDASH]->val1;
	dir_x = dirx[sc->data[SC_WUGDASH]->val2];
	dir_y = diry[sc->data[SC_WUGDASH]->val2];

	to_x = bl->x;
	to_y = bl->y;
	for(i=0;i<AREA_SIZE;i++)
	{
		if(!map_getcell(bl->m,to_x+dir_x,to_y+dir_y,CELL_CHKPASS))
			break;

		if(sc->data[SC_WUGDASH] && map_count_oncell(bl->m, to_x+dir_x, to_y+dir_y, BL_PC|BL_MOB|BL_NPC))
			break;
			
		to_x += dir_x;
		to_y += dir_y;
	}

	if(to_x == bl->x && to_y == bl->y) {

		unit_bl2ud(bl)->state.running = 0;
		status_change_end(bl,SC_WUGDASH,-1);

		if( sd ){
			clif_fixpos(bl);
			skill_castend_damage_id(bl, &sd->bl, RA_WUGDASH, lv, gettick(), SD_LEVEL);
		}
		return 0;
	}
	if (unit_walktoxy(bl, to_x, to_y, 1))
		return 1;
	do {
		to_x -= dir_x;
		to_y -= dir_y;
	} while (--i > 0 && !unit_walktoxy(bl, to_x, to_y, 1));
	if (i==0) {

		unit_bl2ud(bl)->state.running = 0;
		status_change_end(bl,SC_WUGDASH,-1);
		
		if( sd ){
			clif_fixpos(bl);
			skill_castend_damage_id(bl, &sd->bl, RA_WUGDASH, lv, gettick(), SD_LEVEL);
		}
		return 0;
	}
	return 1;
}

//Makes bl attempt to run dist cells away from target. Uses hard-paths.
int unit_escape(struct block_list *bl, struct block_list *target, short dist)
{
	int dir = map_calc_dir(target, bl->x, bl->y);
	while( dist > 0 && map_getcell(bl->m, bl->x + dist*dirx[dir], bl->y + dist*diry[dir], CELL_CHKNOREACH) )
		dist--;
	return ( dist > 0 && unit_walktoxy(bl, bl->x + dist*dirx[dir], bl->y + dist*diry[dir], 0) );
}

//Instant warp function.
int unit_movepos(struct block_list *bl, short dst_x, short dst_y, int easy, bool checkpath)
{
	short dx,dy;
	uint8 dir;
	struct unit_data        *ud = NULL;
	struct map_session_data *sd = NULL;

	nullpo_ret(bl);
	sd = BL_CAST(BL_PC, bl);
	ud = unit_bl2ud(bl);

	if( ud == NULL) return 0;

	unit_stop_walking(bl,1);
	unit_stop_attack(bl);

	if( checkpath && (map_getcell(bl->m,dst_x,dst_y,CELL_CHKNOPASS) || !path_search(NULL,bl->m,bl->x,bl->y,dst_x,dst_y,easy,CELL_CHKNOREACH)) )
		return 0; // unreachable

	dir = map_calc_dir(bl, dst_x,dst_y);
	ud->dir = dir;

	dx = dst_x - bl->x;
	dy = dst_y - bl->y;

	map_foreachinmovearea(clif_outsight, bl, AREA_SIZE, dx, dy, sd?BL_ALL:BL_PC, bl);

	map_moveblock(bl, dst_x, dst_y, gettick());
	
	ud->walktimer = -2; // arbitrary non-INVALID_TIMER value to make the clif code send walking packets
	map_foreachinmovearea(clif_insight, bl, AREA_SIZE, -dx, -dy, sd?BL_ALL:BL_PC, bl);
	ud->walktimer = INVALID_TIMER;
		
	if(sd) {
		if( sd->touching_id )
			npc_touchnext_areanpc(sd,false);
		if(map_getcell(bl->m,bl->x,bl->y,CELL_CHKNPC)) {
			npc_touch_areanpc(sd,bl->m,bl->x,bl->y);
			if (bl->prev == NULL) //Script could have warped char, abort remaining of the function.
				return 0;
		} else
			sd->areanpc_id=0;

		if( sd->status.pet_id > 0 && sd->pd && sd->pd->pet.intimate > 0 )
		{ // Check if pet needs to be teleported. [Skotlex]
			int flag = 0;
			struct block_list* bl = &sd->pd->bl;
			if( !checkpath && !path_search(NULL,bl->m,bl->x,bl->y,dst_x,dst_y,0,CELL_CHKNOPASS) )
				flag = 1;
			else if (!check_distance_bl(&sd->bl, bl, AREA_SIZE)) //Too far, teleport.
				flag = 2;
			if( flag )
			{
				unit_movepos(bl,sd->bl.x,sd->bl.y, 0, 0);
				clif_slide(bl,bl->x,bl->y);
			}
		}
	}
	return 1;
}

int unit_setdir(struct block_list *bl,unsigned char dir)
{
	struct unit_data *ud;
	nullpo_ret(bl );
	ud = unit_bl2ud(bl);
	if (!ud) return 0;
	ud->dir = dir;
	if (bl->type == BL_PC) 
		((TBL_PC *)bl)->head_dir = 0;
	clif_changed_dir(bl, AREA);
	return 0;
}

uint8 unit_getdir(struct block_list *bl)
{
	struct unit_data *ud;
	nullpo_ret(bl );
	ud = unit_bl2ud(bl);
	if (!ud) return 0;
	return ud->dir;
}

// Pushes a unit by given amount of cells into given direction. Only
// map cell restrictions are respected.
// flag:
//  &1  Do not send position update packets.
int unit_blown(struct block_list* bl, int dx, int dy, int count, int flag)
{
	int nx, ny, result;
	struct map_session_data* sd;
	struct skill_unit* su = NULL;

	if(count)
	{
		sd = BL_CAST(BL_PC, bl);
		su = BL_CAST(BL_SKILL, bl);

		result = path_blownpos(bl->m, bl->x, bl->y, dx, dy, count);

		nx = result>>16;
		ny = result&0xffff;

		if(!su)
		{
			unit_stop_walking(bl, 0);
		}

		dx = nx-bl->x;
		dy = ny-bl->y;

		if(dx || dy)
		{
			map_foreachinmovearea(clif_outsight, bl, AREA_SIZE, dx, dy, bl->type == BL_PC ? BL_ALL : BL_PC, bl);

			if(su)
			{
				skill_unit_move_unit_group(su->group, bl->m, dx, dy);
			}
			else
			{
				map_moveblock(bl, nx, ny, gettick());
			}

			map_foreachinmovearea(clif_insight, bl, AREA_SIZE, -dx, -dy, bl->type == BL_PC ? BL_ALL : BL_PC, bl);

			if(!(flag&1))
			{
				clif_blown(bl);
			}

			if(sd)
			{
				if(sd->touching_id)
				{
					npc_touchnext_areanpc(sd, false);
				}
				if(map_getcell(bl->m, bl->x, bl->y, CELL_CHKNPC))
				{
					npc_touch_areanpc(sd, bl->m, bl->x, bl->y);
				}
				else
				{
					sd->areanpc_id = 0;
				}
			}
		}

		count = distance(dx, dy);
	}

	return count;  // return amount of knocked back cells
}

//Warps a unit/ud to a given map/position. 
//In the case of players, pc_setpos is used.
//it respects the no warp flags, so it is safe to call this without doing nowarpto/nowarp checks.
int unit_warp(struct block_list *bl,short m,short x,short y,clr_type type)
{
	struct unit_data *ud;
	nullpo_ret(bl);
	ud = unit_bl2ud(bl);
	
	if(bl->prev==NULL || !ud)
		return 1;

	if (type == CLR_DEAD)
		//Type 1 is invalid, since you shouldn't warp a bl with the "death" 
		//animation, it messes up with unit_remove_map! [Skotlex]
		return 1;
	
	if( m<0 ) m=bl->m;
	
	switch (bl->type) {
		case BL_MOB:
			if (map[bl->m].flag.monster_noteleport && ((TBL_MOB*)bl)->master_id == 0)
				return 1;
			if (m != bl->m && map[m].flag.nobranch && battle_config.mob_warp&4)
				return 1;
			break;
		case BL_PC:
			if (map[bl->m].flag.noteleport)
				return 1;
			break;
	}
	
	if (x<0 || y<0)
  	{	//Random map position.
		if (!map_search_freecell(NULL, m, &x, &y, -1, -1, 1)) {
			ShowWarning("unit_warp failed. Unit Id:%d/Type:%d, target position map %d (%s) at [%d,%d]\n", bl->id, bl->type, m, map[m].name, x, y);
			return 2;
			
		}
	} else if (map_getcell(m,x,y,CELL_CHKNOREACH))
	{	//Invalid target cell
		ShowWarning("unit_warp: Specified non-walkable target cell: %d (%s) at [%d,%d]\n", m, map[m].name, x,y);
		
		if (!map_search_freecell(NULL, m, &x, &y, 4, 4, 1))
	 	{	//Can't find a nearby cell
			ShowWarning("unit_warp failed. Unit Id:%d/Type:%d, target position map %d (%s) at [%d,%d]\n", bl->id, bl->type, m, map[m].name, x, y);
			return 2;
		}
	}

	if (bl->type == BL_PC) //Use pc_setpos
		return pc_setpos((TBL_PC*)bl, map_id2index(m), x, y, type);
	
	if (!unit_remove_map(bl, type))
		return 3;
	
	if (bl->m != m && battle_config.clear_unit_onwarp &&
		battle_config.clear_unit_onwarp&bl->type)
		skill_clear_unitgroup(bl);

	bl->x=ud->to_x=x;
	bl->y=ud->to_y=y;
	bl->m=m;

	map_addblock(bl);
	clif_spawn(bl);
	skill_unit_move(bl,gettick(),1);

	return 0;
}

/*==========================================
 * Caused the target object to stop moving.
 * Flag values:
 * &0x1: Issue a fixpos packet afterwards
 * &0x2: Force the unit to move one cell if it hasn't yet
 * &0x4: Enable moving to the next cell when unit was already half-way there
 *       (may cause on-touch/place side-effects, such as a scripted map change)
 *------------------------------------------*/
int unit_stop_walking(struct block_list *bl,int type)
{
	struct unit_data *ud;
	const struct TimerData* td;
	unsigned int tick;
	nullpo_ret(bl);

	ud = unit_bl2ud(bl);
	if(!ud || ud->walktimer == INVALID_TIMER)
		return 0;
	//NOTE: We are using timer data after deleting it because we know the 
	//delete_timer function does not messes with it. If the function's 
	//behaviour changes in the future, this code could break!
	td = get_timer(ud->walktimer);
	delete_timer(ud->walktimer, unit_walktoxy_timer);
	ud->walktimer = INVALID_TIMER;
	ud->state.change_walk_target = 0;
	tick = gettick();
	if( (type&0x02 && !ud->walkpath.path_pos) //Force moving at least one cell.
	||  (type&0x04 && td && DIFF_TICK(td->tick, tick) <= td->data/2) //Enough time has passed to cover half-cell
	) {	
		ud->walkpath.path_len = ud->walkpath.path_pos+1;
		unit_walktoxy_timer(INVALID_TIMER, tick, bl->id, ud->walkpath.path_pos);
	}

	if(type&0x01)
		clif_fixpos(bl);
	
	ud->walkpath.path_len = 0;
	ud->walkpath.path_pos = 0;
	ud->to_x = bl->x;
	ud->to_y = bl->y;
	if(bl->type == BL_PET && type&~0xff)
		ud->canmove_tick = gettick() + (type>>8);

	//Readded, the check in unit_set_walkdelay means dmg during running won't fall through to this place in code [Kevin]
	if (ud->state.running)
		status_change_end(bl, SC_RUN, INVALID_TIMER);
	return 1;
}

int unit_skilluse_id(struct block_list *src, int target_id, short skill_num, short skill_lv)
{
	if(skill_num < 0) return 0;

	return unit_skilluse_id2(
		src, target_id, skill_num, skill_lv,
		skill_castfix(src, skill_num, skill_lv),
		skill_get_castcancel(skill_num)
	);
}

int unit_is_walking(struct block_list *bl)
{
	struct unit_data *ud = unit_bl2ud(bl);
	nullpo_ret(bl);
	if(!ud) return 0;
	return (ud->walktimer != INVALID_TIMER);
}

/*==========================================
 * Determines if the bl can move based on status changes. [Skotlex]
 *------------------------------------------*/
int unit_can_move(struct block_list *bl)
{
	struct map_session_data *sd;
	struct unit_data *ud;
	struct status_change *sc;
	
	nullpo_ret(bl);
	ud = unit_bl2ud(bl);
	sc = status_get_sc(bl);
	sd = BL_CAST(BL_PC, bl);
	
	if (!ud)
		return 0;
	
	if (ud->skilltimer != INVALID_TIMER && (!sd || !pc_checkskill(sd, SA_FREECAST) || skill_get_inf2(ud->skillid)&INF2_GUILD_SKILL))
		return 0; // prevent moving while casting
	
	if (DIFF_TICK(ud->canmove_tick, gettick()) > 0)
		return 0;
	
	if (sd && (
		pc_issit(sd) ||
		sd->state.vending ||
		sd->state.buyingstore ||
		sd->state.blockedmove
	))
		return 0; //Can't move
	
	if (sc) {
		if (sc->opt1 > 0 && sc->opt1 != OPT1_STONEWAIT && sc->opt1 != OPT1_BURNING)
			return 0;

		if ((sc->option & OPTION_HIDE) && (!sd || pc_checkskill(sd, RG_TUNNELDRIVE) <= 0))
			return 0;

		if (sc->count && (
			sc->data[SC_ANKLE]
			|| sc->data[SC_AUTOCOUNTER]
			|| sc->data[SC_TRICKDEAD]
			|| sc->data[SC_BLADESTOP]
			|| sc->data[SC_BLADESTOP_WAIT]
			|| (sc->data[SC_SPIDERWEB] && sc->data[SC_SPIDERWEB]->val1)
			|| (sc->data[SC_DANCING] && sc->data[SC_DANCING]->val4 && (
				!sc->data[SC_LONGING] ||
				(sc->data[SC_DANCING]->val1&0xFFFF) == CG_MOONLIT ||
				(sc->data[SC_DANCING]->val1&0xFFFF) == CG_HERMODE
			))
			|| (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF)	// cannot move while gospel is in effect
			|| (sc->data[SC_BASILICA] && sc->data[SC_BASILICA]->val4 == bl->id) // Basilica caster cannot move
			|| sc->data[SC_STOP]
			|| sc->data[SC_CLOSECONFINE]
			|| sc->data[SC_CLOSECONFINE2]
			|| (sc->data[SC_CLOAKING] && //Need wall at level 1-2
				sc->data[SC_CLOAKING]->val1 < 3 && !(sc->data[SC_CLOAKING]->val4&1))
			|| sc->data[SC_MADNESSCANCEL]
			|| (sc->data[SC_GRAVITATION] && sc->data[SC_GRAVITATION]->val3 == BCT_SELF)
		))
			return 0;
	}
	return 1;
}

/*==========================================
 * Resume running after a walk delay
 *------------------------------------------*/

int unit_resume_running(int tid, unsigned int tick, int id, intptr_t data)
{

	struct unit_data *ud = (struct unit_data *)data;
	TBL_PC * sd = map_id2sd(id);

	clif_skill_nodamage(ud->bl,ud->bl,TK_RUN,ud->skilllv,
			sc_start4(ud->bl,status_skill2sc(TK_RUN),100,ud->skilllv,unit_getdir(ud->bl),0,0,0));

	if (sd) clif_walkok(sd);

	return 0;

}


/*==========================================
 * Applies walk delay to character, considering that 
 * if type is 0, this is a damage induced delay: if previous delay is active, do not change it.
 * if type is 1, this is a skill induced delay: walk-delay may only be increased, not decreased.
 *------------------------------------------*/
int unit_set_walkdelay(struct block_list *bl, unsigned int tick, int delay, int type)
{
	struct unit_data *ud = unit_bl2ud(bl);
	if (delay <= 0 || !ud) return 0;
	
	if (type) {
		if (DIFF_TICK(ud->canmove_tick, tick+delay) > 0)
			return 0;
	} else {
		//Don't set walk delays when already trapped.
		if (!unit_can_move(bl))
			return 0;
	}
	ud->canmove_tick = tick + delay;
	if (ud->walktimer != INVALID_TIMER)
	{	//Stop walking, if chasing, readjust timers.
		if (delay == 1)
		{	//Minimal delay (walk-delay) disabled. Just stop walking.
			unit_stop_walking(bl,4);
		} else {
			//Resume running after can move again [Kevin]
			if(ud->state.running)
			{
				add_timer(ud->canmove_tick, unit_resume_running, bl->id, (intptr_t)ud);
			}
			else
			{
				unit_stop_walking(bl,2|4);
				if(ud->target)
					add_timer(ud->canmove_tick+1, unit_walktobl_sub, bl->id, ud->target);
			}
		}
	}
	return 1;
}

int unit_skilluse_id2(struct block_list *src, int target_id, short skill_num, short skill_lv, int casttime, int castcancel)
{
	struct unit_data *ud;
	struct status_data *tstatus;
	struct status_change *sc;
	struct map_session_data *sd = NULL;
	struct block_list * target = NULL;
	unsigned int tick = gettick();
	int temp = 0;

	nullpo_ret(src);
	if(status_isdead(src))
		return 0; // 死んでいないか

	sd = BL_CAST(BL_PC, src);
	ud = unit_bl2ud(src);

	if(ud == NULL) return 0;
	sc = status_get_sc(src);	
	if (sc && !sc->count)
		sc = NULL; //Unneeded

	//temp: used to signal combo-skills right now.
	if (sc && sc->data[SC_COMBO] && (sc->data[SC_COMBO]->val1 == skill_num || skill_num == MO_EXTREMITYFIST))
	{
		if (sc->data[SC_COMBO]->val2)
			target_id = sc->data[SC_COMBO]->val2;
		else
			target_id = ud->target;
		temp = 1;
	} else
	if ( target_id == src->id &&
		skill_get_inf(skill_num)&INF_SELF_SKILL &&
		skill_get_inf2(skill_num)&INF2_NO_TARGET_SELF )
	{
		target_id = ud->target; //Auto-select target. [Skotlex]
		temp = 1;
	}

	if (sd) {
		//Target_id checking.
		if(skillnotok(skill_num, sd)) // [MouseJstr]
			return 0;

		switch(skill_num)
		{	//Check for skills that auto-select target
		case MO_CHAINCOMBO:
			if (sc && sc->data[SC_BLADESTOP]){
				if ((target=map_id2bl(sc->data[SC_BLADESTOP]->val4)) == NULL)
					return 0;
			}
			break;
		case WE_MALE:
		case WE_FEMALE:
			if (!sd->status.partner_id)
				return 0;
			target = (struct block_list*)map_charid2sd(sd->status.partner_id);
			if (!target) {
				clif_skill_fail(sd,skill_num,0,0);
				return 0;
			}
			break;
		}
		if (target)
			target_id = target->id;
	}
	if (src->type==BL_HOM)
	switch(skill_num)
	{ //Homun-auto-target skills.
		case HLIF_HEAL:
		case HLIF_AVOID:
		case HAMI_DEFENCE:
		case HAMI_CASTLE:
			target = battle_get_master(src);
			if (!target) return 0;
			target_id = target->id;
	}

	if( !target ) // choose default target
		target = map_id2bl(target_id);

	if( !target || src->m != target->m || !src->prev || !target->prev )
		return 0;

	if( battle_config.ksprotection && sd && mob_ksprotected(src, target) )
		return 0;

	//Normally not needed because clif.c checks for it, but the at/char/script commands don't! [Skotlex]
	if(ud->skilltimer != INVALID_TIMER && skill_num != SA_CASTCANCEL)
		return 0;

	if(skill_get_inf2(skill_num)&INF2_NO_TARGET_SELF && src->id == target_id)
		return 0;

	if(!status_check_skilluse(src, target, skill_num, 0))
		return 0;

	tstatus = status_get_status_data(target);
	//直前のスキル状況の記録
	if(sd) {
		switch(skill_num){
		case SA_CASTCANCEL:
			if(ud->skillid != skill_num){
				sd->skillid_old = ud->skillid;
				sd->skilllv_old = ud->skilllv;
			}
			break;
		case BD_ENCORE:
			//Prevent using the dance skill if you no longer have the skill in your tree. 
			if(!sd->skillid_dance || pc_checkskill(sd,sd->skillid_dance)<=0){
				clif_skill_fail(sd,skill_num,0,0);
				return 0;
			}
			sd->skillid_old = skill_num;
			break;
		case BD_LULLABY:
		case BD_RICHMANKIM:
		case BD_ETERNALCHAOS:
		case BD_DRUMBATTLEFIELD:
		case BD_RINGNIBELUNGEN:
		case BD_ROKISWEIL:
		case BD_INTOABYSS:
		case BD_SIEGFRIED:
		case CG_MOONLIT:
			if (skill_check_pc_partner(sd, skill_num, &skill_lv, 1, 0) < 1)
			{
				clif_skill_fail(sd,skill_num,0,0);
				return 0;
			}
			break;
		}
		if (!skill_check_condition_castbegin(sd, skill_num, skill_lv))
			return 0;
	}

	if( src->type == BL_MOB )
		switch( skill_num )
		{
			case NPC_SUMMONSLAVE:
			case NPC_SUMMONMONSTER:
			case AL_TELEPORT:
				if( ((TBL_MOB*)src)->master_id && ((TBL_MOB*)src)->special_state.ai )
					return 0;
		}

	//Check range when not using skill on yourself or is a combo-skill during attack
	//(these are supposed to always have the same range as your attack)
	if( src->id != target_id && (!temp || ud->attacktimer == INVALID_TIMER) )
	{
		if( skill_get_state(ud->skillid) == ST_MOVE_ENABLE )
		{
			if( !unit_can_reach_bl(src, target, skill_get_range2(src, skill_num,skill_lv) + 1, 1, NULL, NULL) )
				return 0; // Walk-path check failed.
		}
		else if( src->type == BL_MER && skill_num == MA_REMOVETRAP )
		{
			if( !battle_check_range(battle_get_master(src), target, skill_get_range2(src, skill_num, skill_lv) + 1) )
				return 0; // Aegis calc remove trap based on Master position, ignoring mercenary O.O
		}
		else if( !battle_check_range(src, target, skill_get_range2(src, skill_num,skill_lv) + (skill_num == RG_CLOSECONFINE?0:1)) )
			return 0; // Arrow-path check failed.
	}

	if (!temp) //Stop attack on non-combo skills [Skotlex]
		unit_stop_attack(src);
	else if(ud->attacktimer != INVALID_TIMER) //Elsewise, delay current attack sequence
		ud->attackabletime = tick + status_get_adelay(src);
	
	ud->state.skillcastcancel = castcancel;

	//temp: Used to signal force cast now.
	temp = 0;
	
	switch(skill_num){
	case ALL_RESURRECTION:
		if(battle_check_undead(tstatus->race,tstatus->def_ele)) {	
			temp = 1;
		} else if (!status_isdead(target))
			return 0; //Can't cast on non-dead characters.
	break;
	case MO_FINGEROFFENSIVE:
		if(sd)
			casttime += casttime * min(skill_lv, sd->spiritball);
	break;
	case MO_EXTREMITYFIST:
		if (sc && sc->data[SC_COMBO] &&
		   (sc->data[SC_COMBO]->val1 == MO_COMBOFINISH ||
			sc->data[SC_COMBO]->val1 == CH_TIGERFIST ||
			sc->data[SC_COMBO]->val1 == CH_CHAINCRUSH))
			casttime = 0;
		temp = 1;
	break;
	case SA_SPELLBREAKER:
		temp = 1;
	break;
	case ST_CHASEWALK:
		if (sc && sc->data[SC_CHASEWALK])
			casttime = 0;
	break;
	case TK_RUN:
		if (sc && sc->data[SC_RUN])
			casttime = 0;
	break;
	case HP_BASILICA:
		if( sc && sc->data[SC_BASILICA] )
			casttime = 0; // No Casting time on basilica cancel
	break;
	case KN_CHARGEATK:
		{
		unsigned int k = (distance_bl(src,target)-1)/3; //+100% every 3 cells of distance
		if( k > 2 ) k = 2; // ...but hard-limited to 300%.
		casttime += casttime * k; 
		}
	break;	
	case GD_EMERGENCYCALL: //Emergency Call double cast when the user has learned Leap [Daegaladh]
		if( sd && pc_checkskill(sd,TK_HIGHJUMP) )
			casttime *= 2;
		break;
	}
  	
	// moved here to prevent Suffragium from ending if skill fails
	if (!(skill_get_castnodex(skill_num, skill_lv)&2))
		casttime = skill_castfix_sc(src, casttime, skill_num, skill_lv);

	if( casttime > 0 || temp )
	{ 
		unit_stop_walking(src,1);
		clif_skillcasting(src, src->id, target_id, 0,0, skill_num, skill_get_ele(skill_num, skill_lv), casttime);

		if (sd && target->type == BL_MOB)
		{
			TBL_MOB *md = (TBL_MOB*)target;
			mobskill_event(md, src, tick, -1); //Cast targetted skill event.
			if (tstatus->mode&(MD_CASTSENSOR_IDLE|MD_CASTSENSOR_CHASE) &&
				battle_check_target(target, src, BCT_ENEMY) > 0)
			{
				switch (md->state.skillstate) {
				case MSS_RUSH:
				case MSS_FOLLOW:
					if (!(tstatus->mode&MD_CASTSENSOR_CHASE))
						break;
					md->target_id = src->id;
					md->state.aggressive = (tstatus->mode&MD_ANGRY)?1:0;
					md->min_chase = md->db->range3;
					break;
				case MSS_IDLE:
				case MSS_WALK:
					if (!(tstatus->mode&MD_CASTSENSOR_IDLE))
						break;
					md->target_id = src->id;
					md->state.aggressive = (tstatus->mode&MD_ANGRY)?1:0;
					md->min_chase = md->db->range3;
					break;
				}
			}
		}
	}

	if( casttime <= 0 )
		ud->state.skillcastcancel = 0;

	if( !sd || sd->skillitem != skill_num || skill_get_cast(skill_num,skill_lv) )
		ud->canact_tick = tick + casttime + 100;
	if( sd )
	{
		switch( skill_num )
		{
		case CG_ARROWVULCAN:
			sd->canequip_tick = tick + casttime;
			break;
		}
	}
	ud->skilltarget  = target_id;
	ud->skillx       = 0;
	ud->skilly       = 0;
	ud->skillid      = skill_num;
	ud->skilllv      = skill_lv;

 	if( sc && sc->data[SC_CLOAKING] && !(sc->data[SC_CLOAKING]->val4&4) && skill_num != AS_CLOAKING )
	{
		status_change_end(src, SC_CLOAKING, INVALID_TIMER);
		if (!src->prev) return 0; //Warped away!
	}

	if( casttime > 0 )
	{
		ud->skilltimer = add_timer( tick+casttime, skill_castend_id, src->id, 0 );
		if( sd && pc_checkskill(sd,SA_FREECAST) > 0 )
			status_calc_bl(&sd->bl, SCB_SPEED);
	}
	else
		skill_castend_id(ud->skilltimer,tick,src->id,0);

	return 1;
}

int unit_skilluse_pos(struct block_list *src, short skill_x, short skill_y, short skill_num, short skill_lv)
{
	if(skill_num < 0)
		return 0;
	return unit_skilluse_pos2(
		src, skill_x, skill_y, skill_num, skill_lv,
		skill_castfix(src, skill_num, skill_lv),
		skill_get_castcancel(skill_num)
	);
}

int unit_skilluse_pos2( struct block_list *src, short skill_x, short skill_y, short skill_num, short skill_lv, int casttime, int castcancel)
{
	struct map_session_data *sd = NULL;
	struct unit_data        *ud = NULL;
	struct status_change *sc;
	struct block_list    bl;
	unsigned int tick = gettick();

	nullpo_ret(src);

	if(!src->prev) return 0; // map 上に存在するか
	if(status_isdead(src)) return 0;

	sd = BL_CAST(BL_PC, src);
	ud = unit_bl2ud(src);
	if(ud == NULL) return 0;

	if(ud->skilltimer != INVALID_TIMER) //Normally not needed since clif.c checks for it, but at/char/script commands don't! [Skotlex]
		return 0;
	
	sc = status_get_sc(src);
	if (sc && !sc->count)
		sc = NULL;
	
	if( sd )
	{
		if( skillnotok(skill_num, sd) || !skill_check_condition_castbegin(sd, skill_num, skill_lv) )
			return 0;
		/**
		 * "WHY IS IT HEREE": pneuma cannot be cancelled past this point, the client displays the animation even,
		 * if we cancel it from nodamage_id, so it has to be here for it to not display the animation.
		 **/
		if( skill_num == AL_PNEUMA && map_getcell(src->m, skill_x, skill_y, CELL_CHKLANDPROTECTOR) ) {
			clif_skill_fail(sd,skill_num,0,0);
			return 0;
		}
	}

	if (!status_check_skilluse(src, NULL, skill_num, 0))
		return 0;

	if( map_getcell(src->m, skill_x, skill_y, CELL_CHKWALL) )
	{// can't cast ground targeted spells on wall cells
		if (sd) clif_skill_fail(sd,skill_num,0,0);
		return 0;
	}

	/* 射程と障害物チェック */
	bl.type = BL_NUL;
	bl.m = src->m;
	bl.x = skill_x;
	bl.y = skill_y;

	if( skill_get_state(ud->skillid) == ST_MOVE_ENABLE )
	{
		if( !unit_can_reach_bl(src, &bl, skill_get_range2(src, skill_num,skill_lv) + 1, 1, NULL, NULL) )
			return 0; //Walk-path check failed.
	}
	else if( !battle_check_range(src, &bl, skill_get_range2(src, skill_num,skill_lv) + 1) )
		return 0; //Arrow-path check failed.

	unit_stop_attack(src);

	// moved here to prevent Suffragium from ending if skill fails
	if (!(skill_get_castnodex(skill_num, skill_lv)&2))
		casttime = skill_castfix_sc(src, casttime, skill_num, skill_lv);

	ud->state.skillcastcancel = castcancel&&casttime>0?1:0;
	if( !sd || sd->skillitem != skill_num || skill_get_cast(skill_num,skill_lv) )
		ud->canact_tick  = tick + casttime + 100;
//	if( sd )
//	{
//		switch( skill_num )
//		{
//		case ????:
//			sd->canequip_tick = tick + casttime;
//		}
//	}
	ud->skillid      = skill_num;
	ud->skilllv      = skill_lv;
	ud->skillx       = skill_x;
	ud->skilly       = skill_y;
	ud->skilltarget  = 0;

	if (sc && sc->data[SC_CLOAKING] && !(sc->data[SC_CLOAKING]->val4&4))
	{
		status_change_end(src, SC_CLOAKING, INVALID_TIMER);
		if (!src->prev) return 0; //Warped away!
	}

	if( casttime > 0 )
	{
		unit_stop_walking(src,1);
		clif_skillcasting(src, src->id, 0, skill_x, skill_y, skill_num, skill_get_ele(skill_num, skill_lv), casttime);
		ud->skilltimer = add_timer( tick+casttime, skill_castend_pos, src->id, 0 );
		if( sd && pc_checkskill(sd,SA_FREECAST) > 0 )
			status_calc_bl(&sd->bl, SCB_SPEED);
	}
	else
	{
		ud->skilltimer = INVALID_TIMER;
		skill_castend_pos(ud->skilltimer,tick,src->id,0);
	}
	return 1;
}

int unit_stop_attack(struct block_list *bl)
{
	struct unit_data *ud = unit_bl2ud(bl);
	nullpo_ret(bl);

	if(!ud || ud->attacktimer == INVALID_TIMER)
		return 0;

	delete_timer( ud->attacktimer, unit_attack_timer );
	ud->attacktimer = INVALID_TIMER;
	ud->target = 0;
	return 0;
}

//Means current target is unattackable. For now only unlocks mobs.
int unit_unattackable(struct block_list *bl)
{
	struct unit_data *ud = unit_bl2ud(bl);
	if (ud) {
		ud->target = 0;
		ud->state.attack_continue = 0;
	}
	
	if(bl->type == BL_MOB)
		mob_unlocktarget((struct mob_data*)bl, gettick()) ;
	else if(bl->type == BL_PET)
		pet_unlocktarget((struct pet_data*)bl);
	return 0;
}

/*==========================================
 * 攻撃要求
 * typeが1なら継続攻撃
 *------------------------------------------*/
int unit_attack(struct block_list *src,int target_id,int continuous)
{
	struct block_list *target;
	struct unit_data  *ud;

	nullpo_ret(ud = unit_bl2ud(src));

	target = map_id2bl(target_id);
	if( target==NULL || status_isdead(target) )
	{
		unit_unattackable(src);
		return 1;
	}

	if( src->type == BL_PC )
	{
		TBL_PC* sd = (TBL_PC*)src;
		if( target->type == BL_NPC )
		{ // monster npcs [Valaris]
			npc_click(sd,(TBL_NPC*)target); // submitted by leinsirk10 [Celest]
			return 0;
		}
		if( pc_is90overweight(sd) )
		{ // overweight - stop attacking
			unit_stop_attack(src);
			return 0;
		}
	}

	if( battle_check_target(src,target,BCT_ENEMY) <= 0 || !status_check_skilluse(src, target, 0, 0) )
	{
		unit_unattackable(src);
		return 1;
	}

	ud->target = target_id;
	ud->state.attack_continue = continuous;
	if (continuous) //If you're to attack continously, set to auto-case character
		ud->chaserange = status_get_range(src);

	//Just change target/type. [Skotlex]
	if(ud->attacktimer != INVALID_TIMER)
		return 0;

	//Set Mob's ANGRY/BERSERK states.
	if(src->type == BL_MOB)
		((TBL_MOB*)src)->state.skillstate = ((TBL_MOB*)src)->state.aggressive?MSS_ANGRY:MSS_BERSERK;

	if(DIFF_TICK(ud->attackabletime, gettick()) > 0)
		//Do attack next time it is possible. [Skotlex]
		ud->attacktimer=add_timer(ud->attackabletime,unit_attack_timer,src->id,0);
	else //Attack NOW.
		unit_attack_timer(INVALID_TIMER, gettick(), src->id, 0);

	return 0;
}

//Cancels an ongoing combo, resets attackable time and restarts the 
//attack timer to resume attacking after amotion time. [Skotlex]
int unit_cancel_combo(struct block_list *bl)
{
	struct unit_data  *ud;

	if (!status_change_end(bl, SC_COMBO, INVALID_TIMER))
		return 0; //Combo wasn't active.

	ud = unit_bl2ud(bl);
	nullpo_ret(ud);

	ud->attackabletime = gettick() + status_get_amotion(bl);

	if (ud->attacktimer == INVALID_TIMER)
		return 1; //Nothing more to do.
	
	delete_timer(ud->attacktimer, unit_attack_timer);
	ud->attacktimer=add_timer(ud->attackabletime,unit_attack_timer,bl->id,0);
	return 1;
}
/*==========================================
 *
 *------------------------------------------*/
bool unit_can_reach_pos(struct block_list *bl,int x,int y, int easy)
{
	nullpo_retr(false, bl);

	if( bl->x==x && bl->y==y )	// 同じマス
		return true;

	return path_search(NULL,bl->m,bl->x,bl->y,x,y,easy,CELL_CHKNOREACH);
}

/*==========================================
 *
 *------------------------------------------*/
bool unit_can_reach_bl(struct block_list *bl,struct block_list *tbl, int range, int easy, short *x, short *y)
{
	int i;
	short dx,dy;
	nullpo_retr(false, bl);
	nullpo_retr(false, tbl);

	if( bl->m != tbl->m)
		return false;
	
	if( bl->x==tbl->x && bl->y==tbl->y )
		return true;

	if(range>0 && !check_distance_bl(bl, tbl, range))
		return false;

	// It judges whether it can adjoin or not.
	dx=tbl->x - bl->x;
	dy=tbl->y - bl->y;
	dx=(dx>0)?1:((dx<0)?-1:0);
	dy=(dy>0)?1:((dy<0)?-1:0);
	
	if (map_getcell(tbl->m,tbl->x-dx,tbl->y-dy,CELL_CHKNOPASS))
	{	//Look for a suitable cell to place in.
		for(i=0;i<9 && map_getcell(tbl->m,tbl->x-dirx[i],tbl->y-diry[i],CELL_CHKNOPASS);i++);
		if (i==9) return false; //No valid cells.
		dx = dirx[i];
		dy = diry[i];
	}

	if (x) *x = tbl->x-dx;
	if (y) *y = tbl->y-dy;
	return path_search(NULL,bl->m,bl->x,bl->y,tbl->x-dx,tbl->y-dy,easy,CELL_CHKNOREACH);
}
/*==========================================
 * Calculates position of Pet/Mercenary/Homunculus
 *------------------------------------------*/
int	unit_calc_pos(struct block_list *bl, int tx, int ty, int dir)
{
	int dx, dy, x, y, i, k;
	struct unit_data *ud = unit_bl2ud(bl);
	nullpo_ret(ud);

	if( dir < 0 || dir > 7 )
		return 1;

	ud->to_x = tx;
	ud->to_y = ty;

	// 2 cells from Master Position
	dx = -dirx[dir] * 2;
	dy = -diry[dir] * 2;
	x = tx + dx;
	y = ty + dy;

	if( !unit_can_reach_pos(bl, x, y, 0) )
	{
		if( dx > 0 ) x--; else if( dx < 0 ) x++;
		if( dy > 0 ) y--; else if( dy < 0 ) y++;
		if( !unit_can_reach_pos(bl, x, y, 0) )
		{
			for( i = 0; i < 12; i++ )
			{
				k = rand()%8; // Pick a Random Dir
				dx = -dirx[k] * 2;
				dy = -diry[k] * 2;
				x = tx + dx;
				y = ty + dy;
				if( unit_can_reach_pos(bl, x, y, 0) )
					break;
				else
				{
					if( dx > 0 ) x--; else if( dx < 0 ) x++;
					if( dy > 0 ) y--; else if( dy < 0 ) y++;
					if( unit_can_reach_pos(bl, x, y, 0) )
						break;
				}
			}
			if( i == 12 )
			{
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

/*==========================================
 * PCの攻撃 (timer関数)
 *------------------------------------------*/
static int unit_attack_timer_sub(struct block_list* src, int tid, unsigned int tick)
{
	struct block_list *target;
	struct unit_data *ud;
	struct status_data *sstatus;
	struct map_session_data *sd = NULL;
	struct mob_data *md = NULL;
	int range;
	
	if( (ud=unit_bl2ud(src))==NULL )
		return 0;
	if( ud->attacktimer != tid )
	{
		ShowError("unit_attack_timer %d != %d\n",ud->attacktimer,tid);
		return 0;
	}

	sd = BL_CAST(BL_PC, src);
	md = BL_CAST(BL_MOB, src);
	ud->attacktimer = INVALID_TIMER;
	target=map_id2bl(ud->target);

	if( src == NULL || src->prev == NULL || target==NULL || target->prev == NULL )
		return 0;

	if( status_isdead(src) || status_isdead(target) || !status_check_skilluse(src, target, 0, 0) )
		return 0; // can't attack under these conditions

	if( src->m != target->m )
	{
		if( src->type == BL_MOB && mob_warpchase((TBL_MOB*)src, target) )
			return 1; // Follow up.
		return 0;
	}

	if( ud->skilltimer != INVALID_TIMER && !(sd && pc_checkskill(sd,SA_FREECAST) > 0) )
		return 0; // can't attack while casting
	
	if( !battle_config.sdelay_attack_enable && DIFF_TICK(ud->canact_tick,tick) > 0 && !(sd && pc_checkskill(sd,SA_FREECAST) > 0) )
	{ // attacking when under cast delay has restrictions:
		if( tid == INVALID_TIMER )
		{ //requested attack.
			if(sd) clif_skill_fail(sd,1,4,0);
			return 0;
		}
		//Otherwise, we are in a combo-attack, delay this until your canact time is over. [Skotlex]
		if( ud->state.attack_continue )
		{
			if( DIFF_TICK(ud->canact_tick, ud->attackabletime) > 0 )
				ud->attackabletime = ud->canact_tick;
			ud->attacktimer=add_timer(ud->attackabletime,unit_attack_timer,src->id,0);
		}
		return 1;
	}

	sstatus = status_get_status_data(src);
	range = sstatus->rhw.range;
	
	if( !sd || sd->status.weapon != W_BOW )
		range++; //Dunno why everyone but bows gets this extra range...
	if( unit_is_walking(target) )
		range++; //Extra range when chasing

	if( !check_distance_bl(src,target,range) )
	{ //Chase if required.
		if(sd)
			clif_movetoattack(sd,target);
		else if(ud->state.attack_continue)
			unit_walktobl(src,target,ud->chaserange,ud->state.walk_easy|2);
		return 1;
	}
	if( !battle_check_range(src,target,range) )
	{
	  	//Within range, but no direct line of attack
		if( ud->state.attack_continue )
		{
			if(ud->chaserange > 2) ud->chaserange-=2;
			unit_walktobl(src,target,ud->chaserange,ud->state.walk_easy|2);
		}
		return 1;
	}

	//Sync packet only for players.
	//Non-players use the sync packet on the walk timer. [Skotlex]
	if (tid == INVALID_TIMER && sd) clif_fixpos(src);

	if( DIFF_TICK(ud->attackabletime,tick) <= 0 )
	{
		if (battle_config.attack_direction_change && (src->type&battle_config.attack_direction_change)) {
			ud->dir = map_calc_dir(src, target->x,target->y );
		}
		if(ud->walktimer != INVALID_TIMER)
			unit_stop_walking(src,1);
		if(md) {
			if (mobskill_use(md,tick,-1))
				return 1;
			if (sstatus->mode&MD_ASSIST && DIFF_TICK(md->last_linktime, tick) < MIN_MOBLINKTIME)
			{	// Link monsters nearby [Skotlex]
				md->last_linktime = tick;
				map_foreachinrange(mob_linksearch, src, md->db->range2, BL_MOB, md->class_, target, tick);
			}
		}
		if(src->type == BL_PET && pet_attackskill((TBL_PET*)src, target->id))
			return 1;
		
		map_freeblock_lock();
		ud->attacktarget_lv = battle_weapon_attack(src,target,tick,0);

		if(sd && sd->status.pet_id > 0 && sd->pd && battle_config.pet_attack_support)
			pet_target_check(sd,target,0);
		map_freeblock_unlock();

		ud->attackabletime = tick + sstatus->adelay;
//		You can't move if you can't attack neither.
		if (src->type&battle_config.attack_walk_delay)
			unit_set_walkdelay(src, tick, sstatus->amotion, 1);
	}

	if(ud->state.attack_continue)
		ud->attacktimer = add_timer(ud->attackabletime,unit_attack_timer,src->id,0);

	return 1;
}

static int unit_attack_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list *bl;
	bl = map_id2bl(id);
	if(bl && unit_attack_timer_sub(bl, tid, tick) == 0)
		unit_unattackable(bl);
	return 0;
}

/*==========================================
 * Cancels an ongoing skill cast.
 * flag&1: Cast-Cancel invoked.
 * flag&2: Cancel only if skill is cancellable.
 *------------------------------------------*/
int unit_skillcastcancel(struct block_list *bl,int type)
{
	struct map_session_data *sd = NULL;
	struct unit_data *ud = unit_bl2ud( bl);
	unsigned int tick=gettick();
	int ret=0, skill;
	
	nullpo_ret(bl);
	if (!ud || ud->skilltimer == INVALID_TIMER)
		return 0; //Nothing to cancel.

	sd = BL_CAST(BL_PC, bl);

	if (type&2) {
		//See if it can be cancelled.
		if (!ud->state.skillcastcancel)
			return 0;

		if (sd && (sd->special_state.no_castcancel2 ||
			(sd->special_state.no_castcancel && !map_flag_gvg(bl->m) && !map[bl->m].flag.battleground))) //fixed flags being read the wrong way around [blackhole89]
			return 0;
	}
	
	ud->canact_tick = tick;

	if(type&1 && sd)
		skill = sd->skillid_old;
	else
		skill = ud->skillid;
	
	if (skill_get_inf(skill) & INF_GROUND_SKILL)
		ret=delete_timer( ud->skilltimer, skill_castend_pos );
	else
		ret=delete_timer( ud->skilltimer, skill_castend_id );
	if(ret<0)
		ShowError("delete timer error : skillid : %d\n",ret);

	ud->skilltimer = INVALID_TIMER;

	if( sd && pc_checkskill(sd,SA_FREECAST) > 0 )
		status_calc_bl(&sd->bl, SCB_SPEED);

	if( sd )
	{
		switch( skill )
		{
		case CG_ARROWVULCAN:
			sd->canequip_tick = tick;
			break;
		}
	}

	if(bl->type==BL_MOB) ((TBL_MOB*)bl)->skillidx  = -1;

	clif_skillcastcancel(bl);
	return 1;
}

// unit_data の初期化処理
void unit_dataset(struct block_list *bl)
{
	struct unit_data *ud;
	nullpo_retv(ud = unit_bl2ud(bl));

	memset( ud, 0, sizeof( struct unit_data) );
	ud->bl             = bl;
	ud->walktimer      = INVALID_TIMER;
	ud->skilltimer     = INVALID_TIMER;
	ud->attacktimer    = INVALID_TIMER;
	ud->attackabletime = 
	ud->canact_tick    = 
	ud->canmove_tick   = gettick();
}

/*==========================================
 * Returns 1 if this unit is attacking target 'id'
 *------------------------------------------*/
static int unit_counttargeted_sub(struct block_list* bl, va_list ap)
{
	int id = va_arg(ap, int);
	int target_lv = va_arg(ap, int); // extra condition
	struct unit_data* ud;

	if(bl->id == id)
		return 0;

	ud = unit_bl2ud(bl);

	if (ud && ud->target == id && ud->attacktimer != INVALID_TIMER && ud->attacktarget_lv >= target_lv)
		return 1;

	return 0;	
}

/*==========================================
 * Counts the number of units attacking 'bl'
 *------------------------------------------*/
int unit_counttargeted(struct block_list* bl, int target_lv)
{
	nullpo_ret(bl);
	return (map_foreachinrange(unit_counttargeted_sub, bl, AREA_SIZE, BL_CHAR, bl->id, target_lv));
}

/*==========================================
 *
 *------------------------------------------*/
int unit_fixdamage(struct block_list *src,struct block_list *target,unsigned int tick,int sdelay,int ddelay,int damage,int div,int type,int damage2)
{
	nullpo_ret(target);

	if(damage+damage2 <= 0)
		return 0;
	
	return status_fix_damage(src,target,damage+damage2,clif_damage(target,target,tick,sdelay,ddelay,damage,div,type,damage2));
}

/*==========================================
 * 見た目のサイズを変更する
 *------------------------------------------*/
int unit_changeviewsize(struct block_list *bl,short size)
{
	nullpo_ret(bl);

	size=(size<0)?-1:(size>0)?1:0;

	if(bl->type == BL_PC) {
		((TBL_PC*)bl)->state.size=size;
	} else if(bl->type == BL_MOB) {
		((TBL_MOB*)bl)->special_state.size=size;
	} else
		return 0;
	if(size!=0)
		clif_specialeffect(bl,421+size, AREA);
	return 0;
}

/*==========================================
 * Removes a bl/ud from the map.
 * Returns 1 on success. 0 if it couldn't be removed or the bl was free'd
 * if clrtype is 1 (death), appropiate cleanup is performed. 
 * Otherwise it is assumed bl is being warped.
 * On-Kill specific stuff is not performed here, look at status_damage for that.
 *------------------------------------------*/
int unit_remove_map_(struct block_list *bl, clr_type clrtype, const char* file, int line, const char* func)
{
	struct unit_data *ud = unit_bl2ud(bl);
	struct status_change *sc = status_get_sc(bl);
	nullpo_ret(ud);

	if(bl->prev == NULL)
		return 0; //Already removed?

	map_freeblock_lock();

	ud->target = 0; //Unlock walk/attack target.
	if (ud->walktimer != INVALID_TIMER)
		unit_stop_walking(bl,0);
	if (ud->attacktimer != INVALID_TIMER)
		unit_stop_attack(bl);
	if (ud->skilltimer != INVALID_TIMER)
		unit_skillcastcancel(bl,0);
// Do not reset can-act delay. [Skotlex]
	ud->attackabletime = ud->canmove_tick /*= ud->canact_tick*/ = gettick();
	
	if(sc && sc->count ) { //map-change/warp dispells.
		status_change_end(bl, SC_BLADESTOP, INVALID_TIMER);
		status_change_end(bl, SC_BASILICA, INVALID_TIMER);
		status_change_end(bl, SC_ANKLE, INVALID_TIMER);
		status_change_end(bl, SC_TRICKDEAD, INVALID_TIMER);
		status_change_end(bl, SC_BLADESTOP_WAIT, INVALID_TIMER);
		status_change_end(bl, SC_RUN, INVALID_TIMER);
		status_change_end(bl, SC_DANCING, INVALID_TIMER);
		status_change_end(bl, SC_WARM, INVALID_TIMER);
		status_change_end(bl, SC_DEVOTION, INVALID_TIMER);
		status_change_end(bl, SC_MARIONETTE, INVALID_TIMER);
		status_change_end(bl, SC_MARIONETTE2, INVALID_TIMER);
		status_change_end(bl, SC_CLOSECONFINE, INVALID_TIMER);
		status_change_end(bl, SC_CLOSECONFINE2, INVALID_TIMER);
		status_change_end(bl, SC_HIDING, INVALID_TIMER);
		status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
		status_change_end(bl, SC_CHASEWALK, INVALID_TIMER);
		if (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF)
			status_change_end(bl, SC_GOSPEL, INVALID_TIMER);
		status_change_end(bl, SC_CHANGE, INVALID_TIMER);
		status_change_end(bl, SC_STOP, INVALID_TIMER);
	}

	if (bl->type&BL_CHAR) {
		skill_unit_move(bl,gettick(),4);
		skill_cleartimerskill(bl);
	}

	switch( bl->type )
	{
	case BL_PC:
	{
		struct map_session_data *sd = (struct map_session_data*)bl;

		//Leave/reject all invitations.
		if(sd->chatID)
			chat_leavechat(sd,0);
		if(sd->trade_partner)
			trade_tradecancel(sd);
		vending_closevending(sd);
		buyingstore_close(sd);
		searchstore_close(sd);
		if(sd->state.storage_flag == 1)
			storage_storage_quit(sd,0);
		else if (sd->state.storage_flag == 2)
			storage_guild_storage_quit(sd,0);
		sd->state.storage_flag = 0; //Force close it when being warped.
		if(sd->party_invite>0)
			party_reply_invite(sd,sd->party_invite,0);
		if(sd->guild_invite>0)
			guild_reply_invite(sd,sd->guild_invite,0);
		if(sd->guild_alliance>0)
			guild_reply_reqalliance(sd,sd->guild_alliance_account,0);
		if(sd->menuskill_id)
			sd->menuskill_id = sd->menuskill_val = 0;
		if( sd->touching_id )
			npc_touchnext_areanpc(sd,true);

		sd->npc_shopid = 0;
		sd->adopt_invite = 0;

		if(sd->pvp_timer != INVALID_TIMER) {
			delete_timer(sd->pvp_timer,pc_calc_pvprank_timer);
			sd->pvp_timer = INVALID_TIMER;
			sd->pvp_rank = 0;
		}
		if(sd->duel_group > 0)
			duel_leave(sd->duel_group, sd);

		if(pc_issit(sd)) {
			pc_setstand(sd);
			skill_sit(sd,0);
		}
		party_send_dot_remove(sd);//minimap dot fix [Kevin]
		guild_send_dot_remove(sd);
		bg_send_dot_remove(sd);

		if( map[bl->m].users <= 0 || sd->state.debug_remove_map )
		{// this is only place where map users is decreased, if the mobs were removed too soon then this function was executed too many times [FlavioJS]
			if( sd->debug_file == NULL || !(sd->state.debug_remove_map) )
			{
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
		else
		if (--map[bl->m].users == 0 && battle_config.dynamic_mobs)	//[Skotlex]
			map_removemobs(bl->m);
		if( !(sd->sc.option&OPTION_INVISIBLE) )
		{// decrement the number of active pvp players on the map
			--map[bl->m].users_pvp;
		}
		if( map[bl->m].instance_id )
		{
			instance[map[bl->m].instance_id].users--;
			instance_check_idle(map[bl->m].instance_id);
		}
		sd->state.debug_remove_map = 1; // temporary state to track double remove_map's [FlavioJS]
		sd->debug_file = file;
		sd->debug_line = line;
		sd->debug_func = func;

		break;
	}
	case BL_MOB:
	{
		struct mob_data *md = (struct mob_data*)bl;
		// Drop previous target mob_slave_keep_target: no.
		if (!battle_config.mob_slave_keep_target)
			md->target_id=0;

		md->attacked_id=0;
		md->state.skillstate= MSS_IDLE;

		break;
	}
	case BL_PET:
	{
		struct pet_data *pd = (struct pet_data*)bl;
		if( pd->pet.intimate <= 0 && !(pd->msd && !pd->msd->state.active) )
		{	//If logging out, this is deleted on unit_free
			clif_clearunit_area(bl,clrtype);
			map_delblock(bl);
			unit_free(bl,CLR_OUTSIGHT);
			map_freeblock_unlock();
			return 0;
		}

		break;
	}
	case BL_HOM:
	{
		struct homun_data *hd = (struct homun_data *)bl;
		ud->canact_tick = ud->canmove_tick; //It appears HOM do reset the can-act tick.
		if( !hd->homunculus.intimacy && !(hd->master && !hd->master->state.active) )
		{	//If logging out, this is deleted on unit_free
			clif_emotion(bl, E_SOB);
			clif_clearunit_area(bl,clrtype);
			map_delblock(bl);
			unit_free(bl,CLR_OUTSIGHT);
			map_freeblock_unlock();
			return 0;
		}
		break;
	}
	case BL_MER:
	{
		struct mercenary_data *md = (struct mercenary_data *)bl;
		ud->canact_tick = ud->canmove_tick;
		if( mercenary_get_lifetime(md) <= 0 && !(md->master && !md->master->state.active) )
		{
			clif_clearunit_area(bl,clrtype);
			map_delblock(bl);
			unit_free(bl,CLR_OUTSIGHT);
			map_freeblock_unlock();
			return 0;
		}
		break;
	}
	default: ;// do nothing
	}
	/**
	 * BL_MOB is handled by mob_dead unless the monster is not dead.
	 **/
	if( bl->type != BL_MOB || !status_isdead(bl) )
		clif_clearunit_area(bl,clrtype);
	map_delblock(bl);
	map_freeblock_unlock();
	return 1;
}

void unit_remove_map_pc(struct map_session_data *sd, clr_type clrtype)
{
	unit_remove_map(&sd->bl,clrtype);

	if (clrtype == CLR_TELEPORT) clrtype = CLR_OUTSIGHT; //CLR_TELEPORT is the warp from logging out, but pets/homunc need to just 'vanish' instead of showing the warping out animation.

	if(sd->pd)
		unit_remove_map(&sd->pd->bl, clrtype);
	if(merc_is_hom_active(sd->hd))
		unit_remove_map(&sd->hd->bl, clrtype);
	if(sd->md)
		unit_remove_map(&sd->md->bl, clrtype);
}

void unit_free_pc(struct map_session_data *sd)
{
	if (sd->pd) unit_free(&sd->pd->bl,CLR_OUTSIGHT);
	if (sd->hd) unit_free(&sd->hd->bl,CLR_OUTSIGHT);
	if (sd->md) unit_free(&sd->md->bl,CLR_OUTSIGHT);
	unit_free(&sd->bl,CLR_TELEPORT);
}

/*==========================================
 * Function to free all related resources to the bl
 * if unit is on map, it is removed using the clrtype specified
 *------------------------------------------*/
int unit_free(struct block_list *bl, clr_type clrtype)
{
	struct unit_data *ud = unit_bl2ud( bl );
	nullpo_ret(ud);

	map_freeblock_lock();
	if( bl->prev )	//Players are supposed to logout with a "warp" effect.
		unit_remove_map(bl, clrtype);

	switch( bl->type )
	{
		case BL_PC:
		{
			struct map_session_data *sd = (struct map_session_data*)bl;

			if( status_isdead(bl) )
				pc_setrestartvalue(sd,2);

			pc_delinvincibletimer(sd);
			pc_delautobonus(sd,sd->autobonus,ARRAYLENGTH(sd->autobonus),false);
			pc_delautobonus(sd,sd->autobonus2,ARRAYLENGTH(sd->autobonus2),false);
			pc_delautobonus(sd,sd->autobonus3,ARRAYLENGTH(sd->autobonus3),false);
			
			if( sd->followtimer != INVALID_TIMER )
				pc_stop_following(sd);
				
			if( sd->duel_invite > 0 )
				duel_reject(sd->duel_invite, sd);
		
			// Notify friends that this char logged out. [Skotlex]
			map_foreachpc(clif_friendslist_toggle_sub, sd->status.account_id, sd->status.char_id, 0);
			party_send_logout(sd);
			guild_send_memberinfoshort(sd,0);
			pc_cleareventtimer(sd);
			pc_inventory_rental_clear(sd);
			pc_delspiritball(sd,sd->spiritball,1);

			if( sd->reg )
			{	//Double logout already freed pointer fix... [Skotlex]
				aFree(sd->reg);
				sd->reg = NULL;
				sd->reg_num = 0;
			}
			if( sd->regstr )
			{
				int i;
				for( i = 0; i < sd->regstr_num; ++i )
					if( sd->regstr[i].data )
						aFree(sd->regstr[i].data);
				aFree(sd->regstr);
				sd->regstr = NULL;
				sd->regstr_num = 0;
			}
			if( sd->st && sd->st->state != RUN )
			{// free attached scripts that are waiting
				script_free_state(sd->st);
				sd->st = NULL;
				sd->npc_id = 0;
			}
			break;
		}
		case BL_PET:
		{
			struct pet_data *pd = (struct pet_data*)bl;
			struct map_session_data *sd = pd->msd;
			pet_hungry_timer_delete(pd);
			if( pd->a_skill )
			{
				aFree(pd->a_skill);
				pd->a_skill = NULL;
			}
			if( pd->s_skill )
			{
				if (pd->s_skill->timer != INVALID_TIMER) {
					if (pd->s_skill->id)
						delete_timer(pd->s_skill->timer, pet_skill_support_timer);
					else
						delete_timer(pd->s_skill->timer, pet_heal_timer);
				}
				aFree(pd->s_skill);
				pd->s_skill = NULL;
			}
			if( pd->recovery )
			{
				if(pd->recovery->timer != INVALID_TIMER)
					delete_timer(pd->recovery->timer, pet_recovery_timer);
				aFree(pd->recovery);
				pd->recovery = NULL;
			}
			if( pd->bonus )
			{
				if (pd->bonus->timer != INVALID_TIMER)
					delete_timer(pd->bonus->timer, pet_skill_bonus_timer);
				aFree(pd->bonus);
				pd->bonus = NULL;
			}
			if( pd->loot )
			{
				pet_lootitem_drop(pd,sd);
				if (pd->loot->item)
					aFree(pd->loot->item);
				aFree (pd->loot);
				pd->loot = NULL;
			}
			if( pd->pet.intimate > 0 )
				intif_save_petdata(pd->pet.account_id,&pd->pet);
			else
			{	//Remove pet.
				intif_delete_petdata(pd->pet.pet_id);
				if (sd) sd->status.pet_id = 0;
			}
			if( sd )
				sd->pd = NULL;
			break;
		}
		case BL_MOB:
		{
			struct mob_data *md = (struct mob_data*)bl;
			if( md->spawn_timer != INVALID_TIMER )
			{
				delete_timer(md->spawn_timer,mob_delayspawn);
				md->spawn_timer = INVALID_TIMER;
			}
			if( md->deletetimer != INVALID_TIMER )
			{
				delete_timer(md->deletetimer,mob_timer_delete);
				md->deletetimer = INVALID_TIMER;
			}
			if( md->lootitem )
			{
				aFree(md->lootitem);
				md->lootitem=NULL;
			}
			if( md->guardian_data )
			{
				struct guild_castle* gc = md->guardian_data->castle;
				if( md->guardian_data->number >= 0 && md->guardian_data->number < MAX_GUARDIANS )
				{
					gc->guardian[md->guardian_data->number].id = 0;
				}
				else
				{
					int i;
					ARR_FIND(0, gc->temp_guardians_max, i, gc->temp_guardians[i] == md->bl.id);
					if( i < gc->temp_guardians_max )
						gc->temp_guardians[i] = 0;
				}
				aFree(md->guardian_data);
				md->guardian_data = NULL;
			}
			if( md->spawn )
			{
				md->spawn->active--;
				if( !md->spawn->state.dynamic )
				{// permanently remove the mob
					if( --md->spawn->num == 0 )
					{// Last freed mob is responsible for deallocating the group's spawn data.
						aFree(md->spawn);
						md->spawn = NULL;
					}
				}
			}
			if( md->base_status)
			{
				aFree(md->base_status);
				md->base_status = NULL;
			}
			if( mob_is_clone(md->class_) )
				mob_clone_delete(md);
			break;
		}
		case BL_HOM:
		{
			struct homun_data *hd = (TBL_HOM*)bl;
			struct map_session_data *sd = hd->master;
			merc_hom_hungry_timer_delete(hd);
			if( hd->homunculus.intimacy > 0 )
				merc_save(hd);
			else
			{
				intif_homunculus_requestdelete(hd->homunculus.hom_id);
				if( sd )
					sd->status.hom_id = 0;
			}
			if( sd )
				sd->hd = NULL;
			break;
		}
		case BL_MER:
		{
			struct mercenary_data *md = (TBL_MER*)bl;
			struct map_session_data *sd = md->master;
			if( mercenary_get_lifetime(md) > 0 )
				mercenary_save(md);
			else
			{
				intif_mercenary_delete(md->mercenary.mercenary_id);
				if( sd )
					sd->status.mer_id = 0;
			}
			if( sd )
				sd->md = NULL;

			merc_contract_stop(md);
			break;
		}
	}

	skill_clear_unitgroup(bl);
	status_change_clear(bl,1);
	map_deliddb(bl);
	if( bl->type != BL_PC ) //Players are handled by map_quit
		map_freeblock(bl);
	map_freeblock_unlock();
	return 0;
}

int do_init_unit(void)
{
	add_timer_func_list(unit_attack_timer,  "unit_attack_timer");
	add_timer_func_list(unit_walktoxy_timer,"unit_walktoxy_timer");
	add_timer_func_list(unit_walktobl_sub, "unit_walktobl_sub");
	add_timer_func_list(unit_delay_walktoxy_timer,"unit_delay_walktoxy_timer");
	return 0;
}

int do_final_unit(void)
{
	// nothing to do
	return 0;
}
