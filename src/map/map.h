// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAP_H_
#define _MAP_H_

#ifndef _CBASETYPES_H_
#include "../common/cbasetypes.h"
#endif
#include "../common/mmo.h"
#include "../common/mapindex.h"
#include "../common/db.h"

#include "itemdb.h" // MAX_ITEMGROUP
#include "status.h" // SC_MAX

#include <stdarg.h>

//Uncomment to enable the Cell Stack Limit mod.
//It's only config is the battle_config cell_stack_limit.
//Only chars affected are those defined in BL_CHAR (mobs and players currently)
//#define CELL_NOSTACK

//Uncomment to enable circular area checks.
//By default, all range checks in Aegis are of Square shapes, so a weapon range
//  of 10 allows you to attack from anywhere within a 21x21 area.
//Enabling this changes such checks to circular checks, which is more realistic,
//  but is not the official behaviour.
//#define CIRCULAR_AREA

#define MAX_NPC_PER_MAP 512
#define BLOCK_SIZE 8
#define AREA_SIZE battle_config.area_size
#define DAMAGELOG_SIZE 30
#define LOOTITEM_SIZE 10
#define MAX_SKILL_LEVEL 100
#define MAX_SKILLUNITGROUP 25
#define MAX_SKILLUNITGROUPTICKSET 25
#define MAX_SKILLTIMERSKILL 15
#define MAX_MOBSKILL 40
#define MAX_MOB_LIST_PER_MAP 128
#define MAX_EVENTQUEUE 2
#define MAX_EVENTTIMER 32
#define NATURAL_HEAL_INTERVAL 500
#define MAX_FLOORITEM 500000
#define MAX_LEVEL 99
#define MAX_WALKPATH 32
#define MAX_DROP_PER_MAP 48
#define MAX_IGNORE_LIST 20 // official is 14
#define MAX_VENDING 12
#define MOBID_EMPERIUM 1288
#define MOBID_BARRICADEB 1905
#define MOBID_BARRICADEA 1906 // Undestruble

#define MAX_PC_BONUS 10
#define MAX_DUEL 1024

//The following system marks a different job ID system used by the map server,
//which makes a lot more sense than the normal one. [Skotlex]
//
//These marks the "level" of the job.
#define JOBL_2_1 0x100 //256
#define JOBL_2_2 0x200 //512
#define JOBL_2 0x300

#define JOBL_UPPER 0x1000 //4096
#define JOBL_BABY 0x2000  //8192

//for filtering and quick checking.
#define MAPID_UPPERMASK 0x0fff
#define MAPID_BASEMASK 0x00ff
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
};

//Max size for inputs to Graffiti, Talkie Box and Vending text prompts
#define MESSAGE_SIZE (79 + 1)
//String length you can write in the 'talking box'
#define CHATBOX_SIZE (70 + 1)
//Chatroom-related string sizes
#define CHATROOM_TITLE_SIZE (36 + 1)
#define CHATROOM_PASS_SIZE (8 + 1)
//Max allowed chat text length
#define CHAT_SIZE_MAX 150

#define DEFAULT_AUTOSAVE_INTERVAL 5*60*1000

//Specifies maps where players may hit each other
#define map_flag_vs(m) (map[m].flag.pvp || map[m].flag.gvg_dungeon || map[m].flag.gvg || (agit_flag && map[m].flag.gvg_castle))
//Specifies maps that have special GvG/WoE restrictions
#define map_flag_gvg(m) (map[m].flag.gvg || (agit_flag && map[m].flag.gvg_castle))
//Specifies if the map is tagged as GvG/WoE (regardless of agit_flag status)
#define map_flag_gvg2(m) (map[m].flag.gvg || map[m].flag.gvg_castle)

//This stackable implementation does not means a BL can be more than one type at a time, but it's 
//meant to make it easier to check for multiple types at a time on invocations such as map_foreach* calls [Skotlex]
enum bl_type { 
	BL_NUL   = 0x000,
	BL_PC    = 0x001,
	BL_MOB   = 0x002,
	BL_PET   = 0x004,
	BL_HOM   = 0x008,
	BL_ITEM  = 0x010,
	BL_SKILL = 0x020,
	BL_NPC   = 0x040,
	BL_CHAT  = 0x080,
};

//For common mapforeach calls. Since pets cannot be affected, they aren't included here yet.
#define BL_CHAR (BL_PC|BL_MOB|BL_HOM)
#define BL_ALL 0xfff

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
	RC_MAX
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
	ATF_LONG=0x08
};

struct block_list {
	struct block_list *next,*prev;
	int id;
	short m,x,y;
	enum bl_type type;
};

struct walkpath_data {
	unsigned char path_len,path_pos;
	unsigned char path[MAX_WALKPATH];
};
struct shootpath_data {
	int rx,ry,len;
	int x[MAX_WALKPATH];
	int y[MAX_WALKPATH];
};

struct skill_timerskill {
	int timer;
	int src_id;
	int target_id;
	int map;
	short x,y;
	short skill_id,skill_lv;
	int type; // a BF_ type (NOTE: some places use this as general-purpose storage...)
	int flag;
};

