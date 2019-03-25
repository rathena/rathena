// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MAP_HPP
#define MAP_HPP

#include <algorithm>
#include <stdarg.h>
#include <unordered_map>
#include <vector>

#include "../common/cbasetypes.hpp"
#include "../common/core.hpp" // CORE_ST_LAST
#include "../common/db.hpp"
#include "../common/mapindex.hpp"
#include "../common/mmo.hpp"
#include "../common/msg_conf.hpp"
#include "../common/timer.hpp"
#include "../config/core.hpp"

struct npc_data;
struct item_data;
struct Channel;

enum E_MAPSERVER_ST {
	MAPSERVER_ST_RUNNING = CORE_ST_LAST,
	MAPSERVER_ST_STARTING,
	MAPSERVER_ST_SHUTDOWN,
	MAPSERVER_ST_LAST
};

struct map_data *map_getmapdata(int16 m);
#define msg_config_read(cfgName,isnew) map_msg_config_read(cfgName,isnew)
#define msg_txt(sd,msg_number) map_msg_txt(sd,msg_number)
#define do_final_msg() map_do_final_msg()
int map_msg_config_read(const char *cfgName,int lang);
const char* map_msg_txt(struct map_session_data *sd,int msg_number);
void map_do_final_msg(void);
void map_msg_reload(void);

#define MAX_NPC_PER_MAP 512
#define AREA_SIZE battle_config.area_size
#define DAMAGELOG_SIZE 30
#define LOOTITEM_SIZE 10
#define MAX_MOBSKILL 50		//Max 128, see mob skill_idx type if need this higher
#define MAX_MOB_LIST_PER_MAP 128
#define MAX_EVENTQUEUE 2
#define MAX_EVENTTIMER 32
#define NATURAL_HEAL_INTERVAL 500
#define MIN_FLOORITEM 2
#define MAX_FLOORITEM START_ACCOUNT_NUM
#define MAX_LEVEL 175
#define MAX_DROP_PER_MAP 48
#define MAX_IGNORE_LIST 20 	// official is 14
#define MAX_VENDING 12
#define MAX_MAP_SIZE 512*512 	// Wasn't there something like this already? Can't find it.. [Shinryo]

//The following system marks a different job ID system used by the map server,
//which makes a lot more sense than the normal one. [Skotlex]
//
//These marks the "level" of the job.
#define JOBL_2_1 0x100 //256
#define JOBL_2_2 0x200 //512
#define JOBL_2 0x300 //768

#define JOBL_UPPER 0x1000 //4096
#define JOBL_BABY 0x2000  //8192
#define JOBL_THIRD 0x4000 //16384

//for filtering and quick checking.
#define MAPID_BASEMASK 0x00ff
#define MAPID_UPPERMASK 0x0fff
#define MAPID_THIRDMASK (JOBL_THIRD|MAPID_UPPERMASK)

