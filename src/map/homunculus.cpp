// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus.hpp"

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

struct s_homunculus_db homunculus_db[MAX_HOMUNCULUS_CLASS];	//[orn]
struct homun_skill_tree_entry hskill_tree[MAX_HOMUNCULUS_CLASS][MAX_HOM_SKILL_TREE];

static int hom_hungry(int tid, unsigned int tick, int id, intptr_t data);
static uint16 homunculus_count;
static unsigned int hexptbl[MAX_LEVEL];

//For holding the view data of npc classes. [Skotlex]
static struct view_data hom_viewdb[MAX_HOMUNCULUS_CLASS];

struct s_homun_intimacy_grade {
	//const char *grade;
	uint32 min_value;
};

/// Intimacy grade, order based on enum e_homun_grade
static struct s_homun_intimacy_grade intimacy_grades[] = {
	{ /*"Hate with passion",*/   100 },
	{ /*"Hate",             */   400 },
	{ /*"Awkward",          */  1100 },
	{ /*"Shy",              */ 10100 },
	{ /*"Neutral",          */ 25100 },
	{ /*"Cordial",          */ 75100 },
	{ /*"Loyal",            */ 91100 },
};

/**
* Check if the skill is a valid homunculus skill based skill range or availablity in skill db
* @param skill_id
* @return -1 if invalid skill or skill index for homunculus skill in s_homunculus::hskill
*/
short hom_skill_get_index(uint16 skill_id) {
	if (!SKILL_CHK_HOMUN(skill_id))
		return -1;
	skill_id -= HM_SKILLBASE;
	if (skill_id >= MAX_HOMUNSKILL)
		return -1;
	return skill_id;
}

/**
* Check homunculus class for array look up
* @param class_
* @return Class index or -1 if invalid class
*/
static short hom_class2index(int class_) {
	if (homdb_checkid(class_))
		return class_ - HM_CLASS_BASE;
	return -1;
}

/**
* Get homunculus view data
* @param class_ Homunculus class
* @return vd
*/
struct view_data* hom_get_viewdata(int class_)
{	//Returns the viewdata for homunculus
	if (homdb_checkid(class_))
		return &hom_viewdb[class_-HM_CLASS_BASE];
	return NULL;
}

/**
* Get homunculus type of specified class_
* @param class_
* @return enum homun_type
*/
enum homun_type hom_class2type(int class_) {
	int mid = hom_class2mapid(class_);
	if((mid&(HOM_REG|HOM_EVO)) == (HOM_REG|HOM_EVO))
		return HT_EVO;
	else if(mid&(HOM_REG))
		return HT_REG;
	else if(mid&(HOM_S))
		return HT_S;
	else
		return HT_INVALID;
}

/**
* Get homunculus MAPID from specified class
* @param hom_class
* @return Homunculus MAPID (see enum hom_mapid)
*/
int hom_class2mapid(int hom_class)
{
	switch(hom_class)
	{
		// Normal Homunculus
		case 6001: case 6005:	return MAPID_LIF;
		case 6002: case 6006:	return MAPID_AMISTR;
		case 6003: case 6007:	return MAPID_FILIR;
		case 6004: case 6008:	return MAPID_VANILMIRTH;
		// Evolved Homunculus
		case 6009: case 6013:	return MAPID_LIF_E;
		case 6010: case 6014:	return MAPID_AMISTR_E;
		case 6011: case 6015:	return MAPID_FILIR_E;
		case 6012: case 6016:	return MAPID_VANILMIRTH_E;
		// Homunculus S
		case 6048:				return MAPID_EIRA;
		case 6049:				return MAPID_BAYERI;
		case 6050:				return MAPID_SERA;
		case 6051:				return MAPID_DIETER;
		case 6052:				return MAPID_ELANOR;

		default:				return -1;
	}
}

/**
* Add homunculus spirit ball
* @param hd
* @param max Maximum number of spirit ball
*/
void hom_addspiritball(TBL_HOM *hd, int max) {
	nullpo_retv(hd);

	if (max > MAX_SPIRITBALL)
		max = MAX_SPIRITBALL;
	if (hd->homunculus.spiritball < 0)
		hd->homunculus.spiritball = 0;

	if (hd->homunculus.spiritball && hd->homunculus.spiritball >= max)
		hd->homunculus.spiritball = max;
	else
		hd->homunculus.spiritball++;

	clif_spiritball(&hd->bl);
}

/**
* Delete homunculus spirit ball
* @param hd
* @param count Number spirit ball will be deleted
* @param type 1 - Update client
*/
void hom_delspiritball(TBL_HOM *hd, int count, int type) {
	nullpo_retv(hd);

	if (hd->homunculus.spiritball <= 0) {
		hd->homunculus.spiritball = 0;
		return;
	}
	if (count <= 0)
		return;
	if (count > MAX_SPIRITBALL)
		count = MAX_SPIRITBALL;
	if (count > hd->homunculus.spiritball)
		count = hd->homunculus.spiritball;

	hd->homunculus.spiritball -= count;
	if (!type)
		clif_spiritball(&hd->bl);
}

/**
* Update homunculus info to its master after receiving damage
* @param hd
*/
void hom_damage(struct homun_data *hd) {
	if (hd->master)
		clif_hominfo(hd->master,hd,0);
}

/**
* Set homunculus's dead status
* @param hd
* @return flag &1 - Standard dead, &2 - Remove object from map, &4 - Delete object from memory
*/
int hom_dead(struct homun_data *hd)
{
	//There's no intimacy penalties on death (from Tharis)
	struct map_session_data *sd = hd->master;

	clif_emotion(&hd->bl, ET_KEK);

	//Delete timers when dead.
	hom_hungry_timer_delete(hd);
	hd->homunculus.hp = 0;

	if (!sd) //unit remove map will invoke unit free
		return 3;

	clif_emotion(&sd->bl, ET_CRY);
	//Remove from map (if it has no intimacy, it is auto-removed from memory)
	return 3;
}

/**
* Vaporize a character's homunculus
* @param sd
* @param flag 1: then HP needs to be 80% or above. 2: then set to morph state.
*/
int hom_vaporize(struct map_session_data *sd, int flag)
{
	struct homun_data *hd;

	nullpo_ret(sd);

	hd = sd->hd;
	if (!hd || hd->homunculus.vaporize)
		return 0;

	if (status_isdead(&hd->bl))
		return 0; //Can't vaporize a dead homun.

	if (flag == HOM_ST_REST && get_percentage(hd->battle_status.hp, hd->battle_status.max_hp) < 80)
		return 0;

	hd->regen.state.block = 3; //Block regen while vaporized.
	//Delete timers when vaporized.
	hom_hungry_timer_delete(hd);
	hd->homunculus.vaporize = flag ? flag : HOM_ST_REST;
	if (battle_config.hom_setting&HOMSET_RESET_REUSESKILL_VAPORIZED)
		memset(hd->blockskill, 0, sizeof(hd->blockskill));
	clif_hominfo(sd, sd->hd, 0);
	hom_save(hd);
	return unit_remove_map(&hd->bl, CLR_OUTSIGHT);
}