struct skill_unit_group;
struct skill_unit {
	struct block_list bl;

	struct skill_unit_group *group;

	int limit;
	int val1,val2;
	short alive,range;
};

struct skill_unit_group {
	int src_id;
	int party_id;
	int guild_id;
	int map;
	int target_flag; //Holds BCT_* flag for battle_check_target
	int bl_flag;	//Holds BL_* flag for map_foreachin* functions
	unsigned int tick;
	int limit,interval;

	short skill_id,skill_lv;
	int val1,val2,val3;
	char *valstr;
	int unit_id;
	int group_id;
	int unit_count,alive_count;
	struct skill_unit *unit;
	struct {
		unsigned ammo_consume : 1;
		unsigned magic_power : 1;
		unsigned song_dance : 2; //0x1 Song/Dance, 0x2 Ensemble
	} state;
};
struct skill_unit_group_tickset {
	unsigned int tick;
	int id;
};

struct unit_data {
	struct block_list *bl;
	struct walkpath_data walkpath;
	struct skill_timerskill *skilltimerskill[MAX_SKILLTIMERSKILL];
	struct skill_unit_group *skillunit[MAX_SKILLUNITGROUP];
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	short attacktarget_lv;
	short to_x,to_y;
	short skillx,skilly;
	short skillid,skilllv;
	int   skilltarget;
	int   skilltimer;
	int   target;
	int   attacktimer;
	int   walktimer;
	int	chaserange;
	unsigned int attackabletime;
	unsigned int canact_tick;
	unsigned int canmove_tick;
	uint8 dir;
	unsigned char walk_count;
	struct {
		unsigned change_walk_target : 1 ;
		unsigned skillcastcancel : 1 ;
		unsigned attack_continue : 1 ;
		unsigned walk_easy : 1 ;
		unsigned running : 1;
		unsigned speed_changed : 1;
	} state;
};

//Basic damage info of a weapon
//Required because players have two of these, one in status_data and another
//for their left hand weapon.
struct weapon_atk {
	unsigned short atk, atk2;
	unsigned short range;
	unsigned char ele;
};

//For holding basic status (which can be modified by status changes)
struct status_data {
	unsigned int
		hp, sp,
		max_hp, max_sp;
	unsigned short
		str, agi, vit, int_, dex, luk,
		batk,
		matk_min, matk_max,
		speed,
		amotion, adelay, dmotion,
		mode;
	short 
		hit, flee, cri, flee2,
		def2, mdef2,
		aspd_rate;
	unsigned char
		def_ele, ele_lv,
		size, race;
	signed char
		def, mdef;
	struct weapon_atk rhw, lhw; //Right Hand/Left Hand Weapon.
};

struct script_reg {
	int index;
	int data;
};
struct script_regstr {
	int index;
	char* data;
};

struct status_change_entry {
	int timer;
	int val1,val2,val3,val4;
};

struct status_change {
	unsigned int option;// effect state (bitfield)
	unsigned int opt3;// skill state (bitfield)
	unsigned short opt1;// body state
	unsigned short opt2;// health state (bitfield)
	unsigned char count;
	//TODO: See if it is possible to implement the following SC's without requiring extra parameters while the SC is inactive.
	unsigned char jb_flag; //Joint Beat type flag
	unsigned short mp_matk_min, mp_matk_max; //Previous matk min/max for ground spells (Amplify magic power)
	int sg_id; //ID of the previous Storm gust that hit you
	unsigned char sg_counter; //Storm gust counter (previous hits from storm gust)
	struct status_change_entry *data[SC_MAX];
};

struct s_vending {
	short index;
	short amount;
	unsigned int value;
};

struct weapon_data {
	int atkmods[3];
	// all the variables except atkmods get zero'ed in each call of status_calc_pc
	// NOTE: if you want to add a non-zeroed variable, you need to update the memset call
	//  in status_calc_pc as well! All the following are automatically zero'ed. [Skotlex]
	int overrefine;
	int star;
	int ignore_def_ele;
	int ignore_def_race;
	int def_ratio_atk_ele;
	int def_ratio_atk_race;
	int addele[ELE_MAX];
	int addrace[RC_MAX];
	int addrace2[RC_MAX];
	int addsize[3];

	struct drain_data {
		short rate;
		short per;
		short value;
		unsigned type:1;
	} hp_drain[RC_MAX], sp_drain[RC_MAX];

	struct {
		short class_, rate;
	}	add_dmg[MAX_PC_BONUS];
};

struct view_data {
	unsigned short
	  	class_,
		weapon,
		shield, //Or left-hand weapon.
		head_top,
		head_mid,
		head_bottom,
		hair_style,
		hair_color,
		cloth_color;
	char sex;
	unsigned dead_sit : 2;
};

//Additional regen data that only players have.
struct regen_data_sub {
	unsigned short
		hp,sp;

	//tick accumulation before healing.
	struct {
		unsigned int hp,sp;
	} tick;
	