//First Jobs
//Note the oddity of the novice:
//Super Novices are considered the 2-1 version of the novice! Novices are considered a first class type, too...
enum e_mapid {
//Novice And 1-1 Jobs
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
	MAPID_HANBOK,
	MAPID_GANGSI,
	MAPID_OKTOBERFEST,
	MAPID_SUMMONER,
	MAPID_SUMMER2,
//2-1 Jobs
	MAPID_SUPER_NOVICE = JOBL_2_1|MAPID_NOVICE,
	MAPID_KNIGHT,
	MAPID_WIZARD,
	MAPID_HUNTER,
	MAPID_PRIEST,
	MAPID_BLACKSMITH,
	MAPID_ASSASSIN,
	MAPID_STAR_GLADIATOR,
	MAPID_REBELLION = JOBL_2_1|MAPID_GUNSLINGER,
	MAPID_KAGEROUOBORO,
	MAPID_DEATH_KNIGHT = JOBL_2_1|MAPID_GANGSI,
//2-2 Jobs
	MAPID_CRUSADER = JOBL_2_2|MAPID_SWORDMAN,
	MAPID_SAGE,
	MAPID_BARDDANCER,
	MAPID_MONK,
	MAPID_ALCHEMIST,
	MAPID_ROGUE,
	MAPID_SOUL_LINKER,
	MAPID_DARK_COLLECTOR = JOBL_2_2|MAPID_GANGSI,
//Trans Novice And Trans 1-1 Jobs
	MAPID_NOVICE_HIGH = JOBL_UPPER|MAPID_NOVICE,
	MAPID_SWORDMAN_HIGH,
	MAPID_MAGE_HIGH,
	MAPID_ARCHER_HIGH,
	MAPID_ACOLYTE_HIGH,
	MAPID_MERCHANT_HIGH,
	MAPID_THIEF_HIGH,
//Trans 2-1 Jobs
	MAPID_LORD_KNIGHT = JOBL_UPPER|MAPID_KNIGHT,
	MAPID_HIGH_WIZARD,
	MAPID_SNIPER,
	MAPID_HIGH_PRIEST,
	MAPID_WHITESMITH,
	MAPID_ASSASSIN_CROSS,
//Trans 2-2 Jobs
	MAPID_PALADIN = JOBL_UPPER|MAPID_CRUSADER,
	MAPID_PROFESSOR,
	MAPID_CLOWNGYPSY,
	MAPID_CHAMPION,
	MAPID_CREATOR,
	MAPID_STALKER,
//Baby Novice And Baby 1-1 Jobs
	MAPID_BABY = JOBL_BABY|MAPID_NOVICE,
	MAPID_BABY_SWORDMAN,
	MAPID_BABY_MAGE,
	MAPID_BABY_ARCHER,
	MAPID_BABY_ACOLYTE,
	MAPID_BABY_MERCHANT,
	MAPID_BABY_THIEF,
	MAPID_BABY_TAEKWON,
	MAPID_BABY_GUNSLINGER = JOBL_BABY|MAPID_GUNSLINGER,
	MAPID_BABY_NINJA,
	MAPID_BABY_SUMMONER = JOBL_BABY|MAPID_SUMMONER,
//Baby 2-1 Jobs
	MAPID_SUPER_BABY = JOBL_BABY|MAPID_SUPER_NOVICE,
	MAPID_BABY_KNIGHT,
	MAPID_BABY_WIZARD,
	MAPID_BABY_HUNTER,
	MAPID_BABY_PRIEST,
	MAPID_BABY_BLACKSMITH,
	MAPID_BABY_ASSASSIN,
	MAPID_BABY_STAR_GLADIATOR,
	MAPID_BABY_REBELLION = JOBL_BABY|MAPID_REBELLION,
	MAPID_BABY_KAGEROUOBORO,
//Baby 2-2 Jobs
	MAPID_BABY_CRUSADER = JOBL_BABY|MAPID_CRUSADER,
	MAPID_BABY_SAGE,
	MAPID_BABY_BARDDANCER,
	MAPID_BABY_MONK,
	MAPID_BABY_ALCHEMIST,
	MAPID_BABY_ROGUE,
	MAPID_BABY_SOUL_LINKER,
//3-1 Jobs
	MAPID_SUPER_NOVICE_E = JOBL_THIRD|MAPID_SUPER_NOVICE,
	MAPID_RUNE_KNIGHT,
	MAPID_WARLOCK,
	MAPID_RANGER,
	MAPID_ARCH_BISHOP,
	MAPID_MECHANIC,
	MAPID_GUILLOTINE_CROSS,
	MAPID_STAR_EMPEROR,
//3-2 Jobs
	MAPID_ROYAL_GUARD = JOBL_THIRD|MAPID_CRUSADER,
	MAPID_SORCERER,
	MAPID_MINSTRELWANDERER,
	MAPID_SURA,
	MAPID_GENETIC,
	MAPID_SHADOW_CHASER,
	MAPID_SOUL_REAPER,
//Trans 3-1 Jobs
	MAPID_RUNE_KNIGHT_T = JOBL_THIRD|MAPID_LORD_KNIGHT,
	MAPID_WARLOCK_T,
	MAPID_RANGER_T,
	MAPID_ARCH_BISHOP_T,
	MAPID_MECHANIC_T,
	MAPID_GUILLOTINE_CROSS_T,
//Trans 3-2 Jobs
	MAPID_ROYAL_GUARD_T = JOBL_THIRD|MAPID_PALADIN,
	MAPID_SORCERER_T,
	MAPID_MINSTRELWANDERER_T,
	MAPID_SURA_T,
	MAPID_GENETIC_T,
	MAPID_SHADOW_CHASER_T,
//Baby 3-1 Jobs
	MAPID_SUPER_BABY_E = JOBL_THIRD|MAPID_SUPER_BABY,
	MAPID_BABY_RUNE,
	MAPID_BABY_WARLOCK,
	MAPID_BABY_RANGER,
	MAPID_BABY_BISHOP,
	MAPID_BABY_MECHANIC,
	MAPID_BABY_CROSS,
	MAPID_BABY_STAR_EMPEROR,
//Baby 3-2 Jobs
	MAPID_BABY_GUARD = JOBL_THIRD|MAPID_BABY_CRUSADER,
	MAPID_BABY_SORCERER,
	MAPID_BABY_MINSTRELWANDERER,
	MAPID_BABY_SURA,
	MAPID_BABY_GENETIC,
	MAPID_BABY_CHASER,
	MAPID_BABY_SOUL_REAPER,
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

#define DEFAULT_AUTOSAVE_INTERVAL 5*60*1000

//This stackable implementation does not means a BL can be more than one type at a time, but it's
//meant to make it easier to check for multiple types at a time on invocations such as map_foreach* calls [Skotlex]
enum bl_type : uint16{
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
	BL_ELEM  = 0x200,

