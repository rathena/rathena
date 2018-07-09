// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "elemental.hpp"

#include <cstring>
#include <ctgmath> //floor
#include <math.h>
#include <stdlib.h>

#include "../common/cbasetypes.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"
#include "../common/utils.hpp"

#include "battle.hpp"
#include "clif.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "pc.hpp"
#include "trade.hpp"

struct s_elemental_db elemental_db[MAX_ELEMENTAL_CLASS]; // Elemental Database
static uint16 elemental_count;

int elemental_search_index(int class_) {
	int i;
	ARR_FIND(0, elemental_count, i, elemental_db[i].class_ == class_);
	return (i == elemental_count)?-1:i;
}

bool elemental_class(int class_) {
	return (bool)(elemental_search_index(class_) > -1);
}

struct view_data * elemental_get_viewdata(int class_) {
	int i = elemental_search_index(class_);
	if( i < 0 )
		return 0;

	return &elemental_db[i].vd;
}

int elemental_create(struct map_session_data *sd, int class_, unsigned int lifetime) {
	struct s_elemental ele;
	struct s_elemental_db *db;
	int i;

	nullpo_retr(1,sd);

	if( (i = elemental_search_index(class_)) < 0 )
		return 0;

	db = &elemental_db[i];
	memset(&ele,0,sizeof(struct s_elemental));

	ele.char_id = sd->status.char_id;
	ele.class_ = class_;
	ele.mode = EL_MODE_PASSIVE; // Initial mode
	i = db->status.size+1; // summon level

	//[(Caster's Max HP/ 3 ) + (Caster's INT x 10 )+ (Caster's Job Level x 20 )] x [(Elemental Summon Level + 2) / 3]
	ele.hp = ele.max_hp = (sd->battle_status.max_hp/3 + sd->battle_status.int_*10 + sd->status.job_level*20) * ((i + 2) / 3);
	//Caster's Max SP /4
	ele.sp = ele.max_sp = sd->battle_status.max_sp/4;
	//Caster's [ Max SP / (18 / Elemental Summon Skill Level) 1- 100 ]
	ele.atk = (sd->battle_status.max_sp / (18 / i)  * 1 - 100);
	//Caster's [ Max SP / (18 / Elemental Summon Skill Level) ]
	ele.atk2 = sd->battle_status.max_sp / (18 / i);
	//Caster's HIT + (Caster's Base Level)
	ele.hit = sd->battle_status.hit + sd->status.base_level;
	//[Elemental Summon Skill Level x (Caster's INT / 2 + Caster's DEX / 4)]
	ele.matk = i * (sd->battle_status.int_ / 2 + sd->battle_status.dex / 4);
	//150 + [Caster's DEX / 10] + [Elemental Summon Skill Level x 3 ]
	ele.amotion = 150 + sd->battle_status.dex / 10 + i * 3;
	//Caster's DEF + (Caster's Base Level / (5 - Elemental Summon Skill Level)
	ele.def = sd->battle_status.def + sd->status.base_level / (5-i);
	//Caster's MDEF + (Caster's INT / (5 - Elemental Summon Skill Level)
	ele.mdef = sd->battle_status.mdef + sd->battle_status.int_ / (5-i);
	//Caster's FLEE + (Caster's Base Level / (5 - Elemental Summon Skill Level)
	ele.flee = sd->battle_status.flee + sd->status.base_level / (5-i);

	//per individual bonuses
	switch(db->class_){
	case ELEMENTALID_AGNI_S:	case ELEMENTALID_AGNI_M:
	case ELEMENTALID_AGNI_L: //ATK + (Summon Agni Skill Level x 20) / HIT + (Summon Agni Skill Level x 10)
		ele.atk += i * 20;
		ele.atk2 += i * 20;
		ele.hit += i * 10;
		break;
	case ELEMENTALID_AQUA_S:	case ELEMENTALID_AQUA_M:
	case ELEMENTALID_AQUA_L: //MDEF + (Summon Aqua Skill Level x 10) / MATK + (Summon Aqua Skill Level x 20)
		ele.mdef += i * 10;
		ele.matk += i * 20;
		break;
	case ELEMENTALID_VENTUS_S:	case ELEMENTALID_VENTUS_M:
	case ELEMENTALID_VENTUS_L: //FLEE + (Summon Ventus Skill Level x 20) / MATK + (Summon Ventus Skill Level x 10)
		ele.flee += i * 20;
		ele.matk += i * 10;
		break;
	case ELEMENTALID_TERA_S:	case ELEMENTALID_TERA_M:
	case ELEMENTALID_TERA_L: //DEF + (Summon Tera Skill Level x 25) / ATK + (Summon Tera Skill Level x 5)
		ele.def += i * 25;
		ele.atk += i * 5;
		ele.atk2 += i * 5;
		break;
	}

	if( (i=pc_checkskill(sd,SO_EL_SYMPATHY)) > 0 ){
		ele.hp = ele.max_hp += ele.max_hp * 5 * i / 100;
		ele.sp = ele.max_sp += ele.max_sp * 5 * i / 100;
		ele.atk += 25 * i;
		ele.atk2 += 25 * i;
		ele.matk += 25 * i;
	}

	ele.life_time = lifetime;

	// Request Char Server to create this elemental
	intif_elemental_create(&ele);

	return 1;
}

