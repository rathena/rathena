// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAP_H_
#define _MAP_H_

#include "../common/cbasetypes.h"
#include "../common/core.h" // CORE_ST_LAST
#include "../common/mmo.h"
#include "../common/mapindex.h"
#include "../common/db.h"

/**
 * [rAthena.org]
 **/
#include "./config/core.h"

#include <stdarg.h>

struct npc_data;
struct item_data;

enum E_MAPSERVER_ST
{
	MAPSERVER_ST_RUNNING = CORE_ST_LAST,
	MAPSERVER_ST_SHUTDOWN,
	MAPSERVER_ST_LAST
};


#define MAX_NPC_PER_MAP 512
#define AREA_SIZE battle_config.area_size
#define DAMAGELOG_SIZE 30
#define LOOTITEM_SIZE 10
#define MAX_MOBSKILL 50
#define MAX_MOB_LIST_PER_MAP 128
#define MAX_EVENTQUEUE 2
#define MAX_EVENTTIMER 32
#define NATURAL_HEAL_INTERVAL 500
#define MIN_FLOORITEM 2
#define MAX_FLOORITEM START_ACCOUNT_NUM
#define MAX_LEVEL 150
#define MAX_DROP_PER_MAP 48
#define MAX_IGNORE_LIST 20 // official is 14
#define MAX_VENDING 12
#define MAX_MAP_SIZE 512*512 // Wasn't there something like this already? Can't find it.. [Shinryo]
#define MOBID_EMPERIUM 1288
// Added definitions for WoESE objects. [L0ne_W0lf]
#define MOBID_BARRICADE1 1905
#define MOBID_BARRICADE2 1906
#define MOBID_GUARIDAN_STONE1 1907
#define MOBID_GUARIDAN_STONE2 1908

//The following system marks a different job ID system used by the map server,
//which makes a lot more sense than the normal one. [Skotlex]
//
//These marks the "level" of the job.
#define JOBL_2_1 0x100 //256
#define JOBL_2_2 0x200 //512
#define JOBL_2 0x300

#define JOBL_UPPER 0x1000 //4096
#define JOBL_BABY 0x2000  //8192
#define JOBL_THIRD 0x4000 //16384