	BL_ALL   = 0xFFF,
};

/// For common mapforeach calls. Since pets cannot be affected, they aren't included here yet.
#define BL_CHAR (BL_PC|BL_MOB|BL_HOM|BL_MER|BL_ELEM)

/// NPC Subtype
enum npc_subtype : uint8{
	NPCTYPE_WARP, /// Warp
	NPCTYPE_SHOP, /// Shop
	NPCTYPE_SCRIPT, /// Script
	NPCTYPE_CASHSHOP, /// Cashshop
	NPCTYPE_ITEMSHOP, /// Itemshop
	NPCTYPE_POINTSHOP, /// Pointshop
	NPCTYPE_TOMB, /// Monster tomb
	NPCTYPE_MARKETSHOP, /// Marketshop
};

enum e_race : int8{
	RC_NONE_ = -1, //don't give us bonus
	RC_FORMLESS = 0,
	RC_UNDEAD,
	RC_BRUTE,
	RC_PLANT,
	RC_INSECT,
	RC_FISH,
	RC_DEMON,
	RC_DEMIHUMAN,
	RC_ANGEL,
	RC_DRAGON,
	RC_PLAYER,
	RC_ALL,
	RC_MAX //auto upd enum for array size
};

enum e_classAE : int8{
	CLASS_NONE = -1, //don't give us bonus
	CLASS_NORMAL = 0,
	CLASS_BOSS,
	CLASS_GUARDIAN,
	CLASS_BATTLEFIELD,
	CLASS_ALL,
	CLASS_MAX //auto upd enum for array len
};

enum e_race2 : uint8{
	RC2_NONE = 0,
	RC2_GOBLIN,
	RC2_KOBOLD,
	RC2_ORC,
	RC2_GOLEM,
	RC2_GUARDIAN,
	RC2_NINJA,
	RC2_GVG,
	RC2_BATTLEFIELD,
	RC2_TREASURE,
	RC2_BIOLAB,
	RC2_MANUK,
	RC2_SPLENDIDE,
	RC2_SCARABA,
	RC2_OGH_ATK_DEF,
	RC2_OGH_HIDDEN,
	RC2_BIO5_SWORDMAN_THIEF,
	RC2_BIO5_ACOLYTE_MERCHANT,
	RC2_BIO5_MAGE_ARCHER,
	RC2_BIO5_MVP,
	RC2_MAX
};

/// Element list
enum e_element : int8{
	ELE_NONE=-1,
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
	ELE_ALL,
	ELE_MAX
};

#define MAX_ELE_LEVEL 4 /// Maximum Element level

/**
 * Types of spirit charms
 * NOTE: Code assumes that this matches the first entries in enum elements
 */
enum spirit_charm_types {
	CHARM_TYPE_NONE = 0,
	CHARM_TYPE_WATER,
	CHARM_TYPE_LAND,
	CHARM_TYPE_FIRE,
	CHARM_TYPE_WIND
};

enum mob_ai {
	AI_NONE = 0,
	AI_ATTACK,
	AI_SPHERE,
	AI_FLORA,
	AI_ZANZOU,
	AI_LEGION,
	AI_FAW,
	AI_MAX
};

enum auto_trigger_flag {
	ATF_SELF   = 0x01,
	ATF_TARGET = 0x02,
	ATF_SHORT  = 0x04,
	ATF_LONG   = 0x08,
	ATF_WEAPON = 0x10,
	ATF_MAGIC  = 0x20,
	ATF_MISC   = 0x40,
};

struct block_list {
	struct block_list *next,*prev;
	int id;
	int16 m,x,y;
	enum bl_type type;
};


// Mob List Held in memory for Dynamic Mobs [Wizputer]
// Expanded to specify all mob-related spawn data by [Skotlex]
struct spawn_data {
	short id; //ID, used because a mob can change it's class
	unsigned short m, x, y;	//Spawn information (map, point, spawn-area around point)
	signed short xs, ys;
	unsigned short num; //Number of mobs using this structure
	unsigned short active;//Number of mobs that are already spawned (for mob_remove_damaged: no)
	unsigned int delay1, delay2; //Spawn delay (fixed base + random variance)
	unsigned int level;
	struct {
		unsigned int size : 2; //Holds if mob has to be tiny/large
		enum mob_ai ai; //Special ai for summoned monsters.
		unsigned int dynamic : 1; //Whether this data is indexed by a map's dynamic mob list
		unsigned int boss : 1; //0: Non-boss monster | 1: Boss monster
	} state;
	char name[NAME_LENGTH], eventname[EVENT_NAME_LENGTH]; //Name/event
};

struct flooritem_data {
	struct block_list bl;
	unsigned char subx,suby;
	int cleartimer;
	int first_get_charid,second_get_charid,third_get_charid;
	t_tick first_get_tick,second_get_tick,third_get_tick;
	struct item item;
	unsigned short mob_id; ///< ID of monster who dropped it. 0 for non-monster who dropped it.
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

	SP_KILLEDGID=118,
	SP_BASEJOB=119,	// 100+19 - celest
	SP_BASECLASS=120,	//Hmm.. why 100+19? I just use the next one... [Skotlex]
	SP_KILLERRID=121,
	SP_KILLEDRID=122,
	SP_SITTING=123,
	SP_CHARMOVE=124,
	SP_CHARRENAME=125,
	SP_CHARFONT=126,
	SP_BANK_VAULT = 127,
	SP_ROULETTE_BRONZE = 128,
	SP_ROULETTE_SILVER = 129,
	SP_ROULETTE_GOLD = 130,
	SP_CASHPOINTS, SP_KAFRAPOINTS,
	SP_PCDIECOUNTER, SP_COOKMASTERY,

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
	SP_ADD_DAMAGE_CLASS,SP_ADD_MAGIC_DAMAGE_CLASS,SP_ADD_DEF_MONSTER,SP_ADD_MDEF_MONSTER, // 1043-1046
	SP_ADD_MONSTER_DROP_ITEM,SP_DEF_RATIO_ATK_ELE,SP_DEF_RATIO_ATK_RACE,SP_UNBREAKABLE_GARMENT, // 1047-1050
	SP_HIT_RATE,SP_FLEE_RATE,SP_FLEE2_RATE,SP_DEF_RATE,SP_DEF2_RATE,SP_MDEF_RATE,SP_MDEF2_RATE, // 1051-1057
	SP_SPLASH_RANGE,SP_SPLASH_ADD_RANGE,SP_AUTOSPELL,SP_HP_DRAIN_RATE,SP_SP_DRAIN_RATE, // 1058-1062
	SP_SHORT_WEAPON_DAMAGE_RETURN,SP_LONG_WEAPON_DAMAGE_RETURN,SP_WEAPON_COMA_ELE,SP_WEAPON_COMA_RACE, // 1063-1066
	SP_ADDEFF2,SP_BREAK_WEAPON_RATE,SP_BREAK_ARMOR_RATE,SP_ADD_STEAL_RATE, // 1067-1070
	SP_MAGIC_DAMAGE_RETURN,SP_ALL_STATS=1073,SP_AGI_VIT,SP_AGI_DEX_STR,SP_PERFECT_HIDE, // 1071-1076
	SP_NO_KNOCKBACK,SP_CLASSCHANGE, // 1077-1078
	SP_HP_DRAIN_VALUE,SP_SP_DRAIN_VALUE, // 1079-1080
	SP_WEAPON_ATK,SP_WEAPON_DAMAGE_RATE, // 1081-1082
	SP_DELAYRATE,SP_HP_DRAIN_VALUE_RACE, SP_SP_DRAIN_VALUE_RACE, // 1083-1085
	SP_IGNORE_MDEF_RACE_RATE,SP_IGNORE_DEF_RACE_RATE,SP_SKILL_HEAL2,SP_ADDEFF_ONSKILL, //1086-1089
	SP_ADD_HEAL_RATE,SP_ADD_HEAL2_RATE, SP_EQUIP_ATK, //1090-1092

