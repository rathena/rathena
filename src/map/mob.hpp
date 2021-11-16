// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MOB_HPP
#define MOB_HPP

#include <vector>

#include "../common/database.hpp"
#include "../common/mmo.hpp" // struct item
#include "../common/timer.hpp"

#include "status.hpp" // struct status data, struct status_change
#include "unit.hpp" // unit_stop_walking(), unit_stop_attack()

struct guardian_data;

//This is the distance at which @autoloot works,
//if the item drops farther from the player than this,
//it will not be autolooted. [Skotlex]
//Note: The range is unlimited unless this define is set.
//#define AUTOLOOT_DISTANCE AREA_SIZE

//The number of drops all mobs have and the max drop-slot that the steal skill will attempt to steal from.
#define MAX_MOB_DROP 10
#define MAX_MVP_DROP 3
#define MAX_MOB_DROP_ADD 5
#define MAX_MVP_DROP_ADD 2
#define MAX_MOB_DROP_TOTAL (MAX_MOB_DROP+MAX_MOB_DROP_ADD)
#define MAX_MVP_DROP_TOTAL (MAX_MVP_DROP+MAX_MVP_DROP_ADD)

//Min time between AI executions
const t_tick MIN_MOBTHINKTIME = 100;
//Min time before mobs do a check to call nearby friends for help (or for slaves to support their master)
const t_tick MIN_MOBLINKTIME = 1000;
//Min time between random walks
const t_tick MIN_RANDOMWALKTIME = 4000;

//Distance that slaves should keep from their master.
#define MOB_SLAVEDISTANCE 2

//Used to determine default enemy type of mobs (for use in eachinrange calls)
#define DEFAULT_ENEMY_TYPE(md) (md->special_state.ai?BL_CHAR:BL_MOB|BL_PC|BL_HOM|BL_MER)

/**
 * Mob constants
 * Added definitions for WoE:SE objects and other [L0ne_W0lf], [aleos]
 */
enum MOBID {
	MOBID_PORING			= 1002,
	MOBID_RED_PLANT			= 1078,
	MOBID_BLUE_PLANT,
	MOBID_GREEN_PLANT,
	MOBID_YELLOW_PLANT,
	MOBID_WHITE_PLANT,
	MOBID_SHINING_PLANT,
	MOBID_BLACK_MUSHROOM	= 1084,
	MOBID_MARINE_SPHERE		= 1142,
	MOBID_EMPERIUM			= 1288,
	MOBID_G_PARASITE		= 1555,
	MOBID_G_FLORA			= 1575,
	MOBID_G_HYDRA			= 1579,
	MOBID_G_MANDRAGORA		= 1589,
	MOBID_G_GEOGRAPHER		= 1590,
	MOBID_GUARDIAN_STONE1	= 1907,
	MOBID_GUARDIAN_STONE2,
	MOBID_SILVERSNIPER		= 2042,
	MOBID_MAGICDECOY_FIRE,
	MOBID_MAGICDECOY_WATER,
	MOBID_MAGICDECOY_EARTH,
	MOBID_MAGICDECOY_WIND,
	MOBID_ZANZOU			= 2308,
	MOBID_S_HORNET			= 2158,
	MOBID_S_GIANT_HORNET,
	MOBID_S_LUCIOLA_VESPA,
	MOBID_GUILD_SKILL_FLAG	= 20269,
	MOBID_ABR_BATTLE_WARIOR = 20834,
	MOBID_ABR_DUAL_CANNON,
	MOBID_ABR_MOTHER_NET,
	MOBID_ABR_INFINITY,
	MOBID_BIONIC_WOODENWARRIOR = 20848,
	MOBID_BIONIC_WOODEN_FAIRY,
	MOBID_BIONIC_CREEPER,
	MOBID_BIONIC_HELLTREE,
};

///Mob skill states.
enum MobSkillState {
	MSS_ANY = -1,
	MSS_IDLE,
	MSS_WALK,
	MSS_LOOT,
	MSS_DEAD,
	MSS_BERSERK, //Aggressive mob attacking
	MSS_ANGRY,   //Mob retaliating from being attacked.
	MSS_RUSH,    //Mob following a player after being attacked.
	MSS_FOLLOW,  //Mob following a player without being attacked.
	MSS_ANYTARGET,
};

