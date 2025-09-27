// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MAP_HPP
#define MAP_HPP

#include <algorithm>
#include <cstdarg>
#include <string>
#include <unordered_map>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/core.hpp> // CORE_ST_LAST
#include <common/db.hpp>
#include <common/mapindex.hpp>
#include <common/mmo.hpp>
#include <common/msg_conf.hpp>
#include <common/timer.hpp>
#include <config/core.hpp>

#include "navi.hpp"
#include "script.hpp"
#include "path.hpp"

using rathena::server_core::Core;
using rathena::server_core::e_core_type;

namespace rathena{
	namespace server_map{
		class MapServer : public Core{
			protected:
				bool initialize( int32 argc, char* argv[] ) override;
				void finalize() override;
				void handle_crash() override;
				void handle_shutdown() override;

			public:
				MapServer() : Core( e_core_type::MAP ){

				}
		};
	}
}

struct npc_data;
struct item_data;
struct Channel;

struct map_data *map_getmapdata(int16 m);
#define msg_config_read(cfgName,isnew) map_msg_config_read(cfgName,isnew)
#define msg_txt(sd,msg_number) map_msg_txt(sd,msg_number)
#define do_final_msg() map_do_final_msg()
int32 map_msg_config_read(const char *cfgName,int32 lang);
const char* map_msg_txt(map_session_data *sd,int32 msg_number);
void map_do_final_msg(void);
void map_msg_reload(void);

#define MAX_NPC_PER_MAP 512
#define AREA_SIZE battle_config.area_size
#ifndef DAMAGELOG_SIZE 
	#define DAMAGELOG_SIZE 20
#endif
#define LOOTITEM_SIZE 10
#define MAX_MOBSKILL 50		//Max 128, see mob skill_idx type if need this higher
#define MAX_MOB_LIST_PER_MAP 128
#define MAX_EVENTQUEUE 2
#define MAX_EVENTTIMER 32
#define NATURAL_HEAL_INTERVAL 500
#define MIN_FLOORITEM 2
#define MAX_FLOORITEM START_ACCOUNT_NUM
#define MAX_LEVEL 275
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
#define JOBL_FOURTH 0x8000 //32768

//for filtering and quick checking.
#define MAPID_BASEMASK 0x00ff
#define MAPID_UPPERMASK 0x0fff
#define MAPID_THIRDMASK (JOBL_THIRD|MAPID_UPPERMASK)
#define MAPID_FOURTHMASK (JOBL_FOURTH|MAPID_THIRDMASK|JOBL_UPPER)

//First Jobs
//Note the oddity of the novice:
//Super Novices are considered the 2-1 version of the novice! Novices are considered a first class type, too...
enum e_mapid : uint64{
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
	MAPID_BABY_RUNE_KNIGHT,
	MAPID_BABY_WARLOCK,
	MAPID_BABY_RANGER,
	MAPID_BABY_ARCH_BISHOP,
	MAPID_BABY_MECHANIC,
	MAPID_BABY_GUILLOTINE_CROSS,
	MAPID_BABY_STAR_EMPEROR,
//Baby 3-2 Jobs
	MAPID_BABY_ROYAL_GUARD = JOBL_THIRD|MAPID_BABY_CRUSADER,
	MAPID_BABY_SORCERER,
	MAPID_BABY_MINSTRELWANDERER,
	MAPID_BABY_SURA,
	MAPID_BABY_GENETIC,
	MAPID_BABY_SHADOW_CHASER,
	MAPID_BABY_SOUL_REAPER,
//4-1 Jobs
	MAPID_HYPER_NOVICE = JOBL_FOURTH|JOBL_THIRD|JOBL_UPPER|MAPID_SUPER_NOVICE,
	MAPID_DRAGON_KNIGHT,
	MAPID_ARCH_MAGE,
	MAPID_WINDHAWK,
	MAPID_CARDINAL,
	MAPID_MEISTER,
	MAPID_SHADOW_CROSS,
	MAPID_SKY_EMPEROR,
	MAPID_NIGHT_WATCH = JOBL_FOURTH|JOBL_THIRD|JOBL_UPPER|MAPID_REBELLION,
	MAPID_SHINKIRO_SHIRANUI,
	MAPID_SPIRIT_HANDLER = JOBL_FOURTH|JOBL_THIRD|JOBL_UPPER|JOBL_2_1|MAPID_SUMMONER,
//4-2 Jobs
	MAPID_IMPERIAL_GUARD = JOBL_FOURTH|JOBL_THIRD|JOBL_UPPER|MAPID_CRUSADER,
	MAPID_ELEMENTAL_MASTER,
	MAPID_TROUBADOURTROUVERE,
	MAPID_INQUISITOR,
	MAPID_BIOLO,
	MAPID_ABYSS_CHASER,
	MAPID_SOUL_ASCETIC,
// Additional constants
	MAPID_ALL = UINT64_MAX
};

