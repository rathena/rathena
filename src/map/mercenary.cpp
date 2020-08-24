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
//eduardo
#include "mob.hpp"
#include "battle.hpp"
#include "map.hpp"

using namespace rathena;

std::map<uint16, struct s_mercenary_db> mercenary_db_data;
//eduardo
std::map<uint16, struct mob_db> partner_db_data;

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

//eduardo
static bool partner_readdb_sub(char* str[], int columns, int current)
{	//straight from mob_parse_dbrow
	uint16 class_ = atoi(str[0]);
	struct mob_db *db, entry;
	int mob_id;// , i;
	struct status_data *status;
	struct mob_data data;
	//std::string nimi;
	//int ele;
	//std::string mob_name;
	//memset(&entry, 0, sizeof(entry));
	// status = &entry->status;
	// ShowMessage("\n aaaaa %s \n", str[1]);

	mob_id = class_;// atoi(str[0]);
	db = &partner_db_data[mob_id];
	db->vd.class_ = mob_id;
	db->id_ = mob_id;
	db->lv = atoi(str[2]);
	safestrncpy(db->name, str[1], NAME_LENGTH);
	status = &db->status;
	status->max_hp = atoi(str[3]);
	status->max_sp = atoi(str[4]);
	status->rhw.range = atoi(str[5]);
	status->rhw.atk = atoi(str[6]);
	status->rhw.atk2 = status->rhw.atk + atoi(str[7]);
	status->def = atoi(str[8]);
	status->mdef = atoi(str[9]);
	status->str = atoi(str[10]);
	status->agi = atoi(str[11]);
	status->vit = atoi(str[12]);
	status->int_ = atoi(str[13]);
	status->dex = atoi(str[14]);
	status->luk = atoi(str[15]);
	db->range2 = atoi(str[16]);
	db->range3 = atoi(str[17]);
	status->size = atoi(str[18]);
	status->race = atoi(str[19]);
	// ele = 12;//atoi(str[20]);
			 //status->def_ele = ele%20;
			 //status->ele_lv = (unsigned char)floor(ele/20.);
			 //db->class_
	status->mode = static_cast<enum e_mode>(strtol(str[21], NULL, 0));
	status->aspd_rate = atoi(str[22]);
	status->speed = atoi(str[23]);
	status->adelay = atoi(str[24]);
	status->amotion = atoi(str[25]);
	status->dmotion = atoi(str[26]);
	db->truest = atoi(str[27]);
	db->petname = atoi(str[29]);
	db->attProb = atoi(str[30]);

	memcpy(&db->status, status, sizeof(struct status_data));

	db = &partner_db_data[mob_id];
	//db = mob_db(mob_id);
	if (db == NULL) {
		try{
			db = &partner_db_data[mob_id];
		}catch( const std::bad_alloc& ){
			ShowError( "Memory allocation for monster %hu failed.\n", mob_id );
			return false;
		}
	}

	//memcpy(db, &entry, sizeof(struct mob_db));
	return true;
}

//eduardo
bool partner_create(struct map_session_data *sd, uint16 class_, unsigned int lifetime) 
{	//mercenary_create + clif_scriptinputstr ---> mob_clone_spawn2
	//int mob_id;
	//struct mob_db *db;
	//struct mob_data *md;
	int x, y, flag, i=0;
	struct map_session_data *pl_sd=NULL;

	// sd = map_id2sd(sd->bl.id);
	
	memset(&sd->npc_id, 0, sizeof(sd->npc_id));
	
	if (strcmp(sd->partnerSelected,"")!=0){
		std::string charName;
		charName = std::string(sd->partnerSelected);
		std::string full;
		full = std::string("@clonea ");
		full.append(charName);
		// is_atcommand(sd->fd, sd, full.c_str(), 1);
		// clif_scloneequest(target_sd, sd->status.name);
		//clif_traderequest(map_nick2sd(charName.c_str(), true), sd->status.name);
		// strcpy(sd->partnerSelected2, sd->partnerSelected);
		//sd->partnerSelected_ = sd->partnerSelected;
		pl_sd = map_nick2sd(charName.c_str(),true);
		if (!pl_sd || pl_sd == NULL) {
			clif_displaymessage(sd->fd, "no active player with this nickname");	
			memset(&sd->partnerSelected, 0, sizeof(sd->partnerSelected));
			return false;
		}
		if(pc_get_group_level(pl_sd) > pc_get_group_level(sd)) {
			clif_displaymessage(sd->fd, msg_txt(sd,126));	// Cannot clone a player of higher GM level than yourself.
			return false;
		}
		//parent_cmd = atcommand_alias_db.checkAlias(command+1);
		flag = 2;		
		if(pc_isdead(sd)){
		    clif_displaymessage(sd->fd, msg_txt(sd,129+flag*2));
		    return false;
		}
		if (mob_countslave(&sd->bl) >= battle_config.atc_slave_clone_limit){
			clif_displaymessage(sd->fd, "too many slaves. Liberate them.");	
			memset(&sd->partnerSelected, 0, sizeof(sd->partnerSelected));
			return false;
		}

		do {
			x = sd->bl.x + (rnd() % 10 - 5);
			y = sd->bl.y + (rnd() % 10 - 5);
		} while (map_getcell(sd->bl.m,x,y,CELL_CHKNOPASS) && i++ < 10);

		if (i >= 10) {
			x = sd->bl.x;
			y = sd->bl.y;
		}

		if( 
		mob_clone_spawn2(
			map_nick2sd(charName.c_str(), true),
			sd->bl.m, 
			sd->bl.x,
			sd->bl.y,
			"", 
			sd->bl.id,
			MD_NONE, 
			1, 
			0, 
			"default", 
			map_nick2sd(sd->partnerSelected,true)->status.char_id
		)) {
			clif_displaymessage(sd->fd, "an sclone spawned");	
			return false;		}

		memset(&sd->partnerSelected, 0, sizeof(sd->partnerSelected));
	} else {
		clif_scriptinputstr(sd, 0);
	}

	return true;
}