	//Regen rates (where every 1 means +100% regen)
	struct {
		unsigned char hp,sp;
	} rate;
};

struct regen_data {

	unsigned short flag; //Marks what stuff you may heal or not.
	unsigned short
		hp,sp,shp,ssp;

	//tick accumulation before healing.
	struct {
		unsigned int hp,sp,shp,ssp;
	} tick;
	
	//Regen rates (where every 1 means +100% regen)
	struct {
		unsigned char
		hp,sp,shp,ssp;
	} rate;
	
	struct {
		unsigned walk:1; //Can you regen even when walking?
		unsigned gc:1;	//Tags when you should have double regen due to GVG castle
		unsigned overweight :2; //overweight state (1: 50%, 2: 90%)
		unsigned block :2; //Block regen flag (1: Hp, 2: Sp)
	} state;

	//skill-regen, sitting-skill-regen (since not all chars with regen need it)
	struct regen_data_sub *sregen, *ssregen;
};

struct party_member_data {
	struct map_session_data *sd;
	unsigned int hp; //For HP,x,y refreshing.
	unsigned short x, y;
};

struct party_data {
	struct party party;
	struct party_member_data data[MAX_PARTY];
	uint8 itemc; //For item distribution, position of last picker in party
	struct {
		unsigned monk : 1; //There's at least one monk in party?
		unsigned sg : 1;	//There's at least one Star Gladiator in party?
		unsigned snovice :1; //There's a Super Novice
		unsigned tk : 1; //There's a taekwon
	} state;
};

struct npc_data;
struct pet_db;
struct homunculus_db;	//[orn]
struct item_data;
struct square;

struct map_session_data {
	struct block_list bl;
	struct unit_data ud;
	struct view_data vd;
	struct status_data base_status, battle_status;
	struct status_change sc;
	struct regen_data regen;
	struct regen_data_sub sregen, ssregen;
	//NOTE: When deciding to add a flag to state or special_state, take into consideration that state is preserved in
	//status_calc_pc, while special_state is recalculated in each call. [Skotlex]
	struct {
		unsigned active : 1; //Marks active player (not active is logging in/out, or changing map servers)
		unsigned menu_or_input : 1;// if a script is waiting for feedback from the player
		unsigned dead_sit : 2;
		unsigned lr_flag : 2;
		unsigned connect_new : 1;
		unsigned arrow_atk : 1;
		unsigned skill_flag : 1;
		unsigned gangsterparadise : 1;
		unsigned rest : 1;
		unsigned storage_flag : 2; //0: closed, 1: Normal Storage open, 2: guild storage open [Skotlex]
		unsigned snovice_call_flag : 2; //Summon Angel (stage 1~3)
		unsigned snovice_dead_flag : 2; //Explosion spirits on death: 0 off, 1 active, 2 used.
		unsigned abra_flag : 1; // Abracadabra bugfix by Aru
		unsigned autotrade : 1;	//By Fantik
		unsigned reg_dirty : 3; //By Skotlex (marks whether registry variables have been saved or not yet)
		unsigned showdelay :1;
		unsigned showexp :1;
		unsigned showzeny :1;
		unsigned mainchat :1; //[LuzZza]
		unsigned noask :1; // [LuzZza]
		unsigned trading :1; //[Skotlex] is 1 only after a trade has started.
		unsigned deal_locked :2; //1: Clicked on OK. 2: Clicked on TRADE
		unsigned monster_ignore :1; // for monsters to ignore a character [Valaris] [zzo]
		unsigned size :2; // for tiny/large types
		unsigned night :1; //Holds whether or not the player currently has the SI_NIGHT effect on. [Skotlex]
		unsigned blockedmove :1;
		unsigned using_fake_npc :1;
		unsigned rewarp :1; //Signals that a player should warp as soon as he is done loading a map. [Skotlex]
		unsigned killer : 1;
		unsigned killable : 1;
		unsigned doridori : 1;
		unsigned ignoreAll : 1;
		unsigned short autoloot;
		unsigned short autolootid; // [Zephyrus]
		unsigned noks : 3; // [Zeph Kill Steal Protection]
		bool changemap;
		struct guild *gmaster_flag;
	} state;
	struct {
		unsigned char no_weapon_damage, no_magic_damage, no_misc_damage;
		unsigned restart_full_recover : 1;
		unsigned no_castcancel : 1;
		unsigned no_castcancel2 : 1;
		unsigned no_sizefix : 1;
		unsigned no_gemstone : 1;
		unsigned intravision : 1; // Maya Purple Card effect [DracoRPG]
		unsigned perfect_hiding : 1; // [Valaris]
		unsigned no_knockback : 1;
		unsigned bonus_coma : 1;
	} special_state;
	int login_id1, login_id2;
	unsigned short class_;	//This is the internal job ID used by the map server to simplify comparisons/queries/etc. [Skotlex]

	int packet_ver;  // 5: old, 6: 7july04, 7: 13july04, 8: 26july04, 9: 9aug04/16aug04/17aug04, 10: 6sept04, 11: 21sept04, 12: 18oct04, 13: 25oct04 ... 18
	struct mmo_charstatus status;
	struct registry save_reg;
	