enum MobDamageLogFlag
{
	MDLF_NORMAL = 0,
	MDLF_HOMUN,
	MDLF_PET,
	MDLF_SELF
};

enum e_size : uint8 {
	SZ_SMALL = 0,
	SZ_MEDIUM,
	SZ_BIG,
	SZ_ALL,
	SZ_MAX
};

/// Random Monster Groups
enum e_random_monster : uint16 {
	MOBG_BRANCH_OF_DEAD_TREE = 0,
	MOBG_PORING_BOX,
	MOBG_BLOODY_DEAD_BRANCH,
	MOBG_RED_POUCH_OF_SURPRISE,
	MOBG_CLASSCHANGE,
	MOBG_TAEKWON_MISSION,
	MOBG_MAX,
};

/// Random Monster Group Flags
enum e_random_monster_flags {
	RMF_NONE			= 0x00, ///< Apply no flags
	RMF_DB_RATE			= 0x01, ///< Apply the summon success chance found in the list (otherwise get any monster from the db)
	RMF_CHECK_MOB_LV	= 0x02, ///< Apply a monster level check
	RMF_MOB_NOT_BOSS	= 0x04, ///< Selected monster should not be a Boss type (except those from MOBG_BLOODY_DEAD_BRANCH)
	RMF_MOB_NOT_SPAWN	= 0x08, ///< Selected monster must have normal spawn
	RMF_MOB_NOT_PLANT	= 0x10, ///< Selected monster should not be a Plant type
	RMF_ALL				= 0xFF, ///< Apply all flags
};

enum e_mob_bosstype : uint8{
	BOSSTYPE_NONE,
	BOSSTYPE_MINIBOSS,
	BOSSTYPE_MVP
};

/// Monster Aegis AI types
enum e_aegis_monstertype : uint16 {
	MONSTER_TYPE_01 = 0x81,
	MONSTER_TYPE_02 = 0x83,
	MONSTER_TYPE_03 = 0x1089,
	MONSTER_TYPE_04 = 0x3885,
	MONSTER_TYPE_05 = 0x2085,
	MONSTER_TYPE_06 = 0,
	MONSTER_TYPE_07 = 0x108B,
	MONSTER_TYPE_08 = 0x7085,
	MONSTER_TYPE_09 = 0x3095,
	MONSTER_TYPE_10 = 0x84,
	MONSTER_TYPE_11 = 0x84,
	MONSTER_TYPE_12 = 0x2085,
	MONSTER_TYPE_13 = 0x308D,
	//MONSTER_TYPE_14
	//MONSTER_TYPE_15
	//MONSTER_TYPE_16
	MONSTER_TYPE_17 = 0x91,
	//MONSTER_TYPE_18
	MONSTER_TYPE_19 = 0x3095,
	MONSTER_TYPE_20 = 0x3295,
	MONSTER_TYPE_21 = 0x3695,
	//MONSTER_TYPE_22
	//MONSTER_TYPE_23
	MONSTER_TYPE_24 = 0xA1,
	MONSTER_TYPE_25 = 0x1,
	MONSTER_TYPE_26 = 0xB695,
	MONSTER_TYPE_27 = 0x8084,
};

/// Aegis monster class types
enum e_aegis_monsterclass : int8 {
	CLASS_NONE = -1,
	CLASS_NORMAL = 0,
	CLASS_BOSS,
	CLASS_GUARDIAN,
	CLASS_BATTLEFIELD = 4,
	CLASS_EVENT,
	CLASS_ALL,
	CLASS_MAX,
};

struct s_mob_skill {
	enum MobSkillState state;
	uint16 skill_id,skill_lv;
	short permillage;
	int casttime,delay;
	short cancel;
	short cond1,cond2;
	short target;
	int val[5];
	short emotion;
	unsigned short msg_id;
};

struct s_mob_chat {
	uint16 msg_id;
	uint32 color;
	std::string msg;
};