//for filtering and quick checking.
#define MAPID_UPPERMASK 0x0fff
#define MAPID_BASEMASK 0x00ff
#define MAPID_THIRDMASK (JOBL_THIRD|MAPID_UPPERMASK)
//First Jobs
//Note the oddity of the novice:
//Super Novices are considered the 2-1 version of the novice! Novices are considered a first class type, too...
enum {
	MAPID_NOVICE = 0x0,
	MAPID_SWORDMAN,
	MAPID_MAGE,
	MAPID_ARCHER,
	MAPID_ACOLYTE,
	MAPID_MERCHANT,
	MAPID_THIEF,
	MAPID_TAEKWON,
	MAPID_WEDDING,
	MAPID_GUNSLINGER,
	MAPID_NINJA,
	MAPID_XMAS,
	MAPID_SUMMER,
//2_1 classes
	MAPID_SUPER_NOVICE = JOBL_2_1|0x0,
	MAPID_KNIGHT,
	MAPID_WIZARD,
	MAPID_HUNTER,
	MAPID_PRIEST,
	MAPID_BLACKSMITH,
	MAPID_ASSASSIN,
	MAPID_STAR_GLADIATOR,
//2_2 classes
	MAPID_CRUSADER = JOBL_2_2|0x1,
	MAPID_SAGE,
	MAPID_BARDDANCER,
	MAPID_MONK,
	MAPID_ALCHEMIST,
	MAPID_ROGUE,
	MAPID_SOUL_LINKER,
//1-1, advanced
	MAPID_NOVICE_HIGH = JOBL_UPPER|0x0,
	MAPID_SWORDMAN_HIGH,
	MAPID_MAGE_HIGH,
	MAPID_ARCHER_HIGH,
	MAPID_ACOLYTE_HIGH,
	MAPID_MERCHANT_HIGH,
	MAPID_THIEF_HIGH,
//2_1 advanced
	MAPID_LORD_KNIGHT = JOBL_UPPER|JOBL_2_1|0x1,
	MAPID_HIGH_WIZARD,
	MAPID_SNIPER,
	MAPID_HIGH_PRIEST,
	MAPID_WHITESMITH,
	MAPID_ASSASSIN_CROSS,
//2_2 advanced
	MAPID_PALADIN = JOBL_UPPER|JOBL_2_2|0x1,
	MAPID_PROFESSOR,
	MAPID_CLOWNGYPSY,
	MAPID_CHAMPION,
	MAPID_CREATOR,
	MAPID_STALKER,
//1-1 baby
	MAPID_BABY = JOBL_BABY|0x0,
	MAPID_BABY_SWORDMAN,
	MAPID_BABY_MAGE,
	MAPID_BABY_ARCHER,
	MAPID_BABY_ACOLYTE,
	MAPID_BABY_MERCHANT,
	MAPID_BABY_THIEF,
	MAPID_BABY_TAEKWON,
//2_1 baby
	MAPID_SUPER_BABY = JOBL_BABY|JOBL_2_1|0x0,
	MAPID_BABY_KNIGHT,
	MAPID_BABY_WIZARD,
	MAPID_BABY_HUNTER,
	MAPID_BABY_PRIEST,
	MAPID_BABY_BLACKSMITH,
	MAPID_BABY_ASSASSIN,
	MAPID_BABY_STAR_GLADIATOR,
//2_2 baby
	MAPID_BABY_CRUSADER = JOBL_BABY|JOBL_2_2|0x1,
	MAPID_BABY_SAGE,
	MAPID_BABY_BARDDANCER,
	MAPID_BABY_MONK,
	MAPID_BABY_ALCHEMIST,
	MAPID_BABY_ROGUE,
	MAPID_BABY_SOUL_LINKER,
	MAPID_RUNE_KNIGHT = JOBL_THIRD|JOBL_2_1|0x1,
	MAPID_WARLOCK,
	MAPID_RANGER,
	MAPID_ARCH_BISHOP,
	MAPID_MECHANIC,
	MAPID_GUILLOTINE_CROSS,
	MAPID_ROYAL_GUARD = JOBL_THIRD|JOBL_2_2|0x1,
	MAPID_SORCERER,
	MAPID_MINSTRELWANDERER,
	MAPID_SURA,
	MAPID_GENETIC,
	MAPID_SHADOW_CHASER,
	MAPID_RUNE_KNIGHT_T = JOBL_THIRD|JOBL_UPPER|JOBL_2_1|0x1,
	MAPID_WARLOCK_T,
	MAPID_RANGER_T,
	MAPID_ARCH_BISHOP_T,
	MAPID_MECHANIC_T,
	MAPID_GUILLOTINE_CROSS_T,
	MAPID_ROYAL_GUARD_T = JOBL_THIRD|JOBL_UPPER|JOBL_2_2|0x1,
	MAPID_SORCERER_T,
	MAPID_MINSTRELWANDERER_T,
	MAPID_SURA_T,
	MAPID_GENETIC_T,
	MAPID_SHADOW_CHASER_T,

};

//Max size for inputs to Graffiti, Talkie Box and Vending text prompts
#define MESSAGE_SIZE (79 + 1)
//String length you can write in the 'talking box'
#define CHATBOX_SIZE (70 + 1)
//Chatroom-related string sizes
#define CHATROOM_TITLE_SIZE (36 + 1)
#define CHATROOM_PASS_SIZE (8 + 1)
//Max allowed chat text length
#define CHAT_SIZE_MAX (255 + 1)
//24 for npc name + 24 for label + 2 for a "::" and 1 for EOS
#define EVENT_NAME_LENGTH ( NAME_LENGTH * 2 + 3 )

#define DEFAULT_AUTOSAVE_INTERVAL 5*60*1000

//Specifies maps where players may hit each other
#define map_flag_vs(m) (map[m].flag.pvp || map[m].flag.gvg_dungeon || map[m].flag.gvg || ((agit_flag || agit2_flag) && map[m].flag.gvg_castle) || map[m].flag.battleground)
//Specifies maps that have special GvG/WoE restrictions
#define map_flag_gvg(m) (map[m].flag.gvg || ((agit_flag || agit2_flag) && map[m].flag.gvg_castle))
//Specifies if the map is tagged as GvG/WoE (regardless of agit_flag status)
#define map_flag_gvg2(m) (map[m].flag.gvg || map[m].flag.gvg_castle)
// No Kill Steal Protection
#define map_flag_ks(m) (map[m].flag.town || map[m].flag.pvp || map[m].flag.gvg || map[m].flag.battleground)

