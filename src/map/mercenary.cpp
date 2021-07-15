// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary.hpp"

#include <map>
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
#include "../common/utilities.hpp"
#include "../common/utils.hpp"

#include "clif.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "pc.hpp"
#include "trade.hpp"

using namespace rathena;

MercenaryDatabase mercenary_db;

/**
* Get View Data of Mercenary by Class ID
* @param class_ The Class ID
* @return View Data of Mercenary
**/
struct view_data *mercenary_get_viewdata( uint16 class_ ){
	std::shared_ptr<s_mercenary_db> db = mercenary_db.find(class_);

	if( db ){
		return &db->vd;
	}else{
		return nullptr;
	}
}

/**
* Create a new Mercenary for Player
* @param sd The Player
* @param class_ Mercenary Class
* @param lifetime Contract duration
* @return false if failed, true otherwise
**/
bool mercenary_create(struct map_session_data *sd, uint16 class_, unsigned int lifetime) {
	struct s_mercenary merc;
	nullpo_retr(false,sd);

	std::shared_ptr<s_mercenary_db> db = mercenary_db.find(class_);

	if( !db ){
		return false;
	}

	memset(&merc,0,sizeof(struct s_mercenary));

	merc.char_id = sd->status.char_id;
	merc.class_ = class_;
	merc.hp = db->status.max_hp;
	merc.sp = db->status.max_sp;
	merc.life_time = lifetime;

	// Request Char Server to create this mercenary
	intif_mercenary_create(&merc);

	return true;
}

/**
* Get current Mercenary lifetime
* @param md The Mercenary
* @return The Lifetime
**/
t_tick mercenary_get_lifetime(struct mercenary_data *md) {
	const struct TimerData * td;
	if( md == NULL || md->contract_timer == INVALID_TIMER )
		return 0;

	td = get_timer(md->contract_timer);
	return (td != NULL) ? DIFF_TICK(td->tick, gettick()) : 0;
}

/**
* Get Guild type of Mercenary
* @param md Mercenary
* @return enum e_MercGuildType
**/
enum e_MercGuildType mercenary_get_guild(struct mercenary_data *md){
	uint16 class_;

	if( md == NULL || md->db == NULL )
		return NONE_MERC_GUILD;

	class_ = md->db->class_;

	if( class_ >= MERID_MER_ARCHER01 && class_ <= MERID_MER_ARCHER10 )
		return ARCH_MERC_GUILD;
	if( class_ >= MERID_MER_LANCER01 && class_ <= MERID_MER_LANCER10 )
		return SPEAR_MERC_GUILD;
	if( class_ >= MERID_MER_SWORDMAN01 && class_ <= MERID_MER_SWORDMAN10 )
		return SWORD_MERC_GUILD;

	return NONE_MERC_GUILD;
}

/**
* Get Faith value of Mercenary
* @param md Mercenary
* @return the Faith value
**/
int mercenary_get_faith(struct mercenary_data *md) {
	struct map_session_data *sd;
	enum e_MercGuildType guild;

	if( md == NULL || md->db == NULL || (sd = md->master) == NULL )
		return 0;

	guild = mercenary_get_guild(md);

	switch( guild ){
		case ARCH_MERC_GUILD:
			return sd->status.arch_faith;
		case SPEAR_MERC_GUILD:
			return sd->status.spear_faith;
		case SWORD_MERC_GUILD:
			return sd->status.sword_faith;
		case NONE_MERC_GUILD:
		default:
			return 0;
	}
}

/**
* Set faith value of Mercenary
* @param md The Mercenary
* @param value Faith Value
**/
void mercenary_set_faith(struct mercenary_data *md, int value) {
	struct map_session_data *sd;
	enum e_MercGuildType guild;
	int *faith;

	if( md == NULL || md->db == NULL || (sd = md->master) == NULL )
		return;

	guild = mercenary_get_guild(md);

	switch( guild ){
		case ARCH_MERC_GUILD:
			faith = &sd->status.arch_faith;
			break;
		case SPEAR_MERC_GUILD:
			faith = &sd->status.spear_faith;
			break;
		case SWORD_MERC_GUILD:
			faith = &sd->status.sword_faith;
			break;
		case NONE_MERC_GUILD:
			return;
	}

	*faith += value;
	*faith = cap_value(*faith, 0, SHRT_MAX);
	clif_mercenary_updatestatus(sd, SP_MERCFAITH);
}