//Max size for inputs to Graffiti, Talkie Box and Vending text prompts
#define MESSAGE_SIZE (79 + 1)
// Max size for inputs to Graffiti, Talkie Box text prompts
#if PACKETVER_MAIN_NUM >= 20190904 || PACKETVER_RE_NUM >= 20190904 || PACKETVER_ZERO_NUM >= 20190828
#define TALKBOX_MESSAGE_SIZE 21
#else
#define TALKBOX_MESSAGE_SIZE (79 + 1)
#endif
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
	NPCTYPE_BARTER, /// Barter
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
	RC_PLAYER_HUMAN,
	RC_PLAYER_DORAM,
	RC_ALL,
	RC_MAX //auto upd enum for array size
};

enum e_race2 : uint8{
	RC2_NONE = 0,
	RC2_GOBLIN,
	RC2_KOBOLD,
	RC2_ORC,
	RC2_GOLEM,
	RC2_GUARDIAN, // Deprecated to CLASS_GUARDIAN
	RC2_NINJA,
	RC2_GVG,
	RC2_BATTLEFIELD, // Deprecated to CLASS_BATTLEFIELD
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
	RC2_CLOCKTOWER,
	RC2_THANATOS,
	RC2_FACEWORM,
	RC2_HEARTHUNTER,
	RC2_ROCKRIDGE,
	RC2_WERNER_LAB,
	RC2_TEMPLE_DEMON,
	RC2_ILLUSION_VAMPIRE,
	RC2_MALANGDO,
	RC2_EP172ALPHA,
	RC2_EP172BETA,
	RC2_EP172BATH,
	RC2_ILLUSION_TURTLE,
	RC2_RACHEL_SANCTUARY,
	RC2_ILLUSION_LUANDA,
	RC2_ILLUSION_FROZEN,
	RC2_ILLUSION_MOONLIGHT,
	RC2_EP16_DEF,
	RC2_EDDA_ARUNAFELTZ,
	RC2_LASAGNA,
	RC2_GLAST_HEIM_ABYSS,
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
	ELE_MAX,
	ELE_WEAPON,
	ELE_ENDOWED,
	ELE_RANDOM,
};

static std::unordered_map<std::string, e_element> um_eleid2elename {
	{ "Neutral", ELE_NEUTRAL },
	{ "Water", ELE_WATER },
	{ "Earth", ELE_EARTH },
	{ "Fire", ELE_FIRE },
	{ "Wind", ELE_WIND },
	{ "Poison", ELE_POISON },
	{ "Holy", ELE_HOLY },
	{ "Dark", ELE_DARK },
	{ "Ghost", ELE_GHOST },
	{ "Undead", ELE_UNDEAD },
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
	AI_GUILD,
	AI_WAVEMODE,
	AI_ABR,
	AI_BIONIC,
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
	int32 id;
	int16 m,x,y;
	enum bl_type type;
};