//This stackable implementation does not means a BL can be more than one type at a time, but it's 
//meant to make it easier to check for multiple types at a time on invocations such as map_foreach* calls [Skotlex]
enum bl_type { 
	BL_NUL   = 0x000,
	BL_PC    = 0x001,
	BL_MOB   = 0x002,
	BL_PET   = 0x004,
	BL_HOM   = 0x008,
	BL_MER   = 0x010,
	BL_ITEM  = 0x020,
	BL_SKILL = 0x040,
	BL_NPC   = 0x080,
	BL_CHAT  = 0x100,

	BL_ALL   = 0xFFF,
};

//For common mapforeach calls. Since pets cannot be affected, they aren't included here yet.
#define BL_CHAR (BL_PC|BL_MOB|BL_HOM|BL_MER)

enum npc_subtype { WARP, SHOP, SCRIPT, CASHSHOP };

enum {
	RC_FORMLESS=0,
	RC_UNDEAD,
	RC_BRUTE,
	RC_PLANT,
	RC_INSECT,
	RC_FISH,
	RC_DEMON,
	RC_DEMIHUMAN,
	RC_ANGEL,
	RC_DRAGON,
	RC_BOSS,
	RC_NONBOSS,
	RC_NONDEMIHUMAN,
	RC_MAX
};

enum {
	RC2_NONE = 0,
	RC2_GOBLIN,
	RC2_KOBOLD,
	RC2_ORC,
	RC2_GOLEM,
	RC2_GUARDIAN,
	RC2_NINJA,
	RC2_MAX
};

enum {
	ELE_NEUTRAL=0,
	ELE_WATER,
	ELE_EARTH,
	ELE_FIRE,
	ELE_WIND,
	ELE_POISON,
	ELE_HOLY,
	ELE_DARK,
	ELE_GHOST,
	ELE_UNDEAD,
	ELE_MAX
};

enum auto_trigger_flag {
	ATF_SELF=0x01,
	ATF_TARGET=0x02,
	ATF_SHORT=0x04,
	ATF_LONG=0x08,
	ATF_WEAPON=0x10,
	ATF_MAGIC=0x20,
	ATF_MISC=0x40,
};

struct block_list {
	struct block_list *next,*prev;
	int id;
	short m,x,y;
	enum bl_type type;
};


// Mob List Held in memory for Dynamic Mobs [Wizputer]
// Expanded to specify all mob-related spawn data by [Skotlex]
struct spawn_data {
	short class_; //Class, used because a mob can change it's class
	unsigned short m,x,y;	//Spawn information (map, point, spawn-area around point)
	signed short xs,ys;
	unsigned short num; //Number of mobs using this structure
	unsigned short active; //Number of mobs that are already spawned (for mob_remove_damaged: no)
	unsigned int delay1,delay2; //Spawn delay (fixed base + random variance)
	struct {
		unsigned int size :2; //Holds if mob has to be tiny/large
		unsigned int ai :2;	//Holds if mob is special ai.
		unsigned int dynamic :1; //Whether this data is indexed by a map's dynamic mob list
		unsigned int boss : 1;
	} state;
	char name[NAME_LENGTH],eventname[EVENT_NAME_LENGTH]; //Name/event
};




struct flooritem_data {
	struct block_list bl;
	unsigned char subx,suby;
	int cleartimer;
	int first_get_charid,second_get_charid,third_get_charid;
	unsigned int first_get_tick,second_get_tick,third_get_tick;
	struct item item_data;
};

enum _sp {
	SP_SPEED,SP_BASEEXP,SP_JOBEXP,SP_KARMA,SP_MANNER,SP_HP,SP_MAXHP,SP_SP,	// 0-7
	SP_MAXSP,SP_STATUSPOINT,SP_0a,SP_BASELEVEL,SP_SKILLPOINT,SP_STR,SP_AGI,SP_VIT,	// 8-15
	SP_INT,SP_DEX,SP_LUK,SP_CLASS,SP_ZENY,SP_SEX,SP_NEXTBASEEXP,SP_NEXTJOBEXP,	// 16-23
	SP_WEIGHT,SP_MAXWEIGHT,SP_1a,SP_1b,SP_1c,SP_1d,SP_1e,SP_1f,	// 24-31
	SP_USTR,SP_UAGI,SP_UVIT,SP_UINT,SP_UDEX,SP_ULUK,SP_26,SP_27,	// 32-39
	SP_28,SP_ATK1,SP_ATK2,SP_MATK1,SP_MATK2,SP_DEF1,SP_DEF2,SP_MDEF1,	// 40-47
	SP_MDEF2,SP_HIT,SP_FLEE1,SP_FLEE2,SP_CRITICAL,SP_ASPD,SP_36,SP_JOBLEVEL,	// 48-55
	SP_UPPER,SP_PARTNER,SP_CART,SP_FAME,SP_UNBREAKABLE,	//56-60
	SP_CARTINFO=99,	// 99