	SP_RESTART_FULL_RECOVER=2000,SP_NO_CASTCANCEL,SP_NO_SIZEFIX,SP_NO_MAGIC_DAMAGE,SP_NO_WEAPON_DAMAGE,SP_NO_GEMSTONE, // 2000-2005
	SP_NO_CASTCANCEL2,SP_NO_MISC_DAMAGE,SP_UNBREAKABLE_WEAPON,SP_UNBREAKABLE_ARMOR, SP_UNBREAKABLE_HELM, // 2006-2010
	SP_UNBREAKABLE_SHIELD, SP_LONG_ATK_RATE, // 2011-2012

	SP_CRIT_ATK_RATE, SP_CRITICAL_ADDRACE, SP_NO_REGEN, SP_ADDEFF_WHENHIT, SP_AUTOSPELL_WHENHIT, // 2013-2017
	SP_SKILL_ATK, SP_UNSTRIPABLE, SP_AUTOSPELL_ONSKILL, // 2018-2020
	SP_SP_GAIN_VALUE, SP_HP_REGEN_RATE, SP_HP_LOSS_RATE, SP_ADDRACE2, SP_HP_GAIN_VALUE, // 2021-2025
	SP_SUBSIZE, SP_HP_DRAIN_VALUE_CLASS, SP_ADD_ITEM_HEAL_RATE, SP_SP_DRAIN_VALUE_CLASS, SP_EXP_ADDRACE,	// 2026-2030
	SP_SP_GAIN_RACE, SP_SUBRACE2, SP_UNBREAKABLE_SHOES,	// 2031-2033
	SP_UNSTRIPABLE_WEAPON,SP_UNSTRIPABLE_ARMOR,SP_UNSTRIPABLE_HELM,SP_UNSTRIPABLE_SHIELD,  // 2034-2037
	SP_INTRAVISION, SP_ADD_MONSTER_DROP_ITEMGROUP, SP_SP_LOSS_RATE, // 2038-2040
	SP_ADD_SKILL_BLOW, SP_SP_VANISH_RATE, SP_MAGIC_SP_GAIN_VALUE, SP_MAGIC_HP_GAIN_VALUE, SP_ADD_MONSTER_ID_DROP_ITEM, //2041-2045
	SP_EMATK, SP_COMA_CLASS, SP_COMA_RACE, SP_SKILL_USE_SP_RATE, //2046-2049
	SP_SKILL_COOLDOWN,SP_SKILL_FIXEDCAST, SP_SKILL_VARIABLECAST, SP_FIXCASTRATE, SP_VARCASTRATE, //2050-2054
	SP_SKILL_USE_SP,SP_MAGIC_ATK_ELE, SP_ADD_FIXEDCAST, SP_ADD_VARIABLECAST,  //2055-2058
	SP_SET_DEF_RACE,SP_SET_MDEF_RACE,SP_HP_VANISH_RATE,  //2059-2061

