// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "elemental.hpp"

#include <cstring>
#include <cmath>
#include <stdlib.h>

#include <common/cbasetypes.hpp>
#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utils.hpp>

#include "battle.hpp"
#include "clif.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "pc.hpp"
#include "trade.hpp"

using namespace rathena;

ElementalDatabase elemental_db;

struct view_data * elemental_get_viewdata(int class_) {
	std::shared_ptr<s_elemental_db> db = elemental_db.find(class_);
	if (db == nullptr)
		return 0;

	return &db->vd;
}

int elemental_create(map_session_data *sd, int class_, unsigned int lifetime) {
	nullpo_retr(1,sd);

	std::shared_ptr<s_elemental_db> db = elemental_db.find(class_);
	if (db == nullptr) {
		ShowError("elemental_create: Unknown elemental class %d.\n", class_);
		return 0;
	}

	s_elemental ele = {};

	ele.char_id = sd->status.char_id;
	ele.class_ = class_;
	ele.mode = EL_MODE_PASSIVE; // Initial mode
	int i = db->status.size+1; // summon level

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
	case ELEMENTALID_AGNI_S:
	case ELEMENTALID_AGNI_M:
	case ELEMENTALID_AGNI_L:
	case ELEMENTALID_ARDOR://ATK + (Summon Agni Skill Level x 20) / HIT + (Summon Agni Skill Level x 10)
		ele.atk += i * 20;
		ele.atk2 += i * 20;
		ele.hit += i * 10;
		break;
	case ELEMENTALID_AQUA_S:
	case ELEMENTALID_AQUA_M:
	case ELEMENTALID_AQUA_L:
	case ELEMENTALID_DILUVIO://MDEF + (Summon Aqua Skill Level x 10) / MATK + (Summon Aqua Skill Level x 20)
		ele.mdef += i * 10;
		ele.matk += i * 20;
		break;
	case ELEMENTALID_VENTUS_S:
	case ELEMENTALID_VENTUS_M:
	case ELEMENTALID_VENTUS_L:
	case ELEMENTALID_PROCELLA://FLEE + (Summon Ventus Skill Level x 20) / MATK + (Summon Ventus Skill Level x 10)
		ele.flee += i * 20;
		ele.matk += i * 10;
		break;
	case ELEMENTALID_TERA_S:
	case ELEMENTALID_TERA_M:
	case ELEMENTALID_TERA_L:
	case ELEMENTALID_TERREMOTUS:
	case ELEMENTALID_SERPENS://DEF + (Summon Tera Skill Level x 25) / ATK + (Summon Tera Skill Level x 5)
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

	if ((i = pc_checkskill(sd, EM_ELEMENTAL_SPIRIT_M)) > 0 && db->class_ >= ELEMENTALID_DILUVIO && db->class_ <= ELEMENTALID_SERPENS) {
		ele.hp = ele.max_hp += 10000 + 3000 * i;
		ele.sp = ele.max_sp += 100 * i;
		ele.atk += 100 + 20 * i;
		ele.atk2 += 100 + 20 * i;
		ele.matk += 20 * i;
		ele.def += 20 * i;
		ele.mdef += 4 * i;
		ele.flee += 10 * i;
	}

	ele.life_time = lifetime;

	// Request Char Server to create this elemental
	intif_elemental_create(&ele);

	return 1;
}

t_tick elemental_get_lifetime(s_elemental_data *ed) {
	if( ed == NULL || ed->summon_timer == INVALID_TIMER )
		return 0;

	const struct TimerData * td = get_timer(ed->summon_timer);
	return (td != NULL) ? DIFF_TICK(td->tick, gettick()) : 0;
}