	SP_BASEJOB=119,	// 100+19 - celest
	SP_BASECLASS=120,	//Hmm.. why 100+19? I just use the next one... [Skotlex]
	SP_KILLERRID=121,
	SP_KILLEDRID=122,

	// Mercenaries
	SP_MERCFLEE=165, SP_MERCKILLS=189, SP_MERCFAITH=190,
	
	// original 1000-
	SP_ATTACKRANGE=1000,	SP_ATKELE,SP_DEFELE,	// 1000-1002
	SP_CASTRATE, SP_MAXHPRATE, SP_MAXSPRATE, SP_SPRATE, // 1003-1006
	SP_ADDELE, SP_ADDRACE, SP_ADDSIZE, SP_SUBELE, SP_SUBRACE, // 1007-1011
	SP_ADDEFF, SP_RESEFF,	// 1012-1013
	SP_BASE_ATK,SP_ASPD_RATE,SP_HP_RECOV_RATE,SP_SP_RECOV_RATE,SP_SPEED_RATE, // 1014-1018
	SP_CRITICAL_DEF,SP_NEAR_ATK_DEF,SP_LONG_ATK_DEF, // 1019-1021
	SP_DOUBLE_RATE, SP_DOUBLE_ADD_RATE, SP_SKILL_HEAL, SP_MATK_RATE, // 1022-1025
	SP_IGNORE_DEF_ELE,SP_IGNORE_DEF_RACE, // 1026-1027
	SP_ATK_RATE,SP_SPEED_ADDRATE,SP_SP_REGEN_RATE, // 1028-1030
	SP_MAGIC_ATK_DEF,SP_MISC_ATK_DEF, // 1031-1032
	SP_IGNORE_MDEF_ELE,SP_IGNORE_MDEF_RACE, // 1033-1034
	SP_MAGIC_ADDELE,SP_MAGIC_ADDRACE,SP_MAGIC_ADDSIZE, // 1035-1037
	SP_PERFECT_HIT_RATE,SP_PERFECT_HIT_ADD_RATE,SP_CRITICAL_RATE,SP_GET_ZENY_NUM,SP_ADD_GET_ZENY_NUM, // 1038-1042
	SP_ADD_DAMAGE_CLASS,SP_ADD_MAGIC_DAMAGE_CLASS,SP_ADD_DEF_CLASS,SP_ADD_MDEF_CLASS, // 1043-1046
	SP_ADD_MONSTER_DROP_ITEM,SP_DEF_RATIO_ATK_ELE,SP_DEF_RATIO_ATK_RACE,SP_UNBREAKABLE_GARMENT, // 1047-1050
	SP_HIT_RATE,SP_FLEE_RATE,SP_FLEE2_RATE,SP_DEF_RATE,SP_DEF2_RATE,SP_MDEF_RATE,SP_MDEF2_RATE, // 1051-1057
	SP_SPLASH_RANGE,SP_SPLASH_ADD_RANGE,SP_AUTOSPELL,SP_HP_DRAIN_RATE,SP_SP_DRAIN_RATE, // 1058-1062
	SP_SHORT_WEAPON_DAMAGE_RETURN,SP_LONG_WEAPON_DAMAGE_RETURN,SP_WEAPON_COMA_ELE,SP_WEAPON_COMA_RACE, // 1063-1066
	SP_ADDEFF2,SP_BREAK_WEAPON_RATE,SP_BREAK_ARMOR_RATE,SP_ADD_STEAL_RATE, // 1067-1070
	SP_MAGIC_DAMAGE_RETURN,SP_ALL_STATS=1073,SP_AGI_VIT,SP_AGI_DEX_STR,SP_PERFECT_HIDE, // 1071-1076
	SP_NO_KNOCKBACK,SP_CLASSCHANGE, // 1077-1078
	SP_HP_DRAIN_VALUE,SP_SP_DRAIN_VALUE, // 1079-1080
	SP_WEAPON_ATK,SP_WEAPON_ATK_RATE, // 1081-1082
	SP_DELAYRATE,SP_HP_DRAIN_RATE_RACE,SP_SP_DRAIN_RATE_RACE, // 1083-1085
	SP_IGNORE_MDEF_RATE,SP_IGNORE_DEF_RATE,SP_SKILL_HEAL2,SP_ADDEFF_ONSKILL, //1086-1089
	SP_ADD_HEAL_RATE,SP_ADD_HEAL2_RATE, //1090-1091