	SP_IGNORE_DEF_CLASS, SP_DEF_RATIO_ATK_CLASS, SP_ADDCLASS, SP_SUBCLASS, SP_MAGIC_ADDCLASS, //2062-2066
	SP_WEAPON_COMA_CLASS, SP_IGNORE_MDEF_CLASS_RATE, SP_EXP_ADDCLASS, SP_ADD_CLASS_DROP_ITEM, //2067-2070
	SP_ADD_CLASS_DROP_ITEMGROUP, SP_ADDMAXWEIGHT, SP_ADD_ITEMGROUP_HEAL_RATE,  // 2071-2073
	SP_HP_VANISH_RACE_RATE, SP_SP_VANISH_RACE_RATE, SP_ABSORB_DMG_MAXHP, SP_SUB_SKILL, SP_SUBDEF_ELE, // 2074-2078
	SP_STATE_NORECOVER_RACE, SP_CRITICAL_RANGEATK, SP_MAGIC_ADDRACE2, SP_IGNORE_MDEF_RACE2_RATE, // 2079-2082
	SP_WEAPON_ATK_RATE, SP_WEAPON_MATK_RATE, SP_DROP_ADDRACE, SP_DROP_ADDCLASS, SP_NO_MADO_FUEL, // 2083-2087
	SP_IGNORE_DEF_CLASS_RATE, SP_REGEN_PERCENT_HP, SP_REGEN_PERCENT_SP, SP_SKILL_DELAY, SP_NO_WALK_DELAY //2088-2093
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
	LOOK_BODY,			//Purpose Unknown. Doesen't appear to do anything.
	LOOK_RESET_COSTUMES,//Makes all headgear sprites on player vanish when activated.
	LOOK_ROBE,
	// LOOK_FLOOR,	// TODO : fix me!! offcial use this ?
	LOOK_BODY2
};

enum e_mapflag : int16 {
	MF_INVALID = -1,
	MF_MIN = 0,
	MF_NOMEMO = 0,
	MF_NOTELEPORT,
	MF_NOSAVE,
	MF_NOBRANCH,
	MF_NOPENALTY,
	MF_NOZENYPENALTY,
	MF_PVP,
	MF_PVP_NOPARTY,
	MF_PVP_NOGUILD,
	MF_GVG,
	MF_GVG_NOPARTY,	//10
	MF_NOTRADE,
	MF_NOSKILL,
	MF_NOWARP,
	MF_PARTYLOCK,
	MF_NOICEWALL,
	MF_SNOW,
	MF_FOG,
	MF_SAKURA,
	MF_LEAVES,
	//MF_RAIN,	//20 - No longer available, keeping here just in case it's back someday. [Ind]
	// 21 free
	MF_NOGO = 22,
	MF_CLOUDS,
	MF_CLOUDS2,
	MF_FIREWORKS,
	MF_GVG_CASTLE,
	MF_GVG_DUNGEON,
	MF_NIGHTENABLED,
	MF_NOBASEEXP,
	MF_NOJOBEXP,	//30
	MF_NOMOBLOOT,
	MF_NOMVPLOOT,
	MF_NORETURN,
	MF_NOWARPTO,
	MF_PVP_NIGHTMAREDROP,
	MF_RESTRICTED,
	MF_NOCOMMAND,
	MF_NODROP,
	MF_JEXP,
	MF_BEXP,	//40
	MF_NOVENDING,
	MF_LOADEVENT,
	MF_NOCHAT,
	MF_NOEXPPENALTY,
	MF_GUILDLOCK,
	MF_TOWN,
	MF_AUTOTRADE,
	MF_ALLOWKS,
	MF_MONSTER_NOTELEPORT,
	MF_PVP_NOCALCRANK,	//50
	MF_BATTLEGROUND,
	MF_RESET,
	MF_NOMAPCHANNELAUTOJOIN,
	MF_NOUSECART,
	MF_NOITEMCONSUMPTION,
	MF_NOSUNMOONSTARMIRACLE,
	MF_NOMINEEFFECT,
	MF_NOLOCKON,
	MF_NOTOMB,
	MF_SKILL_DAMAGE,	//60
	MF_NOCOSTUME,
	MF_GVG_TE_CASTLE,
	MF_GVG_TE,
	MF_HIDEMOBHPBAR,
	MF_NOLOOT,
	MF_NOEXP,
	MF_PRIVATEAIRSHIP_SOURCE,
	MF_PRIVATEAIRSHIP_DESTINATION,
	MF_SKILL_DURATION,
	MF_MAX
};

/// Enum of damage types
enum e_skill_damage_type : uint8 {
	SKILLDMG_PC,
	SKILLDMG_MOB,
	SKILLDMG_BOSS,
	SKILLDMG_OTHER,
	SKILLDMG_MAX,
	SKILLDMG_CASTER, ///< Only used on getter for caster value
};

/// Struct for MF_SKILL_DAMAGE
struct s_skill_damage {
	unsigned int map; ///< Maps (used for skill_damage_db.txt)
	uint16 caster; ///< Caster type
	int rate[SKILLDMG_MAX]; ///< Used for when all skills are adjusted
};

/// Struct of MF_SKILL_DURATION
struct s_skill_duration {
	uint16 skill_id; ///< Skill ID
	uint16 per; ///< Rate
};

/// Enum for item drop type for MF_PVP_NIGHTMAREDROP
enum e_nightmare_drop_type : uint8 {
	NMDT_INVENTORY = 0x1,
	NMDT_EQUIP = 0x2,
	NMDT_ALL = (NMDT_INVENTORY|NMDT_EQUIP)
};

/// Struct for MF_PVP_NIGHTMAREDROP
struct s_drop_list {
	int drop_id;
	int drop_per;
	enum e_nightmare_drop_type drop_type;
};

/// Union for mapflag values
union u_mapflag_args {
	struct point nosave;
	struct s_drop_list nightmaredrop;
	struct s_skill_damage skill_damage;
	struct s_skill_duration skill_duration;
	int flag_val;
};

// used by map_setcell()
enum cell_t{
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

};

// used by map_getcell()
enum cell_chk : uint8 {
	CELL_GETTYPE,			// Retrieves a cell's 'gat' type

	CELL_CHKWALL,			// Whether the cell is a wall (gat type 1)
	CELL_CHKWATER,			// Whether the cell is water (gat type 3)
	CELL_CHKCLIFF,			// Whether the cell is a cliff/gap (gat type 5)

	CELL_CHKPASS,			// Whether the cell is passable (gat type not 1 and 5)
	CELL_CHKREACH,			// Whether the cell is passable, but ignores the cell stacking limit
	CELL_CHKNOPASS,			// Whether the cell is non-passable (gat types 1 and 5)
	CELL_CHKNOREACH,		// Whether the cell is non-passable, but ignores the cell stacking limit
	CELL_CHKSTACK,			// Whether the cell is full (reached cell stacking limit)

	CELL_CHKNPC,			// Whether the cell has an OnTouch NPC
	CELL_CHKBASILICA,		// Whether the cell has Basilica
	CELL_CHKLANDPROTECTOR,	// Whether the cell has Land Protector
	CELL_CHKNOVENDING,		// Whether the cell denies MC_VENDING skill
	CELL_CHKNOCHAT,			// Whether the cell denies Player Chat Window
	CELL_CHKMAELSTROM,		// Whether the cell has Maelstrom
	CELL_CHKICEWALL,		// Whether the cell has Ice Wall

};

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
	short m, x, y, size;
	int8 dir;
	bool shootable;
};