/**
* Get Mercenary's calls
* @param md Mercenary
* @return Number of calls
**/
int mercenary_get_calls(struct mercenary_data *md) {
	struct map_session_data *sd;
	enum e_MercGuildType guild;

	if( md == NULL || md->db == NULL || (sd = md->master) == NULL )
		return 0;

	guild = mercenary_get_guild(md);

	switch( guild ){
		case ARCH_MERC_GUILD:
			return sd->status.arch_calls;
		case SPEAR_MERC_GUILD:
			return sd->status.spear_calls;
		case SWORD_MERC_GUILD:
			return sd->status.sword_calls;
		case NONE_MERC_GUILD:
		default:
			return 0;
	}
}

/**
* Set Mercenary's calls
* @param md Mercenary
* @param value
**/
void mercenary_set_calls(struct mercenary_data *md, int value) {
	struct map_session_data *sd;
	enum e_MercGuildType guild;
	int *calls;

	if( md == NULL || md->db == NULL || (sd = md->master) == NULL )
		return;

	guild = mercenary_get_guild(md);

	switch( guild ){
		case ARCH_MERC_GUILD:
			calls = &sd->status.arch_calls;
			break;
		case SPEAR_MERC_GUILD:
			calls = &sd->status.spear_calls;
			break;
		case SWORD_MERC_GUILD:
			calls = &sd->status.sword_calls;
			break;
		case NONE_MERC_GUILD:
			return;
	}

	*calls += value;
	*calls = cap_value(*calls, 0, INT_MAX);
}

/**
* Save Mercenary data
* @param md Mercenary
**/
void mercenary_save(struct mercenary_data *md) {
	md->mercenary.hp = md->battle_status.hp;
	md->mercenary.sp = md->battle_status.sp;
	md->mercenary.life_time = mercenary_get_lifetime(md);

	intif_mercenary_save(&md->mercenary);
}

/**
* Ends contract of Mercenary
**/
static TIMER_FUNC(merc_contract_end){
	struct map_session_data *sd;
	struct mercenary_data *md;

	if( (sd = map_id2sd(id)) == NULL )
		return 1;
	if( (md = sd->md) == NULL )
		return 1;

	if( md->contract_timer != tid )
	{
		ShowError("merc_contract_end %d != %d.\n", md->contract_timer, tid);
		return 0;
	}

	md->contract_timer = INVALID_TIMER;
	mercenary_delete(md, 0); // Mercenary soldier's duty hour is over.

	return 0;
}

/**
* Delete Mercenary
* @param md Mercenary
* @param reply
**/
int mercenary_delete(struct mercenary_data *md, int reply) {
	struct map_session_data *sd = md->master;
	md->mercenary.life_time = 0;

	mercenary_contract_stop(md);

	if( !sd )
		return unit_free(&md->bl, CLR_OUTSIGHT);

	if( md->devotion_flag )
	{
		md->devotion_flag = 0;
		status_change_end(&sd->bl, SC_DEVOTION, INVALID_TIMER);
	}

	switch( reply )
	{
		case 0: mercenary_set_faith(md, 1); break; // +1 Loyalty on Contract ends.
		case 1: mercenary_set_faith(md, -1); break; // -1 Loyalty on Mercenary killed
	}

	clif_mercenary_message(sd, reply);
	return unit_remove_map(&md->bl, CLR_OUTSIGHT);
}

/**
* Stop contract of Mercenary
* @param md Mercenary
**/
void mercenary_contract_stop(struct mercenary_data *md) {
	nullpo_retv(md);
	if( md->contract_timer != INVALID_TIMER )
		delete_timer(md->contract_timer, merc_contract_end);
	md->contract_timer = INVALID_TIMER;
}

/**
* Init contract of Mercenary
* @param md Mercenary
**/
void merc_contract_init(struct mercenary_data *md) {
	if( md->contract_timer == INVALID_TIMER )
		md->contract_timer = add_timer(gettick() + md->mercenary.life_time, merc_contract_end, md->master->bl.id, 0);

	md->regen.state.block = 0;
}

/**
 * Received mercenary data from char-serv
 * @param merc : mercenary datas
 * @param flag : if inter-serv request was sucessfull
 * @return false:failure, true:sucess
 */