// Mob List Held in memory for Dynamic Mobs [Wizputer]
// Expanded to specify all mob-related spawn data by [Skotlex]
struct spawn_data {
	int16 id; //ID, used because a mob can change it's class
	uint16 m, x, y;	//Spawn information (map, point, spawn-area around point)
	int16 xs, ys;
	uint16 num; //Number of mobs using this structure
	uint16 active;//Number of mobs that are already spawned (for mob_remove_damaged: no)
	uint32 delay1, delay2; //Spawn delay (fixed base + random variance)
	uint32 level;
	struct {
		uint32 size : 2; //Holds if mob has to be tiny/large
		enum mob_ai ai; //Special ai for summoned monsters.
		uint32 dynamic : 1; //Whether this data is indexed by a map's dynamic mob list
		uint32 boss : 1; //0: Non-boss monster | 1: Boss monster
	} state;
	char name[NAME_LENGTH], eventname[EVENT_NAME_LENGTH]; //Name/event
	char filepath[256];
};

struct flooritem_data : public block_list {
	unsigned char subx,suby;
	int32 cleartimer;
	int32 first_get_charid,second_get_charid,third_get_charid;
	t_tick first_get_tick,second_get_tick,third_get_tick;
	struct item item;
	uint16 mob_id; ///< ID of monster who dropped it. 0 for non-monster who dropped it.
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
	SP_ACHIEVEMENT_LEVEL,

	// Mercenaries
	SP_MERCFLEE=165, SP_MERCKILLS=189, SP_MERCFAITH=190,

	// 4th jobs
	SP_POW=219, SP_STA, SP_WIS, SP_SPL, SP_CON, SP_CRT,	// 219-224
	SP_PATK, SP_SMATK, SP_RES, SP_MRES, SP_HPLUS, SP_CRATE,	// 225-230
	SP_TRAITPOINT, SP_AP, SP_MAXAP,	// 231-233
	SP_UPOW=247, SP_USTA, SP_UWIS, SP_USPL, SP_UCON, SP_UCRT,	// 247-252

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
	SP_PATK_RATE,SP_SMATK_RATE,SP_RES_RATE,SP_MRES_RATE,SP_HPLUS_RATE,SP_CRATE_RATE,SP_ALL_TRAIT_STATS,SP_MAXAPRATE,// 1093-1100

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
	SP_IGNORE_DEF_CLASS_RATE, SP_REGEN_PERCENT_HP, SP_REGEN_PERCENT_SP, SP_SKILL_DELAY, SP_NO_WALK_DELAY, //2088-2092
	SP_LONG_SP_GAIN_VALUE, SP_LONG_HP_GAIN_VALUE, SP_SHORT_ATK_RATE, SP_MAGIC_SUBSIZE, SP_CRIT_DEF_RATE, // 2093-2097
	SP_MAGIC_SUBDEF_ELE, SP_REDUCE_DAMAGE_RETURN, SP_ADD_ITEM_SPHEAL_RATE, SP_ADD_ITEMGROUP_SPHEAL_RATE, // 2098-2101
	SP_WEAPON_SUBSIZE, SP_ABSORB_DMG_MAXHP2, // 2102-2103
	SP_SP_IGNORE_RES_RACE_RATE, SP_SP_IGNORE_MRES_RACE_RATE, SP_EMATK_HIDDEN, SP_SKILL_RATIO // 2104-2107
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
	MF_FORCEMINEFFECT,
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
	MF_NOCASHSHOP, // 70
	MF_NORODEX,
	MF_NORENEWALEXPPENALTY,
	MF_NORENEWALDROPPENALTY,
	MF_NOPETCAPTURE,
	MF_NOBUYINGSTORE,
	MF_NODYNAMICNPC,
	MF_NOBANK,
	MF_SPECIALPOPUP,
	MF_NOMACROCHECKER,
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
	uint32 map; ///< Maps (used for skill_damage_db.txt)
	uint16 caster; ///< Caster type
	int32 rate[SKILLDMG_MAX]; ///< Used for when all skills are adjusted
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
	int32 drop_id;
	int32 drop_per;
	enum e_nightmare_drop_type drop_type;
};