struct questinfo_req {
	unsigned int quest_id;
	unsigned state : 2; // 0: Doesn't have, 1: Inactive, 2: Active, 3: Complete //! TODO: CONFIRM ME!!
};

struct questinfo {
	struct npc_data *nd;
	unsigned short icon;
	unsigned char color;
	int quest_id;
	unsigned short min_level,
		max_level;
	uint8 req_count;
	uint8 jobid_count;
	struct questinfo_req *req;
	unsigned short *jobid;
};

struct map_data {
	char name[MAP_NAME_LENGTH];
	uint16 index; // The map index used by the mapindex* functions.
	struct mapcell* cell; // Holds the information of each map cell (NULL if the map is not on this map-server).
	struct block_list **block;
	struct block_list **block_mob;
	int16 m;
	int16 xs,ys; // map dimensions (in cells)
	int16 bxs,bys; // map dimensions (in blocks)
	int16 bgscore_lion, bgscore_eagle; // Battleground ScoreBoard
	int npc_num;
	int users;
	int users_pvp;
	int iwall_num; // Total of invisible walls in this map

	std::unordered_map<int16, int> flag;
	struct point save;
	std::vector<s_drop_list> drop_list;
	uint32 zone; // zone number (for item/skill restrictions)
	struct s_skill_damage damage_adjust; // Used for overall skill damage adjustment
	std::unordered_map<uint16, s_skill_damage> skill_damage; // Used for single skill damage adjustment
	std::unordered_map<uint16, int> skill_duration;

	struct npc_data *npc[MAX_NPC_PER_MAP];
	struct spawn_data *moblist[MAX_MOB_LIST_PER_MAP]; // [Wizputer]
	int mob_delete_timer;	// Timer ID for map_removemobs_timer [Skotlex]

	// Instance Variables
	unsigned short instance_id;
	int instance_src_map;

	/* rAthena Local Chat */
	struct Channel *channel;

	/* ShowEvent Data Cache */
	struct questinfo *qi_data;
	unsigned short qi_count;
	
