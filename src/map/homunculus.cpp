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

static TIMER_FUNC(hom_hungry);

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

const std::string HomExpDatabase::getDefaultLocation() {
	return std::string(db_path) + "/exp_homun.yml";
}

uint64 HomExpDatabase::parseBodyNode(const YAML::Node &node) {
	if (!this->nodesExist(node, { "Level", "Exp" })) {
		return 0;
	}

	uint16 level;

	if (!this->asUInt16(node, "Level", level))
		return 0;

	uint64 exp;

	if (!this->asUInt64(node, "Exp", exp))
		return 0;

	if (level == 0) {
		this->invalidWarning(node["Level"], "The minimum level is 1.\n");
		return 0;
	}
	if (level >= MAX_LEVEL) {
		this->invalidWarning(node["Level"], "Homunculus level %d exceeds maximum level %d, skipping.\n", level, MAX_LEVEL);
		return 0;
	}

	std::shared_ptr<s_homun_exp_db> homun_exp = this->find(level);
	bool exists = homun_exp != nullptr;

	if (!exists) {
		homun_exp = std::make_shared<s_homun_exp_db>();
		homun_exp->level = level;
	}

	homun_exp->exp = static_cast<t_exp>(exp);

	if (!exists)
		this->put(level, homun_exp);

	return 1;
}

HomExpDatabase homun_exp_db;

/**
 * Returns the experience required to level up according to the table.
 * @param level: Homunculus level
 * @return Experience
 */
t_exp HomExpDatabase::get_nextexp(uint16 level) {
	auto next_exp = this->find(level);
	if (next_exp)
		return next_exp->exp;
	else
		return 0;
}

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

#ifdef RENEWAL
	status_change_end(&sd->bl, status_skill2sc(AM_CALLHOMUN), INVALID_TIMER);
#endif

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
	if (battle_config.hom_setting&HOMSET_RESET_REUSESKILL_VAPORIZED) {
		hd->blockskill.clear();
		hd->blockskill.shrink_to_fit();
	}
	clif_hominfo(sd, sd->hd, 0);
	hom_save(hd);

#ifdef RENEWAL
	status_change_end(&sd->bl, status_skill2sc(AM_CALLHOMUN), INVALID_TIMER);