	SP_RESTART_FULL_RECOVER=2000,SP_NO_CASTCANCEL,SP_NO_SIZEFIX,SP_NO_MAGIC_DAMAGE,SP_NO_WEAPON_DAMAGE,SP_NO_GEMSTONE, // 2000-2005
	SP_NO_CASTCANCEL2,SP_NO_MISC_DAMAGE,SP_UNBREAKABLE_WEAPON,SP_UNBREAKABLE_ARMOR, SP_UNBREAKABLE_HELM, // 2006-2010
	SP_UNBREAKABLE_SHIELD, SP_LONG_ATK_RATE, // 2011-2012

	SP_CRIT_ATK_RATE, SP_CRITICAL_ADDRACE, SP_NO_REGEN, SP_ADDEFF_WHENHIT, SP_AUTOSPELL_WHENHIT, // 2013-2017
	SP_SKILL_ATK, SP_UNSTRIPABLE, SP_AUTOSPELL_ONSKILL, // 2018-2020
	SP_SP_GAIN_VALUE, SP_HP_REGEN_RATE, SP_HP_LOSS_RATE, SP_ADDRACE2, SP_HP_GAIN_VALUE, // 2021-2025
	SP_SUBSIZE, SP_HP_DRAIN_VALUE_RACE, SP_ADD_ITEM_HEAL_RATE, SP_SP_DRAIN_VALUE_RACE, SP_EXP_ADDRACE,	// 2026-2030
	SP_SP_GAIN_RACE, SP_SUBRACE2, SP_UNBREAKABLE_SHOES,	// 2031-2033
	SP_UNSTRIPABLE_WEAPON,SP_UNSTRIPABLE_ARMOR,SP_UNSTRIPABLE_HELM,SP_UNSTRIPABLE_SHIELD,  // 2034-2037
	SP_INTRAVISION, SP_ADD_MONSTER_DROP_ITEMGROUP, SP_SP_LOSS_RATE, // 2038-2040
	SP_ADD_SKILL_BLOW, SP_SP_VANISH_RATE, SP_MAGIC_SP_GAIN_VALUE, SP_MAGIC_HP_GAIN_VALUE, SP_ADD_CLASS_DROP_ITEM, //2041-2045
	SP_WEAPON_MATK, SP_BASE_MATK, SP_SP_GAIN_RACE_ATTACK, SP_HP_GAIN_RACE_ATTACK //2046-2049
};

enum _look {
	LOOK_BASE,
	LOOK_HAIR,
	LOOK_WEAPON,
	LOOK_HEAD_BOTTOM,
	LOOK_HEAD_TOP,
	LOOK_HEAD_MID,
	LOOK_HAIR_COLOR,
	LOOK_CLOTHES_COLOR,
	LOOK_SHIELD,
	LOOK_SHOES,
	LOOK_BODY,
	LOOK_FLOOR,
	LOOK_ROBE,
};

// used by map_setcell()
typedef enum {
	CELL_WALKABLE,
	CELL_SHOOTABLE,
	CELL_WATER,

	CELL_NPC,
	CELL_BASILICA,
	CELL_LANDPROTECTOR,
	CELL_NOVENDING,
	CELL_NOCHAT,
	CELL_MAELSTROM,
	CELL_ICEWALL,

} cell_t;

// used by map_getcell()
typedef enum {
	CELL_GETTYPE,		// retrieves a cell's 'gat' type

	CELL_CHKWALL,		// wall (gat type 1)
	CELL_CHKWATER,		// water (gat type 3)
	CELL_CHKCLIFF,		// cliff/gap (gat type 5)

	CELL_CHKPASS,		// passable cell (gat type non-1/5)
	CELL_CHKREACH,		// Same as PASS, but ignores the cell-stacking mod.
	CELL_CHKNOPASS,		// non-passable cell (gat types 1 and 5)
	CELL_CHKNOREACH,	// Same as NOPASS, but ignores the cell-stacking mod.
	CELL_CHKSTACK,		// whether cell is full (reached cell stacking limit) 

	CELL_CHKNPC,
	CELL_CHKBASILICA,
	CELL_CHKLANDPROTECTOR,
	CELL_CHKNOVENDING,
	CELL_CHKNOCHAT,
	CELL_CHKMAELSTROM,
	CELL_CHKICEWALL,

} cell_chk;