	/* speeds up clif_updatestatus processing by causing hpmeter to run only when someone with the permission can view it */
	unsigned short hpmeter_visible;
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

int map_getcell(int16 m,int16 x,int16 y,cell_chk cellchk);
int map_getcellp(struct map_data* m,int16 x,int16 y,cell_chk cellchk);
void map_setcell(int16 m, int16 x, int16 y, cell_t cell, bool flag);
void map_setgatcell(int16 m, int16 x, int16 y, int gat);

extern struct map_data map[];
extern int map_num;

extern int autosave_interval;
extern int minsave_interval;
extern int16 save_settings;
extern int night_flag; // 0=day, 1=night [Yor]
extern int enable_spy; //Determines if @spy commands are active.

// Agit Flags
extern bool agit_flag;
extern bool agit2_flag;
extern bool agit3_flag;
#define is_agit_start() (agit_flag || agit2_flag || agit3_flag)

/**
 * Specifies maps where players may hit each other
 * @param mapdata: Map Data
 * @return True on success or false otherwise
 */
inline bool mapdata_flag_vs(struct map_data *mapdata) {
	if (mapdata == nullptr)
		return false;

	if (mapdata->flag[MF_PVP] || mapdata->flag[MF_GVG_DUNGEON] || mapdata->flag[MF_GVG] || ((agit_flag || agit2_flag) && mapdata->flag[MF_GVG_CASTLE]) || mapdata->flag[MF_GVG_TE] || (agit3_flag && mapdata->flag[MF_GVG_TE_CASTLE]) || mapdata->flag[MF_BATTLEGROUND])
		return true;

	return false;
}

/**
 * Versus map: PVP, BG, GVG, GVG Dungeons, and GVG Castles (regardless of agit_flag status)
 * @param mapdata: Map Data
 * @return True on success or false otherwise
 */
inline bool mapdata_flag_vs2(struct map_data *mapdata) {
	if (mapdata == nullptr)
		return false;

	if (mapdata->flag[MF_PVP] || mapdata->flag[MF_GVG_DUNGEON] || mapdata->flag[MF_GVG] || mapdata->flag[MF_GVG_CASTLE] || mapdata->flag[MF_GVG_TE] || mapdata->flag[MF_GVG_TE_CASTLE] || mapdata->flag[MF_BATTLEGROUND])
		return true;

	return false;
}

/**
 * Specifies maps that have special GvG/WoE restrictions
 * @param mapdata: Map Data
 * @return True on success or false otherwise
 */
inline bool mapdata_flag_gvg(struct map_data *mapdata) {
	if (mapdata == nullptr)
		return false;

	if (mapdata->flag[MF_GVG] || ((agit_flag || agit2_flag) && mapdata->flag[MF_GVG_CASTLE]) || mapdata->flag[MF_GVG_TE] || (agit3_flag && mapdata->flag[MF_GVG_TE_CASTLE]))
		return true;

	return false;
}

/**
 * Specifies if the map is tagged as GvG/WoE (regardless of agit_flag status)
 * @param mapdata: Map Data
 * @return True on success or false otherwise
 */
inline bool mapdata_flag_gvg2(struct map_data *mapdata) {
	if (mapdata == nullptr)
		return false;

	if (mapdata->flag[MF_GVG] || mapdata->flag[MF_GVG_TE] || mapdata->flag[MF_GVG_CASTLE] || mapdata->flag[MF_GVG_TE_CASTLE])
		return true;

	return false;
}

/**
 * No Kill Steal Protection
 * @param mapdata: Map Data
 * @return True on success or false otherwise
 */
inline bool mapdata_flag_ks(struct map_data *mapdata) {
	if (mapdata == nullptr)
		return false;

	if (mapdata->flag[MF_TOWN] || mapdata->flag[MF_PVP] || mapdata->flag[MF_GVG] || mapdata->flag[MF_GVG_TE] || mapdata->flag[MF_BATTLEGROUND])
		return true;

	return false;
}

/**
 * WOE:TE Maps (regardless of agit_flag status)
 * @param mapdata: Map Data
 * @return True on success or false otherwise
 * @author Cydh
 */
inline bool mapdata_flag_gvg2_te(struct map_data *mapdata) {
	if (mapdata == nullptr)
		return false;

	if (mapdata->flag[MF_GVG_TE] || mapdata->flag[MF_GVG_TE_CASTLE])
		return true;

	return false;
}

/**
 * Check if map is GVG maps exclusion for item, skill, and status restriction check (regardless of agit_flag status)
 * @param mapdata: Map Data
 * @return True on success or false otherwise
 * @author Cydh
 */
inline bool mapdata_flag_gvg2_no_te(struct map_data *mapdata) {
	if (mapdata == nullptr)
		return false;

	if (mapdata->flag[MF_GVG] || mapdata->flag[MF_GVG_CASTLE])
		return true;

	return false;
}

/// Backwards compatibility
inline bool map_flag_vs(int16 m) {
	if (m < 0)
		return false;

	struct map_data *mapdata = &map[m];

	return mapdata_flag_vs(mapdata);
}

inline bool map_flag_vs2(int16 m) {
	if (m < 0)
		return false;

	struct map_data *mapdata = &map[m];

	return mapdata_flag_vs2(mapdata);
}

inline bool map_flag_gvg(int16 m) {
	if (m < 0)
		return false;

	struct map_data *mapdata = &map[m];

	return mapdata_flag_gvg(mapdata);
}

inline bool map_flag_gvg2(int16 m) {
	if (m < 0)
		return false;

	struct map_data *mapdata = &map[m];

	return mapdata_flag_gvg2(mapdata);
}

inline bool map_flag_ks(int16 m) {
	if (m < 0)
		return false;

	struct map_data *mapdata = &map[m];

	return mapdata_flag_ks(mapdata);
}

inline bool map_flag_gvg2_te(int16 m) {
	if (m < 0)
		return false;

	struct map_data *mapdata = &map[m];

	return mapdata_flag_gvg2_te(mapdata);
}

inline bool map_flag_gvg2_no_te(int16 m) {
	if (m < 0)
		return false;

	struct map_data *mapdata = &map[m];

	return mapdata_flag_gvg2_no_te(mapdata);
}

extern char motd_txt[];
extern char help_txt[];
extern char help2_txt[];
extern char charhelp_txt[];
extern char channel_conf[];

extern char wisp_server_name[];

struct s_map_default {
	char mapname[MAP_NAME_LENGTH];
	unsigned short x;
	unsigned short y;
};
extern struct s_map_default map_default;

/// Type of 'save_settings'
enum save_settings_type {
	CHARSAVE_NONE		= 0x000, /// Never
	CHARSAVE_TRADE		= 0x001, /// After trading
	CHARSAVE_VENDING	= 0x002, /// After vending (open/transaction)
	CHARSAVE_STORAGE	= 0x004, /// After closing storage/guild storage.
	CHARSAVE_PET		= 0x008, /// After hatching/returning to egg a pet.
	CHARSAVE_MAIL		= 0x010, /// After successfully sending a mail with attachment
	CHARSAVE_AUCTION	= 0x020, /// After successfully submitting an item for auction
	CHARSAVE_QUEST		= 0x040, /// After successfully get/delete/complete a quest
	CHARSAVE_BANK		= 0x080, /// After every bank transaction (deposit/withdraw)
	CHARSAVE_ATTENDANCE	= 0x100, /// After every attendence reward
	CHARSAVE_ALL		= 0xFFF, /// Always
};

// users
void map_setusers(int);
int map_getusers(void);
int map_usercount(void);

// blocklist lock
int map_freeblock(struct block_list *bl);
int map_freeblock_lock(void);
int map_freeblock_unlock(void);
// blocklist manipulation
int map_addblock(struct block_list* bl);
int map_delblock(struct block_list* bl);
int map_moveblock(struct block_list *, int, int, t_tick);
int map_foreachinrange(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...);
int map_foreachinallrange(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...);
int map_foreachinshootrange(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...);
int map_foreachinarea(int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...);
int map_foreachinallarea(int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...);
int map_foreachinshootarea(int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...);
int map_forcountinrange(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int count, int type, ...);
int map_forcountinarea(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int count, int type, ...);
int map_foreachinmovearea(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int type, ...);
int map_foreachincell(int (*func)(struct block_list*,va_list), int16 m, int16 x, int16 y, int type, ...);
int map_foreachinpath(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int type, ...);
int map_foreachindir(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int offset, int type, ...);
int map_foreachinmap(int (*func)(struct block_list*,va_list), int16 m, int type, ...);
//blocklist nb in one cell
int map_count_oncell(int16 m,int16 x,int16 y,int type,int flag);
struct skill_unit *map_find_skill_unit_oncell(struct block_list *,int16 x,int16 y,uint16 skill_id,struct skill_unit *, int flag);
// search and creation
int map_get_new_object_id(void);
int map_search_freecell(struct block_list *src, int16 m, int16 *x, int16 *y, int16 rx, int16 ry, int flag);
bool map_closest_freecell(int16 m, int16 *x, int16 *y, int type, int flag);
//
int map_quit(struct map_session_data *);
// npc
bool map_addnpc(int16 m,struct npc_data *);

// map item
TIMER_FUNC(map_clearflooritem_timer);
TIMER_FUNC(map_removemobs_timer);
void map_clearflooritem(struct block_list* bl);
int map_addflooritem(struct item *item, int amount, int16 m, int16 x, int16 y, int first_charid, int second_charid, int third_charid, int flags, unsigned short mob_id, bool canShowEffect = false);

// instances
int map_addinstancemap(const char *name, unsigned short instance_id);
int map_delinstancemap(int m);
void map_data_copyall(void);
void map_data_copy(struct map_data *dst_map, struct map_data *src_map);

// player to map session
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
struct pet_data* map_id2pd(int id);
struct elemental_data* map_id2ed(int id);
struct chat_data* map_id2cd(int id);
struct block_list * map_id2bl(int id);
bool map_blid_exists( int id );

#define map_id2index(id) map[(id)].index
const char* map_mapid2mapname(int m);
int16 map_mapindex2mapid(unsigned short mapindex);
int16 map_mapname2mapid(const char* name);
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
struct map_session_data * map_nick2sd(const char* nick, bool allow_partial);
struct mob_data * map_getmob_boss(int16 m);
struct mob_data * map_id2boss(int id);

// reload config file looking only for npcs
void map_reloadnpc(bool clear);

struct questinfo *map_add_questinfo(int m, struct questinfo *qi);
bool map_remove_questinfo(int m, struct npc_data *nd);
struct questinfo *map_has_questinfo(int m, struct npc_data *nd, int quest_id);

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

int map_check_dir(int s_dir,int t_dir);
uint8 map_calc_dir(struct block_list *src,int16 x,int16 y);
uint8 map_calc_dir_xy(int16 srcx, int16 srcy, int16 x, int16 y, uint8 srcdir);
int map_random_dir(struct block_list *bl, int16 *x, int16 *y); // [Skotlex]

int cleanup_sub(struct block_list *bl, va_list ap);

int map_delmap(char* mapname);
void map_flags_init(void);

bool map_iwall_exist(const char* wall_name);
bool map_iwall_set(int16 m, int16 x, int16 y, int size, int8 dir, bool shootable, const char* wall_name);
void map_iwall_get(struct map_session_data *sd);
bool map_iwall_remove(const char *wall_name);

int map_addmobtolist(unsigned short m, struct spawn_data *spawn);	// [Wizputer]
void map_spawnmobs(int16 m); // [Wizputer]
void map_removemobs(int16 m); // [Wizputer]
void map_addmap2db(struct map_data *m);
void map_removemapdb(struct map_data *m);

void map_skill_damage_add(struct map_data *m, uint16 skill_id, int rate[SKILLDMG_MAX], uint16 caster);
void map_skill_duration_add(struct map_data *mapd, uint16 skill_id, uint16 per);

enum e_mapflag map_getmapflag_by_name(char* name);
bool map_getmapflag_name(enum e_mapflag mapflag, char* output);
int map_getmapflag_sub(int16 m, enum e_mapflag mapflag, union u_mapflag_args *args);
bool map_setmapflag_sub(int16 m, enum e_mapflag mapflag, bool status, union u_mapflag_args *args);
#define map_getmapflag(m, mapflag) map_getmapflag_sub(m, mapflag, NULL)
#define map_setmapflag(m, mapflag, status) map_setmapflag_sub(m, mapflag, status, NULL)

#define CHK_ELEMENT(ele) ((ele) > ELE_NONE && (ele) < ELE_MAX) /// Check valid Element
#define CHK_ELEMENT_LEVEL(lv) ((lv) >= 1 && (lv) <= MAX_ELE_LEVEL) /// Check valid element level
#define CHK_RACE(race) ((race) > RC_NONE_ && (race) < RC_MAX) /// Check valid Race
#define CHK_RACE2(race2) ((race2) >= RC2_NONE && (race2) < RC2_MAX) /// Check valid Race2
#define CHK_CLASS(class_) ((class_) > CLASS_NONE && (class_) < CLASS_MAX) /// Check valid Class

//Other languages supported
extern const char*MSG_CONF_NAME_RUS;
extern const char*MSG_CONF_NAME_SPN;
extern const char*MSG_CONF_NAME_GRM;
extern const char*MSG_CONF_NAME_CHN;
extern const char*MSG_CONF_NAME_MAL;
extern const char*MSG_CONF_NAME_IDN;
extern const char*MSG_CONF_NAME_FRN;
extern const char*MSG_CONF_NAME_POR;
extern const char*MSG_CONF_NAME_THA;

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
typedef struct elemental_data	TBL_ELEM;

#define BL_CAST(type_, bl) \
	( ((bl) == (struct block_list*)NULL || (bl)->type != (type_)) ? (T ## type_ *)NULL : (T ## type_ *)(bl) )

#include "../common/sql.hpp"

extern int db_use_sqldbs;

extern Sql* mmysql_handle;
extern Sql* qsmysql_handle;
extern Sql* logmysql_handle;

extern char buyingstores_table[32];
extern char buyingstore_items_table[32];
extern char item_table[32];
extern char item2_table[32];
extern char mob_table[32];
extern char mob2_table[32];
extern char mob_skill_table[32];
extern char mob_skill2_table[32];
extern char vendings_table[32];
extern char vending_items_table[32];
extern char market_table[32];
extern char roulette_table[32];
extern char guild_storage_log_table[32];

void do_shutdown(void);

#endif /* MAP_HPP */