int elemental_get_lifetime(struct elemental_data *ed) {
	const struct TimerData * td;
	if( ed == NULL || ed->summon_timer == INVALID_TIMER )
		return 0;

	td = get_timer(ed->summon_timer);
	return (td != NULL) ? DIFF_TICK(td->tick, gettick()) : 0;
}

int elemental_save(struct elemental_data *ed) {
	ed->elemental.mode = ed->battle_status.mode;
	ed->elemental.hp = ed->battle_status.hp;
	ed->elemental.sp = ed->battle_status.sp;
	ed->elemental.max_hp = ed->battle_status.max_hp;
	ed->elemental.max_sp = ed->battle_status.max_sp;
	ed->elemental.atk = ed->battle_status.rhw.atk;
	ed->elemental.atk2 = ed->battle_status.rhw.atk2;
	ed->elemental.matk = ed->battle_status.matk_min;
	ed->elemental.def = ed->battle_status.def;
	ed->elemental.mdef = ed->battle_status.mdef;
	ed->elemental.flee = ed->battle_status.flee;
	ed->elemental.hit = ed->battle_status.hit;
	ed->elemental.life_time = elemental_get_lifetime(ed);
	intif_elemental_save(&ed->elemental);
	return 1;
}

static int elemental_summon_end(int tid, unsigned int tick, int id, intptr_t data) {
	struct map_session_data *sd;
	struct elemental_data *ed;

	if( (sd = map_id2sd(id)) == NULL )
		return 1;
	if( (ed = sd->ed) == NULL )
		return 1;

	if( ed->summon_timer != tid ) {
		ShowError("elemental_summon_end %d != %d.\n", ed->summon_timer, tid);
		return 0;
	}

	ed->summon_timer = INVALID_TIMER;
	elemental_delete(ed); // Elemental's summon time is over.

	return 0;
}

void elemental_summon_stop(struct elemental_data *ed) {
	nullpo_retv(ed);
	if( ed->summon_timer != INVALID_TIMER )
		delete_timer(ed->summon_timer, elemental_summon_end);
	ed->summon_timer = INVALID_TIMER;
}

int elemental_delete(struct elemental_data *ed) {
	struct map_session_data *sd;

	nullpo_ret(ed);

	sd = ed->master;
	ed->elemental.life_time = 0;

	elemental_clean_effect(ed);
	elemental_summon_stop(ed);

	if( !sd )
		return unit_free(&ed->bl, CLR_OUTSIGHT);

	sd->ed = NULL;
	sd->status.ele_id = 0;

	return unit_remove_map(&ed->bl, CLR_OUTSIGHT);
}

void elemental_summon_init(struct elemental_data *ed) {
	if( ed->summon_timer == INVALID_TIMER )
		ed->summon_timer = add_timer(gettick() + ed->elemental.life_time, elemental_summon_end, ed->master->bl.id, 0);

	ed->regen.state.block = 0;
}

/**
 * Inter-serv has sent us the elemental data from sql, fill it in map-serv memory
 * @param ele : The elemental data received from char-serv
 * @param flag : 0:not created, 1:was saved/loaded
 * @return 0:failed, 1:sucess
 */