class MobChatDatabase : public TypesafeYamlDatabase<uint16, s_mob_chat> {
public:
	MobChatDatabase() : TypesafeYamlDatabase("MOB_CHAT_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node &node);
};

struct s_mob_item_drop_ratio {
	t_itemid nameid;
	uint16 drop_ratio;
	std::vector<uint16> mob_ids;
};

class MobItemRatioDatabase : public TypesafeYamlDatabase<t_itemid, s_mob_item_drop_ratio> {
public:
	MobItemRatioDatabase() : TypesafeYamlDatabase("MOB_ITEM_RATIO_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node &node);
};

struct spawn_info {
	unsigned short mapindex;
	unsigned short qty;
};

/// Loooitem struct
struct s_mob_lootitem {
	struct item item;	   ///< Item info
	unsigned short mob_id; ///< ID of monster that dropped the item
};

/// Struct for monster's drop item
struct s_mob_drop {
	t_itemid nameid;
	uint32 rate;
	uint16 randomopt_group;
	bool steal_protected;
};

struct s_mob_db {
	uint32 id;
	std::string sprite, name, jname;
	t_exp base_exp;
	t_exp job_exp;
	t_exp mexp;
	uint16 range2, range3;
	std::vector<e_race2> race2;	// celest
	uint16 lv;
	s_mob_drop dropitem[MAX_MOB_DROP_TOTAL], mvpitem[MAX_MVP_DROP_TOTAL];
	status_data status;
	view_data vd;
	uint32 option;
	std::vector<std::shared_ptr<s_mob_skill>> skill;
	uint16 damagetaken;

	e_mob_bosstype get_bosstype();
};

class MobDatabase : public TypesafeCachedYamlDatabase <uint32, s_mob_db> {
private:
	bool parseDropNode(std::string nodeName, YAML::Node node, uint8 max, s_mob_drop *drops);

public:
	MobDatabase() : TypesafeCachedYamlDatabase("MOB_DB", 2, 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node &node);
	void loadingFinished();
};

extern MobDatabase mob_db;

struct mob_data {
	struct block_list bl;
	struct unit_data  ud;
	struct view_data *vd;
	bool vd_changed;
	struct status_data status, *base_status; //Second one is in case of leveling up mobs, or tiny/large mobs.
	struct status_change sc;
	std::shared_ptr<s_mob_db> db;	//For quick data access (saves doing mob_db(md->mob_id) all the time) [Skotlex]
	char name[NAME_LENGTH];
	struct s_specialState {
		unsigned int size : 2; //Small/Big monsters.
		enum mob_ai ai; //Special ai for summoned monsters.
		unsigned int clone : 1;/* is clone? 1:0 */
	} special_state; //Special mob information that does not needs to be zero'ed on mob respawn.
	struct s_MobState {
		unsigned int aggressive : 1; //Signals whether the mob AI is in aggressive mode or reactive mode. [Skotlex]
		unsigned int steal_coin_flag : 1;
		unsigned int soul_change_flag : 1; // Celest
		unsigned int alchemist: 1;
		unsigned int npc_killmonster: 1; //for new killmonster behavior
		unsigned int rebirth: 1; // NPC_Rebirth used
		unsigned int boss : 1;
		unsigned int copy_master_mode : 1; ///< Whether the spawned monster should copy the master's mode.
		enum MobSkillState skillstate;
		unsigned char steal_flag; //number of steal tries (to prevent steal exploit on mobs with few items) [Lupus]
		unsigned char attacked_count; //For rude attacked.
		int provoke_flag; // Celest
	} state;
	struct guardian_data* guardian_data;
	struct s_dmglog {
		int id; //char id
		unsigned int dmg;
		unsigned int flag : 2; //0: Normal. 1: Homunc exp. 2: Pet exp
	} dmglog[DAMAGELOG_SIZE];
	uint32 spotted_log[DAMAGELOG_SIZE];
	struct spawn_data *spawn; //Spawn data.
	int spawn_timer; //Required for Convex Mirror
	struct s_mob_lootitem *lootitems;
	short mob_id;
	unsigned int tdmg; //Stores total damage given to the mob, for exp calculations. [Skotlex]
	int level;
	int target_id,attacked_id,norm_attacked_id;
	int areanpc_id; //Required in OnTouchNPC (to avoid multiple area touchs)
	int bg_id; // BattleGround System