//eduardo
int counter22 = 0;

//eduardo
int merskill_use(struct mercenary_data *md, t_tick tick, int event, int tid)
{
	struct block_list *tbl;
	uint16 skill_id;
	uint8 skill_lv;
	int casttime, inf;

	if (md->ud.skilltimer != INVALID_TIMER) { return 0; }
	if (event == -1 && DIFF_TICK(md->ud.canact_tick, tick) > 0)
		return 0;

	// unsigned int intimacy = 0;
	// short idx = -1;

	int j = rnd() % MAX_MERCSKILL;

	if (!md) return -1;

	if (md->db->skill[j].id>0){
		skill_id = md->db->skill[j].id;

		skill_lv = md->db->skill[j].lv;
		casttime = skill_castfix(&md->bl, skill_id, skill_lv);

		inf = skill_get_inf(skill_id);
		if (inf&INF_ATTACK_SKILL) {
			map_freeblock_lock();
			tbl = map_id2bl(tid);	if (!tbl) { return -1; }
			if (skill_id != 0){
				unit_skilluse_id2(&md->bl, tid, skill_id, skill_lv, casttime, false);
			}
			map_freeblock_unlock();
		} else if(inf&INF_GROUND_SKILL) {
			tbl = map_id2bl(tid);
			map_freeblock_lock();
			if (skill_id != 0){
				if (
					!battle_check_range(&md->bl, tbl, skill_get_range2(&md->bl, skill_id, skill_lv, true))
					|| !unit_skilluse_pos2(&md->bl, tbl->x+rnd_value(-2,2), tbl->y+rnd_value(-2,2), skill_id, skill_lv, casttime, false)
				){
					map_freeblock_unlock();
					return -1;
				}
			}
			map_freeblock_unlock();
		} else if (inf&INF_SELF_SKILL) {
			tid = md->bl.id;
			map_freeblock_lock();
			tbl = map_id2bl(tid);	if (!tbl) { return -1; }
			if (skill_id != 0){
				unit_skilluse_id2(&md->bl, tid, skill_id, skill_lv, casttime, false);
			}
			map_freeblock_unlock();
		} else if (inf&INF_SUPPORT_SKILL) {
			if (md->master2){
				tid = md->master2->bl.id;
			} else if (md->master){
				tid = md->master->bl.id;
			}
			map_freeblock_lock();
			tbl = map_id2bl(tid);	if (!tbl) { return -1; }
			if (skill_id != 0){
				unit_skilluse_id2(&md->bl, tid, skill_id, skill_lv, casttime, false);	
			}
			map_freeblock_unlock();
		} else return -1;		

		return 1;
	} else {
		counter22++;
		if (counter22%5==0){
			return 0;
		} else merskill_use(md, gettick(), -1, tid);
	}
	return 0;
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

//eduardo
//unused because mercenary duplicate is kinda weak & wasteful; and we have pain split, so.
void do_init_partner(void){
	/* specifically for partners.txt */
	// partner_readdb();
	const char *filename[]={ "partners.txt",DBIMPORT"/partners.txt"};
	uint8 i;

	partner_db_data.clear();

	for(i = 0; i<ARRAYLENGTH(filename); i++){
		sv_readdb(".", filename[i], ',', 5, 57, -1, &partner_readdb_sub, i > 0);
	}
	// mercenary_read_skilldb();

	//add_timer_func_list(mercenary_contract, "mercenary_contract");
}

/**
* Do Final Mercenary datas
**/
void do_final_mercenary(void){
	mercenary_db_data.clear();
}