struct mapcell
{
	// terrain flags
	unsigned char
		walkable : 1,
		shootable : 1,
		water : 1;

	// dynamic flags
	unsigned char
		npc : 1,
		basilica : 1,
		landprotector : 1,
		novending : 1,
		nochat : 1,
		maelstrom : 1,
		icewall : 1;

#ifdef CELL_NOSTACK
	unsigned char cell_bl; //Holds amount of bls in this cell.
#endif
};

struct iwall_data {
	char wall_name[50];
	short m, x, y, size, dir;
	bool shootable;
};

struct map_data {
	char name[MAP_NAME_LENGTH];
	unsigned short index; // The map index used by the mapindex* functions.
	struct mapcell* cell; // Holds the information of each map cell (NULL if the map is not on this map-server).
	struct block_list **block;
	struct block_list **block_mob;
	int m;
	short xs,ys; // map dimensions (in cells)
	short bxs,bys; // map dimensions (in blocks)
	short bgscore_lion, bgscore_eagle; // Battleground ScoreBoard
	int npc_num;
	int users;
	int users_pvp;
	int iwall_num; // Total of invisible walls in this map
	struct map_flag {
		unsigned town : 1; // [Suggestion to protect Mail System]
		unsigned autotrade : 1;
		unsigned allowks : 1; // [Kill Steal Protection]
		unsigned nomemo : 1;
		unsigned noteleport : 1;
		unsigned noreturn : 1;
		unsigned monster_noteleport : 1;
		unsigned nosave : 1;
		unsigned nobranch : 1;
		unsigned noexppenalty : 1;
		unsigned pvp : 1;
		unsigned pvp_noparty : 1;
		unsigned pvp_noguild : 1;
		unsigned pvp_nightmaredrop :1;
		unsigned pvp_nocalcrank : 1;
		unsigned gvg_castle : 1;
		unsigned gvg : 1; // Now it identifies gvg versus maps that are active 24/7
		unsigned gvg_dungeon : 1; // Celest
		unsigned gvg_noparty : 1;
		unsigned battleground : 2; // [BattleGround System]
		unsigned nozenypenalty : 1;
		unsigned notrade : 1;
		unsigned noskill : 1;
		unsigned nowarp : 1;
		unsigned nowarpto : 1;
		unsigned noicewall : 1; // [Valaris]
		unsigned snow : 1; // [Valaris]
		unsigned clouds : 1;
		unsigned clouds2 : 1; // [Valaris]
		unsigned fog : 1; // [Valaris]
		unsigned fireworks : 1;
		unsigned sakura : 1; // [Valaris]
		unsigned leaves : 1; // [Valaris]
		/**
		 * No longer available, keeping here just in case it's back someday. [Ind]
		 **/
		//unsigned rain : 1; // [Valaris]
		unsigned nogo : 1; // [Valaris]
		unsigned nobaseexp	: 1; // [Lorky] added by Lupus
		unsigned nojobexp	: 1; // [Lorky]
		unsigned nomobloot	: 1; // [Lorky]
		unsigned nomvploot	: 1; // [Lorky]
		unsigned nightenabled :1; //For night display. [Skotlex]
		unsigned restricted	: 1; // [Komurka]
		unsigned nodrop : 1;
		unsigned novending : 1;
		unsigned loadevent : 1;
		unsigned nochat :1;
		unsigned partylock :1;
		unsigned guildlock :1;
		unsigned src4instance : 1; // To flag this map when it's used as a src map for instances
		unsigned reset :1; // [Daegaladh]
	} flag;
	struct point save;
	struct npc_data *npc[MAX_NPC_PER_MAP];
	struct {
		int drop_id;
		int drop_type;
		int drop_per;
	} drop_list[MAX_DROP_PER_MAP];

	struct spawn_data *moblist[MAX_MOB_LIST_PER_MAP]; // [Wizputer]
	int mob_delete_timer;	// [Skotlex]
	int zone;	// zone number (for item/skill restrictions)
	int jexp;	// map experience multiplicator
	int bexp;	// map experience multiplicator
	int nocommand; //Blocks @/# commands for non-gms. [Skotlex]
	/**
	 * Ice wall reference counter for bugreport:3574
	 * - since there are a thounsand mobs out there in a lot of maps checking on,
	 * - every targetting for icewall on attack path would just be a waste, so,
	 * - this counter allows icewall checking be only run when there is a actual ice wall on the map
	 **/
	int icewall_num;
	// Instance Variables
	int instance_id;
	int instance_src_map;
};