/**
* Delete a homunculus, completely "killing it".
* Emote is the emotion the master should use, send negative to disable.
* @param hd
* @param emote
*/
int hom_delete(struct homun_data *hd, int emote)
{
	struct map_session_data *sd;
	nullpo_ret(hd);
	sd = hd->master;

	if (!sd)
		return unit_free(&hd->bl,CLR_DEAD);

	if (emote >= 0)
		clif_emotion(&sd->bl, emote);

	//This makes it be deleted right away.
	hd->homunculus.intimacy = 0;
	// Send homunculus_dead to client
	hd->homunculus.hp = 0;
	clif_hominfo(sd, hd, 0);
	return unit_remove_map(&hd->bl,CLR_OUTSIGHT);
}

/**
* Calculates homunculus skill tree
* @param hd
* @param flag_envolve
*/
void hom_calc_skilltree(struct homun_data *hd, bool flag_evolve) {
	uint8 i;
	short c = 0;

	nullpo_retv(hd);

	/* load previous homunculus form skills first. */
	if (hd->homunculus.prev_class != 0 && (c = hom_class2index(hd->homunculus.prev_class)) >= 0) {
		for (i = 0; i < MAX_HOM_SKILL_TREE; i++) {
			uint16 skill_id;
			short idx = -1;
			bool fail = false;
			if (!(skill_id = hskill_tree[c][i].id) || (idx = hom_skill_get_index(skill_id)) == -1)
				continue;
			if (hd->homunculus.hskill[idx].id)
				continue; //Skill already known.
			if (!battle_config.skillfree) {
				uint8 j;
				if (hskill_tree[c][i].need_level > hd->homunculus.level)
					continue;
				for (j = 0; j < MAX_HOM_SKILL_REQUIRE; j++) {
					if (hskill_tree[c][i].need[j].id &&
						hom_checkskill(hd,hskill_tree[c][i].need[j].id) < hskill_tree[c][i].need[j].lv)
					{
						fail = true;
						break;
					}
				}
			}
			if (!fail)
				hd->homunculus.hskill[idx].id = skill_id;
		}
	}


	if ((c = hom_class2index(hd->homunculus.class_)) < 0)
		return;

	for (i = 0; i < MAX_HOM_SKILL_TREE; i++) {
		unsigned int intimacy = 0;
		uint16 skill_id;
		short idx = -1;
		bool fail = false;
		if (!(skill_id = hskill_tree[c][i].id) || (idx = hom_skill_get_index(skill_id)) == -1)
			continue;
		if (hd->homunculus.hskill[idx].id)
			continue; //Skill already known.
		intimacy = (flag_evolve) ? 10 : hd->homunculus.intimacy;
		if (intimacy < hskill_tree[c][i].intimacy * 100)
			continue;
		if (!battle_config.skillfree) {
			uint8 j;
			if (hskill_tree[c][i].need_level > hd->homunculus.level)
				continue;
			for (j = 0; j < MAX_HOM_SKILL_REQUIRE; j++) {
				if (hskill_tree[c][i].need[j].id &&
					hom_checkskill(hd,hskill_tree[c][i].need[j].id) < hskill_tree[c][i].need[j].lv)
				{
					fail = true;
					break;
				}
			}
		}
		if (!fail)
			hd->homunculus.hskill[idx].id = skill_id;
	}

	if (hd->master)
		clif_homskillinfoblock(hd->master);
}

/**
* Check skill from homunculus
* @param hd
* @param skill_id
* @return Skill Level or 0 if invalid or unlearned skill
*/
short hom_checkskill(struct homun_data *hd,uint16 skill_id)
{
	short idx = hom_skill_get_index(skill_id);
	if (idx < 0) // Invalid skill
		return 0;

	if (!hd)
		return 0;

	if (hd->homunculus.hskill[idx].id == skill_id)
		return (hd->homunculus.hskill[idx].lv);

	return 0;
}

/**
* Get max level for homunculus skill
* @param id Skill ID
* @param b_class
* @return Skill Level
*/
int hom_skill_tree_get_max(int skill_id, int b_class){
	uint8 i;

	if ((b_class = hom_class2index(b_class)) < 0)
		return 0;
	ARR_FIND(0, MAX_HOM_SKILL_TREE, i, hskill_tree[b_class][i].id == skill_id);
	if (i < MAX_HOM_SKILL_TREE)
		return hskill_tree[b_class][i].max;
	return skill_get_max(skill_id);
}

 /**
 * Get required minimum level to learn the skill
 * @param class_ Homunculus class
 * @param skill_id Homunculus skill ID
 * @return Level required or 0 if invalid
 **/
uint8 hom_skill_get_min_level(int class_, uint16 skill_id) {
	short class_idx = hom_class2index(class_), skill_idx = -1;
	uint8 i;

	if (class_idx == -1 || (skill_idx = hom_skill_get_index(skill_id)) == -1)
		return 0;
	ARR_FIND(0, MAX_HOM_SKILL_REQUIRE, i, hskill_tree[class_idx][i].id == skill_id);
	if (i == MAX_HOM_SKILL_REQUIRE)
		return 0;

	return hskill_tree[class_idx][i].need_level;
}

/**
 * Level up an homunculus skill
 * @param hd
 * @param skill_id
 */
void hom_skillup(struct homun_data *hd, uint16 skill_id)
{
	short idx = 0;
	nullpo_retv(hd);

	if (hd->homunculus.vaporize)
		return;

	if ((idx = hom_skill_get_index(skill_id)) < 0)
		return;
	if (hd->homunculus.skillpts > 0 &&
		hd->homunculus.hskill[idx].id &&
		hd->homunculus.hskill[idx].flag == SKILL_FLAG_PERMANENT && //Don't allow raising while you have granted skills. [Skotlex]
		hd->homunculus.level >= hom_skill_get_min_level(hd->homunculus.class_, skill_id) &&
		hd->homunculus.hskill[idx].lv < hom_skill_tree_get_max(skill_id, hd->homunculus.class_)
		)
	{
		hd->homunculus.hskill[idx].lv++;
		hd->homunculus.skillpts-- ;
		status_calc_homunculus(hd, SCO_NONE);
		if (hd->master) {
			clif_homskillup(hd->master, skill_id);
			clif_hominfo(hd->master,hd,0);
			clif_homskillinfoblock(hd->master);
		}
	}
}