	struct item_data* inventory_data[MAX_INVENTORY]; // direct pointers to itemdb entries (faster than doing item_id lookups)
	short equip_index[11];
	unsigned int weight,max_weight;
	int cart_weight,cart_num;
	int fd;
	unsigned short mapindex;
	unsigned short prev_speed,prev_adelay;
	unsigned char head_dir; //0: Look forward. 1: Look right, 2: Look left.
	unsigned int client_tick;
	int npc_id,areanpc_id,npc_shopid;
	int npc_item_flag; //Marks the npc_id with which you can use items during interactions with said npc (see script command enable_itemuse)
	int npc_menu;
	int npc_amount;
	struct script_state *st;
	char npc_str[CHATBOX_SIZE]; // for passing npc input box text to script engine
	int npc_timer_id; //For player attached npc timers. [Skotlex]
	unsigned int chatID;
	time_t idletime;

	struct{
		char name[NAME_LENGTH];
	} ignore[MAX_IGNORE_LIST];

	int followtimer; // [MouseJstr]
	int followtarget;

	time_t emotionlasttime; // to limit flood with emotion packets

	short skillitem,skillitemlv;
	short skillid_old,skilllv_old;
	short skillid_dance,skilllv_dance;
	char blockskill[MAX_SKILL];	// [celest]
	int cloneskill_id;
	int menuskill_id, menuskill_val;

	int invincible_timer;
	unsigned int canlog_tick;
	unsigned int canuseitem_tick;	// [Skotlex]
	unsigned int cantalk_tick;
	unsigned int cansendmail_tick; // [Mail System Flood Protection]
	unsigned int ks_floodprotect_tick; // [Kill Steal Protection]

	short weapontype1,weapontype2;
	short disguise; // [Valaris]

	struct weapon_data right_weapon, left_weapon;
	
	// here start arrays to be globally zeroed at the beginning of status_calc_pc()
	int param_bonus[6],param_equip[6]; //Stores card/equipment bonuses.
	int subele[ELE_MAX];
	int subrace[RC_MAX];
	int subrace2[RC_MAX];
	int subsize[3];
	int reseff[SC_COMMON_MAX-SC_COMMON_MIN+1];
	int weapon_coma_ele[ELE_MAX];
	int weapon_coma_race[RC_MAX];
	int weapon_atk[16];
	int weapon_atk_rate[16];
	int arrow_addele[ELE_MAX];
	int arrow_addrace[RC_MAX];
	int arrow_addsize[3];
	int magic_addele[ELE_MAX];
	int magic_addrace[RC_MAX];
	int magic_addsize[3];
	int critaddrace[RC_MAX];
	int expaddrace[RC_MAX];
	int ignore_mdef[RC_MAX];
	int itemgrouphealrate[MAX_ITEMGROUP];
	short sp_gain_race[RC_MAX];
	// zeroed arrays end here.
	// zeroed structures start here
	struct s_autospell{
		short id, lv, rate, card_id, flag;
	} autospell[15], autospell2[15];
	struct s_addeffect{
		short id, rate, arrow_rate;
		unsigned char flag;
	} addeff[MAX_PC_BONUS], addeff2[MAX_PC_BONUS];
	struct { //skillatk raises bonus dmg% of skills, skillheal increases heal%, skillblown increases bonus blewcount for some skills.
		unsigned short id;
		short val;
	} skillatk[MAX_PC_BONUS], skillheal[5], skillblown[MAX_PC_BONUS], skillcast[MAX_PC_BONUS];
	struct {
		short value;
		int rate;
		int tick;
	} hp_loss, sp_loss, hp_regen, sp_regen;
	struct {
		short class_, rate;
	}	add_def[MAX_PC_BONUS], add_mdef[MAX_PC_BONUS],
		add_mdmg[MAX_PC_BONUS];
	struct s_add_drop { 
		short id, group;
		int race, rate;
	} add_drop[MAX_PC_BONUS];
	struct {
		int nameid;
		int rate;
	} itemhealrate[MAX_PC_BONUS];
	// zeroed structures end here
	// manually zeroed structures start here.
	struct s_autoscript {
		unsigned short rate, flag;
		struct script_code *script;
	} autoscript[10], autoscript2[10]; //Auto script on attack, when attacked
	// manually zeroed structures end here.
	// zeroed vars start here.
	int arrow_atk,arrow_ele,arrow_cri,arrow_hit;
	int nsshealhp,nsshealsp;
	int critical_def,double_rate;
	int long_attack_atk_rate; //Long range atk rate, not weapon based. [Skotlex]
	int near_attack_def_rate,long_attack_def_rate,magic_def_rate,misc_def_rate;
	int ignore_mdef_ele;
	int ignore_mdef_race;
	int perfect_hit;
	int perfect_hit_add;
	int get_zeny_rate;
	int get_zeny_num; //Added Get Zeny Rate [Skotlex]
	int double_add_rate;
	int short_weapon_damage_return,long_weapon_damage_return;
	int magic_damage_return; // AppleGirl Was Here
	int random_attack_increase_add,random_attack_increase_per; // [Valaris]
	int break_weapon_rate,break_armor_rate;
	int crit_atk_rate;
	int classchange; // [Valaris]
	int speed_add_rate, aspd_add;
	unsigned int setitem_hash, setitem_hash2; //Split in 2 because shift operations only work on int ranges. [Skotlex]
	