/// Stores information about a remote map (for multi-mapserver setups).
/// Beginning of data structure matches 'map_data', to allow typecasting.
struct map_data_other_server {
	char name[MAP_NAME_LENGTH];
	unsigned short index; //Index is the map index used by the mapindex* functions.
	struct mapcell* cell; // If this is NULL, the map is not on this map-server
	uint32 ip;
	uint16 port;
};

int map_getcell(int,int,int,cell_chk);
int map_getcellp(struct map_data*,int,int,cell_chk);
void map_setcell(int m, int x, int y, cell_t cell, bool flag);
void map_setgatcell(int m, int x, int y, int gat);

extern struct map_data map[];
extern int map_num;

extern int autosave_interval;
extern int minsave_interval;
extern int save_settings;
extern int agit_flag;
extern int agit2_flag;
extern int night_flag; // 0=day, 1=night [Yor]
extern int enable_spy; //Determines if @spy commands are active.
extern char db_path[256];

extern char motd_txt[];
extern char help_txt[];
extern char help2_txt[];
extern char charhelp_txt[];

extern char wisp_server_name[];

// 鯖全体情報
void map_setusers(int);
int map_getusers(void);
int map_usercount(void);
// block削除関連
int map_freeblock(struct block_list *bl);
int map_freeblock_lock(void);
int map_freeblock_unlock(void);
// block関連
int map_addblock(struct block_list* bl);
int map_delblock(struct block_list* bl);
int map_moveblock(struct block_list *, int, int, unsigned int);
int map_foreachinrange(int (*func)(struct block_list*,va_list), struct block_list* center, int range, int type, ...);
int map_foreachinshootrange(int (*func)(struct block_list*,va_list), struct block_list* center, int range, int type, ...);
int map_foreachinarea(int (*func)(struct block_list*,va_list), int m, int x0, int y0, int x1, int y1, int type, ...);
int map_forcountinrange(int (*func)(struct block_list*,va_list), struct block_list* center, int range, int count, int type, ...);
int map_forcountinarea(int (*func)(struct block_list*,va_list), int m, int x0, int y0, int x1, int y1, int count, int type, ...);
int map_foreachinmovearea(int (*func)(struct block_list*,va_list), struct block_list* center, int range, int dx, int dy, int type, ...);
int map_foreachincell(int (*func)(struct block_list*,va_list), int m, int x, int y, int type, ...);
int map_foreachinpath(int (*func)(struct block_list*,va_list), int m, int x0, int y0, int x1, int y1, int range, int length, int type, ...);
int map_foreachinmap(int (*func)(struct block_list*,va_list), int m, int type, ...);
//block関連に追加
int map_count_oncell(int m,int x,int y,int type);
struct skill_unit *map_find_skill_unit_oncell(struct block_list *,int x,int y,int skill_id,struct skill_unit *);
// 一時的object関連
int map_get_new_object_id(void);
int map_search_freecell(struct block_list *src, int m, short *x, short *y, int rx, int ry, int flag);
//
int map_quit(struct map_session_data *);
// npc
bool map_addnpc(int,struct npc_data *);

// 床アイテム関連
int map_clearflooritem_timer(int tid, unsigned int tick, int id, intptr_t data);
int map_removemobs_timer(int tid, unsigned int tick, int id, intptr_t data);
#define map_clearflooritem(id) map_clearflooritem_timer(0,0,id,1)
int map_addflooritem(struct item *item_data,int amount,int m,int x,int y,int first_charid,int second_charid,int third_charid,int flags);

// キャラid＝＞キャラ名 変換関連
void map_addnickdb(int charid, const char* nick);
void map_delnickdb(int charid, const char* nick);
void map_reqnickdb(struct map_session_data* sd,int charid);
const char* map_charid2nick(int charid);
struct map_session_data* map_charid2sd(int charid);

struct map_session_data * map_id2sd(int id);
struct mob_data * map_id2md(int id);
struct npc_data * map_id2nd(int id);
struct homun_data* map_id2hd(int id);
struct mercenary_data* map_id2mc(int id);
struct chat_data* map_id2cd(int id);
struct block_list * map_id2bl(int id);