bool mercenary_recv_data(struct s_mercenary *merc, bool flag)
{
	struct map_session_data *sd;
	struct mercenary_data *md;

	std::shared_ptr<s_mercenary_db> db = mercenary_db.find(merc->class_);

	if( (sd = map_charid2sd(merc->char_id)) == NULL )
		return false;
	if( !flag || !db ){ // Not created - loaded - DB info
		sd->status.mer_id = 0;
		return false;
	}

	if( !sd->md ) {
		sd->md = md = (struct mercenary_data*)aCalloc(1,sizeof(struct mercenary_data));
		md->bl.type = BL_MER;
		md->bl.id = npc_get_new_npc_id();
		md->devotion_flag = 0;

		md->master = sd;
		md->db = db;
		memcpy(&md->mercenary, merc, sizeof(struct s_mercenary));
		status_set_viewdata(&md->bl, md->mercenary.class_);
		status_change_init(&md->bl);
		unit_dataset(&md->bl);
		md->ud.dir = sd->ud.dir;

		md->bl.m = sd->bl.m;
		md->bl.x = sd->bl.x;
		md->bl.y = sd->bl.y;
		unit_calc_pos(&md->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
		md->bl.x = md->ud.to_x;
		md->bl.y = md->ud.to_y;

		map_addiddb(&md->bl);
		status_calc_mercenary(md, SCO_FIRST);
		md->contract_timer = INVALID_TIMER;
		md->masterteleport_timer = INVALID_TIMER;
		merc_contract_init(md);
	} else {
		memcpy(&sd->md->mercenary, merc, sizeof(struct s_mercenary));
		md = sd->md;
	}

	if( sd->status.mer_id == 0 )
		mercenary_set_calls(md, 1);
	sd->status.mer_id = merc->mercenary_id;

	if( md && md->bl.prev == NULL && sd->bl.prev != NULL ) {
		if(map_addblock(&md->bl))
			return false;
		clif_spawn(&md->bl);
		clif_mercenary_info(sd);
		clif_mercenary_skillblock(sd);
	}

	return true;
}

/**
* Heals Mercenary
* @param md Mercenary
* @param hp HP amount
* @param sp SP amount
**/
void mercenary_heal(struct mercenary_data *md, int hp, int sp) {
	if (md->master == NULL)
		return;
	if( hp )
		clif_mercenary_updatestatus(md->master, SP_HP);
	if( sp )
		clif_mercenary_updatestatus(md->master, SP_SP);
}

/**
 * Delete Mercenary
 * @param md: Mercenary
 * @return false for status_damage
 */
bool mercenary_dead(struct mercenary_data *md) {
	mercenary_delete(md, 1);
	return false;
}

/**
* Gives bonus to Mercenary
* @param md Mercenary
**/
void mercenary_killbonus(struct mercenary_data *md) {
	const enum sc_type scs[] = { SC_MERC_FLEEUP, SC_MERC_ATKUP, SC_MERC_HPUP, SC_MERC_SPUP, SC_MERC_HITUP };
	uint8 index = rnd() % ARRAYLENGTH(scs);

	sc_start(&md->bl,&md->bl, scs[index], 100, rnd() % 5, 600000);
}

/**
* Mercenary does kill
* @param md Mercenary
**/
void mercenary_kills(struct mercenary_data *md){
	if(md->mercenary.kill_count <= (INT_MAX-1)) //safe cap to INT_MAX
		md->mercenary.kill_count++;

	if( (md->mercenary.kill_count % 50) == 0 )
	{
		mercenary_set_faith(md, 1);
		mercenary_killbonus(md);
	}

	if( md->master )
		clif_mercenary_updatestatus(md->master, SP_MERCKILLS);
}

/**
* Check if Mercenary has the skill
* @param md Mercenary
* @param skill_id The skill
* @return Skill Level or 0 if Mercenary doesn't have the skill
**/
uint16 mercenary_checkskill(struct mercenary_data *md, uint16 skill_id) {
	if (!md || !md->db || md->db->skill.count(skill_id) == 0)
		return 0;
	return md->db->skill[skill_id];
}

const std::string MercenaryDatabase::getDefaultLocation() {
	return std::string(db_path) + "/mercenary_db.yml";
}

/*
 * Reads and parses an entry from the mercenary_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 MercenaryDatabase::parseBodyNode(const YAML::Node &node) {
	uint32 id;

	if (!this->asUInt32(node, "Id", id))
		return 0;

	std::shared_ptr<s_mercenary_db> mercenary = this->find(id);
	bool exists = mercenary != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "AegisName", "Name" }))
			return 0;

		mercenary = std::make_shared<s_mercenary_db>();
		mercenary->class_ = id;
	}

	if (this->nodeExists(node, "AegisName")) {
		std::string name;

		if (!this->asString(node, "AegisName", name))
			return 0;

		if (name.size() > NAME_LENGTH) {
			this->invalidWarning(node["AegisName"], "%s's size exceeds %d, skipping.\n", name.c_str(), NAME_LENGTH);
			return 0;
		}

		safestrncpy(mercenary->sprite, name.c_str(), sizeof(mercenary->sprite));
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		safestrncpy(mercenary->name, name.c_str(), sizeof(mercenary->name));
	}

	if (this->nodeExists(node, "Level")) {
		uint16 level;

		if (!this->asUInt16(node, "Level", level))
			return 0;

		mercenary->lv = level;
	} else {
		if (!exists)
			mercenary->lv = 1;
	}

	if (this->nodeExists(node, "Hp")) {
		uint32 hp;

		if (!this->asUInt32(node, "Hp", hp))
			return 0;

		mercenary->status.max_hp = hp;
	} else {
		if (!exists)
			mercenary->status.max_hp = 1;
	}
	
	if (this->nodeExists(node, "Sp")) {
		uint32 sp;

		if (!this->asUInt32(node, "Sp", sp))
			return 0;

		mercenary->status.max_sp = sp;
	} else {
		if (!exists)
			mercenary->status.max_sp = 1;
	}

	if (this->nodeExists(node, "Attack")) {
		uint16 atk;

		if (!this->asUInt16(node, "Attack", atk))
			return 0;

		mercenary->status.rhw.atk = atk;
	} else {
		if (!exists)
			mercenary->status.rhw.atk = 0;
	}
	
	if (this->nodeExists(node, "Attack2")) {
		uint16 atk;

		if (!this->asUInt16(node, "Attack2", atk))
			return 0;

		mercenary->status.rhw.atk2 = mercenary->status.rhw.atk + atk;
	} else {
		if (!exists)
			mercenary->status.rhw.atk2 = mercenary->status.rhw.atk;
	}

	if (this->nodeExists(node, "Defense")) {
		uint16 def;

		if (!this->asUInt16(node, "Defense", def))
			return 0;

		if (def < DEFTYPE_MIN || def > DEFTYPE_MAX) {
			this->invalidWarning(node["Defense"], "Invalid mercenary defense %d, capping...\n", def);
			def = cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX);
		}

		mercenary->status.def = static_cast<defType>(def);
	} else {
		if (!exists)
			mercenary->status.def = 0;
	}
	
	if (this->nodeExists(node, "MagicDefense")) {
		uint16 def;

		if (!this->asUInt16(node, "MagicDefense", def))
			return 0;

		if (def < DEFTYPE_MIN || def > DEFTYPE_MAX) {
			this->invalidWarning(node["MagicDefense"], "Invalid mercenary magic defense %d, capping...\n", def);
			def = cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX);
		}

		mercenary->status.mdef = static_cast<defType>(def);
	} else {
		if (!exists)
			mercenary->status.mdef = 0;
	}

	if (this->nodeExists(node, "Str")) {
		uint16 stat;

		if (!this->asUInt16(node, "Str", stat))
			return 0;

		mercenary->status.str = stat;
	} else {
		if (!exists)
			mercenary->status.str = 1;
	}

	if (this->nodeExists(node, "Agi")) {
		uint16 stat;

		if (!this->asUInt16(node, "Agi", stat))
			return 0;

		mercenary->status.agi = stat;
	} else {
		if (!exists)
			mercenary->status.agi = 1;
	}

	if (this->nodeExists(node, "Vit")) {
		uint16 stat;

		if (!this->asUInt16(node, "Vit", stat))
			return 0;

		mercenary->status.vit = stat;
	} else {
		if (!exists)
			mercenary->status.vit = 1;
	}

	if (this->nodeExists(node, "Int")) {
		uint16 stat;

		if (!this->asUInt16(node, "Int", stat))
			return 0;

		mercenary->status.int_ = stat;
	} else {
		if (!exists)
			mercenary->status.int_ = 1;
	}

	if (this->nodeExists(node, "Dex")) {
		uint16 stat;

		if (!this->asUInt16(node, "Dex", stat))
			return 0;

		mercenary->status.dex = stat;
	} else {
		if (!exists)
			mercenary->status.dex = 1;
	}

	if (this->nodeExists(node, "Luk")) {
		uint16 stat;

		if (!this->asUInt16(node, "Luk", stat))
			return 0;

		mercenary->status.luk = stat;
	} else {
		if (!exists)
			mercenary->status.luk = 1;
	}

	if (this->nodeExists(node, "AttackRange")) {
		uint16 range;

		if (!this->asUInt16(node, "AttackRange", range))
			return 0;

		mercenary->status.rhw.range = range;
	} else {
		if (!exists)
			mercenary->status.rhw.range = 0;
	}
	
	if (this->nodeExists(node, "SkillRange")) {
		uint16 range;

		if (!this->asUInt16(node, "SkillRange", range))
			return 0;

		mercenary->range2 = range;
	} else {
		if (!exists)
			mercenary->range2 = 0;
	}
	
	if (this->nodeExists(node, "ChaseRange")) {
		uint16 range;

		if (!this->asUInt16(node, "ChaseRange", range))
			return 0;

		mercenary->range3 = range;
	} else {
		if (!exists)
			mercenary->range3 = 0;
	}

	if (this->nodeExists(node, "Size")) {
		std::string size;

		if (!this->asString(node, "Size", size))
			return 0;

		std::string size_constant = "Size_" + size;
		int64 constant;

		if (!script_get_constant(size_constant.c_str(), &constant)) {
			this->invalidWarning(node["Size"], "Unknown mercenary size %s, defaulting to Size_Small.\n", size.c_str());
			constant = SZ_SMALL;
		}

		if (constant < SZ_SMALL || constant > SZ_BIG) {
			this->invalidWarning(node["Size"], "Invalid mercenary size %s, defaulting to Size_Small.\n", size.c_str());
			constant = SZ_SMALL;
		}

		mercenary->status.size = static_cast<e_size>(constant);
	} else {
		if (!exists)
			mercenary->status.size = SZ_SMALL;
	}
	
	if (this->nodeExists(node, "Race")) {
		std::string race;

		if (!this->asString(node, "Race", race))
			return 0;

		std::string race_constant = "RC_" + race;
		int64 constant;

		if (!script_get_constant(race_constant.c_str(), &constant)) {
			this->invalidWarning(node["Race"], "Unknown mercenary race %s, defaulting to RC_FORMLESS.\n", race.c_str());
			constant = RC_FORMLESS;
		}

		if (!CHK_RACE(constant)) {
			this->invalidWarning(node["Race"], "Invalid mercenary race %s, defaulting to RC_FORMLESS.\n", race.c_str());
			constant = RC_FORMLESS;
		}

		mercenary->status.race = static_cast<e_race>(constant);
	} else {
		if (!exists)
			mercenary->status.race = RC_FORMLESS;
	}

	if (this->nodeExists(node, "Element")) {
		std::string ele;

		if (!this->asString(node, "Element", ele))
			return 0;

		std::string ele_constant = "ELE_" + ele;
		int64 constant;

		if (!script_get_constant(ele_constant.c_str(), &constant)) {
			this->invalidWarning(node["Element"], "Unknown mercenary element %s, defaulting to ELE_NEUTRAL.\n", ele.c_str());
			constant = ELE_NEUTRAL;
		}

		if (!CHK_ELEMENT(constant)) {
			this->invalidWarning(node["Element"], "Invalid mercenary element %s, defaulting to ELE_NEUTRAL.\n", ele.c_str());
			constant = ELE_NEUTRAL;
		}

		mercenary->status.def_ele = static_cast<e_element>(constant);
	} else {
		if (!exists)
			mercenary->status.def_ele = ELE_NEUTRAL;
	}

	if (this->nodeExists(node, "ElementLevel")) {
		uint16 level;

		if (!this->asUInt16(node, "ElementLevel", level))
			return 0;

		if (!CHK_ELEMENT_LEVEL(level)) {
			this->invalidWarning(node["ElementLevel"], "Invalid mercenary element level %hu, defaulting to 1.\n", level);
			level = 1;
		}

		mercenary->status.ele_lv = static_cast<uint8>(level);
	} else {
		if (!exists)
			mercenary->status.ele_lv = 1;
	}

	if (this->nodeExists(node, "WalkSpeed")) {
		uint16 speed;

		if (!this->asUInt16(node, "WalkSpeed", speed))
			return 0;

		if (speed < MIN_WALK_SPEED || speed > MAX_WALK_SPEED) {
			this->invalidWarning(node["WalkSpeed"], "Invalid mercenary walk speed %hu, capping...\n", speed);
			speed = cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);
		}

		mercenary->status.speed = speed;
	} else {
		if (!exists)
			mercenary->status.speed = DEFAULT_WALK_SPEED;
	}

	if (this->nodeExists(node, "AttackDelay")) {
		uint16 speed;

		if (!this->asUInt16(node, "AttackDelay", speed))
			return 0;

		mercenary->status.adelay = cap_value(speed, battle_config.monster_max_aspd * 2, 4000);
	} else {
		if (!exists)
			mercenary->status.adelay = cap_value(0, battle_config.monster_max_aspd * 2, 4000);
	}
	
	if (this->nodeExists(node, "AttackMotion")) {
		uint16 speed;

		if (!this->asUInt16(node, "AttackMotion", speed))
			return 0;

		mercenary->status.amotion = cap_value(speed, battle_config.monster_max_aspd, 2000);
	} else {
		if (!exists)
			mercenary->status.amotion = cap_value(0, battle_config.monster_max_aspd, 2000);
	}

	if (this->nodeExists(node, "DamageMotion")) {
		uint16 speed;

		if (!this->asUInt16(node, "DamageMotion", speed))
			return 0;

		if (battle_config.monster_damage_delay_rate != 100)
			speed = speed * battle_config.monster_damage_delay_rate / 100;

		mercenary->status.dmotion = speed;
	} else {
		if (!exists)
			mercenary->status.dmotion = 0;
	}

	if (this->nodeExists(node, "Skills")) {
		const YAML::Node &skillsNode = node["Skills"];

		for (const YAML::Node &skill : skillsNode) {
			std::string skill_name;

			if (!this->asString(skill, "Name", skill_name))
				return 0;

			uint16 skill_id = skill_name2id(skill_name.c_str());

			if (skill_id == 0) {
				this->invalidWarning(skill["Name"], "Invalid skill %s, skipping.\n", skill_name.c_str());
				return 0;
			}

			if (!SKILL_CHK_MERC(skill_id)) {
				this->invalidWarning(skill["Name"], "Skill %s (%u) is out of the mercenary skill range [%u-%u], skipping.\n", skill_name.c_str(), skill_id, MC_SKILLBASE, MC_SKILLBASE + MAX_MERCSKILL - 1);
				return 0;
			}

			uint16 level;

			if (!this->asUInt16(skill, "MaxLevel", level))
				return 0;

			mercenary->skill[skill_id] = level;
		}
	}

	if (!exists)
		this->put(id, mercenary);

	return true;
}

// copied from mob.cpp, required ?
void MercenaryDatabase::loadingFinished() {
	for (auto &data : *this) {
		std::shared_ptr<s_mercenary_db> mercenary = data.second;

		if (battle_config.view_range_rate != 100)
			mercenary->range2 = cap_value(mercenary->range2, 1, mercenary->range2 * battle_config.view_range_rate / 100);
		if (battle_config.chase_range_rate != 100)
			mercenary->range3 = cap_value(mercenary->range3, mercenary->range2, mercenary->range3 * battle_config.chase_range_rate / 100);

		mercenary->range3 += 2;
		mercenary->status.adelay = max(mercenary->status.adelay, mercenary->status.amotion);
		mercenary->status.aspd_rate = 1000;
	}
}

/**
* Init Mercenary datas
**/
void do_init_mercenary(void){
	mercenary_db.load();

	//add_timer_func_list(mercenary_contract, "mercenary_contract");
}

/**
* Do Final Mercenary datas
**/
void do_final_mercenary(void){
	mercenary_db.clear();
}