	short splash_range, splash_add_range;
	short add_steal_rate;
	short sp_gain_value, hp_gain_value;
	short sp_vanish_rate;
	short sp_vanish_per;	
	unsigned short unbreakable;	// chance to prevent ANY equipment breaking [celest]
	unsigned short unbreakable_equip; //100% break resistance on certain equipment
	unsigned short unstripable_equip;

	// zeroed vars end here.

	int castrate,delayrate,hprate,sprate,dsprate;
	int atk_rate;
	int speed_rate,hprecov_rate,sprecov_rate;
	int matk_rate;
	int critical_rate,hit_rate,flee_rate,flee2_rate,def_rate,def2_rate,mdef_rate,mdef2_rate;

	int itemid;
	short itemindex;	//Used item's index in sd->inventory [Skotlex]

	short catch_target_class; // pet catching, stores a pet class to catch (short now) [zzo]

	short spiritball, spiritball_old;
	int spirit_timer[MAX_SKILL_LEVEL];

	unsigned char potion_success_counter; //Potion successes in row counter
	unsigned char mission_count; //Stores the bounty kill count for TK_MISSION
	short mission_mobid; //Stores the target mob_id for TK_MISSION
	int die_counter; //Total number of times you've died
	int devotion[5]; //Stores the account IDs of chars devoted to.
	int reg_num; //Number of registries (type numeric)
	int regstr_num; //Number of registries (type string)

	struct script_reg *reg;
	struct script_regstr *regstr;

	int trade_partner;
	struct { 
		struct {
			short index, amount;
		} item[10];
		int zeny, weight;
	} deal;

	int party_invite,party_invite_account;

	int guild_invite,guild_invite_account;
	int guild_emblem_id,guild_alliance,guild_alliance_account;
	short guild_x,guild_y; // For guildmate position display. [Skotlex] should be short [zzo]
	int guildspy; // [Syrus22]
	int partyspy; // [Syrus22]

	int vender_id;
	int vend_num;
	char message[MESSAGE_SIZE];
	struct s_vending vending[MAX_VENDING];

	struct pet_data *pd;
	struct homun_data *hd;	// [blackhole89]

	struct{
		int  m; //-1 - none, other: map index corresponding to map name.
		unsigned short index; //map index
	}feel_map[3];// 0 - Sun; 1 - Moon; 2 - Stars
	short hate_mob[3];

	int pvp_timer;
	short pvp_point;
	unsigned short pvp_rank, pvp_lastusers;
	unsigned short pvp_won, pvp_lost;

	char eventqueue[MAX_EVENTQUEUE][50];
	int eventtimer[MAX_EVENTTIMER];
	unsigned short eventcount; // [celest]

	unsigned char change_level; // [celest]

	char fakename[NAME_LENGTH]; // fake names [Valaris]

	int duel_group; // duel vars [LuzZza]
	int duel_invite;

	char away_message[128]; // [LuzZza]

	int cashPoints, kafraPoints;

	// Auction System [Zephyrus]
	struct {
		int index, amount;
	} auction;

	// Mail System [Zephyrus]
	struct {
		short nameid;
		int index, amount, zeny;
		struct mail_data inbox;
	} mail;
};

struct npc_timerevent_list {
	int timer,pos;
};
struct npc_label_list {
	char name[NAME_LENGTH];
	int pos;
};
struct npc_item_list {
	unsigned int nameid,value;
};
struct npc_data {
	struct block_list bl;
	struct unit_data  ud; //Because they need to be able to move....
	struct view_data *vd;
	struct status_change sc; //They can't have status changes, but.. they want the visual opt values.
	struct npc_data *master_nd;
	short class_;
	short speed;
	char name[NAME_LENGTH+1];// display name
	char exname[NAME_LENGTH+1];// unique npc name
	int chat_id;
	unsigned int next_walktime;

	void* chatdb; // pointer to a npc_parse struct (see npc_chat.c)
	enum npc_subtype subtype;
	union {
		struct {
			struct script_code *script;
			short xs,ys; // OnTouch area radius
			int guild_id;
			int timer,timerid,timeramount,rid;
			unsigned int timertick;
			struct npc_timerevent_list *timer_event;
			int label_list_num;
			struct npc_label_list *label_list;
			int src_id;
		} scr;
		struct {
			struct npc_item_list* shop_item;
			int count;
		} shop;
		struct {
			short xs,ys; // OnTouch area radius
			short x,y; // destination coords
			unsigned short mapindex; // destination map
		} warp;
	} u;
};