/**
* Homunculus leveled up
* @param hd
*/
int hom_levelup(struct homun_data *hd)
{
	struct s_homunculus *hom;
	struct h_stats *min = NULL, *max = NULL;
	int growth_str, growth_agi, growth_vit, growth_int, growth_dex, growth_luk ;
	int growth_max_hp, growth_max_sp ;
	int m_class;

	if ((m_class = hom_class2mapid(hd->homunculus.class_)) == -1) {
		ShowError("hom_levelup: Invalid class %d. \n", hd->homunculus.class_);
		return 0;
	}

	/// When homunculus is homunculus S, we check to see if we need to apply previous class stats
	if(m_class&HOM_S && hd->homunculus.level < battle_config.hom_S_growth_level) {
		int i;
		if (!hd->homunculus.prev_class) {
			/// We also need to be sure that the previous class exists, otherwise give it something to work with
			hd->homunculus.prev_class = 6001;
		}
		// Give the homunculus the level up stats database it needs
		i = hom_search(hd->homunculus.prev_class,HOMUNCULUS_CLASS);
		if (i < 0) // Nothing should go wrong here, but check anyways
			return 0;
		max = &homunculus_db[i].gmax;
		min = &homunculus_db[i].gmin;
	}

	if (((m_class&HOM_REG) && hd->homunculus.level >= battle_config.hom_max_level)
		|| ((m_class&HOM_S) && hd->homunculus.level >= battle_config.hom_S_max_level)
		|| !hd->exp_next || hd->homunculus.exp < hd->exp_next)
		return 0;

	hom = &hd->homunculus;
	hom->level++ ;
	if (!(hom->level % 3))
		hom->skillpts++ ;	//1 skillpoint each 3 base level

	hom->exp -= hd->exp_next ;
	hd->exp_next = hexptbl[hom->level - 1] ;

	if (!max) {
		max  = &hd->homunculusDB->gmax;
		min  = &hd->homunculusDB->gmin;
	}

	growth_max_hp = rnd_value(min->HP, max->HP);
	growth_max_sp = rnd_value(min->SP, max->SP);
	growth_str = rnd_value(min->str, max->str);
	growth_agi = rnd_value(min->agi, max->agi);
	growth_vit = rnd_value(min->vit, max->vit);
	growth_dex = rnd_value(min->dex, max->dex);
	growth_int = rnd_value(min->int_,max->int_);
	growth_luk = rnd_value(min->luk, max->luk);

	//Aegis discards the decimals in the stat growth values!
	growth_str-=growth_str%10;
	growth_agi-=growth_agi%10;
	growth_vit-=growth_vit%10;
	growth_dex-=growth_dex%10;
	growth_int-=growth_int%10;
	growth_luk-=growth_luk%10;

	hom->max_hp += growth_max_hp;
	hom->max_sp += growth_max_sp;
	hom->str += growth_str;
	hom->agi += growth_agi;
	hom->vit += growth_vit;
	hom->dex += growth_dex;
	hom->int_+= growth_int;
	hom->luk += growth_luk;

	APPLY_HOMUN_LEVEL_STATWEIGHT();

	// Needed to update skill list for mutated homunculus so unlocked skills will appear when the needed level is reached.
	status_calc_homunculus(hd,SCO_NONE);
	clif_hominfo(hd->master,hd,0);
	clif_homskillinfoblock(hd->master);

	if ( hd->master && battle_config.homunculus_show_growth ) {
		char output[256] ;
		sprintf(output,
			"Growth: hp:%d sp:%d str(%.2f) agi(%.2f) vit(%.2f) int(%.2f) dex(%.2f) luk(%.2f) ",
			growth_max_hp, growth_max_sp,
			growth_str/10.0, growth_agi/10.0, growth_vit/10.0,
			growth_int/10.0, growth_dex/10.0, growth_luk/10.0);
		clif_messagecolor(&hd->master->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
	}
	return 1;
}

/**
* Changes homunculus class
* @param hd
* @param class_ old class
* @reutrn Fals if the class cannot be changed, True if otherwise
*/
static bool hom_change_class(struct homun_data *hd, short class_) {
	int i;
	i = hom_search(class_,HOMUNCULUS_CLASS);
	if (i < 0)
		return false;
	hd->homunculusDB = &homunculus_db[i];
	hd->homunculus.class_ = class_;
	status_set_viewdata(&hd->bl, class_);
	hom_calc_skilltree(hd, 1);
	return true;
}

/**
 * Make an homonculus evolve, (changing in evolution class and apply bonus)
 * @param hd : homonculus datas
 * @return 0:failure, 1:success
 */
int hom_evolution(struct homun_data *hd)
{
	struct s_homunculus *hom;
	struct h_stats *max, *min;
	struct map_session_data *sd;
	nullpo_ret(hd);

	if(!hd->homunculusDB->evo_class || hd->homunculus.class_ == hd->homunculusDB->evo_class) {
		clif_emotion(&hd->bl, ET_SWEAT);
		return 0 ;
	}
	sd = hd->master;
	if (!sd)
		return 0;

	if (!hom_change_class(hd, hd->homunculusDB->evo_class)) {
		ShowError("hom_evolution: Can't evolve homunc from %d to %d", hd->homunculus.class_, hd->homunculusDB->evo_class);
		return 0;
	}

	//Apply evolution bonuses
	hom = &hd->homunculus;
	max = &hd->homunculusDB->emax;
	min = &hd->homunculusDB->emin;
	hom->max_hp += rnd_value(min->HP, max->HP);
	hom->max_sp += rnd_value(min->SP, max->SP);
	hom->str += 10*rnd_value(min->str, max->str);
	hom->agi += 10*rnd_value(min->agi, max->agi);
	hom->vit += 10*rnd_value(min->vit, max->vit);
	hom->int_+= 10*rnd_value(min->int_,max->int_);
	hom->dex += 10*rnd_value(min->dex, max->dex);
	hom->luk += 10*rnd_value(min->luk, max->luk);
	hom->intimacy = battle_config.homunculus_evo_intimacy_reset;

	unit_remove_map(&hd->bl, CLR_OUTSIGHT);
	if (map_addblock(&hd->bl))
		return 0;

	clif_spawn(&hd->bl);
	clif_emotion(&sd->bl, ET_BEST);
	clif_specialeffect(&hd->bl,EF_HO_UP,AREA);

	//status_Calc flag&1 will make current HP/SP be reloaded from hom structure
	hom->hp = hd->battle_status.hp;
	hom->sp = hd->battle_status.sp;
	status_calc_homunculus(hd, SCO_FIRST);

	if (!(battle_config.hom_setting&HOMSET_NO_INSTANT_LAND_SKILL))
		skill_unit_move(&sd->hd->bl,gettick(),1); // apply land skills immediately

	return 1 ;
}

/**
 * Make an homonculus mutate in renewal homon
 * @param hd : homonculus datas
 * @param homun_id : id to make it transform into (must be a valid homon class)
 * @return 0:failure, 1:sucess
 */
int hom_mutate(struct homun_data *hd, int homun_id)
{
	struct s_homunculus *hom;
	struct map_session_data *sd;
	int m_class, m_id, prev_class = 0;
	nullpo_ret(hd);

	m_class = hom_class2mapid(hd->homunculus.class_);
	m_id    = hom_class2mapid(homun_id);

	if( m_class == -1 || m_id == -1 || !(m_class&HOM_EVO) || !(m_id&HOM_S) ) {
		clif_emotion(&hd->bl, ET_SWEAT);
		return 0;
	}

	sd = hd->master;
	if (!sd)
		return 0;

	prev_class = hd->homunculus.class_;

	if (!hom_change_class(hd, homun_id)) {
		ShowError("hom_mutate: Can't evolve homunc from %d to %d", hd->homunculus.class_, homun_id);
		return 0;
	}

	unit_remove_map(&hd->bl, CLR_OUTSIGHT);
	if(map_addblock(&hd->bl))
		return 0;

	clif_spawn(&hd->bl);
	clif_emotion(&sd->bl, ET_BEST);
	clif_specialeffect(&hd->bl,EF_HO_UP,AREA);

	//status_Calc flag&1 will make current HP/SP be reloaded from hom structure
	hom = &hd->homunculus;
	hom->hp = hd->battle_status.hp;
	hom->sp = hd->battle_status.sp;
	hom->prev_class = prev_class;
	status_calc_homunculus(hd, SCO_FIRST);

	if (!(battle_config.hom_setting&HOMSET_NO_INSTANT_LAND_SKILL))
		skill_unit_move(&sd->hd->bl,gettick(),1); // apply land skills immediately

	return 1;
}

/**
* Add homunculus exp
* @param hd
* @param exp Added EXP
*/
void hom_gainexp(struct homun_data *hd,int exp)
{
	int m_class;

	nullpo_retv(hd);

	if(hd->homunculus.vaporize)
		return;

	if((m_class = hom_class2mapid(hd->homunculus.class_)) == -1) {
		ShowError("hom_gainexp: Invalid class %d. \n", hd->homunculus.class_);
		return;
	}

	if( hd->exp_next == 0 ||
		((m_class&HOM_REG) && hd->homunculus.level >= battle_config.hom_max_level) ||
		((m_class&HOM_S)   && hd->homunculus.level >= battle_config.hom_S_max_level) )
	{
		hd->homunculus.exp = 0;
		return;
	}

	hd->homunculus.exp += exp;

	if (hd->master && hd->homunculus.exp < hd->exp_next) {
		clif_hominfo(hd->master,hd,0);
		return;
	}

 	// Do the levelup(s)
	while( hd->homunculus.exp > hd->exp_next ){
		// Max level reached or error
		if( !hom_levelup(hd) ){
			break;
		}
	}

	if( hd->exp_next == 0 )
		hd->homunculus.exp = 0 ;

	clif_specialeffect(&hd->bl,EF_HO_UP,AREA);
	status_calc_homunculus(hd, SCO_NONE);
	status_percent_heal(&hd->bl, 100, 100);
}

/**
* Increase homunculu sintimacy
* @param hd
* @param value Added intimacy
* @return New intimacy value
*/
int hom_increase_intimacy(struct homun_data * hd, unsigned int value)
{
	nullpo_ret(hd);
	if (battle_config.homunculus_friendly_rate != 100)
		value = (value * battle_config.homunculus_friendly_rate) / 100;

	if (hd->homunculus.intimacy + value <= 100000)
		hd->homunculus.intimacy += value;
	else
		hd->homunculus.intimacy = 100000;
	return hd->homunculus.intimacy;
}

/**
* Decrease homunculu sintimacy
* @param hd
* @param value Reduced intimacy
* @return New intimacy value
*/
int hom_decrease_intimacy(struct homun_data * hd, unsigned int value)
{
	nullpo_ret(hd);
	if (hd->homunculus.intimacy >= value)
		hd->homunculus.intimacy -= value;
	else
		hd->homunculus.intimacy = 0;

	return hd->homunculus.intimacy;
}

/**
* Update homunculus info to master after healing
* @param hd
*/
void hom_heal(struct homun_data *hd) {
	if (hd->master)
		clif_hominfo(hd->master,hd,0);
}

/**
* Save homunculus data
* @param hd
*/
void hom_save(struct homun_data *hd)
{
	// copy data that must be saved in homunculus struct ( hp / sp )
	TBL_PC *sd;

	nullpo_retv(hd);

	sd = hd->master;
	//Do not check for max_hp/max_sp caps as current could be higher to max due
	//to status changes/skills (they will be capped as needed upon stat
	//calculation on login)
	hd->homunculus.hp = hd->battle_status.hp;
	hd->homunculus.sp = hd->battle_status.sp;
	intif_homunculus_requestsave(sd->status.account_id, &hd->homunculus);
}

/**
* Perform requested action from selected homunculus menu
* @param sd
* @param type
*/
void hom_menu(struct map_session_data *sd, int type)
{
	nullpo_retv(sd);
	if (sd->hd == NULL)
		return;

	switch(type) {
		case 0:
			break;
		case 1:
			hom_food(sd, sd->hd);
			break;
		case 2:
			hom_delete(sd->hd, -1);
			break;
		default:
			ShowError("hom_menu : unknown menu choice : %d\n", type);
			break;
	}
}

/**
* Feed homunculus
* @param sd
* @param hd
*/
int hom_food(struct map_session_data *sd, struct homun_data *hd)
{
	int i, foodID, emotion;

	nullpo_retr(1,sd);
	nullpo_retr(1,hd);

	if (hd->homunculus.vaporize)
		return 1;

	foodID = hd->homunculusDB->foodID;
	i = pc_search_inventory(sd,foodID);
	if (i < 0) {
		clif_hom_food(sd,foodID,0);
		return 1;
	}
	pc_delitem(sd,i,1,0,0,LOG_TYPE_CONSUME);

	if ( hd->homunculus.hunger >= 91 ) {
		hom_decrease_intimacy(hd, 50);
		emotion = ET_KEK;
	} else if ( hd->homunculus.hunger >= 76 ) {
		hom_decrease_intimacy(hd, 5);
		emotion = ET_PROFUSELY_SWEAT;
	} else if ( hd->homunculus.hunger >= 26 ) {
		hom_increase_intimacy(hd, 75);
		emotion = ET_DELIGHT;
	} else if ( hd->homunculus.hunger >= 11 ) {
		hom_increase_intimacy(hd, 100);
		emotion = ET_DELIGHT;
	} else {
		hom_increase_intimacy(hd, 50);
		emotion = ET_DELIGHT;
	}

	hd->homunculus.hunger += 10;	//dunno increase value for each food
	if(hd->homunculus.hunger > 100)
		hd->homunculus.hunger = 100;

	log_feeding(sd, LOG_FEED_HOMUNCULUS, foodID);

	clif_emotion(&hd->bl,emotion);
	clif_send_homdata(sd,SP_HUNGRY,hd->homunculus.hunger);
	clif_send_homdata(sd,SP_INTIMATE,hd->homunculus.intimacy / 100);
	clif_hom_food(sd,foodID,1);

	// Too much food :/
	if(hd->homunculus.intimacy == 0)
		return hom_delete(sd->hd, ET_HUK);

	return 0;
}

/**
* Timer to reduce hunger level
*/
static int hom_hungry(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;
	struct homun_data *hd;

	sd = map_id2sd(id);
	if (!sd)
		return 1;

	if (!sd->status.hom_id || !(hd=sd->hd))
		return 1;

	if (hd->hungry_timer != tid) {
		ShowError("hom_hungry_timer %d != %d\n",hd->hungry_timer,tid);
		return 0;
	}

	hd->hungry_timer = INVALID_TIMER;

	hd->homunculus.hunger--;
	if(hd->homunculus.hunger <= 10) {
		clif_emotion(&hd->bl, ET_FRET);
	} else if(hd->homunculus.hunger == 25) {
		clif_emotion(&hd->bl, ET_SCRATCH);
	} else if(hd->homunculus.hunger == 75) {
		clif_emotion(&hd->bl, ET_OK);
	}

	if (hd->homunculus.hunger < 0) {
		hd->homunculus.hunger = 0;
		// Delete the homunculus if intimacy <= 100
		if (!hom_decrease_intimacy(hd, 100))
			return hom_delete(hd, ET_HUK);
		clif_send_homdata(sd,SP_INTIMATE,hd->homunculus.intimacy / 100);
	}

	clif_send_homdata(sd,SP_HUNGRY,hd->homunculus.hunger);
	hd->hungry_timer = add_timer(tick+hd->homunculusDB->hungryDelay,hom_hungry,sd->bl.id,0); //simple Fix albator
	return 0;
}

/**
* Remove hungry timer from homunculus
* @param hd
*/
int hom_hungry_timer_delete(struct homun_data *hd)
{
	nullpo_ret(hd);
	if (hd->hungry_timer != INVALID_TIMER) {
		delete_timer(hd->hungry_timer,hom_hungry);
		hd->hungry_timer = INVALID_TIMER;
	}
	return 1;
}

/**
* Change homunculus name
*/
int hom_change_name(struct map_session_data *sd,char *name)
{
	int i;
	struct homun_data *hd;
	nullpo_retr(1, sd);

	hd = sd->hd;
	if (!hom_is_active(hd))
		return 1;
	if (hd->homunculus.rename_flag && !battle_config.hom_rename)
		return 1;

	for (i = 0; i < NAME_LENGTH && name[i];i++) {
		if (!(name[i]&0xe0) || name[i] == 0x7f)
			return 1;
	}

	return intif_rename_hom(sd, name);
}

/**
* Acknowledge change name request from inter-server
* @param sd
* @param name
* @param flag
*/
void hom_change_name_ack(struct map_session_data *sd, char* name, int flag)
{
	struct homun_data *hd = sd->hd;
	if (!hom_is_active(hd))
		return;

	normalize_name(name," ");//bugreport:3032

	if (!flag || name[0] == '\0') {
		clif_displaymessage(sd->fd, msg_txt(sd,280)); // You cannot use this name
		return;
	}
	safestrncpy(hd->homunculus.name,name,NAME_LENGTH);
	clif_name_area(&hd->bl);
	hd->homunculus.rename_flag = 1;
	clif_hominfo(sd,hd,0);
}

/**
* Search homunculus info (food or next class)
* @param key
* @param type see enum e_hom_search_type
* @return info found
*/
int hom_search(int key, int type)
{
	int i;

	for (i = 0; i < homunculus_count; i++) {
		if (homunculus_db[i].base_class <= 0)
			continue;
		switch (type) {
			case HOMUNCULUS_CLASS:
				if (homunculus_db[i].base_class == key ||
					homunculus_db[i].evo_class == key)
					return i;
				break;
			case HOMUNCULUS_FOOD:
				if (homunculus_db[i].foodID == key)
					return i;
				break;
			default:
				return -1;
		}
	}
	return -1;
}

/**
* Create homunc structure
* @param sd
* @param hom
*/
void hom_alloc(struct map_session_data *sd, struct s_homunculus *hom)
{
	struct homun_data *hd;
	int i = 0;

	nullpo_retv(sd);

	Assert((sd->status.hom_id == 0 || sd->hd == 0) || sd->hd->master == sd);

	i = hom_search(hom->class_,HOMUNCULUS_CLASS);
	if(i < 0) {
		ShowError("hom_alloc: unknown class [%d] for homunculus '%s', requesting deletion.\n", hom->class_, hom->name);
		sd->status.hom_id = 0;
		intif_homunculus_requestdelete(hom->hom_id);
		return;
	}
	sd->hd = hd = (struct homun_data*)aCalloc(1,sizeof(struct homun_data));
	hd->bl.type = BL_HOM;
	hd->bl.id = npc_get_new_npc_id();

	hd->master = sd;
	hd->homunculusDB = &homunculus_db[i];
	memcpy(&hd->homunculus, hom, sizeof(struct s_homunculus));
	hd->exp_next = hexptbl[hd->homunculus.level - 1];

	status_set_viewdata(&hd->bl, hd->homunculus.class_);
	status_change_init(&hd->bl);
	unit_dataset(&hd->bl);
	hd->ud.dir = sd->ud.dir;

	// Find a random valid pos around the player
	hd->bl.m = sd->bl.m;
	hd->bl.x = sd->bl.x;
	hd->bl.y = sd->bl.y;
	unit_calc_pos(&hd->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
	hd->bl.x = hd->ud.to_x;
	hd->bl.y = hd->ud.to_y;

	map_addiddb(&hd->bl);
	status_calc_homunculus(hd, SCO_FIRST);

	hd->hungry_timer = INVALID_TIMER;
	hd->masterteleport_timer = INVALID_TIMER;
}

/**
* Init homunculus timers
* @param hd
*/
void hom_init_timers(struct homun_data * hd)
{
	if (hd->hungry_timer == INVALID_TIMER)
		hd->hungry_timer = add_timer(gettick()+hd->homunculusDB->hungryDelay,hom_hungry,hd->master->bl.id,0);
	hd->regen.state.block = 0; //Restore HP/SP block.
	hd->masterteleport_timer = INVALID_TIMER;
}

/**
 * Make a player spawn a homonculus (call)
 * @param sd
 * @return False:failure, True:sucess
 */
bool hom_call(struct map_session_data *sd)
{
	struct homun_data *hd;

	if (!sd->status.hom_id) //Create a new homun.
		return hom_create_request(sd, HM_CLASS_BASE + rnd_value(0, 7)) ;

	// If homunc not yet loaded, load it
	if (!sd->hd)
		return intif_homunculus_requestload(sd->status.account_id, sd->status.hom_id) > 0;

	hd = sd->hd;

	if (!hd->homunculus.vaporize)
		return false; //Can't use this if homun wasn't vaporized.

	if (hd->homunculus.vaporize == HOM_ST_MORPH)
		return false; // Can't call homunculus (morph state).

	hom_init_timers(hd);
	hd->homunculus.vaporize = HOM_ST_ACTIVE;
	if (hd->bl.prev == NULL)
	{	//Spawn him
		hd->bl.x = sd->bl.x;
		hd->bl.y = sd->bl.y;
		hd->bl.m = sd->bl.m;
		if(map_addblock(&hd->bl))
			return false;
		clif_spawn(&hd->bl);
		clif_send_homdata(sd,SP_ACK,0);
		clif_hominfo(sd,hd,1);
		clif_hominfo(sd,hd,0); // send this x2. dunno why, but kRO does that [blackhole89]
		clif_homskillinfoblock(sd);
		if (battle_config.slaves_inherit_speed&1)
			status_calc_bl(&hd->bl, SCB_SPEED);
		hom_save(hd);
	} else
		//Warp him to master.
		unit_warp(&hd->bl,sd->bl.m, sd->bl.x, sd->bl.y,CLR_OUTSIGHT);
	return true;
}

/**
 * Receive homunculus data from char server
 * @param account_id : owner account_id of the homon
 * @param sh : homonculus data from char-serv
 * @param flag : does the creation in inter-serv was a success (0:no,1:yes)
 * @return 0:failure, 1:sucess
 */
int hom_recv_data(uint32 account_id, struct s_homunculus *sh, int flag)
{
	struct map_session_data *sd;
	struct homun_data *hd;
	bool created = false;

	sd = map_id2sd(account_id);
	if(!sd)
		return 0;
	if (sd->status.char_id != sh->char_id)
	{
		if (sd->status.hom_id == sh->hom_id)
			sh->char_id = sd->status.char_id; //Correct char id.
		else
			return 0;
	}
	if(!flag) { // Failed to load
		sd->status.hom_id = 0;
		return 0;
	}

	if (!sd->status.hom_id) { //Hom just created.
		sd->status.hom_id = sh->hom_id;
		created = true;
	}
	if (sd->hd) //uh? Overwrite the data.
		memcpy(&sd->hd->homunculus, sh, sizeof(struct s_homunculus));
	else
		hom_alloc(sd, sh);

	hd = sd->hd;
	if (created)
		status_percent_heal(&hd->bl, 100, 100);

	if(hd && hd->homunculus.hp && !hd->homunculus.vaporize && hd->bl.prev == NULL && sd->bl.prev != NULL)
	{
		if(map_addblock(&hd->bl))
			return 0;
		clif_spawn(&hd->bl);
		clif_send_homdata(sd,SP_ACK,0);
		clif_hominfo(sd,hd,1);
		clif_hominfo(sd,hd,0); // send this x2. dunno why, but kRO does that [blackhole89]
		clif_homskillinfoblock(sd);
		hom_init_timers(hd);
	}
	return 1;
}

/**
* Ask homunculus creation to char-server
* @param sd
* @param class_
* @return True:Success; False:Failed
*/
bool hom_create_request(struct map_session_data *sd, int class_)
{
	struct s_homunculus homun;
	struct h_stats *base;
	int i;

	nullpo_ret(sd);

	i = hom_search(class_,HOMUNCULUS_CLASS);
	if(i < 0)
		return false;

	memset(&homun, 0, sizeof(struct s_homunculus));
	//Initial data
	safestrncpy(homun.name, homunculus_db[i].name, NAME_LENGTH-1);
	homun.class_ = class_;
	homun.level = 1;
	homun.hunger = 32; //32%
	homun.intimacy = 2100; //21/1000
	homun.char_id = sd->status.char_id;

	homun.hp = 10 ;
	base = &homunculus_db[i].base;
	homun.max_hp = base->HP;
	homun.max_sp = base->SP;
	homun.str = base->str *10;
	homun.agi = base->agi *10;
	homun.vit = base->vit *10;
	homun.int_= base->int_*10;
	homun.dex = base->dex *10;
	homun.luk = base->luk *10;

	// Request homunculus creation
	intif_homunculus_create(sd->status.account_id, &homun);
	return true;
}

/**
 * Make a player resurect an homon (player must have one)
 * @param sd : player pointer
 * @param per : hp percentage to revive homon
 * @param x : x map coordinate
 * @param y : Y map coordinate
 * @return 0:failure, 1:success
 */
int hom_ressurect(struct map_session_data* sd, unsigned char per, short x, short y)
{
	struct homun_data* hd;
	nullpo_ret(sd);

	if (!sd->status.hom_id)
		return 0; // no homunculus

	if (!sd->hd) //Load homun data;
		return intif_homunculus_requestload(sd->status.account_id, sd->status.hom_id);

	hd = sd->hd;

	if (hd->homunculus.vaporize == HOM_ST_REST)
		return 0; // vaporized homunculi need to be 'called'

	if (!status_isdead(&hd->bl))
		return 0; // already alive

	hom_init_timers(hd);

	if (!hd->bl.prev)
	{	//Add it back to the map.
		hd->bl.m = sd->bl.m;
		hd->bl.x = x;
		hd->bl.y = y;
		if(map_addblock(&hd->bl))
			return 0;
		clif_spawn(&hd->bl);
	}
	return status_revive(&hd->bl, per, 0);
}

/**
* Revive homunculus
* @param hd
* @param hp
* @param sp
*/
void hom_revive(struct homun_data *hd, unsigned int hp, unsigned int sp)
{
	struct map_session_data *sd = hd->master;
	hd->homunculus.hp = hd->battle_status.hp;
	if (!sd)
		return;
	clif_send_homdata(sd,SP_ACK,0);
	clif_hominfo(sd,hd,1);
	clif_hominfo(sd,hd,0);
	clif_homskillinfoblock(sd);
	if (hd->homunculus.class_ == 6052) //eleanor
		sc_start(&hd->bl,&hd->bl, SC_STYLE_CHANGE, 100, MH_MD_FIGHTING, -1);
}

/**
* Reset homunculus status
* @param hd
*/
void hom_reset_stats(struct homun_data *hd)
{	//Resets a homunc stats back to zero (but doesn't touches hunger or intimacy)
	struct s_homunculus_db *db;
	struct s_homunculus *hom;
	struct h_stats *base;
	hom = &hd->homunculus;
	db = hd->homunculusDB;
	base = &db->base;
	hom->level = 1;
	hom->hp = 10;
	hom->max_hp = base->HP;
	hom->max_sp = base->SP;
	hom->str = base->str *10;
	hom->agi = base->agi *10;
	hom->vit = base->vit *10;
	hom->int_= base->int_*10;
	hom->dex = base->dex *10;
	hom->luk = base->luk *10;
	hom->exp = 0;
	hd->exp_next = hexptbl[0];
	memset(&hd->homunculus.hskill, 0, sizeof hd->homunculus.hskill);
	hd->homunculus.skillpts = 0;
}

/**
* Shuffle homunculus status
* @param hd
*/
int hom_shuffle(struct homun_data *hd)
{
	struct map_session_data *sd;
	int lv, i, skillpts;
	unsigned int exp;
	struct s_skill b_skill[MAX_HOMUNSKILL];

	if (!hom_is_active(hd))
		return 0;

	sd = hd->master;
	lv = hd->homunculus.level;
	exp = hd->homunculus.exp;
	memcpy(&b_skill, &hd->homunculus.hskill, sizeof(b_skill));
	skillpts = hd->homunculus.skillpts;
	//Reset values to level 1.
	hom_reset_stats(hd);
	//Level it back up
	for (i = 1; i < lv && hd->exp_next; i++){
		hd->homunculus.exp += hd->exp_next;
		// Should never happen, but who knows
		if( !hom_levelup(hd) ) {
			break;
		}
	}

	if(hd->homunculus.class_ == hd->homunculusDB->evo_class) {
		//Evolved bonuses
		struct s_homunculus *hom = &hd->homunculus;
		struct h_stats *max = &hd->homunculusDB->emax, *min = &hd->homunculusDB->emin;
		hom->max_hp += rnd_value(min->HP, max->HP);
		hom->max_sp += rnd_value(min->SP, max->SP);
		hom->str += 10*rnd_value(min->str, max->str);
		hom->agi += 10*rnd_value(min->agi, max->agi);
		hom->vit += 10*rnd_value(min->vit, max->vit);
		hom->int_+= 10*rnd_value(min->int_,max->int_);
		hom->dex += 10*rnd_value(min->dex, max->dex);
		hom->luk += 10*rnd_value(min->luk, max->luk);
	}

	hd->homunculus.exp = exp;
	memcpy(&hd->homunculus.hskill, &b_skill, sizeof(b_skill));
	hd->homunculus.skillpts = skillpts;
	clif_homskillinfoblock(sd);
	status_calc_homunculus(hd, SCO_NONE);
	status_percent_heal(&hd->bl, 100, 100);
	clif_specialeffect(&hd->bl,EF_HO_UP,AREA);

	return 1;
}

/**
 * Get minimum intimacy value of specified grade
 * @param grade see enum e_homun_grade
 * @return Intimacy value
 **/
uint32 hom_intimacy_grade2intimacy(enum e_homun_grade grade) {
	if (grade < HOMGRADE_HATE_WITH_PASSION || grade > HOMGRADE_LOYAL)
		return 0;
	return intimacy_grades[grade].min_value;
}

/**
 * Get grade of given intimacy value
 * @param intimacy
 * @return Grade, see enum e_homun_grade
 **/
enum e_homun_grade hom_intimacy_intimacy2grade(uint32 intimacy) {
#define CHK_HOMINTIMACY(grade) { if (intimacy >= intimacy_grades[(grade)].min_value) return (grade); }
	CHK_HOMINTIMACY(HOMGRADE_LOYAL)
	CHK_HOMINTIMACY(HOMGRADE_CORDIAL)
	CHK_HOMINTIMACY(HOMGRADE_NEUTRAL)
	CHK_HOMINTIMACY(HOMGRADE_SHY)
	CHK_HOMINTIMACY(HOMGRADE_AWKWARD)
	CHK_HOMINTIMACY(HOMGRADE_HATE)
#undef CHK_HOMINTIMACY
	return HOMGRADE_HATE_WITH_PASSION;
}

/**
* Get initmacy grade
* @param hd
*/
uint8 hom_get_intimacy_grade(struct homun_data *hd) {
	return hom_intimacy_intimacy2grade(hd->homunculus.intimacy);
}

/**
* Read homunculus db
*/
static bool read_homunculusdb_sub(char* str[], int columns, int current)
{
	int classid;
	uint16 i;
	struct s_homunculus_db *db;

	//Base Class,Evo Class
	classid = atoi(str[0]);
	if (classid < HM_CLASS_BASE || classid > HM_CLASS_MAX)
	{
		ShowError("read_homunculusdb : Invalid class %d\n", classid);
		return false;
	}

	//Find the ClassID, already exist or not in homunculus_db
	ARR_FIND(0,homunculus_count,i,homunculus_db[i].base_class == classid);
	if (i >= homunculus_count)
		db = &homunculus_db[homunculus_count];
	else
		db = &homunculus_db[i];

	db->base_class = classid;
	classid = atoi(str[1]);
	if (classid < HM_CLASS_BASE || classid > HM_CLASS_MAX)
	{
		db->base_class = 0;
		ShowError("read_homunculusdb : Invalid class %d\n", classid);
		return false;
	}
	db->evo_class = classid;
	//Name, Food, Hungry Delay, Base Size, Evo Size, Race, Element, ASPD
	safestrncpy(db->name,str[2],NAME_LENGTH-1);
	db->foodID = atoi(str[3]);
	db->hungryDelay = atoi(str[4]);
	db->base_size = atoi(str[5]);
	db->evo_size = atoi(str[6]);
	db->race = atoi(str[7]);
	db->element = atoi(str[8]);
	db->baseASPD = atoi(str[9]);
	//base HP, SP, str, agi, vit, int, dex, luk
	db->base.HP = atoi(str[10]);
	db->base.SP = atoi(str[11]);
	db->base.str = atoi(str[12]);
	db->base.agi = atoi(str[13]);
	db->base.vit = atoi(str[14]);
	db->base.int_= atoi(str[15]);
	db->base.dex = atoi(str[16]);
	db->base.luk = atoi(str[17]);
	//Growth Min/Max HP, SP, str, agi, vit, int, dex, luk
	db->gmin.HP = atoi(str[18]);
	db->gmax.HP = atoi(str[19]);
	db->gmin.SP = atoi(str[20]);
	db->gmax.SP = atoi(str[21]);
	db->gmin.str = atoi(str[22]);
	db->gmax.str = atoi(str[23]);
	db->gmin.agi = atoi(str[24]);
	db->gmax.agi = atoi(str[25]);
	db->gmin.vit = atoi(str[26]);
	db->gmax.vit = atoi(str[27]);
	db->gmin.int_= atoi(str[28]);
	db->gmax.int_= atoi(str[29]);
	db->gmin.dex = atoi(str[30]);
	db->gmax.dex = atoi(str[31]);
	db->gmin.luk = atoi(str[32]);
	db->gmax.luk = atoi(str[33]);
	//Evolution Min/Max HP, SP, str, agi, vit, int, dex, luk
	db->emin.HP = atoi(str[34]);
	db->emax.HP = atoi(str[35]);
	db->emin.SP = atoi(str[36]);
	db->emax.SP = atoi(str[37]);
	db->emin.str = atoi(str[38]);
	db->emax.str = atoi(str[39]);
	db->emin.agi = atoi(str[40]);
	db->emax.agi = atoi(str[41]);
	db->emin.vit = atoi(str[42]);
	db->emax.vit = atoi(str[43]);
	db->emin.int_= atoi(str[44]);
	db->emax.int_= atoi(str[45]);
	db->emin.dex = atoi(str[46]);
	db->emax.dex = atoi(str[47]);
	db->emin.luk = atoi(str[48]);
	db->emax.luk = atoi(str[49]);

	//Check that the min/max values really are below the other one.
	if(db->gmin.HP > db->gmax.HP)
		db->gmin.HP = db->gmax.HP;
	if(db->gmin.SP > db->gmax.SP)
		db->gmin.SP = db->gmax.SP;
	if(db->gmin.str > db->gmax.str)
		db->gmin.str = db->gmax.str;
	if(db->gmin.agi > db->gmax.agi)
		db->gmin.agi = db->gmax.agi;
	if(db->gmin.vit > db->gmax.vit)
		db->gmin.vit = db->gmax.vit;
	if(db->gmin.int_> db->gmax.int_)
		db->gmin.int_= db->gmax.int_;
	if(db->gmin.dex > db->gmax.dex)
		db->gmin.dex = db->gmax.dex;
	if(db->gmin.luk > db->gmax.luk)
		db->gmin.luk = db->gmax.luk;

	if(db->emin.HP > db->emax.HP)
		db->emin.HP = db->emax.HP;
	if(db->emin.SP > db->emax.SP)
		db->emin.SP = db->emax.SP;
	if(db->emin.str > db->emax.str)
		db->emin.str = db->emax.str;
	if(db->emin.agi > db->emax.agi)
		db->emin.agi = db->emax.agi;
	if(db->emin.vit > db->emax.vit)
		db->emin.vit = db->emax.vit;
	if(db->emin.int_> db->emax.int_)
		db->emin.int_= db->emax.int_;
	if(db->emin.dex > db->emax.dex)
		db->emin.dex = db->emax.dex;
	if(db->emin.luk > db->emax.luk)
		db->emin.luk = db->emax.luk;

	if (i >= homunculus_count)
		homunculus_count++;
	return true;
}

/**
* Read homunculus db (check the files)
*/
void read_homunculusdb(void) {
	uint8 i;
	const char *filename[] = {
		DBPATH"homunculus_db.txt",
		DBIMPORT"/homunculus_db.txt",
	};
	homunculus_count = 0;
	memset(homunculus_db,0,sizeof(homunculus_db));
	for(i = 0; i<ARRAYLENGTH(filename); i++){
		sv_readdb(db_path, filename[i], ',', 50, 50, MAX_HOMUNCULUS_CLASS, &read_homunculusdb_sub, i > 0);
	}
}

/**
* Read homunculus skill db
* <hom class>,<skill id>,<max level>,<need level>,<req id1>,<req lv1>,<req id2>,<req lv2>,<req id3>,<req lv3>,<req id4>,<req lv4>,<req id5>,<req lv5>,<intimacy lv req>
*/
static bool read_homunculus_skilldb_sub(char* split[], int columns, int current) {
	uint16 skill_id;
	int8 i;
	short class_idx, idx = -1;

	// check for bounds [celest]
	if ((class_idx = hom_class2index(atoi(split[0]))) == -1) {
		ShowWarning("read_homunculus_skilldb: Invalid homunculus class %d.\n", atoi(split[0]));
		return false;
	}

	skill_id = atoi(split[1]);
	if (hom_skill_get_index(skill_id) == -1) {
		ShowError("read_homunculus_skilldb: Invalid Homunculus skill '%s'.\n", split[1]);
		return false;
	}

	// Search an empty line or a line with the same skill_id (stored in idx)
	ARR_FIND(0, MAX_HOM_SKILL_TREE, idx, !hskill_tree[class_idx][idx].id || hskill_tree[class_idx][idx].id == skill_id);
	if (idx == MAX_HOM_SKILL_TREE) {
		ShowWarning("Unable to load skill %d into homunculus %d's tree. Maximum number of skills per class has been reached.\n", skill_id, atoi(split[0]));
		return false;
	}

	hskill_tree[class_idx][idx].id = skill_id;
	hskill_tree[class_idx][idx].max = atoi(split[2]);
	hskill_tree[class_idx][idx].need_level = atoi(split[3]);

	for (i = 0; i < MAX_HOM_SKILL_REQUIRE; i++) {
		hskill_tree[class_idx][idx].need[i].id = atoi(split[4+i*2]);
		hskill_tree[class_idx][idx].need[i].lv = atoi(split[4+i*2+1]);
	}

	hskill_tree[class_idx][idx].intimacy = atoi(split[14]);
	return true;
}

/**
* Read homunculus skill db (check the files)
*/
static void read_homunculus_skilldb(void) {
	const char *filename[] = { "homun_skill_tree.txt", DBIMPORT"/homun_skill_tree.txt"};
	int i;
	memset(hskill_tree,0,sizeof(hskill_tree));
	for (i = 0; i<ARRAYLENGTH(filename); i++) {
		sv_readdb(db_path, filename[i], ',', 15, 15, -1, &read_homunculus_skilldb_sub, i > 0);
	}
}

/**
* Read homunculus exp db
*/
void read_homunculus_expdb(void)
{
	int i;
	const char *filename[]={
		DBPATH"exp_homun.txt",
		DBIMPORT"/exp_homun.txt"
	};

	memset(hexptbl,0,sizeof(hexptbl));
	for (i = 0; i < ARRAYLENGTH(filename); i++) {
		FILE *fp;
		char line[1024];
		int j=0;
		
		sprintf(line, "%s/%s", db_path, filename[i]);
		fp = fopen(line,"r");
		if (fp == NULL) {
			if (i != 0)
				continue;
			ShowError("Can't read %s\n",line);
			return;
		}
		while (fgets(line, sizeof(line), fp) && j < MAX_LEVEL) {
			if (line[0] == '/' && line[1] == '/')
				continue;

			hexptbl[j] = strtoul(line, NULL, 10);
			if (!hexptbl[j++])
				break;
		}
		if (hexptbl[MAX_LEVEL - 1]) { // Last permitted level have to be 0!
			ShowWarning("read_hexptbl: Reached max level in %s [%d]. Remaining lines were not read.\n ",filename,MAX_LEVEL);
			hexptbl[MAX_LEVEL - 1] = 0;
		}
		fclose(fp);
		ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' levels in '" CL_WHITE "%s/%s" CL_RESET "'.\n", j, db_path, filename[i]);
	}
}

void hom_reload(void){
	read_homunculusdb();
	read_homunculus_expdb();
}

void hom_reload_skill(void){
	read_homunculus_skilldb();
}

void do_init_homunculus(void){
	int class_;

	read_homunculusdb();
	read_homunculus_expdb();
	read_homunculus_skilldb();

	// Add homunc timer function to timer func list [Toms]
	add_timer_func_list(hom_hungry, "hom_hungry");

	//Stock view data for homuncs
	memset(&hom_viewdb, 0, sizeof(hom_viewdb));
	for (class_ = 0; class_ < ARRAYLENGTH(hom_viewdb); class_++)
		hom_viewdb[class_].class_ = HM_CLASS_BASE+class_;
}

void do_final_homunculus(void) {
	//Nothing todo yet
}