int elemental_data_received(struct s_elemental *ele, bool flag) {
	struct map_session_data *sd;
	struct elemental_data *ed;
	struct s_elemental_db *db;
	int i = elemental_search_index(ele->class_);

	if( (sd = map_charid2sd(ele->char_id)) == NULL )
		return 0;

	if( !flag || i < 0 ) { // Not created - loaded - DB info
		sd->status.ele_id = 0;
		return 0;
	}

	db = &elemental_db[i];
	if( !sd->ed ) {	// Initialize it after first summon.
		sd->ed = ed = (struct elemental_data*)aCalloc(1,sizeof(struct elemental_data));
		ed->bl.type = BL_ELEM;
		ed->bl.id = npc_get_new_npc_id();
		ed->master = sd;
		ed->db = db;
		memcpy(&ed->elemental, ele, sizeof(struct s_elemental));
		status_set_viewdata(&ed->bl, ed->elemental.class_);
		ed->vd->head_mid = 10; // Why?
		status_change_init(&ed->bl);
		unit_dataset(&ed->bl);
		ed->ud.dir = sd->ud.dir;

		ed->bl.m = sd->bl.m;
		ed->bl.x = sd->bl.x;
		ed->bl.y = sd->bl.y;
		unit_calc_pos(&ed->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
		ed->bl.x = ed->ud.to_x;
		ed->bl.y = ed->ud.to_y;

		map_addiddb(&ed->bl);
		status_calc_elemental(ed,SCO_FIRST);
		ed->last_spdrain_time = ed->last_thinktime = gettick();
		ed->summon_timer = INVALID_TIMER;
		ed->masterteleport_timer = INVALID_TIMER;
		elemental_summon_init(ed);
	} else {
		memcpy(&sd->ed->elemental, ele, sizeof(struct s_elemental));
		ed = sd->ed;
	}

	sd->status.ele_id = ele->elemental_id;

	if( ed->bl.prev == NULL && sd->bl.prev != NULL ) {
		if(map_addblock(&ed->bl))
			return 0;
		clif_spawn(&ed->bl);
		clif_elemental_info(sd);
		clif_elemental_updatestatus(sd,SP_HP);
		clif_hpmeter_single(sd->fd,ed->bl.id,ed->battle_status.hp,ed->battle_status.max_hp);
		clif_elemental_updatestatus(sd,SP_SP);
	}

	return 1;
}

int elemental_clean_single_effect(struct elemental_data *ed, uint16 skill_id) {
	struct block_list *bl;
	sc_type type = status_skill2sc(skill_id);

	nullpo_ret(ed);

	bl = battle_get_master(&ed->bl);

	if( type ) {
		switch( type ) {
				// Just remove status change.
			case SC_PYROTECHNIC_OPTION:
			case SC_HEATER_OPTION:
			case SC_TROPIC_OPTION:
			case SC_FIRE_CLOAK_OPTION:
			case SC_AQUAPLAY_OPTION:
			case SC_WATER_SCREEN_OPTION:
			case SC_COOLER_OPTION:
			case SC_CHILLY_AIR_OPTION:
			case SC_GUST_OPTION:
			case SC_WIND_STEP_OPTION:
			case SC_BLAST_OPTION:
			case SC_WATER_DROP_OPTION:
			case SC_WIND_CURTAIN_OPTION:
			case SC_WILD_STORM_OPTION:
			case SC_PETROLOGY_OPTION:
			case SC_SOLID_SKIN_OPTION:
			case SC_CURSED_SOIL_OPTION:
			case SC_STONE_SHIELD_OPTION:
			case SC_UPHEAVAL_OPTION:
			case SC_CIRCLE_OF_FIRE_OPTION:
			case SC_TIDAL_WEAPON_OPTION:
				if( bl ) status_change_end(bl,type,INVALID_TIMER);	// Master
				status_change_end(&ed->bl,static_cast<sc_type>(type-1),INVALID_TIMER);	// Elemental Spirit
				break;
			case SC_ZEPHYR:
				if( bl ) status_change_end(bl,type,INVALID_TIMER);
				break;
			default:
				ShowWarning("Invalid SC=%d in elemental_clean_single_effect\n",type);
				break;
		}
	}

	return 1;
}

int elemental_clean_effect(struct elemental_data *ed) {
	struct map_session_data *sd;

	nullpo_ret(ed);

	// Elemental side
	status_change_end(&ed->bl, SC_TROPIC, INVALID_TIMER);
	status_change_end(&ed->bl, SC_HEATER, INVALID_TIMER);
	status_change_end(&ed->bl, SC_AQUAPLAY, INVALID_TIMER);
	status_change_end(&ed->bl, SC_COOLER, INVALID_TIMER);
	status_change_end(&ed->bl, SC_CHILLY_AIR, INVALID_TIMER);
	status_change_end(&ed->bl, SC_PYROTECHNIC, INVALID_TIMER);
	status_change_end(&ed->bl, SC_FIRE_CLOAK, INVALID_TIMER);
	status_change_end(&ed->bl, SC_WATER_DROP, INVALID_TIMER);
	status_change_end(&ed->bl, SC_WATER_SCREEN, INVALID_TIMER);
	status_change_end(&ed->bl, SC_GUST, INVALID_TIMER);
	status_change_end(&ed->bl, SC_WIND_STEP, INVALID_TIMER);
	status_change_end(&ed->bl, SC_BLAST, INVALID_TIMER);
	status_change_end(&ed->bl, SC_WIND_CURTAIN, INVALID_TIMER);
	status_change_end(&ed->bl, SC_WILD_STORM, INVALID_TIMER);
	status_change_end(&ed->bl, SC_PETROLOGY, INVALID_TIMER);
	status_change_end(&ed->bl, SC_SOLID_SKIN, INVALID_TIMER);
	status_change_end(&ed->bl, SC_CURSED_SOIL, INVALID_TIMER);
	status_change_end(&ed->bl, SC_STONE_SHIELD, INVALID_TIMER);
	status_change_end(&ed->bl, SC_UPHEAVAL, INVALID_TIMER);
	status_change_end(&ed->bl, SC_CIRCLE_OF_FIRE, INVALID_TIMER);
	status_change_end(&ed->bl, SC_TIDAL_WEAPON, INVALID_TIMER);

	if( (sd = ed->master) == NULL )
		return 0;

	// Master side
	status_change_end(&sd->bl, SC_TROPIC_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_HEATER_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_AQUAPLAY_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_COOLER_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_CHILLY_AIR_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_PYROTECHNIC_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_FIRE_CLOAK_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_WATER_DROP_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_WATER_SCREEN_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_GUST_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_WIND_STEP_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_BLAST_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_WATER_DROP_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_WIND_CURTAIN_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_WILD_STORM_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_ZEPHYR, INVALID_TIMER);
	status_change_end(&sd->bl, SC_WIND_STEP_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_PETROLOGY_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_SOLID_SKIN_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_CURSED_SOIL_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_STONE_SHIELD_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_UPHEAVAL_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_CIRCLE_OF_FIRE_OPTION, INVALID_TIMER);
	status_change_end(&sd->bl, SC_TIDAL_WEAPON_OPTION, INVALID_TIMER);

	return 1;
}

int elemental_action(struct elemental_data *ed, struct block_list *bl, unsigned int tick) {
	struct skill_condition req;
	uint16 skill_id, skill_lv;
	int i;

	nullpo_ret(ed);
	nullpo_ret(bl);

	if( !ed->master )
		return 0;

	if( ed->target_id )
		elemental_unlocktarget(ed);	// Remove previous target.

	ARR_FIND(0, MAX_ELESKILLTREE, i, ed->db->skill[i].id && (ed->db->skill[i].mode&EL_SKILLMODE_AGGRESSIVE));
	if( i == MAX_ELESKILLTREE )
		return 0;

	skill_id = ed->db->skill[i].id;
	skill_lv = ed->db->skill[i].lv;

	if( elemental_skillnotok(skill_id, ed) )
		return 0;

	if( ed->ud.skilltimer != INVALID_TIMER )
		return 0;
	else if( DIFF_TICK(tick, ed->ud.canact_tick) < 0 )
		return 0;

	ed->target_id = ed->ud.skilltarget = bl->id;	// Set new target
	ed->last_thinktime = tick;

	// Not in skill range.
	if( !battle_check_range(&ed->bl,bl,skill_get_range(skill_id,skill_lv)) ) {
		// Try to walk to the target.
		if( !unit_walktobl(&ed->bl, bl, skill_get_range(skill_id,skill_lv), 2) )
			elemental_unlocktarget(ed);
		else {
			// Walking, waiting to be in range. Client don't handle it, then we must handle it here.
			int walk_dist = distance_bl(&ed->bl,bl) - skill_get_range(skill_id,skill_lv);
			ed->ud.skill_id = skill_id;
			ed->ud.skill_lv = skill_lv;

			if( skill_get_inf(skill_id) & INF_GROUND_SKILL )
				ed->ud.skilltimer = add_timer( tick+status_get_speed(&ed->bl)*walk_dist, skill_castend_pos, ed->bl.id, 0 );
			else
				ed->ud.skilltimer = add_timer( tick+status_get_speed(&ed->bl)*walk_dist, skill_castend_id, ed->bl.id, 0 );
		}
		return 1;

	}

	req = elemental_skill_get_requirements(skill_id, skill_lv);

	if(req.hp || req.sp){
		struct map_session_data *sd = BL_CAST(BL_PC, battle_get_master(&ed->bl));
		if( sd ){
			if( sd->skill_id_old != SO_EL_ACTION && //regardless of remaining HP/SP it can be cast
				(status_get_hp(&ed->bl) < req.hp || status_get_sp(&ed->bl) < req.sp) )
				return 1;
			else
				status_zap(&ed->bl, req.hp, req.sp);
		}
	}

	//Otherwise, just cast the skill.
	if( skill_get_inf(skill_id) & INF_GROUND_SKILL )
		unit_skilluse_pos(&ed->bl, bl->x, bl->y, skill_id, skill_lv);
	else
		unit_skilluse_id(&ed->bl, bl->id, skill_id, skill_lv);

	// Reset target.
	ed->target_id = 0;

	return 1;
}

/*===============================================================
 * Action that elemental perform after changing mode.
 * Activates one of the skills of the new mode.
 *-------------------------------------------------------------*/
int elemental_change_mode_ack(struct elemental_data *ed, enum elemental_skillmode skill_mode) {
	struct block_list *bl = &ed->master->bl;
	uint16 skill_id, skill_lv;
	int i;

	nullpo_ret(ed);

	if( !bl )
		return 0;

	// Select a skill.
	ARR_FIND(0, MAX_ELESKILLTREE, i, ed->db->skill[i].id && (ed->db->skill[i].mode&skill_mode));
	if( i == MAX_ELESKILLTREE )
		return 0;

	skill_id = ed->db->skill[i].id;
	skill_lv = ed->db->skill[i].lv;

	if( elemental_skillnotok(skill_id, ed) )
		return 0;

	if( ed->ud.skilltimer != INVALID_TIMER )
		return 0;
	else if( DIFF_TICK(gettick(), ed->ud.canact_tick) < 0 )
		return 0;

	ed->target_id = bl->id;	// Set new target
	ed->last_thinktime = gettick();

	if( skill_get_inf(skill_id) & INF_GROUND_SKILL )
		unit_skilluse_pos(&ed->bl, bl->x, bl->y, skill_id, skill_lv);
	else
		unit_skilluse_id(&ed->bl,bl->id,skill_id,skill_lv);

	ed->target_id = 0;	// Reset target after casting the skill  to avoid continious attack.

	return 1;
}

/*===============================================================
 * Change elemental mode.
 *-------------------------------------------------------------*/
int elemental_change_mode(struct elemental_data *ed, enum e_mode mode) {
	enum elemental_skillmode skill_mode;

	nullpo_ret(ed);

	// Remove target
	elemental_unlocktarget(ed);

	// Removes the effects of the previous mode.
	if(ed->elemental.mode != mode ) elemental_clean_effect(ed);

	ed->battle_status.mode = ed->elemental.mode = mode;

	// Normalize elemental mode to elemental skill mode.
	if( mode == EL_MODE_AGGRESSIVE ) skill_mode = EL_SKILLMODE_AGGRESSIVE;	// Aggressive spirit mode -> Aggressive spirit skill.
	else if( mode == EL_MODE_ASSIST ) skill_mode = EL_SKILLMODE_ASSIST;		// Assist spirit mode -> Assist spirit skill.
	else skill_mode = EL_SKILLMODE_PASSIVE;									// Passive spirit mode -> Passive spirit skill.

	// Use a skill inmediately after every change mode.
	if( skill_mode != EL_SKILLMODE_AGGRESSIVE )
		return elemental_change_mode_ack(ed, skill_mode);

	return 1;
}

void elemental_heal(struct elemental_data *ed, int hp, int sp) {
	if (ed->master == NULL)
		return;
	if( hp )
		clif_elemental_updatestatus(ed->master, SP_HP);
	if( sp )
		clif_elemental_updatestatus(ed->master, SP_SP);
}

int elemental_dead(struct elemental_data *ed) {
	elemental_delete(ed);
	return 0;
}

int elemental_unlocktarget(struct elemental_data *ed) {
	nullpo_ret(ed);

	ed->target_id = 0;
	elemental_stop_attack(ed);
	elemental_stop_walking(ed,1);
	return 0;
}

bool elemental_skillnotok(uint16 skill_id, struct elemental_data *ed) {
	uint16 idx = skill_get_index(skill_id);
	nullpo_retr(1,ed);
	return idx == 0 ? false : skill_isNotOk(skill_id,ed->master); // return false or check if it,s ok for master as well
}

struct skill_condition elemental_skill_get_requirements(uint16 skill_id, uint16 skill_lv){
	struct skill_condition req;
	uint16 idx = skill_get_index(skill_id);

	memset(&req,0,sizeof(req));

	if( idx == 0 ) // invalid skill id
		return req;

	skill_lv = cap_value(skill_lv, 1, MAX_SKILL_LEVEL);

	req.hp = skill_db[idx]->require.hp[skill_lv-1];
	req.sp = skill_db[idx]->require.sp[skill_lv-1];

	return req;
}

int elemental_set_target( struct map_session_data *sd, struct block_list *bl ) {
	struct elemental_data *ed = sd->ed;

	nullpo_ret(ed);
	nullpo_ret(bl);

	if( ed->bl.m != bl->m || !check_distance_bl(&ed->bl, bl, ed->db->range2) )
		return 0;

	if( !status_check_skilluse(&ed->bl, bl, 0, 0) )
		return 0;

	if( ed->target_id == 0 )
		ed->target_id = bl->id;

	return 1;
}

static int elemental_ai_sub_timer_activesearch(struct block_list *bl, va_list ap) {
	struct elemental_data *ed;
	struct block_list **target;
	int dist;

	nullpo_ret(bl);

	ed = va_arg(ap,struct elemental_data *);
	target = va_arg(ap,struct block_list**);

	//If can't seek yet, not an enemy, or you can't attack it, skip.
	if( (*target) == bl || !status_check_skilluse(&ed->bl, bl, 0, 0) )
		return 0;

	if( battle_check_target(&ed->bl,bl,BCT_ENEMY) <= 0 )
		return 0;

	switch( bl->type ) {
		case BL_PC:
			if( !map_flag_vs(ed->bl.m) )
				return 0;
		default:
			dist = distance_bl(&ed->bl, bl);
			if( ((*target) == NULL || !check_distance_bl(&ed->bl, *target, dist)) && battle_check_range(&ed->bl,bl,ed->db->range2) ) { //Pick closest target?
				(*target) = bl;
				ed->target_id = bl->id;
				ed->min_chase = dist + ed->db->range3;
				if( ed->min_chase > AREA_SIZE )
					ed->min_chase = AREA_SIZE;
				return 1;
			}
			break;
	}
	return 0;
}

static int elemental_ai_sub_timer(struct elemental_data *ed, struct map_session_data *sd, unsigned int tick) {
	struct block_list *target = NULL;
	int master_dist, view_range;
	enum e_mode mode;

	nullpo_ret(ed);
	nullpo_ret(sd);

	if( ed->bl.prev == NULL || sd == NULL || sd->bl.prev == NULL )
		return 0;

	// Check if caster can sustain the summoned elemental
	if( DIFF_TICK(tick,ed->last_spdrain_time) >= 10000 ){// Drain SP every 10 seconds
		int sp = 5;

		switch(ed->vd->class_){
			case ELEMENTALID_AGNI_M:	case ELEMENTALID_AQUA_M:
			case ELEMENTALID_VENTUS_M:	case ELEMENTALID_TERA_M:
				sp = 8;
				break;
			case ELEMENTALID_AGNI_L:	case ELEMENTALID_AQUA_L:
			case ELEMENTALID_VENTUS_L:	case ELEMENTALID_TERA_L:
				sp = 11;
				break;
		}

		if( status_get_sp(&sd->bl) < sp ){ // Can't sustain delete it.
			elemental_delete(sd->ed);
			return 0;
		}

		status_zap(&sd->bl,0,sp);
		ed->last_spdrain_time = tick;
	}

	if( DIFF_TICK(tick,ed->last_thinktime) < MIN_ELETHINKTIME )
		return 0;

	ed->last_thinktime = tick;

	if( ed->ud.skilltimer != INVALID_TIMER )
		return 0;

	if( ed->ud.walktimer != INVALID_TIMER && ed->ud.walkpath.path_pos <= 2 )
		return 0; //No thinking when you just started to walk.

	if(ed->ud.walkpath.path_pos < ed->ud.walkpath.path_len && ed->ud.target == sd->bl.id)
		return 0; //No thinking until be near the master.

	if( ed->sc.count && ed->sc.data[SC_BLIND] )
		view_range = 3;
	else
		view_range = ed->db->range2;

	mode = status_get_mode(&ed->bl);

	master_dist = distance_bl(&sd->bl, &ed->bl);
	if( master_dist > AREA_SIZE ) {	// Master out of vision range.
		elemental_unlocktarget(ed);
		unit_warp(&ed->bl,sd->bl.m,sd->bl.x,sd->bl.y,CLR_TELEPORT);
		clif_elemental_updatestatus(sd,SP_HP);
		clif_elemental_updatestatus(sd,SP_SP);
		return 0;
	} else if( master_dist > MAX_ELEDISTANCE ) {	// Master too far, chase.
		short x = sd->bl.x, y = sd->bl.y;
		if( ed->target_id )
			elemental_unlocktarget(ed);
		if( ed->ud.walktimer != INVALID_TIMER && ed->ud.target == sd->bl.id )
			return 0; //Already walking to him
		if( DIFF_TICK(tick, ed->ud.canmove_tick) < 0 )
			return 0; //Can't move yet.
		if( map_search_freecell(&ed->bl, sd->bl.m, &x, &y, MIN_ELEDISTANCE, MIN_ELEDISTANCE, 1)
		   && unit_walktoxy(&ed->bl, x, y, 0) )
			return 0;
	}

	if( mode == EL_MODE_AGGRESSIVE ) {
		target = map_id2bl(ed->ud.target);

		if( !target )
			map_foreachinallrange(elemental_ai_sub_timer_activesearch, &ed->bl, view_range, BL_CHAR, ed, &target, status_get_mode(&ed->bl));

		if( !target ) { //No targets available.
			elemental_unlocktarget(ed);
			return 1;
		}

		if( battle_check_range(&ed->bl,target,view_range) && rnd()%100 < 2 ) { // 2% chance to cast attack skill.
			if(	elemental_action(ed,target,tick) )
				return 1;
		}

		//Attempt to attack.
		//At this point we know the target is attackable, we just gotta check if the range matches.
		if( ed->ud.target == target->id && ed->ud.attacktimer != INVALID_TIMER ) //Already locked.
			return 1;

		if( battle_check_range(&ed->bl, target, ed->base_status.rhw.range) ) {//Target within range, engage
			unit_attack(&ed->bl,target->id,1);
			return 1;
		}

		//Follow up if possible.
		if( !unit_walktobl(&ed->bl, target, ed->base_status.rhw.range, 2) )
			elemental_unlocktarget(ed);
	}

	return 0;
}

static int elemental_ai_sub_foreachclient(struct map_session_data *sd, va_list ap) {
	unsigned int tick = va_arg(ap,unsigned int);
	if(sd->status.ele_id && sd->ed)
		elemental_ai_sub_timer(sd->ed,sd,tick);

	return 0;
}

static int elemental_ai_timer(int tid, unsigned int tick, int id, intptr_t data) {
	map_foreachpc(elemental_ai_sub_foreachclient,tick);
	return 0;
}

/**
* Reads Elemental DB lines
* ID,Sprite_Name,Name,LV,HP,SP,Range1,ATK1,ATK2,DEF,MDEF,STR,AGI,VIT,INT,DEX,LUK,Range2,Range3,Scale,Race,Element,Speed,aDelay,aMotion,dMotion
*/
static bool read_elementaldb_sub(char* str[], int columns, int current) {
	uint16 class_ = atoi(str[0]), i, ele;
	struct s_elemental_db *db;
	struct status_data *status;

	//Find the ID, already exist or not in elemental_db
	ARR_FIND(0,elemental_count,i,elemental_db[i].class_ == class_);
	if (i >= elemental_count)
		db = &elemental_db[elemental_count];
	else
		db = &elemental_db[i];
	
	db->class_ = atoi(str[0]);
	safestrncpy(db->sprite, str[1], NAME_LENGTH);
	safestrncpy(db->name, str[2], NAME_LENGTH);
	db->lv = atoi(str[3]);

	status = &db->status;
	db->vd.class_ = db->class_;

	status->max_hp = atoi(str[4]);
	status->max_sp = atoi(str[5]);
	status->rhw.range = atoi(str[6]);
	status->rhw.atk = atoi(str[7]);
	status->rhw.atk2 = atoi(str[8]);
	status->def = atoi(str[9]);
	status->mdef = atoi(str[10]);
	status->str = atoi(str[11]);
	status->agi = atoi(str[12]);
	status->vit = atoi(str[13]);
	status->int_ = atoi(str[14]);
	status->dex = atoi(str[15]);
	status->luk = atoi(str[16]);
	db->range2 = atoi(str[17]);
	db->range3 = atoi(str[18]);
	status->size = atoi(str[19]);
	status->race = atoi(str[20]);

	ele = atoi(str[21]);
	status->def_ele = ele%20;
	status->ele_lv = (unsigned char)floor(ele/20.);
	if( !CHK_ELEMENT(status->def_ele) ) {
		ShowWarning("read_elementaldb_sub: Elemental %d has invalid element type %d (max element is %d)\n", db->class_, status->def_ele, ELE_ALL - 1);
		status->def_ele = ELE_NEUTRAL;
	}
	if( !CHK_ELEMENT_LEVEL(status->ele_lv) ) {
		ShowWarning("read_elementaldb_sub: Elemental %d has invalid element level %d (max is %d)\n", db->class_, status->ele_lv, MAX_ELE_LEVEL);
		status->ele_lv = 1;
	}

	status->aspd_rate = 1000;
	status->speed = atoi(str[22]);
	status->adelay = atoi(str[23]);
	status->amotion = atoi(str[24]);
	status->dmotion = atoi(str[25]);

	if (i >= elemental_count)
		elemental_count++;
	return true;
}

void read_elementaldb(void) {
	const char *filename[] = { "elemental_db.txt", DBIMPORT"/elemental_db.txt" };
	uint8 i;

	elemental_count = 0;
	memset(elemental_db, 0, sizeof(elemental_db));
	for(i = 0; i<ARRAYLENGTH(filename); i++){
		sv_readdb(db_path, filename[i], ',', 26, 26, -1, &read_elementaldb_sub, i > 0);
	}
}

/**
* Reads Elemental Skill DB lines
* ElementalID,SkillID,SkillLevel,ReqMode
*/
static bool read_elemental_skilldb_sub(char* str[], int columns, int current) {
	uint16 class_ = atoi(str[0]), skill_id, skill_lv, skillmode;
	uint8 i;
	struct s_elemental_db *db;

	ARR_FIND(0, MAX_ELEMENTAL_CLASS, i, class_ == elemental_db[i].class_);
	if( i == MAX_ELEMENTAL_CLASS ) {
		ShowError("read_elemental_skilldb_sub: Class not found in elemental_db for skill entry, line %d.\n", current);
		return false;
	}

	skill_id = atoi(str[1]);
	if( !SKILL_CHK_ELEM(skill_id) ) {
		ShowError("read_elemental_skilldb_sub: Invalid Elemental skill '%d'.\n", skill_id);
		return false;
	}

	db = &elemental_db[i];
	skill_lv = atoi(str[2]);

	skillmode = atoi(str[3]);
	if( skillmode < EL_SKILLMODE_PASSIVE || skillmode > EL_SKILLMODE_AGGRESSIVE ) {
		ShowError("read_elemental_skilldb_sub: Skillmode out of range, line %d.\n",current);
		return false;
	}
	ARR_FIND( 0, MAX_ELESKILLTREE, i, db->skill[i].id == 0 || db->skill[i].id == skill_id );
	if( i == MAX_ELESKILLTREE ) {
		ShowWarning("read_elemental_skilldb_sub: Unable to load skill %d into Elemental %d's tree. Maximum number of skills per elemental has been reached.\n", skill_id, class_);
		return false;
	}
	db->skill[i].id = skill_id;
	db->skill[i].lv = skill_lv;
	db->skill[i].mode = skillmode;
	return true;
}

void read_elemental_skilldb(void) {
	const char *filename[] = { "elemental_skill_db.txt", DBIMPORT"/elemental_skill_db.txt" };
	uint8 i;
	for(i = 0; i<ARRAYLENGTH(filename); i++){
		sv_readdb(db_path, filename[i], ',', 4, 4, -1, &read_elemental_skilldb_sub, i > 0);
	}
}

void reload_elementaldb(void) {
	read_elementaldb();
	reload_elemental_skilldb();
}

void reload_elemental_skilldb(void) {
	read_elemental_skilldb();
}

void do_init_elemental(void) {
	read_elementaldb();
	read_elemental_skilldb();

	add_timer_func_list(elemental_ai_timer,"elemental_ai_timer");
	add_timer_interval(gettick()+MIN_ELETHINKTIME,elemental_ai_timer,0,0,MIN_ELETHINKTIME);
}

void do_final_elemental(void) {
	return;
}