//For quick linking to a guardian's info. [Skotlex]
struct guardian_data {
	int number; //0-MAX_GUARDIANS-1 = Guardians. MAX_GUARDIANS = Emperium.
	int guild_id;
	int emblem_id;
	int guardup_lv; //Level of GD_GUARDUP skill.
	char guild_name[NAME_LENGTH];
	struct guild_castle* castle;
};

// Mob List Held in memory for Dynamic Mobs [Wizputer]
// Expanded to specify all mob-related spawn data by [Skotlex]
struct spawn_data {
	short class_; //Class, used because a mob can change it's class
	unsigned short m,x,y;	//Spawn information (map, point, spawn-area around point)
	signed short xs,ys;
	unsigned short num; //Number of mobs using this structure.
	unsigned short skip; //Number of mobs to skip when spawning them (for mob_remove_damageed: no)
	unsigned int level; //Custom level.
	unsigned int delay1,delay2; //Min delay before respawning after spawn/death
	struct {
		unsigned size :2; //Holds if mob has to be tiny/large
		unsigned ai :2;	//Holds if mob is special ai.
	} state;
	char name[NAME_LENGTH],eventname[50]; //Name/event
};


struct mob_data {
	struct block_list bl;
	struct unit_data  ud;
	struct view_data *vd;
	struct status_data status, *base_status; //Second one is in case of leveling up mobs, or tiny/large mobs.
	struct status_change sc;
	struct mob_db *db;	//For quick data access (saves doing mob_db(md->class_) all the time) [Skotlex]
	struct barricade_data *barricade;
	char name[NAME_LENGTH];
	struct {
		unsigned size : 2; //Small/Big monsters.
		unsigned cached : 1; //Cached mobs for dynamic mob unloading [Skotlex]
		unsigned ai : 2; //Special ai for summoned monsters.
							//0: Normal mob.
							//1: Standard summon, attacks mobs.
							//2: Alchemist Marine Sphere
							//3: Alchemist Summon Flora
	} special_state; //Special mob information that does not needs to be zero'ed on mob respawn.
	struct {
		unsigned skillstate : 8;
		unsigned aggressive : 1; //Signals whether the mob AI is in aggressive mode or reactive mode. [Skotlex]
		unsigned char steal_flag; //number of steal tries (to prevent steal exploit on mobs with few items) [Lupus]
		unsigned steal_coin_flag : 1;
		unsigned soul_change_flag : 1; // Celest
		unsigned alchemist: 1;
		unsigned no_random_walk: 1;
		unsigned killer: 1;
		unsigned spotted: 1;
		unsigned char attacked_count; //For rude attacked.
		int provoke_flag; // Celest
	} state;
	struct guardian_data* guardian_data; 
	struct {
		int id;
		unsigned int dmg;
		unsigned flag : 1; //0: Normal. 1: Homunc exp
	} dmglog[DAMAGELOG_SIZE];
	struct spawn_data *spawn; //Spawn data.
	struct item *lootitem;
	short class_;
	unsigned int tdmg; //Stores total damage given to the mob, for exp calculations. [Skotlex]
	int level;
	int target_id,attacked_id;

	unsigned int next_walktime,last_thinktime,last_linktime,last_pcneartime;
	short move_fail_count;
	short lootitem_count;
	short min_chase;
	
	int deletetimer;
	int master_id,master_dist;

	struct npc_data *nd;
	unsigned short callback_flag;
	
	short skillidx;
	unsigned int skilldelay[MAX_MOBSKILL];
	char npc_event[50];
};

/* [blackhole89] */
struct homun_data {
	struct block_list bl;
	struct unit_data  ud;
	struct view_data *vd;
	struct status_data base_status, battle_status;
	struct status_change sc;
	struct regen_data regen;
	struct s_homunculus_db *homunculusDB;	//[orn]
	struct s_homunculus homunculus ;	//[orn]

	struct map_session_data *master; //pointer back to its master
	int hungry_timer;	//[orn]
	unsigned int exp_next;
	char blockskill[MAX_SKILL];	// [orn]
};

struct pet_data {
	struct block_list bl;
	struct unit_data ud;
	struct view_data vd;
	struct s_pet pet;
	struct status_data status;
	struct mob_db *db;
	struct s_pet_db *petDB;
	int pet_hungry_timer;
	int target_id;
	struct {
		unsigned skillbonus : 1;
	} state;
	int move_fail_count;
	unsigned int next_walktime,last_thinktime;
	short rate_fix;	//Support rate as modified by intimacy (1000 = 100%) [Skotlex]

	struct pet_recovery { //Stat recovery
		unsigned short type;	//Status Change id
		unsigned short delay; //How long before curing (secs).
		int timer;
	} *recovery; //[Valaris] / Reimplemented by [Skotlex]

	struct pet_bonus {
		unsigned short type; //bStr, bVit?
		unsigned short val;	//Qty
		unsigned short duration; //in secs
		unsigned short delay;	//Time before recasting (secs)
		int timer;
	} *bonus; //[Valaris] / Reimplemented by [Skotlex]