int elemental_save(s_elemental_data *ed) {
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

static TIMER_FUNC(elemental_summon_end){
	map_session_data *sd;

	if( (sd = map_id2sd(id)) == NULL )
		return 1;

	s_elemental_data *ed;

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

void elemental_summon_stop(s_elemental_data *ed) {
	nullpo_retv(ed);
	if( ed->summon_timer != INVALID_TIMER )
		delete_timer(ed->summon_timer, elemental_summon_end);
	ed->summon_timer = INVALID_TIMER;
}

int elemental_delete(s_elemental_data *ed) {
	nullpo_ret(ed);

	map_session_data *sd = ed->master;
	ed->elemental.life_time = 0;

	elemental_clean_effect(ed);
	elemental_summon_stop(ed);

	if( !sd )
		return unit_free(&ed->bl, CLR_OUTSIGHT);

	sd->ed = NULL;
	sd->status.ele_id = 0;

	return unit_remove_map(&ed->bl, CLR_OUTSIGHT);
}

void elemental_summon_init(s_elemental_data *ed) {
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
int elemental_data_received(s_elemental *ele, bool flag) {
	map_session_data *sd;
	t_tick tick = gettick();

	if( (sd = map_charid2sd(ele->char_id)) == NULL )
		return 0;

	std::shared_ptr<s_elemental_db> db = elemental_db.find(ele->class_);

	if( !flag || db == nullptr ) { // Not created - loaded - DB info
		sd->status.ele_id = 0;
		return 0;
	}

	s_elemental_data *ed;

	if( !sd->ed ) {	// Initialize it after first summon.
		sd->ed = ed = (s_elemental_data*)aCalloc(1,sizeof(s_elemental_data));
		ed->bl.type = BL_ELEM;
		ed->bl.id = npc_get_new_npc_id();
		ed->master = sd;
		ed->db = db;
		memcpy(&ed->elemental, ele, sizeof(s_elemental));
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

		// Ticks need to be initialized before adding bl to map_addiddb
		ed->regen.tick.hp = tick;
		ed->regen.tick.sp = tick;

		map_addiddb(&ed->bl);
		status_calc_elemental(ed,SCO_FIRST);
		ed->last_spdrain_time = ed->last_thinktime = gettick();
		ed->summon_timer = INVALID_TIMER;
		ed->masterteleport_timer = INVALID_TIMER;
		elemental_summon_init(ed);
	} else {
		memcpy(&sd->ed->elemental, ele, sizeof(s_elemental));
		ed = sd->ed;
	}

	sd->status.ele_id = ele->elemental_id;

	if( ed->bl.prev == NULL && sd->bl.prev != NULL ) {
		if(map_addblock(&ed->bl))
			return 0;
		clif_spawn(&ed->bl);
		clif_elemental_info(sd);
		clif_elemental_updatestatus(sd,SP_HP);
		clif_hpmeter_single( *sd, ed->bl.id, ed->battle_status.hp, ed->battle_status.max_hp );
		clif_elemental_updatestatus(sd,SP_SP);
	}

	return 1;
}

int elemental_clean_effect(s_elemental_data *ed) {
	nullpo_ret(ed);

	status_db.removeByStatusFlag(&ed->bl, { SCF_REMOVEELEMENTALOPTION });
	status_db.removeByStatusFlag(battle_get_master(&ed->bl), { SCF_REMOVEELEMENTALOPTION });

	return 1;
}

int elemental_action(s_elemental_data *ed, block_list *bl, t_tick tick) {
	nullpo_ret(ed);
	nullpo_ret(bl);

	if( !ed->master )
		return 0;

	if( ed->target_id )
		elemental_unlocktarget(ed);	// Remove previous target.

	std::shared_ptr<s_elemental_skill> skill = util::umap_find(ed->db->skill, EL_SKILLMODE_AGGRESSIVE);	// only one skill per mode is supported
	if (skill == nullptr)
		return 0;

	uint16 skill_id = skill->id;
	uint16 skill_lv = skill->lv;

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
				ed->ud.skilltimer = add_timer( tick+(t_tick)status_get_speed(&ed->bl)*walk_dist, skill_castend_pos, ed->bl.id, 0 );
			else
				ed->ud.skilltimer = add_timer( tick+(t_tick)status_get_speed(&ed->bl)*walk_dist, skill_castend_id, ed->bl.id, 0 );
		}
		return 1;
	}

	s_skill_condition req = elemental_skill_get_requirements(skill_id, skill_lv);

	if(req.hp || req.sp){
		map_session_data *sd = BL_CAST(BL_PC, battle_get_master(&ed->bl));
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
int elemental_change_mode_ack(s_elemental_data *ed, e_elemental_skillmode skill_mode) {
	nullpo_ret(ed);

	block_list *bl = &ed->master->bl;
	if( !bl )
		return 0;

	std::shared_ptr<s_elemental_skill> skill = util::umap_find(ed->db->skill, skill_mode);
	if (skill == nullptr)
		return 0;

	uint16 skill_id = skill->id;
	uint16 skill_lv = skill->lv;

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
int elemental_change_mode(s_elemental_data *ed, int mode) {
	nullpo_ret(ed);

	// Remove target
	elemental_unlocktarget(ed);

	// Removes the effects of the previous mode.
	if(ed->elemental.mode != mode ) elemental_clean_effect(ed);

	ed->battle_status.mode = ed->elemental.mode = mode;
	e_elemental_skillmode skill_mode;

	// Normalize elemental mode to elemental skill mode.
	if( mode == EL_MODE_AGGRESSIVE ) skill_mode = EL_SKILLMODE_AGGRESSIVE;	// Aggressive spirit mode -> Aggressive spirit skill.
	else if( mode == EL_MODE_ASSIST ) skill_mode = EL_SKILLMODE_ASSIST;		// Assist spirit mode -> Assist spirit skill.
	else skill_mode = EL_SKILLMODE_PASSIVE;									// Passive spirit mode -> Passive spirit skill.

	// Use a skill inmediately after every change mode.
	if( skill_mode != EL_SKILLMODE_AGGRESSIVE )
		return elemental_change_mode_ack(ed, skill_mode);

	return 1;
}

void elemental_heal(s_elemental_data *ed, int hp, int sp) {
	if (ed->master == NULL)
		return;
	if( hp )
		clif_elemental_updatestatus(ed->master, SP_HP);
	if( sp )
		clif_elemental_updatestatus(ed->master, SP_SP);
}

int elemental_dead(s_elemental_data *ed) {
	elemental_delete(ed);
	return 0;
}

int elemental_unlocktarget(s_elemental_data *ed) {
	nullpo_ret(ed);

	ed->target_id = 0;
	elemental_stop_attack(ed);
	elemental_stop_walking(ed,1);
	return 0;
}

bool elemental_skillnotok(uint16 skill_id, s_elemental_data *ed) {
	uint16 idx = skill_get_index(skill_id);
	nullpo_retr(1,ed);
	return idx == 0 ? false : skill_isNotOk(skill_id,ed->master); // return false or check if it,s ok for master as well
}

struct s_skill_condition elemental_skill_get_requirements(uint16 skill_id, uint16 skill_lv){
	s_skill_condition req = {};
	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	if( !skill ) // invalid skill id
		return req;

	skill_lv = cap_value(skill_lv, 1, MAX_SKILL_LEVEL);
	req.hp = skill->require.hp[skill_lv - 1];
	req.sp = skill->require.sp[skill_lv - 1];

	return req;
}

int elemental_set_target( map_session_data *sd, block_list *bl ) {
	s_elemental_data *ed = sd->ed;

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

static int elemental_ai_sub_timer_activesearch(block_list *bl, va_list ap) {
	nullpo_ret(bl);

	s_elemental_data *ed;
	block_list **target;

	ed = va_arg(ap, s_elemental_data *);
	target = va_arg(ap, block_list**);

	//If can't seek yet, not an enemy, or you can't attack it, skip.
	if( (*target) == bl || !status_check_skilluse(&ed->bl, bl, 0, 0) )
		return 0;

	if( battle_check_target(&ed->bl,bl,BCT_ENEMY) <= 0 )
		return 0;

	int dist;

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

static int elemental_ai_sub_timer(s_elemental_data *ed, map_session_data *sd, t_tick tick) {
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

	int master_dist, view_range;

	if( ed->sc.count && ed->sc.getSCE(SC_BLIND) )
		view_range = 3;
	else
		view_range = ed->db->range2;

	int mode = status_get_mode(&ed->bl);

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

	block_list *target = NULL;

	if( mode == EL_MODE_AGGRESSIVE ) {
		target = map_id2bl(ed->ud.target);

		if( !target )
			map_foreachinallrange(elemental_ai_sub_timer_activesearch, &ed->bl, view_range, BL_CHAR, ed, &target, status_get_mode(&ed->bl));

		if( !target ) { //No targets available.
			elemental_unlocktarget(ed);
			return 1;
		}

		if( battle_check_range(&ed->bl,target,view_range) && rnd_chance(2, 100) ) { // 2% chance to cast attack skill.
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

static int elemental_ai_sub_foreachclient(map_session_data *sd, va_list ap) {
	t_tick tick = va_arg(ap,t_tick);
	if(sd->status.ele_id && sd->ed)
		elemental_ai_sub_timer(sd->ed,sd,tick);

	return 0;
}

static TIMER_FUNC(elemental_ai_timer){
	map_foreachpc(elemental_ai_sub_foreachclient,tick);
	return 0;
}

const std::string ElementalDatabase::getDefaultLocation() {
	return std::string(db_path) + "/elemental_db.yml";
}

/*
 * Reads and parses an entry from the elemental_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 ElementalDatabase::parseBodyNode(const ryml::NodeRef& node) {
	int32 id;

	if (!this->asInt32(node, "Id", id))
		return 0;

	if( !( ( id >= ELEMENTALID_AGNI_S && id <= ELEMENTALID_TERA_L ) || ( id >= ELEMENTALID_DILUVIO && id <= ELEMENTALID_SERPENS ) ) ) {
		this->invalidWarning( node["Id"], "Invalid Id %d (valid ranges: %d-%d and %d-%d).\n", id, ELEMENTALID_AGNI_S, ELEMENTALID_TERA_L, ELEMENTALID_DILUVIO, ELEMENTALID_SERPENS );
		return 0;
	}

	std::shared_ptr<s_elemental_db> elemental = this->find(id);
	bool exists = elemental != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "AegisName", "Name", "Size", "Element", "ElementLevel" }))
			return 0;

		elemental = std::make_shared<s_elemental_db>();
		elemental->class_ = id;
		elemental->vd.class_ = id;
	}

	if (this->nodeExists(node, "AegisName")) {
		std::string name;

		if (!this->asString(node, "AegisName", name))
			return 0;

		if (name.size() > NAME_LENGTH) {
			this->invalidWarning(node["AegisName"], "AegisName \"%s\" exceeds maximum of %d characters, capping...\n", name.c_str(), NAME_LENGTH - 1);
		}

		name.resize(NAME_LENGTH);
		elemental->sprite = name;
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;


		if (name.size() > NAME_LENGTH) {
			this->invalidWarning(node["Name"], "Name \"%s\" exceeds maximum of %d characters, capping...\n", name.c_str(), NAME_LENGTH - 1);
		}

		name.resize(NAME_LENGTH);
		elemental->name = name;
	}

	if (this->nodeExists(node, "Level")) {
		uint16 level;

		if (!this->asUInt16(node, "Level", level))
			return 0;

		if (level > MAX_LEVEL) {
			this->invalidWarning(node["Level"], "Level %d exceeds MAX_LEVEL, capping to %d.\n", level, MAX_LEVEL);
			level = MAX_LEVEL;
		}

		elemental->lv = level;
	} else {
		if (!exists)
			elemental->lv = 1;
	}

	if (this->nodeExists(node, "Hp")) {
		uint32 hp;

		if (!this->asUInt32(node, "Hp", hp))
			return 0;

		elemental->status.max_hp = hp;
	} else {
		if (!exists)
			elemental->status.max_hp = 0;
	}

	if (this->nodeExists(node, "Sp")) {
		uint32 sp;

		if (!this->asUInt32(node, "Sp", sp))
			return 0;

		elemental->status.max_sp = sp;
	} else {
		if (!exists)
			elemental->status.max_sp = 1;
	}

	if (this->nodeExists(node, "Attack")) {
		uint16 atk;

		if (!this->asUInt16(node, "Attack", atk))
			return 0;

		elemental->status.rhw.atk = atk;
	} else {
		if (!exists)
			elemental->status.rhw.atk = 1;
	}

	if (this->nodeExists(node, "Attack2")) {
		uint16 atk;

		if (!this->asUInt16(node, "Attack2", atk))
			return 0;

#ifdef RENEWAL
		elemental->status.rhw.matk = atk;
#else
		elemental->status.rhw.atk2 = atk;
#endif
	} else {
		if (!exists)
#ifdef RENEWAL
			elemental->status.rhw.matk = 0;
#else
			elemental->status.rhw.atk2 = 0;
#endif
	}

	if (this->nodeExists(node, "Defense")) {
		int32 def;

		if (!this->asInt32(node, "Defense", def))
			return 0;

		if (def < DEFTYPE_MIN || def > DEFTYPE_MAX) {
			this->invalidWarning(node["Defense"], "Invalid defense %d, capping...\n", def);
			def = cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX);
		}

		elemental->status.def = static_cast<defType>(def);
	} else {
		if (!exists)
			elemental->status.def = 0;
	}

	if (this->nodeExists(node, "MagicDefense")) {
		int32 def;

		if (!this->asInt32(node, "MagicDefense", def))
			return 0;

		if (def < DEFTYPE_MIN || def > DEFTYPE_MAX) {
			this->invalidWarning(node["MagicDefense"], "Invalid magic defense %d, capping...\n", def);
			def = cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX);
		}

		elemental->status.mdef = static_cast<defType>(def);
	} else {
		if (!exists)
			elemental->status.mdef = 0;
	}

	if (this->nodeExists(node, "Str")) {
		uint16 stat;

		if (!this->asUInt16(node, "Str", stat))
			return 0;

		elemental->status.str = stat;
	} else {
		if (!exists)
			elemental->status.str = 0;
	}

	if (this->nodeExists(node, "Agi")) {
		uint16 stat;

		if (!this->asUInt16(node, "Agi", stat))
			return 0;

		elemental->status.agi = stat;
	} else {
		if (!exists)
			elemental->status.agi = 0;
	}

	if (this->nodeExists(node, "Vit")) {
		uint16 stat;

		if (!this->asUInt16(node, "Vit", stat))
			return 0;

		elemental->status.vit = stat;
	} else {
		if (!exists)
			elemental->status.vit = 0;
	}

	if (this->nodeExists(node, "Int")) {
		uint16 stat;

		if (!this->asUInt16(node, "Int", stat))
			return 0;

		elemental->status.int_ = stat;
	} else {
		if (!exists)
			elemental->status.int_ = 0;
	}

	if (this->nodeExists(node, "Dex")) {
		uint16 stat;

		if (!this->asUInt16(node, "Dex", stat))
			return 0;

		elemental->status.dex = stat;
	} else {
		if (!exists)
			elemental->status.dex = 0;
	}

	if (this->nodeExists(node, "Luk")) {
		uint16 stat;

		if (!this->asUInt16(node, "Luk", stat))
			return 0;

		elemental->status.luk = stat;
	} else {
		if (!exists)
			elemental->status.luk = 0;
	}

	if (this->nodeExists(node, "AttackRange")) {
		uint16 range;

		if (!this->asUInt16(node, "AttackRange", range))
			return 0;

		elemental->status.rhw.range = range;
	} else {
		if (!exists)
			elemental->status.rhw.range = 0;
	}

	if (this->nodeExists(node, "SkillRange")) {
		uint16 range;

		if (!this->asUInt16(node, "SkillRange", range))
			return 0;

		elemental->range2 = range;
	} else {
		if (!exists)
			elemental->range2 = 5;
	}

	if (this->nodeExists(node, "ChaseRange")) {
		uint16 range;

		if (!this->asUInt16(node, "ChaseRange", range))
			return 0;

		elemental->range3 = range;
	} else {
		if (!exists)
			elemental->range3 = 12;
	}

	if (this->nodeExists(node, "Size")) {
		std::string size;

		if (!this->asString(node, "Size", size))
			return 0;

		std::string size_constant = "Size_" + size;
		int64 constant;

		if (!script_get_constant(size_constant.c_str(), &constant) || constant < SZ_SMALL || constant > SZ_BIG) {
			this->invalidWarning(node["Size"], "Invalid size %s, defaulting to Small.\n", size.c_str());
			constant = SZ_SMALL;
		}

		elemental->status.size = static_cast<e_size>(constant);
	}

	if (this->nodeExists(node, "Race")) {
		std::string race;

		if (!this->asString(node, "Race", race))
			return 0;

		std::string race_constant = "RC_" + race;
		int64 constant;

		if (!script_get_constant(race_constant.c_str(), &constant) || !CHK_RACE(constant)) {
			this->invalidWarning(node["Race"], "Invalid race %s, defaulting to Formless.\n", race.c_str());
			constant = RC_FORMLESS;
		}

		elemental->status.race = static_cast<e_race>(constant);
	} else {
		if (!exists)
			elemental->status.race = RC_FORMLESS;
	}

	if (this->nodeExists(node, "Element")) {
		std::string ele;

		if (!this->asString(node, "Element", ele))
			return 0;

		std::string ele_constant = "ELE_" + ele;
		int64 constant;

		if (!script_get_constant(ele_constant.c_str(), &constant) || !CHK_ELEMENT(constant)) {
			this->invalidWarning(node["Element"], "Invalid element %s, defaulting to Neutral.\n", ele.c_str());
			constant = ELE_NEUTRAL;
		}

		elemental->status.def_ele = static_cast<e_element>(constant);
	}

	if (this->nodeExists(node, "ElementLevel")) {
		uint16 level;

		if (!this->asUInt16(node, "ElementLevel", level))
			return 0;

		if (!CHK_ELEMENT_LEVEL(level)) {
			this->invalidWarning(node["ElementLevel"], "Invalid element level %hu, defaulting to 1.\n", level);
			level = 1;
		}

		elemental->status.ele_lv = static_cast<uint8>(level);
	}

	if (this->nodeExists(node, "WalkSpeed")) {
		uint16 speed;

		if (!this->asUInt16(node, "WalkSpeed", speed))
			return 0;

		if (speed < MIN_WALK_SPEED || speed > MAX_WALK_SPEED) {
			this->invalidWarning(node["WalkSpeed"], "Invalid walk speed %hu, capping...\n", speed);
			speed = cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);
		}

		elemental->status.speed = speed;
	} else {
		if (!exists)
			elemental->status.speed = 200;
	}

	if (this->nodeExists(node, "AttackDelay")) {
		uint16 speed;

		if (!this->asUInt16(node, "AttackDelay", speed))
			return 0;

		elemental->status.adelay = cap_value(speed, 0, 4000);
	} else {
		if (!exists)
			elemental->status.adelay = 504;
	}

	if (this->nodeExists(node, "AttackMotion")) {
		uint16 speed;

		if (!this->asUInt16(node, "AttackMotion", speed))
			return 0;

		elemental->status.amotion = cap_value(speed, 0, 2000);
	} else {
		if (!exists)
			elemental->status.amotion = 1020;
	}

	if (this->nodeExists(node, "DamageMotion")) {
		uint16 speed;

		if (!this->asUInt16(node, "DamageMotion", speed))
			return 0;

		elemental->status.dmotion = speed;
	} else {
		if (!exists)
			elemental->status.dmotion = 360;
	}

	elemental->status.aspd_rate = 1000;

	if (this->nodeExists(node, "Mode")) {
		const ryml::NodeRef& ModeNode = node["Mode"];

		for (const auto &Modeit : ModeNode) {
			std::string mode_name;
			c4::from_chars(Modeit.key(), &mode_name);

			std::string mode_constant = "EL_SKILLMODE_" + mode_name;
			int64 constant;

			if (!script_get_constant(mode_constant.c_str(), &constant) || constant < EL_SKILLMODE_PASSIVE || constant > EL_SKILLMODE_AGGRESSIVE) {
				this->invalidWarning(node["Mode"], "Invalid mode %s, skipping.\n", mode_name.c_str());
				continue;
			}

			e_elemental_skillmode mode = static_cast<e_elemental_skillmode>(constant);

			std::shared_ptr<s_elemental_skill> entry = util::umap_find(elemental->skill, mode);
			bool mode_exists = entry != nullptr;

			if (!mode_exists)
				entry = std::make_shared<s_elemental_skill>();

			const ryml::NodeRef& SkillNode = ModeNode[Modeit.key()];
			std::string skill_name;

			if (!this->asString(SkillNode, "Skill", skill_name))
				return 0;

			uint16 skill_id = skill_name2id(skill_name.c_str());

			if (skill_id == 0) {
				this->invalidWarning(SkillNode["Skill"], "Skipping invalid skill %s for mode %s.\n", skill_name.c_str(), mode_name.c_str());
				return 0;
			}

			if (!SKILL_CHK_ELEM(skill_id)) {
				this->invalidWarning(SkillNode["Skill"], "Skill %s (%u) is out of the skill range [%u-%u], skipping.\n", skill_name.c_str(), skill_id, EL_SKILLBASE, EL_SKILLBASE + MAX_ELEMENTALSKILL - 1);
				return 0;
			}

			entry->id = skill_id;

			if (this->nodeExists(SkillNode, "Level")) {
				uint16 level;

				if (!this->asUInt16(SkillNode, "Level", level))
					return 0;

				if (level > skill_get_max(skill_id)) {
					this->invalidWarning(SkillNode["Level"], "Skill %s's level %hu exceeds max level %hu.\n", skill_name.c_str(), level, skill_get_max(skill_id));
					return 0;
				}

				entry->lv = level;
			} else {
				if (!mode_exists)
					entry->lv = 1;
			}

			if (entry->lv == 0) {
				elemental->skill.erase(mode);
				return 0;
			}

			if (!mode_exists)
				elemental->skill.insert({ mode, entry });
		}
	}

	if (!exists)
		this->put(id, elemental);

	return 1;
}

void do_init_elemental(void) {
	elemental_db.load();

	add_timer_func_list(elemental_ai_timer,"elemental_ai_timer");
	add_timer_interval(gettick()+MIN_ELETHINKTIME,elemental_ai_timer,0,0,MIN_ELETHINKTIME);
}

void do_final_elemental(void) {
	elemental_db.clear();
}