#define map_id2index(id) map[(id)].index
int map_mapindex2mapid(unsigned short mapindex);
int map_mapname2mapid(const char* name);
int map_mapname2ipport(unsigned short name, uint32* ip, uint16* port);
int map_setipport(unsigned short map, uint32 ip, uint16 port);
int map_eraseipport(unsigned short map, uint32 ip, uint16 port);
int map_eraseallipport(void);
void map_addiddb(struct block_list *);
void map_deliddb(struct block_list *bl);
void map_foreachpc(int (*func)(struct map_session_data* sd, va_list args), ...);
void map_foreachmob(int (*func)(struct mob_data* md, va_list args), ...);
void map_foreachnpc(int (*func)(struct npc_data* nd, va_list args), ...);
void map_foreachregen(int (*func)(struct block_list* bl, va_list args), ...);
void map_foreachiddb(int (*func)(struct block_list* bl, va_list args), ...);
struct map_session_data * map_nick2sd(const char*);
struct mob_data * map_getmob_boss(int m);
struct mob_data * map_id2boss(int id);

/// Bitfield of flags for the iterator.
enum e_mapitflags
{
	MAPIT_NORMAL = 0,
//	MAPIT_PCISPLAYING = 1,// Unneeded as pc_db/id_db will only hold auth'ed, active players.
};
struct s_mapiterator;
struct s_mapiterator*   mapit_alloc(enum e_mapitflags flags, enum bl_type types);
void                    mapit_free(struct s_mapiterator* mapit);
struct block_list*      mapit_first(struct s_mapiterator* mapit);
struct block_list*      mapit_last(struct s_mapiterator* mapit);
struct block_list*      mapit_next(struct s_mapiterator* mapit);
struct block_list*      mapit_prev(struct s_mapiterator* mapit);
bool                    mapit_exists(struct s_mapiterator* mapit);
#define mapit_getallusers() mapit_alloc(MAPIT_NORMAL,BL_PC)
#define mapit_geteachpc()   mapit_alloc(MAPIT_NORMAL,BL_PC)
#define mapit_geteachmob()  mapit_alloc(MAPIT_NORMAL,BL_MOB)
#define mapit_geteachnpc()  mapit_alloc(MAPIT_NORMAL,BL_NPC)
#define mapit_geteachiddb() mapit_alloc(MAPIT_NORMAL,BL_ALL)

// その他
int map_check_dir(int s_dir,int t_dir);
unsigned char map_calc_dir( struct block_list *src,int x,int y);
int map_random_dir(struct block_list *bl, short *x, short *y); // [Skotlex]

int cleanup_sub(struct block_list *bl, va_list ap);

int map_delmap(char* mapname);
void map_flags_init(void);

bool map_iwall_set(int m, int x, int y, int size, int dir, bool shootable, const char* wall_name);
void map_iwall_get(struct map_session_data *sd);
void map_iwall_remove(const char *wall_name);

int map_addmobtolist(unsigned short m, struct spawn_data *spawn);	// [Wizputer]
void map_spawnmobs(int); // [Wizputer]
void map_removemobs(int); // [Wizputer]
void do_reconnect_map(void); //Invoked on map-char reconnection [Skotlex]
void map_addmap2db(struct map_data *m);
void map_removemapdb(struct map_data *m);

extern char *INTER_CONF_NAME;
extern char *LOG_CONF_NAME;
extern char *MAP_CONF_NAME;
extern char *BATTLE_CONF_FILENAME;
extern char *ATCOMMAND_CONF_FILENAME;
extern char *SCRIPT_CONF_NAME;
extern char *MSG_CONF_NAME;
extern char *GRF_PATH_FILENAME;

//Useful typedefs from jA [Skotlex]
typedef struct map_session_data TBL_PC;
typedef struct npc_data         TBL_NPC;
typedef struct mob_data         TBL_MOB;
typedef struct flooritem_data   TBL_ITEM;
typedef struct chat_data        TBL_CHAT;
typedef struct skill_unit       TBL_SKILL;
typedef struct pet_data         TBL_PET;
typedef struct homun_data       TBL_HOM;
typedef struct mercenary_data   TBL_MER;

#define BL_CAST(type_, bl) \
	( ((bl) == (struct block_list*)NULL || (bl)->type != (type_)) ? (T ## type_ *)NULL : (T ## type_ *)(bl) )


extern char main_chat_nick[16];

#include "../common/sql.h"

extern int db_use_sqldbs;

extern Sql* mmysql_handle;
extern Sql* logmysql_handle;

extern char item_db_db[32];
extern char item_db2_db[32];
extern char item_db_re_db[32];
extern char mob_db_db[32];
extern char mob_db2_db[32];
extern char mob_skill_db_db[32];
extern char mob_skill_db2_db[32];

void do_shutdown(void);

#endif /* _MAP_H_ */
