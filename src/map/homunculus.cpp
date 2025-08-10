// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus.hpp"

#include <cstdlib>

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

uint64 HomExpDatabase::parseBodyNode(const ryml::NodeRef& node) {
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
int16 hom_skill_get_index(uint16 skill_id) {
	if (!SKILL_CHK_HOMUN(skill_id))
		return -1;
	skill_id -= HM_SKILLBASE;
	if (skill_id >= MAX_HOMUNSKILL)
		return -1;
	return skill_id;
}

/**
* Get homunculus view data
* @param class_ Homunculus class
* @return vd
*/
struct view_data* hom_get_viewdata(int32 class_)
{	//Returns the viewdata for homunculus
	if (homdb_checkid(class_))
		return &hom_viewdb[class_-HM_CLASS_BASE];
	return nullptr;
}

/**
* Get homunculus type of specified class_
* @param class_
* @return enum homun_type
*/
enum homun_type hom_class2type(int32 class_) {
	int32 mid = hom_class2mapid(class_);
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
int32 hom_class2mapid(int32 hom_class)
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
void hom_addspiritball(TBL_HOM *hd, int32 max) {
	nullpo_retv(hd);

	if (max > MAX_SPIRITBALL)
		max = MAX_SPIRITBALL;
	if (hd->homunculus.spiritball < 0)
		hd->homunculus.spiritball = 0;

	if (hd->homunculus.spiritball && hd->homunculus.spiritball >= max)
		hd->homunculus.spiritball = max;
	else
		hd->homunculus.spiritball++;

	clif_spiritball(hd);
}

/**
* Delete homunculus spirit ball
* @param hd
* @param count Number spirit ball will be deleted
* @param type 1 - Update client
*/
void hom_delspiritball(TBL_HOM *hd, int32 count, int32 type) {
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
		clif_spiritball(hd);
}

/**
* Set homunculus's dead status
* @param hd
* @return flag &1 - Standard dead, &2 - Remove object from map, &4 - Delete object from memory
*/
int32 hom_dead(struct homun_data *hd)
{
	//There's no intimacy penalties on death (from Tharis)
	map_session_data *sd = hd->master;

	//Delete timers when dead.
	hom_hungry_timer_delete(hd);
	hd->homunculus.hp = 0;

	if (!sd) //unit remove map will invoke unit free
		return 3;

#ifdef RENEWAL
	status_change_end(sd, SC_HOMUN_TIME);
#endif

	//Remove from map (if it has no intimacy, it is auto-removed from memory)
	return 3;
}

/**
* Vaporize a character's homunculus
* @param sd
* @param flag 1: then HP needs to be 80% or above. 2: then set to morph state.
*/
int32 hom_vaporize(map_session_data *sd, int32 flag)
{
	struct homun_data *hd;

	nullpo_ret(sd);

	hd = sd->hd;
	if (!hd || hd->homunculus.vaporize)
		return 0;

	if (status_isdead(*hd))
		return 0; //Can't vaporize a dead homun.

	if (flag == HOM_ST_REST && get_percentage(hd->battle_status.hp, hd->battle_status.max_hp) < 80)
		return 0;

	hd->regen.state.block = 3; //Block regen while vaporized.
	//Delete timers when vaporized.
	hom_hungry_timer_delete(hd);
	hd->homunculus.vaporize = flag ? flag : HOM_ST_REST;
	if (battle_config.hom_delay_reset_vaporize) {
		skill_blockhomun_clear(*hd);
	}
	status_change_clear(hd, 1);
	clif_hominfo(sd, sd->hd, 0);
	hom_save(hd);

#ifdef RENEWAL
	status_change_end(sd, SC_HOMUN_TIME);
#endif

	return unit_remove_map(hd, CLR_OUTSIGHT);
}

/**
* Delete a homunculus, completely "killing it".
* @param hd
*/
int32 hom_delete(struct homun_data *hd)
{
	map_session_data *sd;
	nullpo_ret(hd);
	sd = hd->master;

	if (!sd)
		return unit_free(hd,CLR_DEAD);

	//This makes it be deleted right away.
	hd->homunculus.intimacy = 0;
	// Send homunculus_dead to client
	hd->homunculus.hp = 0;
	clif_hominfo(sd, hd, 0);
	return unit_remove_map(hd,CLR_OUTSIGHT);
}

/**
 * Calculates homunculus skill tree for specific evolve/class.
 * @param hd: Homunculus data
 * @param skill_tree: Homunculus db skill tree
 */
void hom_calc_skilltree_sub(homun_data &hd, std::vector<s_homun_skill_tree_entry> &skill_tree) {
	bool evolved = false;

	if (hd.homunculus.class_ == hd.homunculusDB->evo_class)
		evolved = true;

	for (const auto &skit : skill_tree) {
		uint16 skill_id = skit.id;
		int16 idx = hom_skill_get_index(skill_id);

		if (skill_id == 0 || idx == -1)
			continue;
		if (hd.homunculus.hskill[idx].id)
			continue; //Skill already known.

		bool fail = false;

		if (!battle_config.skillfree) {
			if (skit.intimacy > 0 && hd.homunculus.intimacy < skit.intimacy) {
				continue;
			}
			if (skit.evolution && !evolved) {
				continue;
			}
			if (skit.need_level > hd.homunculus.level)
				continue;
			for (const auto &needit : skit.need) {
				if (needit.first > 0 && hom_checkskill(&hd, needit.first) < needit.second) {
					fail = true;
					break;
				}
			}
		}
		if (!fail)
			hd.homunculus.hskill[idx].id = skill_id;
	}
}

/**
* Calculates homunculus skill tree
* @param hd: Homunculus data
*/
void hom_calc_skilltree(homun_data *hd) {
	nullpo_retv(hd);

	std::shared_ptr<s_homunculus_db> homun_current = homunculus_db.homun_search(hd->homunculus.class_);

	// If the current class can't be loaded, then for sure there's no prev_class!
	if (homun_current == nullptr)
		return;

	std::shared_ptr<s_homunculus_db> homun = homunculus_db.homun_search(hd->homunculus.prev_class);

	/* load previous homunculus form skills first. */
	if (homun != nullptr) {
		hom_calc_skilltree_sub(*hd, homun->skill_tree);
	}

	hom_calc_skilltree_sub(*hd, homun_current->skill_tree);

	clif_homskillinfoblock( *hd );
}

/**
* Check skill from homunculus
* @param hd
* @param skill_id
* @return Skill Level or 0 if invalid or unlearned skill
*/
int16 hom_checkskill(struct homun_data *hd,uint16 skill_id)
{
	int16 idx = hom_skill_get_index(skill_id);
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
int32 hom_skill_tree_get_max(int32 skill_id, int32 b_class){
	std::shared_ptr<s_homunculus_db> homun = homunculus_db.homun_search(b_class);

	if (homun == nullptr)
		return 0;

	for (const auto &skit : homun->skill_tree) {
		if (skit.id == skill_id)
			return skit.max;
	}

	return 0;
}

 /**
 * Get required minimum level to learn the skill
 * @param class_ Homunculus class
 * @param skill_id Homunculus skill ID
 * @return Level required or 0 if invalid
 **/
uint16 hom_skill_get_min_level(int32 class_, uint16 skill_id) {
	std::shared_ptr<s_homunculus_db> homun = homunculus_db.homun_search(class_);

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
	int16 idx = 0;
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

		clif_homskillup( *hd, skill_id );

		if (hd->master) {
			clif_hominfo(hd->master,hd,0);
		}

		clif_homskillinfoblock( *hd );
	}
}

/**
* Homunculus leveled up
* @param hd
*/
int32 hom_levelup(struct homun_data *hd)
{
	int32 m_class;

	if ((m_class = hom_class2mapid(hd->homunculus.class_)) == -1) {
		ShowError("hom_levelup: Invalid class %d.\n", hd->homunculus.class_);
		return 0;
	}

	struct s_hom_stats *min = nullptr, *max = nullptr;

	/// When homunculus is homunculus S, we check to see if we need to apply previous class stats
	if(m_class&HOM_S && hd->homunculus.level < battle_config.hom_S_growth_level) {
		std::shared_ptr<s_homunculus_db> homun_s_db = homunculus_db.homun_search(hd->homunculus.prev_class);

		if (homun_s_db == nullptr) {
			ShowError("hom_levelup: Failed to find database entry for %d.\n", hd->homunculus.prev_class);
			return 0;
		}

		max = &homun_s_db->gmax;
		min = &homun_s_db->gmin;
	}

	if (((m_class&HOM_REG) && hd->homunculus.level >= battle_config.hom_max_level)
		|| ((m_class&HOM_S) && hd->homunculus.level >= battle_config.hom_S_max_level)
		|| !hd->exp_next || hd->homunculus.exp < hd->exp_next)
		return 0;

	s_homunculus &hom = hd->homunculus;

	hom.level++;
	if (!(hom.level % 3))
		hom.skillpts++;	//1 skillpoint each 3 base level

	hom.exp -= hd->exp_next;
	hd->exp_next = homun_exp_db.get_nextexp(hom.level);

	if (!max) {
		max  = &hd->homunculusDB->gmax;
		min  = &hd->homunculusDB->gmin;
	}

	int32 growth_max_hp = rnd_value(min->HP, max->HP);
	int32 growth_max_sp = rnd_value(min->SP, max->SP);
	int32 growth_str = rnd_value(min->str, max->str);
	int32 growth_agi = rnd_value(min->agi, max->agi);
	int32 growth_vit = rnd_value(min->vit, max->vit);
	int32 growth_dex = rnd_value(min->dex, max->dex);
	int32 growth_int = rnd_value(min->int_,max->int_);
	int32 growth_luk = rnd_value(min->luk, max->luk);

	//Aegis discards the decimals in the stat growth values!
	growth_str-=growth_str%10;
	growth_agi-=growth_agi%10;
	growth_vit-=growth_vit%10;
	growth_dex-=growth_dex%10;
	growth_int-=growth_int%10;
	growth_luk-=growth_luk%10;

	hom.max_hp += growth_max_hp;
	hom.max_sp += growth_max_sp;
	hom.str += growth_str;
	hom.agi += growth_agi;
	hom.vit += growth_vit;
	hom.dex += growth_dex;
	hom.int_+= growth_int;
	hom.luk += growth_luk;

	APPLY_HOMUN_LEVEL_STATWEIGHT();

	// Needed to update skill list for mutated homunculus so unlocked skills will appear when the needed level is reached.
	status_calc_homunculus(hd,SCO_NONE);
	clif_hominfo(hd->master,hd,0);
	clif_homunculus_updatestatus(*hd->master, SP_BASEEXP);
	clif_homskillinfoblock( *hd );

	if ( hd->master && battle_config.homunculus_show_growth ) {
		char output[256] ;
		sprintf(output,
			"Growth: hp:%d sp:%d str(%.2f) agi(%.2f) vit(%.2f) int(%.2f) dex(%.2f) luk(%.2f) ",
			growth_max_hp, growth_max_sp,
			growth_str/10.0, growth_agi/10.0, growth_vit/10.0,
			growth_int/10.0, growth_dex/10.0, growth_luk/10.0);
		clif_messagecolor(hd->master, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
	}
	return 1;
}

/**
* Changes homunculus class
* @param hd: Homunculus data
* @param class_: New class
* @reutrn Fails if the class cannot be changed, otherwise true
*/
static bool hom_change_class(struct homun_data *hd, int32 class_) {
	std::shared_ptr<s_homunculus_db> homun = homunculus_db.homun_search(class_);

	if (homun == nullptr)
		return false;

	hd->homunculusDB = homun;
	hd->homunculus.class_ = class_;
	status_set_viewdata(hd, class_);
	return true;
}

/**
 * Make an homonculus evolve, (changing in evolution class and apply bonus)
 * @param hd : homonculus datas
 * @return 0:failure, 1:success
 */
int32 hom_evolution(struct homun_data *hd)
{
	nullpo_ret(hd);

	if(!hd->homunculusDB->evo_class || hd->homunculus.class_ == hd->homunculusDB->evo_class) {
		clif_emotion(*hd, ET_SCRATCH);
		return 0 ;
	}

	map_session_data *sd = hd->master;

	if (!sd)
		return 0;

	if (!hom_change_class(hd, hd->homunculusDB->evo_class)) {
		ShowError("hom_evolution: Can't evolve homunc from %d to %d\n", hd->homunculus.class_, hd->homunculusDB->evo_class);
		return 0;
	}

	// Apply evolution bonuses
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

	clif_class_change( *hd, hd->homunculusDB->evo_class );

	// status_Calc flag&1 will make current HP/SP be reloaded from hom structure
	hom->hp = hd->battle_status.hp;
	hom->sp = hd->battle_status.sp;
	status_calc_homunculus(hd, SCO_FIRST);

	clif_hominfo(sd, hd, 0);
	// Official servers don't recaculate the skill tree after evolution
	// but we do it for convenience
	hom_calc_skilltree(hd);

	if (!(battle_config.hom_setting&HOMSET_NO_INSTANT_LAND_SKILL))
		skill_unit_move(sd->hd,gettick(),1); // apply land skills immediately

	return 1 ;
}

/**
 * Make an homonculus mutate in renewal homon
 * @param hd : homonculus datas
 * @param homun_id : id to make it transform into (must be a valid homon class)
 * @return 0:failure, 1:sucess
 */
int32 hom_mutate(struct homun_data *hd, int32 homun_id)
{
	struct s_homunculus *hom;
	map_session_data *sd;
	int32 m_class, m_id, prev_class = 0;
	nullpo_ret(hd);

	m_class = hom_class2mapid(hd->homunculus.class_);
	m_id    = hom_class2mapid(homun_id);

	if( m_class == -1 || m_id == -1 || !(m_class&HOM_EVO) || !(m_id&HOM_S) ) {
		clif_emotion(*hd, ET_SCRATCH);
		return 0;
	}

	sd = hd->master;
	if (!sd)
		return 0;

	prev_class = hd->homunculus.class_;

	if (!hom_change_class(hd, homun_id)) {
		ShowError("hom_mutate: Can't evolve homunc from %d to %d\n", hd->homunculus.class_, homun_id);
		return 0;
	}

	clif_class_change(*hd, homun_id );

	// status_Calc flag&1 will make current HP/SP be reloaded from hom structure
	hom = &hd->homunculus;
	hom->hp = hd->battle_status.hp;
	hom->sp = hd->battle_status.sp;
	hom->prev_class = prev_class;
	status_calc_homunculus(hd, SCO_FIRST);

	clif_hominfo(sd, hd, 0);
	// Official servers don't recaculate the skill tree after evolution
	// but we do it for convenience
	hom_calc_skilltree(hd);

	if (!(battle_config.hom_setting&HOMSET_NO_INSTANT_LAND_SKILL))
		skill_unit_move(sd->hd, gettick(), 1); // apply land skills immediately

	return 1;
}

/**
* Add homunculus exp
* @param hd
* @param exp Added EXP
*/
void hom_gainexp(struct homun_data *hd,t_exp exp)
{
	int32 m_class;

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
		clif_homunculus_updatestatus(*hd->master, SP_BASEEXP);
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

	clif_specialeffect(hd,EF_HO_UP,AREA);
	status_calc_homunculus(hd, SCO_NONE);
	status_percent_heal(hd, 100, 100);
}

/**
* Increase homunculus intimacy
* @param hd
* @param value Added intimacy
* @return New intimacy value
*/
int32 hom_increase_intimacy(struct homun_data * hd, uint32 value)
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
int32 hom_decrease_intimacy(struct homun_data * hd, uint32 value)
{
	nullpo_ret(hd);
	if (hd->homunculus.intimacy >= value)
		hd->homunculus.intimacy -= value;
	else
		hd->homunculus.intimacy = 0;

	return hd->homunculus.intimacy;
}

/**
* Update homunculus status after healing/damage
* @param hd Homunculus data
* @param hp HP amount
* @param sp SP amount
*/
void hom_heal(homun_data& hd, bool hp, bool sp) {
	if (hd.master == nullptr)
		return;

	if (hp)
		clif_homunculus_updatestatus(*hd.master, SP_HP);
	if (sp)
		clif_homunculus_updatestatus(*hd.master, SP_SP);
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

	// Clear skill cooldown array.
	for (uint16 i = 0; i < MAX_SKILLCOOLDOWN; i++)
		hd->homunculus.scd[i] = {};

	// Store current cooldown entries.
	uint16 count = 0;
	t_tick tick = gettick();

	for (const auto &entry : hd->scd) {
		const TimerData *timer = get_timer(entry.second);

		if (timer == nullptr || timer->func != skill_blockhomun_end || DIFF_TICK(timer->tick, tick) < 0)
			continue;

		hd->homunculus.scd[count] = { entry.first, DIFF_TICK(timer->tick, tick) };

		count++;
	}

	intif_homunculus_requestsave(sd->status.account_id, &hd->homunculus);
}

/**
* Perform requested action from selected homunculus menu
* @param sd
* @param type
*/
void hom_menu(map_session_data *sd, int32 type)
{
	nullpo_retv(sd);
	if (sd->hd == nullptr)
		return;

	switch(type) {
		case 0:
			break;
		case 1:
			hom_food(sd, sd->hd);
			break;
		case 2:
			hom_delete(sd->hd);
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
int32 hom_food(map_session_data *sd, struct homun_data *hd)
{
	int32 i, foodID, emotion;

	nullpo_retr(1,sd);
	nullpo_retr(1,hd);

	if (hd->homunculus.vaporize)
		return 1;

	foodID = hd->homunculusDB->foodID;
	i = pc_search_inventory(sd,foodID);
	if (i < 0) {
		clif_hom_food( *sd, foodID, 0 );
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

	clif_emotion(*hd, static_cast<emotion_type>(emotion));
	clif_send_homdata( *hd, SP_HUNGRY );
	clif_send_homdata( *hd, SP_INTIMATE );
	clif_hom_food( *sd, foodID, 1 );

	// Too much food :/
	if(hd->homunculus.intimacy == 0)
		return hom_delete(sd->hd);

	return 0;
}

/**
* Timer to reduce hunger level
*/
static TIMER_FUNC(hom_hungry){
	map_session_data *sd;
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
		clif_emotion( *hd, ET_FRET );
	} else if(hd->homunculus.hunger == 25) {
		clif_emotion( *hd, ET_SCRATCH );
	} else if(hd->homunculus.hunger == 75) {
		clif_emotion( *hd, ET_OK );
	}

	if( battle_config.feature_homunculus_autofeed && hd->homunculus.autofeed && hd->homunculus.hunger <= battle_config.feature_homunculus_autofeed_rate ){
		hom_food( sd, hd );
	}

	if (hd->homunculus.hunger < 0) {
		hd->homunculus.hunger = 0;
		// Delete the homunculus if intimacy <= 100
		if (!hom_decrease_intimacy(hd, 100))
			return hom_delete(hd);
		clif_send_homdata( *hd, SP_INTIMATE );
	}

	clif_send_homdata( *hd, SP_HUNGRY );

	int32 hunger_delay = (battle_config.homunculus_starving_rate > 0 && hd->homunculus.hunger <= battle_config.homunculus_starving_rate) ? battle_config.homunculus_starving_delay : hd->homunculusDB->hungryDelay; // Every 20 seconds if hunger <= 10

	hd->hungry_timer = add_timer(tick+hunger_delay,hom_hungry,sd->id,0); //simple Fix albator
	return 0;
}

/**
* Remove hungry timer from homunculus
* @param hd
*/
int32 hom_hungry_timer_delete(struct homun_data *hd)
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
int32 hom_change_name(map_session_data *sd,char *name)
{
	int32 i;
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
void hom_change_name_ack(map_session_data *sd, char* name, int32 flag)
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
	hd->homunculus.rename_flag = 1;
	clif_hominfo(sd,hd,1);
	clif_name_area(hd);
	clif_hominfo(sd,hd,0);
}

/**
* Create homunc structure
* @param sd
* @param hom
*/
void hom_alloc(map_session_data *sd, struct s_homunculus *hom)
{
	nullpo_retv(sd);

	Assert((sd->status.hom_id == 0 || sd->hd == 0) || sd->hd->master == sd);

	std::shared_ptr<s_homunculus_db> homun_db = homunculus_db.homun_search(hom->class_);

	if (homun_db == nullptr) {
		ShowError("hom_alloc: unknown class [%d] for homunculus '%s', requesting deletion.\n", hom->class_, hom->name);
		sd->status.hom_id = 0;
		intif_homunculus_requestdelete(hom->hom_id);
		return;
	}

	struct homun_data *hd;
	t_tick tick = gettick();

	sd->hd = hd = (struct homun_data*)aCalloc(1,sizeof(struct homun_data));
	new (sd->hd) homun_data();

	hd->type = BL_HOM;
	hd->id = npc_get_new_npc_id();

	hd->master = sd;
	hd->homunculusDB = homun_db;
	memcpy(&hd->homunculus, hom, sizeof(struct s_homunculus));
	hd->exp_next = homun_exp_db.get_nextexp(hd->homunculus.level);

	status_set_viewdata(hd, hd->homunculus.class_);
	unit_dataset(hd);
	hd->ud.dir = sd->ud.dir;

	// Find a random valid pos around the player
	hd->m = sd->m;
	hd->x = sd->x;
	hd->y = sd->y;
	unit_calc_pos(hd, sd->x, sd->y, sd->ud.dir);
	hd->x = hd->ud.to_x;
	hd->y = hd->ud.to_y;

	// Ticks need to be initialized before adding bl to map_addiddb
	hd->regen.tick.hp = tick;
	hd->regen.tick.sp = tick;

	map_addiddb(hd);
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
		int32 hunger_delay = (battle_config.homunculus_starving_rate > 0 && hd->homunculus.hunger <= battle_config.homunculus_starving_rate) ? battle_config.homunculus_starving_delay : hd->homunculusDB->hungryDelay; // Every 20 seconds if hunger <= 10

		hd->hungry_timer = add_timer(gettick()+hunger_delay,hom_hungry,hd->master->id,0);
	}
	hd->regen.state.block = 0; //Restore HP/SP block.
	hd->masterteleport_timer = INVALID_TIMER;
}

/**
 * Make a player spawn a homonculus (call)
 * @param sd
 * @return False:failure, True:sucess
 */
bool hom_call(map_session_data *sd)
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
	if (hd->prev == nullptr)
	{	//Spawn him
		hd->x = sd->x;
		hd->y = sd->y;
		hd->m = sd->m;
		if(map_addblock(hd))
			return false;
		clif_spawn(hd);
		clif_send_homdata( *hd, SP_ACK );
		// For some reason, official servers send the homunculus info twice, then update the HP/SP again.
		clif_hominfo(sd, hd, 1);
		clif_hominfo(sd, hd, 0);
		clif_homunculus_updatestatus(*sd, SP_HP);
		clif_homunculus_updatestatus(*sd, SP_SP);
		clif_homskillinfoblock( *hd );
		status_calc_bl(hd, { SCB_SPEED });
		hom_save(hd);
	} else
		//Warp him to master.
		unit_warp(hd,sd->m, sd->x, sd->y,CLR_OUTSIGHT);

	// Apply any active skill cooldowns.
	for (const auto &entry : hd->scd) {
		skill_blockhomun_start(*hd, entry.first, entry.second);
	}

#ifdef RENEWAL
	sc_start(sd, sd, SC_HOMUN_TIME, 100, 1, skill_get_time(AM_CALLHOMUN, 1));
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
int32 hom_recv_data(uint32 account_id, struct s_homunculus *sh, int32 flag)
{
	map_session_data *sd;
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
		status_percent_heal(hd, 100, 100);

	if(hd && hd->homunculus.hp && !hd->homunculus.vaporize && hd->prev == nullptr && sd->prev != nullptr)
	{
		if(map_addblock(hd))
			return 0;
		clif_spawn(hd);
		clif_send_homdata( *hd, SP_ACK );
		// For some reason, official servers send the homunculus info twice, then update the HP/SP again.
		clif_hominfo(sd, hd, 1);
		clif_hominfo(sd, hd, 0);
		clif_homunculus_updatestatus(*sd, SP_HP);
		clif_homunculus_updatestatus(*sd, SP_SP);
		clif_homskillinfoblock( *hd );
		hom_init_timers(hd);

#ifdef RENEWAL
		sc_start(sd, sd, SC_HOMUN_TIME, 100, 1, skill_get_time(AM_CALLHOMUN, 1));
#endif
	}

	// Apply any active skill cooldowns.
	for (uint16 i = 0; i < ARRAYLENGTH(hd->homunculus.scd); i++) {
		skill_blockhomun_start(*hd, hd->homunculus.scd[i].skill_id, hd->homunculus.scd[i].tick);
	}

	return 1;
}

/**
* Ask homunculus creation to char-server
* @param sd
* @param class_
* @return True:Success; False:Failed
*/
bool hom_create_request(map_session_data *sd, int32 class_)
{
	nullpo_ret(sd);

	std::shared_ptr<s_homunculus_db> homun_db = homunculus_db.homun_search(class_);

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
int32 hom_ressurect(map_session_data* sd, unsigned char per, int16 x, int16 y)
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

	if (!status_isdead(*hd))
		return 0; // already alive

	hom_init_timers(hd);

	if (!hd->prev)
	{	//Add it back to the map.
		hd->m = sd->m;
		hd->x = x;
		hd->y = y;
		if(map_addblock(hd))
			return 0;
		clif_spawn(hd);
	}

	hd->ud.state.blockedmove = false;

	// Apply any active skill cooldowns.
	for (const auto &entry : hd->scd) {
		skill_blockhomun_start(*hd, entry.first, entry.second);
	}

#ifdef RENEWAL
	sc_start(sd, sd, SC_HOMUN_TIME, 100, 1, skill_get_time(AM_CALLHOMUN, 1));
#endif

	return status_revive(hd, per, 0);
}

/**
* Revive homunculus
* @param hd
* @param hp
* @param sp
*/
void hom_revive(struct homun_data *hd, uint32 hp, uint32 sp)
{
	map_session_data *sd = hd->master;
	hd->homunculus.hp = hd->battle_status.hp;
	if (!sd)
		return;
	clif_send_homdata( *hd, SP_ACK );
	// For some reason, official servers send the homunculus info twice, then update the HP/SP again.
	clif_hominfo(sd, hd, 1);
	clif_hominfo(sd, hd, 0);
	clif_homunculus_updatestatus(*sd, SP_HP);
 	clif_homunculus_updatestatus(*sd, SP_SP);
	clif_homskillinfoblock( *hd );

	if( hd->homunculus.class_ == MER_ELEANOR ){
		sc_start(hd,hd, SC_STYLE_CHANGE, 100, MH_MD_FIGHTING, INFINITE_TICK);
	}
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
int32 hom_shuffle(struct homun_data *hd)
{
	map_session_data *sd;
	int32 lv, i, skillpts;
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
	clif_homskillinfoblock( *hd );
	status_calc_homunculus(hd, SCO_NONE);
	status_percent_heal(hd, 100, 100);
	clif_specialeffect(hd,EF_HO_UP,AREA);

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

bool HomunculusDatabase::parseStatusNode(const std::string &nodeName, const std::string &subNodeName, const ryml::NodeRef &node, s_hom_stats &bonus) {
	uint32 value;

	if (!this->asUInt32(node, nodeName, value))
		return false;

	if (subNodeName.compare("Hp") == 0)
		bonus.HP = value;
	else if (subNodeName.compare("Sp") == 0)
		bonus.SP = value;
	else if (subNodeName.compare("Str") == 0)
		bonus.str = static_cast<uint16>(value);
	else if (subNodeName.compare("Agi") == 0)
		bonus.agi = static_cast<uint16>(value);
	else if (subNodeName.compare("Vit") == 0)
		bonus.vit = static_cast<uint16>(value);
	else if (subNodeName.compare("Int") == 0)
		bonus.int_ = static_cast<uint16>(value);
	else if (subNodeName.compare("Dex") == 0)
		bonus.dex = static_cast<uint16>(value);
	else if (subNodeName.compare("Luk") == 0)
		bonus.luk = static_cast<uint16>(value);

	return true;
}

/**
 * Reads and parses an entry from the homunculus_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 HomunculusDatabase::parseBodyNode(const ryml::NodeRef &node) {
	std::string class_name;

	if (!this->asString(node, "Class", class_name))
		return 0;

	std::string class_name_constant = "MER_" + class_name;
	int64 class_tmp;

	if (!script_get_constant(class_name_constant.c_str(), &class_tmp)) {
		this->invalidWarning(node["Class"], "Invalid homunculus Class \"%s\", skipping.\n", class_name.c_str());
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
		hom->base = { 1 };
		hom->gmin = {};
		hom->gmax = {};
		hom->emin = {};
		hom->emax = {};
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		safestrncpy(hom->name, name.c_str(), sizeof(hom->name));
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

	if (this->nodeExists(node, "Food")) {
		std::string food;

		if (!this->asString(node, "Food", food))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname(food.c_str());

		if (item == nullptr) {
			this->invalidWarning(node["Food"], "Invalid homunculus Food %s, skipping.\n", food.c_str());
			return 0;
		}

		hom->foodID = item->nameid;
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

	if (this->nodeExists(node, "Race")) {
		std::string race;

		if (!this->asString(node, "Race", race))
			return 0;

		std::string race_constant = "RC_" + race;
		int64 constant;

		if (!script_get_constant(race_constant.c_str(), &constant)) {
			this->invalidWarning(node["Race"], "Invalid homunculus Race %s, skipping.\n", race.c_str());
			return 0;
		}

		hom->race = static_cast<e_race>(constant);
	} else {
		if (!exists)
			hom->race = RC_DEMIHUMAN;
	}

	if (this->nodeExists(node, "Element")) {
		std::string element;

		if (!this->asString(node, "Element", element))
			return 0;

		std::string element_constant = "ELE_" + element;
		int64 constant;

		if (!script_get_constant(element_constant.c_str(), &constant)) {
			this->invalidWarning(node["Element"], "Invalid homunculus Element %s, skipping.\n", element.c_str());
			return 0;
		}

		hom->element = static_cast<e_element>(constant);
	} else {
		if (!exists)
			hom->element = ELE_NEUTRAL;
	}

	if (this->nodeExists(node, "Size")) {
		std::string size;

		if (!this->asString(node, "Size", size))
			return 0;

		std::string size_constant = "SIZE_" + size;
		int64 constant;

		if (!script_get_constant(size_constant.c_str(), &constant)) {
			this->invalidWarning(node["Size"], "Invalid homunculus Size %s, skipping.\n", size.c_str());
			return 0;
		}

		hom->base_size = static_cast<e_size>(constant);
	} else {
		if (!exists)
			hom->base_size = SZ_SMALL;
	}
	
	if (this->nodeExists(node, "EvolutionSize")) {
		std::string size;

		if (!this->asString(node, "EvolutionSize", size))
			return 0;

		std::string size_constant = "SIZE_" + size;
		int64 constant;

		if (!script_get_constant(size_constant.c_str(), &constant)) {
			this->invalidWarning(node["EvolutionSize"], "Invalid homunculus EvolutionSize %s, skipping.\n", size.c_str());
			return 0;
		}

		hom->base_size = static_cast<e_size>(constant);
	} else {
		if (!exists)
			hom->base_size = SZ_MEDIUM;
	}

	if (this->nodeExists(node, "AttackDelay")) {
		uint16 aspd;

		if (!this->asUInt16(node, "AttackDelay", aspd))
			return 0;

		if (aspd > MIN_ASPD) {
			this->invalidWarning(node["AttackDelay"], "Homunculus AttackDelay %hu exceeds %d, capping.\n", aspd, MIN_ASPD);
			aspd = MIN_ASPD;
		}

		hom->baseASPD = aspd;
	} else {
		if (!exists)
			hom->baseASPD = 700;
	}

	if (this->nodeExists(node, "Status")) {
		std::vector<std::string> stat_list = { "Hp", "Sp", "Str", "Agi", "Vit", "Int", "Dex", "Luk" };

		for (const auto &statusNode : node["Status"]) {
			if (!this->nodeExists(statusNode, "Type"))
				return 0;

			std::string stat_name;

			if (!this->asString(statusNode, "Type", stat_name))
				return 0;

			if (!util::vector_exists(stat_list, stat_name)) {
				this->invalidWarning(statusNode["Type"], "Invalid Status Type %s, skipping.\n", stat_name.c_str());
				return 0;
			}

			if (this->nodeExists(statusNode, "Base")) {
				if (!this->parseStatusNode("Base", stat_name, statusNode, hom->base)) {
					return 0;
				}
			} else {
				if (!exists) {
					hom->base = { 1 };
				}
			}

			if (this->nodeExists(statusNode, "GrowthMinimum")) {
				if (!this->parseStatusNode("GrowthMinimum", stat_name, statusNode, hom->gmin)) {
					return 0;
				}
			} else {
				if (!exists) {
					hom->gmin = {};
				}
			}

			if (this->nodeExists(statusNode, "GrowthMaximum")) {
				if (!this->parseStatusNode("GrowthMaximum", stat_name, statusNode, hom->gmax)) {
					return 0;
				}
			} else {
				if (!exists) {
					hom->gmax = {};
				}
			}

			if (this->nodeExists(statusNode, "EvolutionMinimum")) {
				if (!this->parseStatusNode("EvolutionMinimum", stat_name, statusNode, hom->emin)) {
					return 0;
				}
			} else {
				if (!exists) {
					hom->emin = {};
				}
			}

			if (this->nodeExists(statusNode, "EvolutionMaximum")) {
				if (!this->parseStatusNode("EvolutionMaximum", stat_name, statusNode, hom->emax)) {
					return 0;
				}
			} else {
				if (!exists) {
					hom->emax = {};
				}
			}
		}

		// Cap values
		if (hom->gmin.HP > hom->gmax.HP) {
			hom->gmin.HP = hom->gmax.HP;
			this->invalidWarning(node, "GrowthMinimum HP %d is greater than GrowthMaximum HP %d for homunculus %s, capping minimum to maximum.\n", hom->gmin.HP, hom->gmax.HP, class_name.c_str());
		}
		if (hom->gmin.SP > hom->gmax.SP) {
			hom->gmin.SP = hom->gmax.SP;
			this->invalidWarning(node, "GrowthMinimum SP %d is greater than GrowthMaximum SP %d for homunculus %s, capping minimum to maximum.\n", hom->gmin.SP, hom->gmax.SP, class_name.c_str());
		}
		if (hom->gmin.str > hom->gmax.str) {
			hom->gmin.str = hom->gmax.str;
			this->invalidWarning(node, "GrowthMinimum STR %d is greater than GrowthMaximum STR %d for homunculus %s, capping minimum to maximum.\n", hom->gmin.str, hom->gmax.str, class_name.c_str());
		}
		if (hom->gmin.agi > hom->gmax.agi) {
			hom->gmin.agi = hom->gmax.agi;
			this->invalidWarning(node, "GrowthMinimum AGI %d is greater than GrowthMaximum AGI %d for homunculus %s, capping minimum to maximum.\n", hom->gmin.agi, hom->gmax.agi, class_name.c_str());
		}
		if (hom->gmin.vit > hom->gmax.vit) {
			hom->gmin.vit = hom->gmax.vit;
			this->invalidWarning(node, "GrowthMinimum VIT %d is greater than GrowthMaximum VIT %d for homunculus %s, capping minimum to maximum.\n", hom->gmin.vit, hom->gmax.vit, class_name.c_str());
		}
		if (hom->gmin.int_ > hom->gmax.int_) {
			hom->gmin.int_ = hom->gmax.int_;
			this->invalidWarning(node, "GrowthMinimum INT %d is greater than GrowthMaximum INT %d for homunculus %s, capping minimum to maximum.\n", hom->gmin.int_, hom->gmax.int_, class_name.c_str());
		}
		if (hom->gmin.dex > hom->gmax.dex) {
			hom->gmin.dex = hom->gmax.dex;
			this->invalidWarning(node, "GrowthMinimum DEX %d is greater than GrowthMaximum DEX %d for homunculus %s, capping minimum to maximum.\n", hom->gmin.dex, hom->gmax.dex, class_name.c_str());
		}
		if (hom->gmin.luk > hom->gmax.luk) {
			hom->gmin.luk = hom->gmax.luk;
			this->invalidWarning(node, "GrowthMinimum LUK %d is greater than GrowthMaximum LUK %d for homunculus %s, capping minimum to maximum.\n", hom->gmin.luk, hom->gmax.luk, class_name.c_str());
		}
		if (hom->emin.HP > hom->emax.HP) {
			hom->emin.HP = hom->emax.HP;
			this->invalidWarning(node, "EvolutionMinimum HP %d is greater than EvolutionMaximum HP %d for homunculus %s, capping minimum to maximum.\n", hom->emin.HP, hom->emax.HP, class_name.c_str());
		}
		if (hom->emin.SP > hom->emax.SP) {
			hom->emin.SP = hom->emax.SP;
			this->invalidWarning(node, "EvolutionMinimum SP %d is greater than EvolutionMaximum SP %d for homunculus %s, capping minimum to maximum.\n", hom->emin.SP, hom->emax.SP, class_name.c_str());
		}
		if (hom->emin.str > hom->emax.str) {
			hom->emin.str = hom->emax.str;
			this->invalidWarning(node, "EvolutionMinimum STR %d is greater than EvolutionMaximum STR %d for homunculus %s, capping minimum to maximum.\n", hom->emin.str, hom->emax.str, class_name.c_str());
		}
		if (hom->emin.agi > hom->emax.agi) {
			hom->emin.agi = hom->emax.agi;
			this->invalidWarning(node, "EvolutionMinimum AGI %d is greater than EvolutionMaximum AGI %d for homunculus %s, capping minimum to maximum.\n", hom->emin.agi, hom->emax.agi, class_name.c_str());
		}
		if (hom->emin.vit > hom->emax.vit) {
			hom->emin.vit = hom->emax.vit;
			this->invalidWarning(node, "EvolutionMinimum VIT %d is greater than EvolutionMaximum VIT %d for homunculus %s, capping minimum to maximum.\n", hom->emin.vit, hom->emax.vit, class_name.c_str());
		}
		if (hom->emin.int_ > hom->emax.int_) {
			hom->emin.int_ = hom->emax.int_;
			this->invalidWarning(node, "EvolutionMinimum INT %d is greater than EvolutionMaximum INT %d for homunculus %s, capping minimum to maximum.\n", hom->emin.int_, hom->emax.int_, class_name.c_str());
		}
		if (hom->emin.dex > hom->emax.dex) {
			hom->emin.dex = hom->emax.dex;
			this->invalidWarning(node, "EvolutionMinimum DEX %d is greater than EvolutionMaximum DEX %d for homunculus %s, capping minimum to maximum.\n", hom->emin.dex, hom->emax.dex, class_name.c_str());
		}
		if (hom->emin.luk > hom->emax.luk) {
			hom->emin.luk = hom->emax.luk;
			this->invalidWarning(node, "EvolutionMinimum LUK %d is greater than EvolutionMaximum LUK %d for homunculus %s, capping minimum to maximum.\n", hom->emin.luk, hom->emax.luk, class_name.c_str());
		}
	}

	if (this->nodeExists(node, "SkillTree")) {
		const ryml::NodeRef &skillsNode = node["SkillTree"];

		for (const ryml::NodeRef &skill : skillsNode) {
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

			if (this->nodeExists(skill, "Clear")) {
				std::vector<s_homun_skill_tree_entry>::iterator it = hom->skill_tree.begin();
				bool found = false;

				while (it != hom->skill_tree.end()) {
					if (it->id == entry.id) { // Skill found, remove it from the skill tree.
						it = hom->skill_tree.erase(it);
						found = true;
					} else {
						it++;
					}
				}

				if (!found)
					this->invalidWarning(skill["Clear"], "Failed to remove nonexistent skill %s from homunuculus %s.\n", skill_db.find(entry.id)->name, class_name.c_str());
				continue;
			}

			if (this->nodeExists(skill, "MaxLevel")) {
				uint16 level;

				if (!this->asUInt16(skill, "MaxLevel", level))
					return 0;

				uint16 db_level = skill_get_max(entry.id);

				if (level > db_level) {
					this->invalidWarning(skill["MaxLevel"], "Skill %s exceeds maximum defined skill level %d from homunuculus %s, capping.\n", skill_db.find(entry.id)->name, db_level, class_name.c_str());
					level = db_level;
				}

				entry.max = level;
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

				entry.need_level = level;
			} else {
				if (!exists)
					entry.need_level = 0;
			}

			if (this->nodeExists(skill, "RequiredIntimacy")) {
				uint16 intimacy;

				if (!this->asUInt16(skill, "RequiredIntimacy", intimacy))
					return 0;

				if (intimacy > 1000) {
					this->invalidWarning(skill["RequiredIntimacy"], "Homunculus Required Intimacy %u exceeds maximum intimacy 1000, capping.\n", intimacy);
					intimacy = 1000;
				}

				entry.intimacy = intimacy * 100;
			} else {
				if (!exists)
					entry.intimacy = 0;
			}

			if (this->nodeExists(skill, "RequireEvolution")) {
				bool evo;

				if (!this->asBool(skill, "RequireEvolution", evo))
					return 0;

				if (evo && hom->base_class == hom->evo_class) {
					this->invalidWarning(skill["RequireEvolution"], "Homunculus %s does not have any evolution making skill %s unobtainable, skipping.\n", class_name.c_str(), skill_db.find(entry.id)->name);
					return 0;
				}

				entry.evolution = evo;
			} else {
				if (!exists)
					entry.evolution = false;
			}

			if (this->nodeExists(skill, "Required")) {
				const ryml::NodeRef &required = skill["Required"];

				for (const ryml::NodeRef &prereqskill : required) {
					uint16 skill_id = 0, skill_lv = 0;

					if (this->nodeExists(prereqskill, "Skill")) {
						std::string skill_name;

						if (!this->asString(prereqskill, "Skill", skill_name))
							return 0;

						skill_id = skill_name2id(skill_name.c_str());

						if (skill_id == 0) {
							this->invalidWarning(prereqskill["Skill"], "Invalid homunculus skill %s, skipping.\n", skill_name.c_str());
							return 0;
						}

						if (!SKILL_CHK_HOMUN(skill_id)) {
							this->invalidWarning(prereqskill["Skill"], "Homunculus skill %s (%u) is out of the homunculus skill range [%u-%u], skipping.\n", skill_name.c_str(), skill_id, HM_SKILLBASE, HM_SKILLBASE + MAX_HOMUNSKILL - 1);
							return 0;
						}
					}

					if (this->nodeExists(prereqskill, "Clear")) {
						bool found = false;

						for (auto &skit : hom->skill_tree) {
							std::unordered_map<uint16, uint16>::iterator it = skit.need.begin();

							while (it != skit.need.end()) {
								if (it->first == skill_id) { // Skill found, remove it from the skill tree.
									it = skit.need.erase(it);
									found = true;
								} else {
									it++;
								}
							}
						}

						if (!found)
							this->invalidWarning(prereqskill["Clear"], "Failed to remove nonexistent prerequisite skill %s from homunuculus %s.\n", skill_db.find(skill_id)->name, class_name.c_str());
						continue;
					}

					if (this->nodeExists(prereqskill, "Level")) {
						if (!this->asUInt16(prereqskill, "Level", skill_lv))
							return 0;
					}

					if (skill_id > 0 && skill_lv > 0)
						entry.need.emplace(skill_id, skill_lv);
				}
			}

			hom->skill_tree.push_back(entry);
		}
	}

	if (!exists)
		this->put(class_id, hom);

	return 1;
}

/**
 * Since evolved homunculus share a database entry, use this search.
 * !TODO: Clean this up so evolved homunculus have their own entry
 * @param class_: Homun class to look up
 * @return Shared pointer of homunculus on success, otherwise nullptr
 */
std::shared_ptr<s_homunculus_db> HomunculusDatabase::homun_search(int32 class_) {
	std::shared_ptr<s_homunculus_db> hom = homunculus_db.find(class_);

	if (hom != nullptr) {
		return hom;
	}

	for (const auto &homit : homunculus_db) {
		hom = homit.second;

		if (hom->evo_class == class_) {
			return hom;
		}
	}

	return nullptr;
}

HomunculusDatabase homunculus_db;

void hom_reload(void){
	homunculus_db.load();
	homun_exp_db.reload();
}

void do_init_homunculus(void){
	int32 class_;

	homunculus_db.load();
	homun_exp_db.load();

	// Add homunc timer function to timer func list [Toms]
	add_timer_func_list(hom_hungry, "hom_hungry");

	//Stock view data for homuncs
	memset(&hom_viewdb, 0, sizeof(hom_viewdb));
	for (class_ = 0; class_ < ARRAYLENGTH(hom_viewdb); class_++)
		hom_viewdb[class_].look[LOOK_BASE] = HM_CLASS_BASE+class_;
}

void do_final_homunculus(void) {
	//Nothing todo yet
}