	struct pet_skill_attack { //Attack Skill
		unsigned short id;
		unsigned short lv;
		unsigned short div_; //0 = Normal skill. >0 = Fixed damage (lv), fixed div_.
		unsigned short rate; //Base chance of skill ocurrance (10 = 10% of attacks)
		unsigned short bonusrate; //How being 100% loyal affects cast rate (10 = At 1000 intimacy->rate+10%
	} *a_skill;	//[Skotlex]

	struct pet_skill_support { //Support Skill
		unsigned short id;
		unsigned short lv;
		unsigned short hp; //Max HP% for skill to trigger (50 -> 50% for Magnificat)
		unsigned short sp; //Max SP% for skill to trigger (100 = no check)
		unsigned short delay; //Time (secs) between being able to recast.
		int timer;
	} *s_skill;	//[Skotlex]

	struct pet_loot {
		struct item *item;
		unsigned short count;
		unsigned short weight;
		unsigned short max;
	} *loot; //[Valaris] / Rewritten by [Skotlex]

	struct map_session_data *msd;
};

struct flooritem_data {
	struct block_list bl;
	unsigned char subx,suby;
	int cleartimer;
	int first_get_charid,second_get_charid,third_get_charid;
	unsigned int first_get_tick,second_get_tick,third_get_tick;
	struct item item_data;
};

struct chat_data {
	struct block_list bl;            // data for this map object
	char title[CHATROOM_TITLE_SIZE]; // room title 
	char pass[CHATROOM_PASS_SIZE];   // password
	bool pub;                        // private/public flag
	uint8 users;                     // current user count
	uint8 limit;                     // join limit
	uint8 trigger;                   // number of users needed to trigger event
	struct map_session_data* usersd[20];
	struct block_list* owner;
	char npc_event[50];
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
	SP_ADD_MONSTER_DROP_ITEM,SP_DEF_RATIO_ATK_ELE,SP_DEF_RATIO_ATK_RACE,SP_ADD_SPEED, // 1047-1050
	SP_HIT_RATE,SP_FLEE_RATE,SP_FLEE2_RATE,SP_DEF_RATE,SP_DEF2_RATE,SP_MDEF_RATE,SP_MDEF2_RATE, // 1051-1057
	SP_SPLASH_RANGE,SP_SPLASH_ADD_RANGE,SP_AUTOSPELL,SP_HP_DRAIN_RATE,SP_SP_DRAIN_RATE, // 1058-1062
	SP_SHORT_WEAPON_DAMAGE_RETURN,SP_LONG_WEAPON_DAMAGE_RETURN,SP_WEAPON_COMA_ELE,SP_WEAPON_COMA_RACE, // 1063-1066
	SP_ADDEFF2,SP_BREAK_WEAPON_RATE,SP_BREAK_ARMOR_RATE,SP_ADD_STEAL_RATE, // 1067-1070
	SP_MAGIC_DAMAGE_RETURN,SP_RANDOM_ATTACK_INCREASE,SP_ALL_STATS,SP_AGI_VIT,SP_AGI_DEX_STR,SP_PERFECT_HIDE, // 1071-1076
	SP_NO_KNOCKBACK,SP_CLASSCHANGE, // 1077-1078
	SP_HP_DRAIN_VALUE,SP_SP_DRAIN_VALUE, // 1079-1080
	SP_WEAPON_ATK,SP_WEAPON_ATK_RATE, // 1081-1082
	SP_DELAYRATE,SP_HP_DRAIN_RATE_RACE,SP_SP_DRAIN_RATE_RACE, // 1083-1085
	SP_IGNORE_MDEF_RATE, //1086
	
	SP_RESTART_FULL_RECOVER=2000,SP_NO_CASTCANCEL,SP_NO_SIZEFIX,SP_NO_MAGIC_DAMAGE,SP_NO_WEAPON_DAMAGE,SP_NO_GEMSTONE, // 2000-2005
	SP_NO_CASTCANCEL2,SP_NO_MISC_DAMAGE,SP_UNBREAKABLE_WEAPON,SP_UNBREAKABLE_ARMOR, SP_UNBREAKABLE_HELM, // 2006-2010
	SP_UNBREAKABLE_SHIELD, SP_LONG_ATK_RATE, // 2011-2012

	SP_CRIT_ATK_RATE, SP_CRITICAL_ADDRACE, SP_NO_REGEN, SP_ADDEFF_WHENHIT, SP_AUTOSPELL_WHENHIT, // 2013-2017
	SP_SKILL_ATK, SP_UNSTRIPABLE, SP_FREE, // 2018-2020
	SP_SP_GAIN_VALUE, SP_HP_REGEN_RATE, SP_HP_LOSS_RATE, SP_ADDRACE2, SP_HP_GAIN_VALUE, // 2021-2025
	SP_SUBSIZE, SP_HP_DRAIN_VALUE_RACE, SP_ADD_ITEM_HEAL_RATE, SP_SP_DRAIN_VALUE_RACE, SP_EXP_ADDRACE,	// 2026-2030
	SP_SP_GAIN_RACE, SP_SUBRACE2, SP_FREE2,	// 2031-2033
	SP_UNSTRIPABLE_WEAPON,SP_UNSTRIPABLE_ARMOR,SP_UNSTRIPABLE_HELM,SP_UNSTRIPABLE_SHIELD,  // 2034-2037
	SP_INTRAVISION, SP_ADD_MONSTER_DROP_ITEMGROUP, SP_SP_LOSS_RATE, // 2038-2040
	SP_ADD_SKILL_BLOW, SP_SP_VANISH_RATE //2041
	//Before adding new bonuses, reuse the currently free slots:
	//2020 (SP_FREE) (previously SP_ADD_DAMAGE_BY_CLASS)
	//2033 (SP_FREE2) (previously SP_ADDEFF_WHENHIT_SHORT)
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
	LOOK_SHOES
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
		nochat : 1;

#ifdef CELL_NOSTACK
	unsigned char cell_bl; //Holds amount of bls in this cell.
#endif
};