/// Union for mapflag values
union u_mapflag_args {
	struct point nosave;
	struct s_drop_list nightmaredrop;
	struct s_skill_damage skill_damage;
	struct s_skill_duration skill_duration;
	int32 flag_val;
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
	CELL_NOBUYINGSTORE,
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
	CELL_CHKNOBUYINGSTORE,	// Whether the cell denies ALL_BUYING_STORE skill

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
		icewall : 1,
		nobuyingstore : 1;

#ifdef CELL_NOSTACK
	unsigned char cell_bl; //Holds amount of bls in this cell.
#endif
};

struct iwall_data {
	char wall_name[50];
	int16 m, x, y, size;
	int8 dir;
	bool shootable;
};

struct map_data {
	char name[MAP_NAME_LENGTH];
	uint16 index; // The map index used by the mapindex* functions.
	struct mapcell* cell; // Holds the information of each map cell (nullptr if the map is not on this map-server).
	struct block_list **block;
	struct block_list **block_mob;
	int16 m;
	int16 xs,ys; // map dimensions (in cells)
	int16 bxs,bys; // map dimensions (in blocks)
	int16 bgscore_lion, bgscore_eagle; // Battleground ScoreBoard
	int32 npc_num; // number total of npc on the map
	int32 npc_num_area; // number of npc with a trigger area on the map
	int32 npc_num_warp; // number of warp npc on the map
	int32 users;
	int32 users_pvp;
	int32 iwall_num; // Total of invisible walls in this map

	struct point save;
	std::vector<s_drop_list> drop_list;
	uint32 zone; // zone number (for item/skill restrictions)
	struct s_skill_damage damage_adjust; // Used for overall skill damage adjustment
	std::unordered_map<uint16, s_skill_damage> skill_damage; // Used for single skill damage adjustment
	std::unordered_map<uint16, int32> skill_duration;

	struct npc_data *npc[MAX_NPC_PER_MAP];
	struct spawn_data *moblist[MAX_MOB_LIST_PER_MAP]; // [Wizputer]
	int32 mob_delete_timer;	// Timer ID for map_removemobs_timer [Skotlex]
	t_tick last_macrocheck;

	// Instance Variables
	int32 instance_id;
	int32 instance_src_map;

	/* rAthena Local Chat */
	struct Channel *channel;

	/* ShowEvent Data Cache */
	std::vector<int32> qi_npc;

	/* speeds up clif_updatestatus processing by causing hpmeter to run only when someone with the permission can view it */
	uint16 hpmeter_visible;
#ifdef MAP_GENERATOR
	struct {
		std::vector<const struct npc_data *> npcs;
		std::vector<const struct navi_link *> warps_into;
		std::vector<const struct navi_link *> warps_outof;
	} navi;
#endif

	int32 getMapFlag(int32 flag) const;
	void setMapFlag(int32 flag, int32 value);
	void initMapFlags();
	void copyFlags(const map_data& other);

private:
	std::vector<int32> flags;
};

/// Stores information about a remote map (for multi-mapserver setups).
/// Beginning of data structure matches 'map_data', to allow typecasting.
struct map_data_other_server {
	char name[MAP_NAME_LENGTH];
	uint16 index; //Index is the map index used by the mapindex* functions.
	struct mapcell* cell; // If this is nullptr, the map is not on this map-server
	uint32 ip;
	uint16 port;
};

/**
 * align for packet ZC_SAY_DIALOG_ALIGN
 **/
enum e_say_dialog_align : uint8 {
	DIALOG_ALIGN_LEFT   = 0,
	DIALOG_ALIGN_RIGHT  = 1,
	DIALOG_ALIGN_CENTER = 2,
	DIALOG_ALIGN_TOP    = 3,
	DIALOG_ALIGN_MIDDLE = 4,
	DIALOG_ALIGN_BOTTOM = 5
};