#endif

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
	nullpo_retv(hd);

	int32 c = hom_class2index(hd->homunculus.prev_class);

	/* load previous homunculus form skills first. */
	if (hd->homunculus.prev_class != 0 && c >= 0) {
		auto homun = homunculus_db.find(c);

		for (const auto &skit : homun->skill_tree) {
			uint16 skill_id = skit.id;
			short idx = hom_skill_get_index(skill_id);

			if (skill_id == 0 || idx == -1)
				continue;
			if (hd->homunculus.hskill[idx].id)
				continue; //Skill already known.

			bool fail = false;

			if (!battle_config.skillfree) {
				if (skit.need_level > hd->homunculus.level)
					continue;
				for (const auto &needit : skit.need) {
					if (needit.first > 0 && hom_checkskill(hd, needit.first) < needit.second) {
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

	auto homun = homunculus_db.find(c);

	for (const auto &skit : homun->skill_tree) {
		uint16 skill_id = skit.id;
		short idx = hom_skill_get_index(skill_id);

		if (skill_id == 0 || idx == -1)
			continue;
		if (hd->homunculus.hskill[idx].id)
			continue; //Skill already known.

		bool fail = false;
		uint32 intimacy = (flag_evolve) ? 10 : hd->homunculus.intimacy;

		if (intimacy < skit.intimacy * 100)
			continue;
		if (!battle_config.skillfree) {
			if (skit.need_level > hd->homunculus.level)
				continue;
			for (const auto &needit : skit.need) {
				if (needit.first > 0 && hom_checkskill(hd, needit.first) < needit.second) {
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
	auto homun = homunculus_db.find(b_class);

	if (homun == nullptr)
		return 0;

	for (const auto &skit : homun->skill_tree) {
		if (skit.id == skill_id)
			return skit.max;
	}

	return skill_get_max(skill_id);
}

 /**
 * Get required minimum level to learn the skill
 * @param class_ Homunculus class
 * @param skill_id Homunculus skill ID
 * @return Level required or 0 if invalid
 **/
uint8 hom_skill_get_min_level(int class_, uint16 skill_id) {
	auto homun = homunculus_db.find(class_);

	if (homun == nullptr)
		return 0;

	for (const auto &skit : homun->skill_tree) {
		if (skit.id == skill_id)
			return skit.need_level;
	}

	return 0;
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
	int m_class;

	if ((m_class = hom_class2mapid(hd->homunculus.class_)) == -1) {
		ShowError("hom_levelup: Invalid class %d. \n", hd->homunculus.class_);
		return 0;
	}

	auto homun_db = homunculus_db.find(hd->homunculus.prev_class);
	struct s_hom_stats *min = nullptr, *max = nullptr;

	/// When homunculus is homunculus S, we check to see if we need to apply previous class stats
	if(m_class&HOM_S && hd->homunculus.level < battle_config.hom_S_growth_level) {
		if (!hd->homunculus.prev_class) {
			/// We also need to be sure that the previous class exists, otherwise give it something to work with
			hd->homunculus.prev_class = MER_LIF;
		}

		max = &homun_db->gmax;
		min = &homun_db->gmin;
	}

	if (((m_class&HOM_REG) && hd->homunculus.level >= battle_config.hom_max_level)
		|| ((m_class&HOM_S) && hd->homunculus.level >= battle_config.hom_S_max_level)
		|| !hd->exp_next || hd->homunculus.exp < hd->exp_next)
		return 0;

	s_homunculus *hom = &hd->homunculus;
	hom->level++;
	if (!(hom->level % 3))
		hom->skillpts++;	//1 skillpoint each 3 base level

	hom->exp -= hd->exp_next;
	hd->exp_next = homun_exp_db.get_nextexp(hom->level);

	if (!max) {
		max  = &hd->homunculusDB->gmax;
		min  = &hd->homunculusDB->gmin;
	}

	int growth_max_hp = rnd_value(min->HP, max->HP);
	int growth_max_sp = rnd_value(min->SP, max->SP);
	int growth_str = rnd_value(min->str, max->str);
	int growth_agi = rnd_value(min->agi, max->agi);
	int growth_vit = rnd_value(min->vit, max->vit);
	int growth_dex = rnd_value(min->dex, max->dex);
	int growth_int = rnd_value(min->int_,max->int_);
	int growth_luk = rnd_value(min->luk, max->luk);

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
	auto homun = homunculus_db.find(class_);

	if (homun == nullptr)
		return false;

	hd->homunculusDB = homun;
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
	nullpo_ret(hd);

	if(!hd->homunculusDB->evo_class || hd->homunculus.class_ == hd->homunculusDB->evo_class) {
		clif_emotion(&hd->bl, ET_SWEAT);
		return 0 ;
	}

	map_session_data *sd = hd->master;

	if (!sd)
		return 0;

	if (!hom_change_class(hd, hd->homunculusDB->evo_class)) {
		ShowError("hom_evolution: Can't evolve homunc from %d to %d", hd->homunculus.class_, hd->homunculusDB->evo_class);
		return 0;
	}

	//Apply evolution bonuses
	s_homunculus *hom = &hd->homunculus;
	s_hom_stats *max = &hd->homunculusDB->emax;
	s_hom_stats *min = &hd->homunculusDB->emin;

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
void hom_gainexp(struct homun_data *hd,t_exp exp)
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
* Increase homunculus intimacy
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
* Decrease homunculus intimacy
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
static TIMER_FUNC(hom_hungry){
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

	if( battle_config.feature_homunculus_autofeed && hd->homunculus.autofeed && hd->homunculus.hunger <= battle_config.feature_homunculus_autofeed_rate ){
		hom_food( sd, hd );
	}

	if (hd->homunculus.hunger < 0) {
		hd->homunculus.hunger = 0;
		// Delete the homunculus if intimacy <= 100
		if (!hom_decrease_intimacy(hd, 100))
			return hom_delete(hd, ET_HUK);
		clif_send_homdata(sd,SP_INTIMATE,hd->homunculus.intimacy / 100);
	}

	clif_send_homdata(sd,SP_HUNGRY,hd->homunculus.hunger);

	int hunger_delay = (battle_config.homunculus_starving_rate > 0 && hd->homunculus.hunger <= battle_config.homunculus_starving_rate) ? battle_config.homunculus_starving_delay : hd->homunculusDB->hungryDelay; // Every 20 seconds if hunger <= 10

	hd->hungry_timer = add_timer(tick+hunger_delay,hom_hungry,sd->bl.id,0); //simple Fix albator
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
* Create homunc structure
* @param sd
* @param hom
*/
void hom_alloc(struct map_session_data *sd, struct s_homunculus *hom)
{
	nullpo_retv(sd);

	Assert((sd->status.hom_id == 0 || sd->hd == 0) || sd->hd->master == sd);

	auto homun_db = homunculus_db.find(hom->class_);

	if (homun_db == nullptr) {
		ShowError("hom_alloc: unknown class [%d] for homunculus '%s', requesting deletion.\n", hom->class_, hom->name);
		sd->status.hom_id = 0;
		intif_homunculus_requestdelete(hom->hom_id);
		return;
	}

	struct homun_data *hd;

	sd->hd = hd = (struct homun_data*)aCalloc(1,sizeof(struct homun_data));
	hd->bl.type = BL_HOM;
	hd->bl.id = npc_get_new_npc_id();

	hd->master = sd;
	hd->homunculusDB = homun_db;
	memcpy(&hd->homunculus, hom, sizeof(struct s_homunculus));
	hd->exp_next = homun_exp_db.get_nextexp(hd->homunculus.level);

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
	if (hd->hungry_timer == INVALID_TIMER) {
		int hunger_delay = (battle_config.homunculus_starving_rate > 0 && hd->homunculus.hunger <= battle_config.homunculus_starving_rate) ? battle_config.homunculus_starving_delay : hd->homunculusDB->hungryDelay; // Every 20 seconds if hunger <= 10

		hd->hungry_timer = add_timer(gettick()+hunger_delay,hom_hungry,hd->master->bl.id,0);
	}
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
		if (battle_config.hom_setting&HOMSET_COPY_SPEED)
			status_calc_bl(&hd->bl, SCB_SPEED);
		hom_save(hd);
	} else
		//Warp him to master.
		unit_warp(&hd->bl,sd->bl.m, sd->bl.x, sd->bl.y,CLR_OUTSIGHT);

#ifdef RENEWAL
	sc_start(&sd->bl, &sd->bl, status_skill2sc(AM_CALLHOMUN), 100, 1, skill_get_time(AM_CALLHOMUN, 1));
#endif

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

#ifdef RENEWAL
	sc_start(&sd->bl, &sd->bl, status_skill2sc(AM_CALLHOMUN), 100, 1, skill_get_time(AM_CALLHOMUN, 1));
#endif

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
	nullpo_ret(sd);

	auto homun_db = homunculus_db.find(class_);

	if (homun_db == nullptr)
		return false;

	struct s_homunculus homun;

	memset(&homun, 0, sizeof(struct s_homunculus));
	//Initial data
	safestrncpy(homun.name, homun_db->name, NAME_LENGTH-1);
	homun.class_ = class_;
	homun.level = 1;
	homun.hunger = 32; //32%
	homun.intimacy = 2100; //21/1000
	homun.char_id = sd->status.char_id;

	homun.hp = 10;

	s_hom_stats base = homun_db->base;

	homun.max_hp = base.HP;
	homun.max_sp = base.SP;
	homun.str = base.str *10;
	homun.agi = base.agi *10;
	homun.vit = base.vit *10;
	homun.int_= base.int_*10;
	homun.dex = base.dex *10;
	homun.luk = base.luk *10;

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

#ifdef RENEWAL
	sc_start(&sd->bl, &sd->bl, status_skill2sc(AM_CALLHOMUN), 100, 1, skill_get_time(AM_CALLHOMUN, 1));
#endif

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
		sc_start(&hd->bl,&hd->bl, SC_STYLE_CHANGE, 100, MH_MD_FIGHTING, INFINITE_TICK);
}

/**
* Reset homunculus status
* @param hd
*/
void hom_reset_stats(struct homun_data *hd)
{	//Resets a homunc stats back to zero (but doesn't touches hunger or intimacy)
	struct s_homunculus *hom = &hd->homunculus;
	struct s_hom_stats *base = &hd->homunculusDB->base;

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
	hd->exp_next = homun_exp_db.get_nextexp(hom->level);
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
	struct s_skill b_skill[MAX_HOMUNSKILL];

	if (!hom_is_active(hd))
		return 0;

	sd = hd->master;
	lv = hd->homunculus.level;
	t_exp exp = hd->homunculus.exp;
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
		struct s_hom_stats *max = &hd->homunculusDB->emax, *min = &hd->homunculusDB->emin;

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

const std::string HomunculusDatabase::getDefaultLocation() {
	return std::string(db_path) + "/homunculus_db.yml";
}

/**
 * Reads and parses an entry from the homunculus_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 HomunculusDatabase::parseBodyNode(const YAML::Node &node) {
	std::string class_name;

	if (!this->asString(node, "BaseClass", class_name))
		return 0;

	std::string class_name_constant = "MER_" + class_name;
	int64 class_tmp;

	if (!script_get_constant(class_name_constant.c_str(), &class_tmp)) {
		this->invalidWarning(node["BaseClass"], "Invalid homunculus BaseClass \"%s\", skipping.\n", class_name.c_str());
		return 0;
	}

	int32 class_id = static_cast<int32>(class_tmp);
	std::shared_ptr<s_homunculus_db> hom = this->find(class_id);
	bool exists = hom != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Name", "Status", "SkillTree" }))
			return 0;

		hom = std::make_shared<s_homunculus_db>();
		hom->base_class = class_id;
	}

	if (this->nodeExists(node, "EvolutionClass")) {
		std::string evo_class_name;

		if (!this->asString(node, "EvolutionClass", evo_class_name))
			return 0;

		std::string evo_class_name_constant = "MER_" + evo_class_name;
		int64 constant;

		if (!script_get_constant(evo_class_name_constant.c_str(), &constant)) {
			this->invalidWarning(node["EvolutionClass"], "Invalid homunculus Evolution Class %s, skipping.\n", evo_class_name.c_str());
			return 0;
		}

		hom->evo_class = static_cast<int32>(constant);
	} else {
		if (!exists)
			hom->evo_class = class_id;
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		safestrncpy(hom->name, name.c_str(), sizeof(hom->name));
	}

	if (this->nodeExists(node, "Food")) {
		std::string food;

		if (!this->asString(node, "Food", food))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname(food.c_str());

		if (item != nullptr)
			hom->foodID = item->nameid;
		else {
			this->invalidWarning(node["Food"], "Invalid homunculus Food %s, defaulting to Pet_Food.\n", food.c_str());
			hom->foodID = ITEMID_PET_FOOD;
		}
	} else {
		if (!exists)
			hom->foodID = ITEMID_PET_FOOD;
	}

	if (this->nodeExists(node, "HungryDelay")) {
		int32 delay;

		if (!this->asInt32(node, "HungryDelay", delay))
			return 0;

		hom->hungryDelay = delay;
	} else {
		if (!exists)
			hom->hungryDelay = 60000;
	}

	if (this->nodeExists(node, "Status")) {
		const YAML::Node &status = node["Status"];

		if (!this->nodesExist(status, { "Race", "Element", "Size", "Hp", "Sp", "Str", "Agi", "Vit", "Int", "Dex", "Luk" }))
			return 0;

		if (this->nodeExists(status, "Race")) {
			std::string race;

			if (!this->asString(status, "Race", race))
				return 0;

			std::string race_constant = "RC_" + race;
			int64 constant;

			if (!script_get_constant(race_constant.c_str(), &constant)) {
				this->invalidWarning(status["Race"], "Invalid homunculus Race %s, defaulting to Demi-Human.\n", race.c_str());
				constant = RC_DEMIHUMAN;
			}

			hom->race = static_cast<e_race>(constant);
		}

		if (this->nodeExists(status, "Element")) {
			std::string element;

			if (!this->asString(status, "Element", element))
				return 0;

			std::string element_constant = "ELE_" + element;
			int64 constant;

			if (!script_get_constant(element_constant.c_str(), &constant)) {
				this->invalidWarning(status["Element"], "Invalid homunculus Element %s, defaulting to Neutral.\n", element.c_str());
				constant = ELE_NEUTRAL;
			}

			hom->element = static_cast<e_element>(constant);
		}

		if (this->nodeExists(status, "Size")) {
			const YAML::Node &sizeNode = node["Size"];
			std::vector<std::string> size_list = { "Base", "Evolution" };

			for (const auto &sizeit : size_list) {
				if (this->nodeExists(sizeNode, sizeit)) {
					std::string size;

					if (!this->asString(sizeNode, sizeit, size))
						return 0;

					std::string size_constant = "SIZE_" + size;
					int64 constant;

					if (!script_get_constant(size_constant.c_str(), &constant)) {
						this->invalidWarning(sizeNode[sizeit], "Invalid homunculus %s Size %s, defaulting to Small.\n", sizeit.c_str(), size.c_str());
						constant = SZ_SMALL;
					}

					if (sizeit.compare("Base") == 0)
						hom->base_size = static_cast<e_size>(constant);
					else
						hom->evo_size = static_cast<e_size>(constant);
				}
			}
		}

		if (this->nodeExists(status, "BaseAspd")) {
			uint16 aspd;

			if (!this->asUInt16(status, "BaseAspd", aspd))
				return 0;

			if (aspd > 2000) {
				this->invalidWarning(status["BaseAspd"], "Homunculus BaseAspd %hu exceeds 2000, capping.\n", aspd);
				aspd = 2000;
			}

			hom->baseASPD = aspd;
		} else {
			if (!exists)
				hom->baseASPD = 700;
		}

		std::vector<std::string> stat_list = { "Hp", "Sp", "Str", "Agi", "Vit", "Int", "Dex", "Luk" };

		for (const auto &statit : stat_list) {
			if (this->nodeExists(status, statit)) {
				const YAML::Node &bonus = status[statit];

				if (this->nodeExists(bonus, "Base")) {
					uint32 base;

					if (!this->asUInt32(bonus, "Base", base))
						return 0;

					if (statit.compare("Hp") == 0)
						hom->base.HP = base;
					else if (statit.compare("Sp") == 0)
						hom->base.SP = base;
					else if (statit.compare("Str") == 0)
						hom->base.str = static_cast<uint16>(base);
					else if (statit.compare("Agi") == 0)
						hom->base.agi = static_cast<uint16>(base);
					else if (statit.compare("Vit") == 0)
						hom->base.vit = static_cast<uint16>(base);
					else if (statit.compare("Int") == 0)
						hom->base.int_ = static_cast<uint16>(base);
					else if (statit.compare("Dex") == 0)
						hom->base.dex = static_cast<uint16>(base);
					else if (statit.compare("Luk") == 0)
						hom->base.luk = static_cast<uint16>(base);
				}

				if (this->nodeExists(bonus, "GrowthBonus")) {
					const YAML::Node &growth = bonus["GrowthBonus"];

					if (this->nodeExists(growth, "Min")) {
						uint32 gbonus;

						if (!this->asUInt32(growth, "Min", gbonus))
							return 0;

						if (statit.compare("Hp") == 0)
							hom->gmin.HP = gbonus;
						else if (statit.compare("Sp") == 0)
							hom->gmin.SP = gbonus;
						else if (statit.compare("Str") == 0)
							hom->gmin.str = static_cast<uint16>(gbonus);
						else if (statit.compare("Agi") == 0)
							hom->gmin.agi = static_cast<uint16>(gbonus);
						else if (statit.compare("Vit") == 0)
							hom->gmin.vit = static_cast<uint16>(gbonus);
						else if (statit.compare("Int") == 0)
							hom->gmin.int_ = static_cast<uint16>(gbonus);
						else if (statit.compare("Dex") == 0)
							hom->gmin.dex = static_cast<uint16>(gbonus);
						else if (statit.compare("Luk") == 0)
							hom->gmin.luk = static_cast<uint16>(gbonus);
					}

					if (this->nodeExists(growth, "Max")) {
						uint32 gbonus;

						if (!this->asUInt32(growth, "Max", gbonus))
							return 0;

						if (statit.compare("Hp") == 0)
							hom->gmax.HP = gbonus;
						else if (statit.compare("Sp") == 0)
							hom->gmax.SP = gbonus;
						else if (statit.compare("Str") == 0)
							hom->gmax.str = static_cast<uint16>(gbonus);
						else if (statit.compare("Agi") == 0)
							hom->gmax.agi = static_cast<uint16>(gbonus);
						else if (statit.compare("Vit") == 0)
							hom->gmax.vit = static_cast<uint16>(gbonus);
						else if (statit.compare("Int") == 0)
							hom->gmax.int_ = static_cast<uint16>(gbonus);
						else if (statit.compare("Dex") == 0)
							hom->gmax.dex = static_cast<uint16>(gbonus);
						else if (statit.compare("Luk") == 0)
							hom->gmax.luk = static_cast<uint16>(gbonus);
					}
				}
				
				if (this->nodeExists(bonus, "EvolutionBonus")) {
					const YAML::Node &evolution = bonus["EvolutionBonus"];

					if (this->nodeExists(evolution, "Min")) {
						uint32 ebonus;

						if (!this->asUInt32(evolution, "Min", ebonus))
							return 0;

						if (statit.compare("Hp") == 0)
							hom->emin.HP = ebonus;
						else if (statit.compare("Sp") == 0)
							hom->emin.SP = ebonus;
						else if (statit.compare("Str") == 0)
							hom->emin.str = static_cast<uint16>(ebonus);
						else if (statit.compare("Agi") == 0)
							hom->emin.agi = static_cast<uint16>(ebonus);
						else if (statit.compare("Vit") == 0)
							hom->emin.vit = static_cast<uint16>(ebonus);
						else if (statit.compare("Int") == 0)
							hom->emin.int_ = static_cast<uint16>(ebonus);
						else if (statit.compare("Dex") == 0)
							hom->emin.dex = static_cast<uint16>(ebonus);
						else if (statit.compare("Luk") == 0)
							hom->emin.luk = static_cast<uint16>(ebonus);
					}

					if (this->nodeExists(evolution, "Max")) {
						uint32 ebonus;

						if (!this->asUInt32(evolution, "Max", ebonus))
							return 0;

						if (statit.compare("Hp") == 0)
							hom->emax.HP = ebonus;
						else if (statit.compare("Sp") == 0)
							hom->emax.SP = ebonus;
						else if (statit.compare("Str") == 0)
							hom->emax.str = static_cast<uint16>(ebonus);
						else if (statit.compare("Agi") == 0)
							hom->emax.agi = static_cast<uint16>(ebonus);
						else if (statit.compare("Vit") == 0)
							hom->emax.vit = static_cast<uint16>(ebonus);
						else if (statit.compare("Int") == 0)
							hom->emax.int_ = static_cast<uint16>(ebonus);
						else if (statit.compare("Dex") == 0)
							hom->emax.dex = static_cast<uint16>(ebonus);
						else if (statit.compare("Luk") == 0)
							hom->emax.luk = static_cast<uint16>(ebonus);
					}
				}
			}
		}

		// Cap values
		if (hom->gmin.HP > hom->gmax.HP)
			hom->gmin.HP = hom->gmax.HP;
		if (hom->gmin.SP > hom->gmax.SP)
			hom->gmin.SP = hom->gmax.SP;
		if (hom->gmin.str > hom->gmax.str)
			hom->gmin.str = hom->gmax.str;
		if (hom->gmin.agi > hom->gmax.agi)
			hom->gmin.agi = hom->gmax.agi;
		if (hom->gmin.vit > hom->gmax.vit)
			hom->gmin.vit = hom->gmax.vit;
		if (hom->gmin.int_ > hom->gmax.int_)
			hom->gmin.int_ = hom->gmax.int_;
		if (hom->gmin.dex > hom->gmax.dex)
			hom->gmin.dex = hom->gmax.dex;
		if (hom->gmin.luk > hom->gmax.luk)
			hom->gmin.luk = hom->gmax.luk;

		if (hom->emin.HP > hom->emax.HP)
			hom->emin.HP = hom->emax.HP;
		if (hom->emin.SP > hom->emax.SP)
			hom->emin.SP = hom->emax.SP;
		if (hom->emin.str > hom->emax.str)
			hom->emin.str = hom->emax.str;
		if (hom->emin.agi > hom->emax.agi)
			hom->emin.agi = hom->emax.agi;
		if (hom->emin.vit > hom->emax.vit)
			hom->emin.vit = hom->emax.vit;
		if (hom->emin.int_ > hom->emax.int_)
			hom->emin.int_ = hom->emax.int_;
		if (hom->emin.dex > hom->emax.dex)
			hom->emin.dex = hom->emax.dex;
		if (hom->emin.luk > hom->emax.luk)
			hom->emin.luk = hom->emax.luk;
	}

	if (this->nodeExists(node, "SkillTree")) {
		const YAML::Node &skillsNode = node["SkillTree"];

		for (const YAML::Node &skill : skillsNode) {
			s_homun_skill_tree_entry entry;

			if (this->nodeExists(skill, "Skill")) {
				std::string skill_name;

				if (!this->asString(skill, "Skill", skill_name))
					return 0;

				uint16 skill_id = skill_name2id(skill_name.c_str());

				if (skill_id == 0) {
					this->invalidWarning(skill["Skill"], "Invalid homunculus skill %s, skipping.\n", skill_name.c_str());
					return 0;
				}

				if (!SKILL_CHK_HOMUN(skill_id)) {
					this->invalidWarning(skill["Skill"], "Homunculus skill %s (%u) is out of the homunculus skill range [%u-%u], skipping.\n", skill_name.c_str(), skill_id, HM_SKILLBASE, HM_SKILLBASE + MAX_HOMUNSKILL - 1);
					return 0;
				}

				entry.id = skill_id;
			}

			if (this->nodeExists(skill, "MaxLevel")) {
				uint16 level;

				if (!this->asUInt16(skill, "MaxLevel", level))
					return 0;

				entry.max = static_cast<uint8>(level);
			}

			if (this->nodeExists(skill, "RequiredLevel")) {
				uint16 level;

				if (!this->asUInt16(skill, "RequiredLevel", level))
					return 0;

				uint16 config_max = battle_config.hom_max_level;

				if ((hom_class2type(class_id) == HT_S))
					config_max = battle_config.hom_S_max_level;

				if (level > config_max) {
					this->invalidWarning(skill["RequiredLevel"], "Homunculus Required Skill level %u exceeds maximum level %u, capping.\n", level, config_max);
					level = config_max;
				}

				entry.need_level = static_cast<uint8>(level);
			} else {
				if (!exists)
					entry.need_level = 0;
			}

			if (this->nodeExists(skill, "RequiredIntimacy")) {
				uint32 intimacy;

				if (!this->asUInt32(skill, "RequiredIntimacy", intimacy))
					return 0;

				if (intimacy > 100000) {
					this->invalidWarning(skill["RequiredIntimacy"], "Homunculus Required Intimacy %u exceeds maximum intimacy 100000, capping.\n", intimacy);
					intimacy = 100000;
				}

				entry.intimacy = intimacy;
			}

			if (this->nodeExists(skill, "Required")) {
				const YAML::Node &required = skill["Required"];
				uint16 skill_id = 0, skill_lv = 0;

				if (this->nodeExists(required, "Skill")) {
					std::string skill_name;

					if (!this->asString(required, "Skill", skill_name))
						return 0;

					if (skill_name2id(skill_name.c_str()) == 0) {
						this->invalidWarning(required["Skill"], "Invalid homunculus skill %s, skipping.\n", skill_name.c_str());
						return 0;
					}

					skill_id = skill_name2id(skill_name.c_str());

					if (skill_id == 0) {
						this->invalidWarning(required["Skill"], "Invalid homunculus skill %s, skipping.\n", skill_name.c_str());
						return 0;
					}

					if (!SKILL_CHK_HOMUN(skill_id)) {
						this->invalidWarning(required["Skill"], "Homunculus skill %s (%u) is out of the homunculus skill range [%u-%u], skipping.\n", skill_name.c_str(), skill_id, HM_SKILLBASE, HM_SKILLBASE + MAX_HOMUNSKILL - 1);
						return 0;
					}
				}

				if (this->nodeExists(required, "Level")) {
					if (!this->asUInt16(required, "Level", skill_lv))
						return 0;
				}

				if (skill_id > 0 && skill_lv > 0)
					entry.need.emplace(skill_id, static_cast<uint8>(skill_lv));
			}

			hom->skill_tree.push_back(entry);
		}
	}

	if (!exists)
		this->put(class_id, hom);

	return 1;
}

HomunculusDatabase homunculus_db;


void hom_reload(void){
	homunculus_db.load();
	homun_exp_db.reload();
}

void do_init_homunculus(void){
	int class_;

	homunculus_db.load();
	homun_exp_db.load();

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
