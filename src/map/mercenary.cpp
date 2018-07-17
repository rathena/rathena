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

std::map<uint16, struct s_mercenary_db> mercenary_db_data;

/**
 * Search Mercenary by class
 * @param class_ Class ID of Mercenary
 * @return A pointer to the mercenary db entry or nullptr if not found
 **/
struct s_mercenary_db *mercenary_db( uint16 class_ ){
	return util::map_find( mercenary_db_data, class_ );
}

/**
* Get View Data of Mercenary by Class ID
* @param class_ The Class ID
* @return View Data of Mercenary
**/
struct view_data *mercenary_get_viewdata( uint16 class_ ){
	struct s_mercenary_db *db = mercenary_db(class_);

	if( db ){
		return &db->vd;
	}else{
		return nullptr;
	}
}

/**
 * Get mercenary skill index for mercenary skill tree
 * @param skill_id
 * @return Index in skill_tree or -1
 **/
short mercenary_skill_get_index(uint16 skill_id) {
	if (!SKILL_CHK_MERC(skill_id))
		return -1;
	skill_id -= MC_SKILLBASE;
	if (skill_id >= MAX_MERCSKILL)
		return -1;
	return skill_id;
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
	struct s_mercenary_db *db;
	nullpo_retr(false,sd);

	db = mercenary_db(class_);

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
int mercenary_get_lifetime(struct mercenary_data *md) {
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
	struct s_mercenary_db *db;

	db = mercenary_db(merc->class_);

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
int mercenary_checkskill(struct mercenary_data *md, uint16 skill_id) {
	short idx = mercenary_skill_get_index(skill_id);

	if( !md || !md->db || idx == -1)
		return 0;
	return md->db->skill[idx].lv;
}

/**
* Read each line of Mercenary's database
**/
static bool mercenary_readdb_sub(char* str[], int columns, int current)
{
	int ele;
	uint16 class_ = atoi(str[0]);
	struct s_mercenary_db *db;
	struct status_data *status;

	db = &mercenary_db_data[class_];

	db->class_ = class_;
	safestrncpy(db->sprite, str[1], NAME_LENGTH);
	safestrncpy(db->name, str[2], NAME_LENGTH);
	db->lv = atoi(str[3]);

	status = &db->status;
	db->vd.class_ = db->class_;

	status->max_hp = atoi(str[4]);
	status->max_sp = atoi(str[5]);
	status->rhw.range = atoi(str[6]);
	status->rhw.atk = atoi(str[7]);
	status->rhw.atk2 = status->rhw.atk + atoi(str[8]);
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
	if( !CHK_ELEMENT(status->def_ele) )
	{
		ShowWarning("Mercenary %d has invalid element type %d (max element is %d)\n", db->class_, status->def_ele, ELE_ALL - 1);
		status->def_ele = ELE_NEUTRAL;
	}
	if( !CHK_ELEMENT_LEVEL(status->ele_lv) )
	{
		ShowWarning("Mercenary %d has invalid element level %d (max is %d)\n", db->class_, status->ele_lv, MAX_ELE_LEVEL);
		status->ele_lv = 1;
	}

	status->aspd_rate = 1000;
	status->speed = atoi(str[22]);
	status->adelay = atoi(str[23]);
	status->amotion = atoi(str[24]);
	status->dmotion = atoi(str[25]);
	
	return true;
}

/**
* Load Mercenary's database
**/
void mercenary_readdb(void) {
	const char *filename[]={ "mercenary_db.txt",DBIMPORT"/mercenary_db.txt"};
	uint8 i;

	mercenary_db_data.clear();

	for(i = 0; i<ARRAYLENGTH(filename); i++){
		sv_readdb(db_path, filename[i], ',', 26, 26, -1, &mercenary_readdb_sub, i > 0);
	}
}

/**
* Read each line of Mercenary's skill
**/
static bool mercenary_read_skilldb_sub(char* str[], int columns, int current)
{// <merc id>,<skill id>,<skill level>
	struct s_mercenary_db *db;
	uint16 class_, skill_id, skill_lv;
	short idx = -1;

	class_ = atoi(str[0]);
	db = mercenary_db(class_);
	if( !db ){
		ShowError("read_mercenary_skilldb : Class %d not found in mercenary_db for skill entry.\n", class_);
		return false;
	}

	skill_id = atoi(str[1]);
	if( (idx = mercenary_skill_get_index(skill_id)) == -1 ) {
		ShowError("read_mercenary_skilldb: Invalid Mercenary skill '%s'.\n", str[1]);
		return false;
	}

	skill_lv = atoi(str[2]);

	db->skill[idx].id = skill_id;
	db->skill[idx].lv = skill_lv;

	return true;
}

/**
* Load Mercenary's skill database
**/
void mercenary_read_skilldb(void){
	const char *filename[]={ "mercenary_skill_db.txt",DBIMPORT"/mercenary_skill_db.txt"};
	uint8 i;

	for(i = 0; i<ARRAYLENGTH(filename); i++){
		sv_readdb(db_path, filename[i], ',', 3, 3, -1, &mercenary_read_skilldb_sub, i > 0);
	}
}

/**
* Init Mercenary datas
**/
void do_init_mercenary(void){
	mercenary_readdb();
	mercenary_read_skilldb();

	//add_timer_func_list(mercenary_contract, "mercenary_contract");
}

/**
* Do Final Mercenary datas
**/
void do_final_mercenary(void){
	mercenary_db_data.clear();
}