struct barricade_data {
	char npc_event[50];
	short m, x, y, count, amount, dir;
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
	int npc_num;
	int users;
	int barricade_num;
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
		unsigned rain : 1; // [Valaris]
		unsigned indoors : 1; // celest
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
int map_foreachinmovearea(int (*func)(struct block_list*,va_list), struct block_list* center, int range, int dx, int dy, int type, ...);
int map_foreachincell(int (*func)(struct block_list*,va_list), int m, int x, int y, int type, ...);
int map_foreachinpath(int (*func)(struct block_list*,va_list), int m, int x0, int y0, int x1, int y1, int range, int length, int type, ...);
int map_foreachinmap(int (*func)(struct block_list*,va_list), int m, int type, ...);
//block関連に追加
int map_count_oncell(int m,int x,int y,int type);
struct skill_unit *map_find_skill_unit_oncell(struct block_list *,int x,int y,int skill_id,struct skill_unit *);
// 一時的object関連
int map_addobject(struct block_list *);
int map_delobject(int);
int map_delobjectnofree(int id);
void map_foreachobject(int (*)(struct block_list*,va_list),int,...);
int map_search_freecell(struct block_list *src, int m, short *x, short *y, int rx, int ry, int flag);
//
int map_quit(struct map_session_data *);
// npc
bool map_addnpc(int,struct npc_data *);

// 床アイテム関連
int map_clearflooritem_timer(int,unsigned int,int,int);
int map_removemobs_timer(int,unsigned int,int,int);
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
struct map_session_data** map_getallusers(int *users);
void map_foreachpc(int (*func)(DBKey,void*,va_list),...);
void map_foreachmob(int (*func)(DBKey,void*,va_list),...);
int map_foreachiddb(int (*)(DBKey,void*,va_list),...);
struct map_session_data * map_nick2sd(const char*);

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
#define mapit_geteachiddb() mapit_alloc(MAPIT_NORMAL,BL_ALL)

// その他
int map_check_dir(int s_dir,int t_dir);
unsigned char map_calc_dir( struct block_list *src,int x,int y);
int map_random_dir(struct block_list *bl, short *x, short *y); // [Skotlex]

int cleanup_sub(struct block_list *bl, va_list ap);

void map_helpscreen(int flag); // [Valaris]
int map_delmap(char* mapname);

int map_addmobtolist(unsigned short m, struct spawn_data *spawn);	// [Wizputer]
void map_spawnmobs(int); // [Wizputer]
void map_removemobs(int); // [Wizputer]
void do_reconnect_map(void); //Invoked on map-char reconnection [Skotlex]

extern char *INTER_CONF_NAME;
extern char *LOG_CONF_NAME;
extern char *MAP_CONF_NAME;
extern char *BATTLE_CONF_FILENAME;
extern char *ATCOMMAND_CONF_FILENAME;
extern char *CHARCOMMAND_CONF_FILENAME;
extern char *SCRIPT_CONF_NAME;
extern char *MSG_CONF_NAME;
extern char *GRF_PATH_FILENAME;

extern char *map_server_dns;

//Useful typedefs from jA [Skotlex]
typedef struct map_session_data TBL_PC;
typedef struct npc_data         TBL_NPC;
typedef struct mob_data         TBL_MOB;
typedef struct flooritem_data   TBL_ITEM;
typedef struct chat_data        TBL_CHAT;
typedef struct skill_unit       TBL_SKILL;
typedef struct pet_data         TBL_PET;
typedef struct homun_data       TBL_HOM;

#define BL_CAST(type_, bl) \
	( ((bl) == (struct block_list*)NULL || (bl)->type != (type_)) ? (T ## type_ *)NULL : (T ## type_ *)(bl) )


extern int lowest_gm_level;
extern char main_chat_nick[16];

#ifndef TXT_ONLY

#include "../common/sql.h"

extern int db_use_sqldbs;

extern Sql* mmysql_handle;
extern Sql* logmysql_handle;

extern char item_db_db[32];
extern char item_db2_db[32];
extern char mob_db_db[32];
extern char mob_db2_db[32];
extern char char_db[32];

#endif /* not TXT_ONLY */

#endif /* _MAP_H_ */