struct inter_conf {
	uint32 start_status_points;
	bool emblem_woe_change;
	uint32 emblem_transparency_limit;
};

extern struct inter_conf inter_config;

int32 map_getcell(int16 m,int16 x,int16 y,cell_chk cellchk);
int32 map_getcellp(struct map_data* m,int16 x,int16 y,cell_chk cellchk);
void map_setcell(int16 m, int16 x, int16 y, cell_t cell, bool flag);
void map_setgatcell(int16 m, int16 x, int16 y, int32 gat);

extern struct map_data map[];
extern int32 map_num;

extern int32 autosave_interval;
extern int32 minsave_interval;
extern int16 save_settings;
extern int32 night_flag; // 0=day, 1=night [Yor]
extern int32 enable_spy; //Determines if @spy commands are active.

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

	if (mapdata->getMapFlag(MF_PVP) || mapdata->getMapFlag(MF_GVG_DUNGEON) || mapdata->getMapFlag(MF_GVG) || ((agit_flag || agit2_flag) && mapdata->getMapFlag(MF_GVG_CASTLE)) || mapdata->getMapFlag(MF_GVG_TE) || (agit3_flag && mapdata->getMapFlag(MF_GVG_TE_CASTLE)) || mapdata->getMapFlag(MF_BATTLEGROUND))
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

	if (mapdata->getMapFlag(MF_PVP) || mapdata->getMapFlag(MF_GVG_DUNGEON) || mapdata->getMapFlag(MF_GVG) || mapdata->getMapFlag(MF_GVG_CASTLE) || mapdata->getMapFlag(MF_GVG_TE) || mapdata->getMapFlag(MF_GVG_TE_CASTLE) || mapdata->getMapFlag(MF_BATTLEGROUND))
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

	if (mapdata->getMapFlag(MF_GVG) || ((agit_flag || agit2_flag) && mapdata->getMapFlag(MF_GVG_CASTLE)) || mapdata->getMapFlag(MF_GVG_TE) || (agit3_flag && mapdata->getMapFlag(MF_GVG_TE_CASTLE)))
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

	if (mapdata->getMapFlag(MF_GVG) || mapdata->getMapFlag(MF_GVG_TE) || mapdata->getMapFlag(MF_GVG_CASTLE) || mapdata->getMapFlag(MF_GVG_TE_CASTLE))
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

	if (mapdata->getMapFlag(MF_TOWN) || mapdata->getMapFlag(MF_PVP) || mapdata->getMapFlag(MF_GVG) || mapdata->getMapFlag(MF_GVG_TE) || mapdata->getMapFlag(MF_BATTLEGROUND))
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

	if (mapdata->getMapFlag(MF_GVG_TE) || mapdata->getMapFlag(MF_GVG_TE_CASTLE))
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

	if (mapdata->getMapFlag(MF_GVG) || mapdata->getMapFlag(MF_GVG_CASTLE))
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
extern char charhelp_txt[];
extern char channel_conf[];

extern char wisp_server_name[];

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
void map_setusers(int32);
int32 map_getusers(void);
int32 map_usercount(void);