	t_tick next_walktime,last_thinktime,last_linktime,last_pcneartime,dmgtick;
	short move_fail_count;
	short lootitem_count;
	short min_chase;
	unsigned char walktoxy_fail_count; //Pathfinding succeeds but the actual walking failed (e.g. Icewall lock)

	int deletetimer;
	int master_id,master_dist;

	int8 skill_idx; // Index of last used skill from db->skill[]
	t_tick skilldelay[MAX_MOBSKILL];
	char npc_event[EVENT_NAME_LENGTH];
	char idle_event[EVENT_NAME_LENGTH];
	/**
	 * Did this monster summon something?
	 * Used to flag summon deletions, saves a worth amount of memory
	 **/
	bool can_summon;
	/**
	 * MvP Tombstone NPC ID
	 **/
	int tomb_nid;

	e_mob_bosstype get_bosstype();
};

class MobAvailDatabase : public YamlDatabase {
public:
	MobAvailDatabase() : YamlDatabase("MOB_AVAIL_DB", 1) {

	}

	void clear() { };
	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node& node);
};

struct s_randomsummon_entry {
	uint16 mob_id;
	uint32 rate;
};

struct s_randomsummon_group {
	uint16 random_id;
	uint16 default_mob_id;
	std::unordered_map<uint16, std::shared_ptr<s_randomsummon_entry>> list;
};

class MobSummonDatabase : public TypesafeYamlDatabase<uint16, s_randomsummon_group> {
public:
	MobSummonDatabase() : TypesafeYamlDatabase("MOB_SUMMONABLE_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node &node);
};

enum e_mob_skill_target {
	MST_TARGET	=	0,
	MST_RANDOM,	//Random Target!
	MST_SELF,
	MST_FRIEND,
	MST_MASTER,
	MST_AROUND5,
	MST_AROUND6,
	MST_AROUND7,
	MST_AROUND8,
	MST_AROUND1,
	MST_AROUND2,
	MST_AROUND3,
	MST_AROUND4,
	MST_AROUND	=	MST_AROUND4,
};

enum e_mob_skill_condition {
	MSC_ALWAYS	=	0x0000,
	MSC_MYHPLTMAXRATE,
	MSC_MYHPINRATE,
	MSC_FRIENDHPLTMAXRATE,
	MSC_FRIENDHPINRATE,
	MSC_MYSTATUSON,
	MSC_MYSTATUSOFF,
	MSC_FRIENDSTATUSON,
	MSC_FRIENDSTATUSOFF,
	MSC_ATTACKPCGT,
	MSC_ATTACKPCGE,
	MSC_SLAVELT,
	MSC_SLAVELE,
	MSC_CLOSEDATTACKED,
	MSC_LONGRANGEATTACKED,
	MSC_AFTERSKILL,
	MSC_SKILLUSED,
	MSC_CASTTARGETED,
	MSC_RUDEATTACKED,
	MSC_MASTERHPLTMAXRATE,
	MSC_MASTERATTACKED,
	MSC_ALCHEMIST,
	MSC_SPAWN,
};

// The data structures for storing delayed item drops
struct item_drop {
	struct item item_data;
	unsigned short mob_id;
	enum bl_type src_type;
	struct item_drop* next;
};
struct item_drop_list {
	int16 m, x, y;                       // coordinates
	int first_charid, second_charid, third_charid; // charid's of players with higher pickup priority
	struct item_drop* item;            // linked list of drops
};

uint16 mobdb_searchname(const char * const str);
std::shared_ptr<s_mob_db> mobdb_search_aegisname( const char* str );
int mobdb_searchname_array(const char *str, uint16 * out, int size);
int mobdb_checkid(const int id);
struct view_data* mob_get_viewdata(int mob_id);
void mob_set_dynamic_viewdata( struct mob_data* md );
void mob_free_dynamic_viewdata( struct mob_data* md );

struct mob_data *mob_once_spawn_sub(struct block_list *bl, int16 m, int16 x, int16 y, const char *mobname, int mob_id, const char *event, unsigned int size, enum mob_ai ai);

int mob_once_spawn(struct map_session_data* sd, int16 m, int16 x, int16 y,
	const char* mobname, int mob_id, int amount, const char* event, unsigned int size, enum mob_ai ai);

int mob_once_spawn_area(struct map_session_data* sd, int16 m,
	int16 x0, int16 y0, int16 x1, int16 y1, const char* mobname, int mob_id, int amount, const char* event, unsigned int size, enum mob_ai ai);

bool mob_ksprotected (struct block_list *src, struct block_list *target);

int mob_spawn_guardian(const char* mapname, int16 x, int16 y, const char* mobname, int mob_id, const char* event, int guardian, bool has_index);	// Spawning Guardians [Valaris]
int mob_spawn_bg(const char* mapname, int16 x, int16 y, const char* mobname, int mob_id, const char* event, unsigned int bg_id);
int mob_guardian_guildchange(struct mob_data *md); //Change Guardian's ownership. [Skotlex]

int mob_randomwalk(struct mob_data *md,t_tick tick);
int mob_warpchase(struct mob_data *md, struct block_list *target);
int mob_target(struct mob_data *md,struct block_list *bl,int dist);
int mob_unlocktarget(struct mob_data *md, t_tick tick);
struct mob_data* mob_spawn_dataset(struct spawn_data *data);
int mob_spawn(struct mob_data *md);
TIMER_FUNC(mob_delayspawn);
int mob_setdelayspawn(struct mob_data *md);
int mob_parse_dataset(struct spawn_data *data);
void mob_log_damage(struct mob_data *md, struct block_list *src, int damage);
void mob_damage(struct mob_data *md, struct block_list *src, int damage);
int mob_dead(struct mob_data *md, struct block_list *src, int type);
void mob_revive(struct mob_data *md, unsigned int hp);
void mob_heal(struct mob_data *md,unsigned int heal);

#define mob_stop_walking(md, type) unit_stop_walking(&(md)->bl, type)
#define mob_stop_attack(md) unit_stop_attack(&(md)->bl)

void mob_clear_spawninfo();
void do_init_mob(void);
void do_final_mob(bool is_reload);

TIMER_FUNC(mob_timer_delete);
int mob_deleteslave(struct mob_data *md);

int mob_random_class (int *value, size_t count);
int mob_get_random_id(int type, enum e_random_monster_flags flag, int lv);
int mob_class_change(struct mob_data *md,int mob_id);
int mob_warpslave(struct block_list *bl, int range);
int mob_linksearch(struct block_list *bl,va_list ap);

bool mob_chat_display_message (mob_data &md, uint16 msg_id);
int mobskill_use(struct mob_data *md,t_tick tick,int event);
int mobskill_event(struct mob_data *md,struct block_list *src,t_tick tick, int flag);
int mob_summonslave(struct mob_data *md2,int *value,int amount,uint16 skill_id);
int mob_countslave(struct block_list *bl);
int mob_count_sub(struct block_list *bl, va_list ap);

int mob_is_clone(int mob_id);

int mob_clone_spawn(struct map_session_data *sd, int16 m, int16 x, int16 y, const char *event, int master_id, enum e_mode mode, int flag, unsigned int duration);
int mob_clone_delete(struct mob_data *md);

void mob_reload_itemmob_data(void);
void mob_reload(void);
void mob_add_spawn(uint16 mob_id, const struct spawn_info& new_spawn);
const std::vector<spawn_info> mob_get_spawns(uint16 mob_id);
bool mob_has_spawn(uint16 mob_id);

int mob_getdroprate(struct block_list *src, std::shared_ptr<s_mob_db> mob, int base_rate, int drop_modifier);

// MvP Tomb System
int mvptomb_setdelayspawn(struct npc_data *nd);
TIMER_FUNC(mvptomb_delayspawn);
void mvptomb_create(struct mob_data *md, char *killer, time_t time);
void mvptomb_destroy(struct mob_data *md);

void mob_setdropitem_option(struct item *itm, struct s_mob_drop *mobdrop);

#define CHK_MOBSIZE(size) ((size) >= SZ_SMALL && (size) < SZ_MAX) /// Check valid Monster Size

#endif /* MOB_HPP */