// blocklist lock
int32 map_freeblock(struct block_list *bl);
int32 map_freeblock_lock(void);
int32 map_freeblock_unlock(void);
// blocklist manipulation
int32 map_addblock(struct block_list* bl);
int32 map_delblock(struct block_list* bl);
int32 map_moveblock(struct block_list *, int32, int32, t_tick);
int32 map_foreachinrange(int32 (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int32 type, ...);
int32 map_foreachinallrange(int32 (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int32 type, ...);
int32 map_foreachinshootrange(int32 (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int32 type, ...);
int32 map_foreachinarea(int32 (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int32 type, ...);
int32 map_foreachinallarea(int32 (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int32 type, ...);
int32 map_foreachinshootarea(int32 (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int32 type, ...);
int32 map_forcountinrange(int32 (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int32 count, int32 type, ...);
int32 map_forcountinarea(int32 (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int32 count, int32 type, ...);
int32 map_foreachinmovearea(int32 (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int32 type, ...);
int32 map_foreachincell(int32 (*func)(struct block_list*,va_list), int16 m, int16 x, int16 y, int32 type, ...);
int32 map_foreachinpath(int32 (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int32 length, int32 type, ...);
int32 map_foreachindir(int32 (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int32 length, int32 offset, int32 type, ...);
int32 map_foreachinmap(int32 (*func)(struct block_list*,va_list), int16 m, int32 type, ...);
//blocklist nb in one cell
int32 map_count_oncell(int16 m,int16 x,int16 y,int32 type,int32 flag);
struct skill_unit *map_find_skill_unit_oncell(struct block_list *,int16 x,int16 y,uint16 skill_id,struct skill_unit *, int32 flag);
// search and creation
int32 map_get_new_object_id(void);
int32 map_search_freecell(struct block_list *src, int16 m, int16 *x, int16 *y, int16 rx, int16 ry, int32 flag, int32 tries = 50);
bool map_closest_freecell(int16 m, int16 *x, int16 *y, int32 type, int32 flag);
bool map_nearby_freecell(int16 m, int16 &x, int16 &y, int32 type, int32 flag);
//
int32 map_quit(map_session_data *);
// npc
bool map_addnpc(int16 m,struct npc_data *);

// map item
TIMER_FUNC(map_clearflooritem_timer);
TIMER_FUNC(map_removemobs_timer);
void map_clearflooritem(struct block_list* bl);
int32 map_addflooritem(struct item *item, int32 amount, int16 m, int16 x, int16 y, int32 first_charid, int32 second_charid, int32 third_charid, int32 flags, uint16 mob_id, bool canShowEffect = false, enum directions dir = DIR_MAX, int32 type = BL_NUL);

// instances
int32 map_addinstancemap(int32 src_m, int32 instance_id, bool no_mapflag);
int32 map_delinstancemap(int32 m);
void map_data_copyall(void);
void map_data_copy(struct map_data *dst_map, struct map_data *src_map);

// player to map session
void map_addnickdb(int32 charid, const char* nick);
void map_delnickdb(int32 charid, const char* nick);
void map_reqnickdb(map_session_data* sd,int32 charid);
const char* map_charid2nick(int32 charid);
map_session_data* map_charid2sd(int32 charid);

map_session_data * map_id2sd(int32 id);
struct mob_data * map_id2md(int32 id);
struct npc_data * map_id2nd(int32 id);
struct homun_data* map_id2hd(int32 id);
struct s_mercenary_data* map_id2mc(int32 id);
struct pet_data* map_id2pd(int32 id);
struct s_elemental_data* map_id2ed(int32 id);
struct chat_data* map_id2cd(int32 id);
struct block_list * map_id2bl(int32 id);
bool map_blid_exists( int32 id );

#define map_id2index(id) map[(id)].index
const char* map_mapid2mapname(int32 m);
int16 map_mapindex2mapid(uint16 mapindex);
int16 map_mapname2mapid(const char* name);
int32 map_mapname2ipport(uint16 name, uint32* ip, uint16* port);
int32 map_setipport(uint16 map, uint32 ip, uint16 port);
int32 map_eraseipport(uint16 map, uint32 ip, uint16 port);
int32 map_eraseallipport(void);
void map_addiddb(struct block_list *);
void map_deliddb(struct block_list *bl);
void map_foreachpc(int32 (*func)(map_session_data* sd, va_list args), ...);
void map_foreachmob(int32 (*func)(struct mob_data* md, va_list args), ...);
void map_foreachnpc(int32 (*func)(struct npc_data* nd, va_list args), ...);
void map_foreachregen(int32 (*func)(struct block_list* bl, va_list args), ...);
void map_foreachiddb(int32 (*func)(struct block_list* bl, va_list args), ...);
map_session_data * map_nick2sd(const char* nick, bool allow_partial);
struct mob_data * map_getmob_boss(int16 m);
struct mob_data * map_id2boss(int32 id);

// reload config file looking only for npcs
void map_reloadnpc(bool clear);

void map_remove_questinfo(int32 m, struct npc_data *nd);

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

int32 map_check_dir(int32 s_dir,int32 t_dir);
uint8 map_calc_dir(struct block_list *src,int16 x,int16 y);
uint8 map_calc_dir_xy(int16 srcx, int16 srcy, int16 x, int16 y, uint8 srcdir);
int32 map_random_dir(struct block_list *bl, int16 *x, int16 *y); // [Skotlex]

int32 cleanup_sub(struct block_list *bl, va_list ap);

int32 map_delmap(char* mapname);
void map_flags_init(void);

bool map_iwall_exist(const char* wall_name);
bool map_iwall_set(int16 m, int16 x, int16 y, int32 size, int8 dir, bool shootable, const char* wall_name);
void map_iwall_get(map_session_data *sd);
bool map_iwall_remove(const char *wall_name);

int32 map_addmobtolist(uint16 m, struct spawn_data *spawn);	// [Wizputer]
void map_spawnmobs(int16 m); // [Wizputer]
void map_removemobs(int16 m); // [Wizputer]
void map_addmap2db(struct map_data *m);
void map_removemapdb(struct map_data *m);

void map_skill_damage_add(struct map_data *m, uint16 skill_id, union u_mapflag_args *args);
void map_skill_duration_add(struct map_data *mapd, uint16 skill_id, uint16 per);

enum e_mapflag map_getmapflag_by_name(char* name);
bool map_getmapflag_name(enum e_mapflag mapflag, char* output);
int32 map_getmapflag_sub(int16 m, enum e_mapflag mapflag, union u_mapflag_args *args);
bool map_setmapflag_sub(int16 m, enum e_mapflag mapflag, bool status, union u_mapflag_args *args);
#define map_getmapflag(m, mapflag) map_getmapflag_sub(m, mapflag, nullptr)
#define map_setmapflag(m, mapflag, status) map_setmapflag_sub(m, mapflag, status, nullptr)

#define CHK_ELEMENT(ele) ((ele) > ELE_NONE && (ele) < ELE_MAX) /// Check valid Element
#define CHK_ELEMENT_LEVEL(lv) ((lv) >= 1 && (lv) <= MAX_ELE_LEVEL) /// Check valid element level
#define CHK_RACE(race) ((race) > RC_NONE_ && (race) < RC_MAX) /// Check valid Race
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
typedef map_session_data TBL_PC;
typedef struct npc_data         TBL_NPC;
typedef struct mob_data         TBL_MOB;
typedef struct flooritem_data   TBL_ITEM;
typedef struct chat_data        TBL_CHAT;
typedef struct skill_unit       TBL_SKILL;
typedef struct pet_data         TBL_PET;
typedef struct homun_data       TBL_HOM;
typedef struct s_mercenary_data   TBL_MER;
typedef struct s_elemental_data	TBL_ELEM;

#define BL_CAST(type_, bl) \
	( ((bl) == nullptr || (bl)->type != (type_)) ? static_cast<T ## type_ *>(nullptr) : static_cast<T ## type_ *>(bl) )

extern int32 db_use_sqldbs;

#ifndef ONLY_CONSTANTS
#include <common/sql.hpp>

extern Sql* mmysql_handle;
extern Sql* qsmysql_handle;
extern Sql* logmysql_handle;
#endif

extern char barter_table[32];
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
extern char partybookings_table[32];
extern char roulette_table[32];
extern char guild_storage_log_table[32];

void do_shutdown(void);

#endif /* MAP_HPP */
