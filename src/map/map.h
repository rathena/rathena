// $Id: map.h,v 1.8 2004/09/25 11:39:17 MouseJstr Exp $
#ifndef _MAP_H_
#define _MAP_H_

#include <stdarg.h>
#include "mmo.h"

#define MAX_PC_CLASS (1+6+6+1+6+1+1+1+1+4023)
#define PC_CLASS_BASE 0
#define PC_CLASS_BASE2 (PC_CLASS_BASE + 4001)
#define PC_CLASS_BASE3 (PC_CLASS_BASE2 + 22)
#define MAX_NPC_PER_MAP 512
#define BLOCK_SIZE 8 // Never zero
#define AREA_SIZE battle_config.area_size
#define LOCAL_REG_NUM 16
#define LIFETIME_FLOORITEM 60
#define DAMAGELOG_SIZE 30
#define LOOTITEM_SIZE 10
#define MAX_SKILL_LEVEL 100
#define MAX_STATUSCHANGE 210
#define MAX_SKILLUNITGROUP 32
#define MAX_MOBSKILLUNITGROUP 8
#define MAX_SKILLUNITGROUPTICKSET 32
#define MAX_SKILLTIMERSKILL 32
#define MAX_MOBSKILLTIMERSKILL 10
#define MAX_MOBSKILL 32
#define MAX_EVENTQUEUE 2
#define MAX_EVENTTIMER 32
#define NATURAL_HEAL_INTERVAL 500
#define MAX_FLOORITEM 500000
#define MAX_LEVEL 255
#define MAX_WALKPATH 32
#define MAX_DROP_PER_MAP 48
#define MAX_IGNORE_LIST 80
#define MAX_VENDING 12

#define DEFAULT_AUTOSAVE_INTERVAL 60*1000

#define OPTION_HIDE 0x40

enum { BL_NUL, BL_PC, BL_NPC, BL_MOB, BL_ITEM, BL_CHAT, BL_SKILL , BL_PET };
enum { WARP, SHOP, SCRIPT, MONS };

struct block_list {
	struct block_list *next,*prev;
	int id;
	short m,x,y;
	unsigned char type;
	unsigned char subtype;
};

struct walkpath_data {
	unsigned char path_len,path_pos,path_half;
	unsigned char path[MAX_WALKPATH];
};
struct shootpath_data {
	int rx,ry,len;
	int x[MAX_WALKPATH];
	int y[MAX_WALKPATH];
};

struct script_reg {
	int index;
	int data;
};
struct script_regstr {
	int index;
	char data[256];
};
struct status_change {
	int timer;
	int val1,val2,val3,val4;
};
struct vending {
	short index;
	unsigned short amount;
	unsigned int value;
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
	int target_flag;
	unsigned int tick;
	int limit,interval;

	int skill_id,skill_lv;
	int val1,val2,val3;
	char *valstr;
	int unit_id;
	int group_id;
	int unit_count,alive_count;
	struct skill_unit *unit;
};
struct skill_unit_group_tickset {
	unsigned int tick;
	int id;
};
struct skill_timerskill {
	int timer;
	int src_id;
	int target_id;
	int map;
	short x,y;
	short skill_id,skill_lv;
	int type;
	int flag;
};

struct npc_data;
struct pet_db;
struct item_data;
struct square;

struct map_session_data {
	struct block_list bl;
	struct {
		unsigned auth : 1;
		unsigned change_walk_target : 1;
		unsigned attack_continue : 1;
		unsigned menu_or_input : 1;
		unsigned dead_sit : 2;
		unsigned skillcastcancel : 1;
		unsigned waitingdisconnect : 1;
		unsigned lr_flag : 2;
		unsigned connect_new : 1;
		unsigned arrow_atk : 1;
		unsigned attack_type : 3;
		unsigned skill_flag : 1;
		unsigned gangsterparadise : 1;
		unsigned produce_flag : 1;
		unsigned make_arrow_flag : 1;
		unsigned potionpitcher_flag : 1;
		unsigned storage_flag : 1;
		unsigned snovice_flag : 4;
		int gmaster_flag;
		// originally by Qamera, adapted by celest
		unsigned event_death : 1;
		unsigned event_kill : 1;
		unsigned event_disconnect : 1;
	} state;
	struct {
		unsigned killer : 1;
		unsigned killable : 1;
		unsigned restart_full_recover : 1;
		unsigned no_castcancel : 1;
		unsigned no_castcancel2 : 1;
		unsigned no_sizefix : 1;
		unsigned no_magic_damage : 1;
		unsigned no_weapon_damage : 1;
		unsigned no_gemstone : 1;
		unsigned infinite_endure : 1;
	} special_state;
	int char_id, login_id1, login_id2, sex;
	int packet_ver;  // 5: old, 6: 7july04, 7: 13july04, 8: 26july04, 9: 9aug04/16aug04/17aug04, 10: 6sept04, 11: 21sept04, 12: 18oct04, 13: 25oct04 (by [Yor])
	struct mmo_charstatus status;
	struct item_data *inventory_data[MAX_INVENTORY];
	short equip_index[11];
	unsigned short unbreakable_equip;
	unsigned short unbreakable;	// chance to prevent equipment breaking [celest]
	int weight,max_weight;
	int cart_weight,cart_max_weight,cart_num,cart_max_num;
	char mapname[24];
	int fd,new_fd;
	short to_x,to_y;
	short speed,prev_speed;
	short opt1,opt2,opt3;
	char dir,head_dir;
	unsigned int client_tick,server_tick;
	struct walkpath_data walkpath;
	int walktimer;
	int next_walktime;
	int npc_id,areanpc_id,npc_shopid;
	int npc_pos;
	int npc_menu;
	int npc_amount;
	int npc_stack,npc_stackmax;
	char *npc_script,*npc_scriptroot;
	char *npc_stackbuf;
	char npc_str[256];
	unsigned int chatID;
	unsigned long idletime;

	struct{
		char name[24];
	} ignore[MAX_IGNORE_LIST];
	int ignoreAll;

	int attacktimer;
	int attacktarget;
	short attacktarget_lv;
	unsigned int attackabletime;

        int followtimer; // [MouseJstr]
        int followtarget;

	time_t emotionlasttime; // to limit flood with emotion packets

	short attackrange,attackrange_;
	int skilltimer;
	int skilltarget;
	short skillx,skilly;
	short skillid,skilllv;
	short skillitem,skillitemlv;
	short skillid_old,skilllv_old;
	short skillid_dance,skilllv_dance;
	struct skill_unit_group skillunit[MAX_SKILLUNITGROUP];
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	struct skill_timerskill skilltimerskill[MAX_SKILLTIMERSKILL];
	char blockskill[MAX_SKILL];	// [celest]	
	//unsigned int skillstatictimer[MAX_SKILL];
	unsigned short timerskill_count; // [celest]
	int cloneskill_id;
	int potion_hp,potion_sp,potion_per_hp,potion_per_sp;

	int invincible_timer;
	unsigned int canact_tick;
	unsigned int canmove_tick;
	unsigned int canlog_tick;
	unsigned int canregen_tick;
	int hp_sub,sp_sub;
	int inchealhptick,inchealsptick,inchealspirithptick,inchealspiritsptick;
// -- moonsoul (new tick for berserk self-damage)
//	int berserkdamagetick;
	int fame;

	short view_class;
	short weapontype1,weapontype2;
	short disguiseflag,disguise; // [Valaris]
	int paramb[6],paramc[6],parame[6],paramcard[6];
	int hit,flee,flee2,aspd,amotion,dmotion;
	int watk,watk2,atkmods[3];
	int def,def2,mdef,mdef2,critical,matk1,matk2;
	int atk_ele,def_ele,star,overrefine;
	int castrate,delayrate,hprate,sprate,dsprate;
	int addele[10],addrace[12],addsize[3],subele[10],subrace[12];
	int addeff[10],addeff2[10],reseff[10];
	int watk_,watk_2,atkmods_[3],addele_[10],addrace_[12],addsize_[3];	//ìÒìÅó¨ÇÃÇΩÇﬂÇ…í«â¡
	int atk_ele_,star_,overrefine_;				//ìÒìÅó¨ÇÃÇΩÇﬂÇ…í«â¡
	int base_atk,atk_rate;
	int weapon_atk[16],weapon_atk_rate[16];
	int arrow_atk,arrow_ele,arrow_cri,arrow_hit,arrow_range;
	int arrow_addele[10],arrow_addrace[12],arrow_addsize[3],arrow_addeff[10],arrow_addeff2[10];
	int nhealhp,nhealsp,nshealhp,nshealsp,nsshealhp,nsshealsp;
	int aspd_rate,speed_rate,hprecov_rate,sprecov_rate,critical_def,double_rate;
	int near_attack_def_rate,long_attack_def_rate,magic_def_rate,misc_def_rate;
	int matk_rate,ignore_def_ele,ignore_def_race,ignore_def_ele_,ignore_def_race_;
	int ignore_mdef_ele,ignore_mdef_race;
	int magic_addele[10],magic_addrace[12],magic_subrace[12];
	int perfect_hit,get_zeny_num;
	int critical_rate,hit_rate,flee_rate,flee2_rate,def_rate,def2_rate,mdef_rate,mdef2_rate;
	int def_ratio_atk_ele,def_ratio_atk_ele_,def_ratio_atk_race,def_ratio_atk_race_;
	int add_damage_class_count,add_damage_class_count_,add_magic_damage_class_count;
	short add_damage_classid[10],add_damage_classid_[10],add_magic_damage_classid[10];
	int add_damage_classrate[10],add_damage_classrate_[10],add_magic_damage_classrate[10];
	short add_def_class_count,add_mdef_class_count;
	short add_def_classid[10],add_mdef_classid[10];
	int add_def_classrate[10],add_mdef_classrate[10];
	short monster_drop_item_count;
	short monster_drop_itemid[10];
	int monster_drop_race[10],monster_drop_itemrate[10];
	int double_add_rate,speed_add_rate,aspd_add_rate,perfect_hit_add, get_zeny_add_num;
	short splash_range,splash_add_range;
	short autospell_id,autospell_lv,autospell_rate;
	short hp_drain_rate,hp_drain_per,sp_drain_rate,sp_drain_per;
	short hp_drain_rate_,hp_drain_per_,sp_drain_rate_,sp_drain_per_;
	short hp_drain_value,sp_drain_value,hp_drain_value_,sp_drain_value_;
	int short_weapon_damage_return,long_weapon_damage_return;
	int weapon_coma_ele[10],weapon_coma_race[12];
	int break_weapon_rate,break_armor_rate;
	short add_steal_rate;
	//--- 02/15's new card effects [celest]
	int crit_atk_rate;
	int critaddrace[12];
	short no_regen;
	int addeff3[10];
	short addeff3_type[10];
	short autospell2_id,autospell2_lv,autospell2_rate,autospell2_type;
	int skillatk[2];
	unsigned short unstripable_equip;
	short add_damage_classid2[10],add_damage_class_count2;
	int add_damage_classrate2[10];
	short sp_gain_value, hp_gain_value;
	short sp_drain_type;
	short ignore_def_mob, ignore_def_mob_;
	int hp_loss_tick, hp_loss_rate;
	short hp_loss_value, hp_loss_type;
	int addrace2[12],addrace2_[12];
	int subsize[3];
	short unequip_losehp[11];
	short unequip_losesp[11];
	int itemid;
	int itemhealrate[7];
	//--- 03/15's new card effects
	int expaddrace[12];
	int subrace2[12];
	short sp_gain_race[12];

	short spiritball, spiritball_old;
	int spirit_timer[MAX_SKILL_LEVEL];
	int magic_damage_return; // AppleGirl Was Here
	int random_attack_increase_add,random_attack_increase_per; // [Valaris]
	int perfect_hiding; // [Valaris]
	int classchange; // [Valaris]

	int die_counter;
	short doridori_counter;

	int reg_num;
	struct script_reg *reg;
	int regstr_num;
	struct script_regstr *regstr;

	struct status_change sc_data[MAX_STATUSCHANGE];
	short sc_count;
	struct square dev;

	int trade_partner;
	int deal_item_index[10];
	int deal_item_amount[10];
	int deal_zeny;
	short deal_locked;

	int party_sended,party_invite,party_invite_account;
	int party_hp,party_x,party_y;

	int guild_sended,guild_invite,guild_invite_account;
	int guild_emblem_id,guild_alliance,guild_alliance_account;
	int guildspy; // [Syrus22]
	int partyspy; // [Syrus22]

	int vender_id;
	int vend_num;
	char message[80];
	struct vending vending[MAX_VENDING];

	int catch_target_class;
	struct s_pet pet;
	struct pet_db *petDB;
	struct pet_data *pd;
	int pet_hungry_timer;

	int pvp_point,pvp_rank,pvp_timer,pvp_lastusers;

	char eventqueue[MAX_EVENTQUEUE][50];
	int eventtimer[MAX_EVENTTIMER];
	unsigned short eventcount; // [celest]

	int last_skillid,last_skilllv;		// Added by RoVeRT

	unsigned char change_level; // [celest]
	int autoloot; //by Upa-Kun
	unsigned nodelay :1;
	unsigned noexp :1;
	unsigned detach :1;

#ifndef TXT_ONLY
	int mail_counter;	// mail counter for mail system [Valaris]
#endif

};

struct npc_timerevent_list {
	int timer,pos;
};
struct npc_label_list {
	char name[24];
	int pos;
};
struct npc_item_list {
	int nameid,value;
};
struct npc_data {
	struct block_list bl;
	short n;
	short class_,dir;
	short speed;
	char name[24];
	char exname[24];
	int chat_id;
	short opt1,opt2,opt3,option;
	short flag;
	int walktimer; // [Valaris]
	short to_x,to_y; // [Valaris]
	struct walkpath_data walkpath;
	unsigned int next_walktime;
	unsigned int canmove_tick;

	struct { // [Valaris]
		unsigned state : 8;
		unsigned change_walk_target : 1;
		unsigned walk_easy : 1;
	} state;

	union {
		struct {
			char *script;
			short xs,ys;
			int guild_id;
			int timer,timerid,timeramount,nexttimer,rid;
			unsigned int timertick;
			struct npc_timerevent_list *timer_event;
			int label_list_num;
			struct npc_label_list *label_list;
			int src_id;
		} scr;
		struct npc_item_list shop_item[1];
		struct {
			short xs,ys;
			short x,y;
			char name[16];
		} warp;
	} u;
	// Ç±Ç±Ç…ÉÅÉìÉoÇí«â¡ÇµÇƒÇÕÇ»ÇÁÇ»Ç¢(shop_itemÇ™â¬ïœí∑ÇÃà◊)

	char eventqueue[MAX_EVENTQUEUE][50];
	int eventtimer[MAX_EVENTTIMER];
	short arenaflag;

	void *chatdb;
};
struct mob_data {
	struct block_list bl;
	short n;
	short base_class,class_,dir,mode,level;
	short m,x0,y0,xs,ys;
	char name[24];
	int spawndelay1,spawndelay2;
	struct {
		unsigned state : 8;
		unsigned skillstate : 8;
		unsigned targettype : 1;
		unsigned steal_flag : 1;
		unsigned steal_coin_flag : 1;
		unsigned skillcastcancel : 1;
		unsigned master_check : 1;
		unsigned change_walk_target : 1;
		unsigned walk_easy : 1;
		unsigned special_mob_ai : 3;
		unsigned soul_change_flag : 1; // Celest
		int provoke_flag; // Celest
	} state;
	int timer;
	short to_x,to_y;
	short target_dir;
	short speed;
	int hp;
	int target_id,attacked_id;
	short target_lv;
	struct walkpath_data walkpath;
	unsigned int next_walktime;
	unsigned int attackabletime;
	unsigned int last_deadtime,last_spawntime,last_thinktime;
	unsigned int canmove_tick;
	short move_fail_count;
	struct {
		int id;
		int dmg;
	} dmglog[DAMAGELOG_SIZE];
	struct item *lootitem;
	short lootitem_count;

	struct status_change sc_data[MAX_STATUSCHANGE];
	short sc_count;
	short opt1,opt2,opt3,option;
	short min_chase;
	int guild_id;
	int deletetimer;

	int skilltimer;
	int skilltarget;
	short skillx,skilly;
	short skillid,skilllv,skillidx;
	unsigned int skilldelay[MAX_MOBSKILL];
	int def_ele;
	int master_id,master_dist;
	int exclusion_src,exclusion_party,exclusion_guild;
	struct skill_timerskill skilltimerskill[MAX_MOBSKILLTIMERSKILL];
	struct skill_unit_group skillunit[MAX_MOBSKILLUNITGROUP];
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	char npc_event[50];
	unsigned char size;
	short recall_flag;
	int recallmob_count;
	short recallcount;
};
struct pet_data {
	struct block_list bl;
	short n;
	short class_,dir;
	short speed;
	char name[24];
	struct {
		unsigned state : 8 ;
		unsigned skillstate : 8 ;
		unsigned change_walk_target : 1 ;
		short skillbonus;
	} state;
	int timer;
	short to_x,to_y;
	short equip;
	struct walkpath_data walkpath;
	int target_id;
	short target_lv;
	int move_fail_count;
	unsigned int attackabletime,next_walktime,last_thinktime;
	int skilltype,skillval,skilltimer,skillduration; // [Valaris]
	//int skillbonustype,skillbonusval,skillbonustimer,skillbonusduration; // [Valaris]
	int skillbonustype,skillbonusval,skillbonustimer;
	struct item *lootitem;
	short loot; // [Valaris]
	short lootmax; // [Valaris]
	short lootitem_count;
	short lootitem_weight;
	int lootitem_timer;
	struct skill_timerskill skilltimerskill[MAX_MOBSKILLTIMERSKILL]; // [Valaris]
	struct skill_unit_group skillunit[MAX_MOBSKILLUNITGROUP]; // [Valaris]
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET]; // [Valaris]
	struct map_session_data *msd;
};

enum { MS_IDLE,MS_WALK,MS_ATTACK,MS_DEAD,MS_DELAY };

enum { NONE_ATTACKABLE,ATTACKABLE };

enum { ATK_LUCKY=1,ATK_FLEE,ATK_DEF};	// àÕÇ‹ÇÍÉyÉiÉãÉeÉBåvéZóp

// ëïîıÉRÅ[Éh
enum {
	EQP_WEAPON		= 0x0002,		// âEéË
	EQP_ARMOR		= 0x0010,		// ëÃ
	EQP_SHIELD		= 0x0020,		// ç∂éË
	EQP_HELM		= 0x0100,		// ì™è„íi
};

struct map_data {
	char name[24];
	unsigned char *gat;	// NULLÇ»ÇÁâ∫ÇÃmap_data_other_serverÇ∆ÇµÇƒàµÇ§
	char *alias; // [MouseJstr]
	struct block_list **block;
	struct block_list **block_mob;
	int *block_count,*block_mob_count;
	int m;
	short xs,ys;
	short bxs,bys;
	int npc_num;
	int users;
	struct {
		unsigned alias : 1;
		unsigned nomemo : 1;
		unsigned noteleport : 1;
		unsigned noreturn : 1;
		unsigned monster_noteleport : 1;
		unsigned nosave : 1;
		unsigned nobranch : 1;
		unsigned nopenalty : 1;
		unsigned pvp : 1;
		unsigned pvp_noparty : 1;
		unsigned pvp_noguild : 1;
		unsigned pvp_nightmaredrop :1;
		unsigned pvp_nocalcrank : 1;
		unsigned gvg : 1;
		unsigned gvg_noparty : 1;
		unsigned nozenypenalty : 1;
		unsigned notrade : 1;
		unsigned noskill : 1;
		unsigned nowarp : 1;
		unsigned nowarpto : 1;
		unsigned nopvp : 1; // [Valaris]
		unsigned noicewall : 1; // [Valaris]
		unsigned snow : 1; // [Valaris]
		unsigned fog : 1; // [Valaris]
		unsigned sakura : 1; // [Valaris]
		unsigned leaves : 1; // [Valaris]
		unsigned rain : 1; // [Valaris]
		unsigned indoors : 1; // celest
		unsigned nogo : 1; // [Valaris]
	} flag;
	struct point save;
	struct npc_data *npc[MAX_NPC_PER_MAP];
	struct {
		int drop_id;
		int drop_type;
		int drop_per;
	} drop_list[MAX_DROP_PER_MAP];
};
struct map_data_other_server {
	char name[24];
	unsigned char *gat;	// NULLå≈íËÇ…ÇµÇƒîªíf
	unsigned long ip;
	unsigned int port;
	struct map_data* map;
};

struct flooritem_data {
	struct block_list bl;
	short subx,suby;
	int cleartimer;
	int first_get_id,second_get_id,third_get_id;
	unsigned int first_get_tick,second_get_tick,third_get_tick;
	struct item item_data;
};

enum {
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

	// original 1000-
	SP_ATTACKRANGE=1000,	SP_ATKELE,SP_DEFELE,	// 1000-1002
	SP_CASTRATE, SP_MAXHPRATE, SP_MAXSPRATE, SP_SPRATE, // 1003-1006
	SP_ADDELE, SP_ADDRACE, SP_ADDSIZE, SP_SUBELE, SP_SUBRACE, // 1007-1011
	SP_ADDEFF, SP_RESEFF,	// 1012-1013
	SP_BASE_ATK,SP_ASPD_RATE,SP_HP_RECOV_RATE,SP_SP_RECOV_RATE,SP_SPEED_RATE, // 1014-1018
	SP_CRITICAL_DEF,SP_NEAR_ATK_DEF,SP_LONG_ATK_DEF, // 1019-1021
	SP_DOUBLE_RATE, SP_DOUBLE_ADD_RATE, SP_MATK, SP_MATK_RATE, // 1022-1025
	SP_IGNORE_DEF_ELE,SP_IGNORE_DEF_RACE, // 1026-1027
	SP_ATK_RATE,SP_SPEED_ADDRATE,SP_ASPD_ADDRATE, // 1028-1030
	SP_MAGIC_ATK_DEF,SP_MISC_ATK_DEF, // 1031-1032
	SP_IGNORE_MDEF_ELE,SP_IGNORE_MDEF_RACE, // 1033-1034
	SP_MAGIC_ADDELE,SP_MAGIC_ADDRACE,SP_MAGIC_SUBRACE, // 1035-1037
	SP_PERFECT_HIT_RATE,SP_PERFECT_HIT_ADD_RATE,SP_CRITICAL_RATE,SP_GET_ZENY_NUM,SP_ADD_GET_ZENY_NUM, // 1038-1042
	SP_ADD_DAMAGE_CLASS,SP_ADD_MAGIC_DAMAGE_CLASS,SP_ADD_DEF_CLASS,SP_ADD_MDEF_CLASS, // 1043-1046
	SP_ADD_MONSTER_DROP_ITEM,SP_DEF_RATIO_ATK_ELE,SP_DEF_RATIO_ATK_RACE,SP_ADD_SPEED, // 1047-1050
	SP_HIT_RATE,SP_FLEE_RATE,SP_FLEE2_RATE,SP_DEF_RATE,SP_DEF2_RATE,SP_MDEF_RATE,SP_MDEF2_RATE, // 1051-1057
	SP_SPLASH_RANGE,SP_SPLASH_ADD_RANGE,SP_AUTOSPELL,SP_HP_DRAIN_RATE,SP_SP_DRAIN_RATE, // 1058-1062
	SP_SHORT_WEAPON_DAMAGE_RETURN,SP_LONG_WEAPON_DAMAGE_RETURN,SP_WEAPON_COMA_ELE,SP_WEAPON_COMA_RACE, // 1063-1066
	SP_ADDEFF2,SP_BREAK_WEAPON_RATE,SP_BREAK_ARMOR_RATE,SP_ADD_STEAL_RATE, // 1067-1070
	SP_MAGIC_DAMAGE_RETURN,SP_RANDOM_ATTACK_INCREASE,SP_ALL_STATS,SP_AGI_VIT,SP_AGI_DEX_STR,SP_PERFECT_HIDE, // 1071-1076
	SP_DISGUISE,SP_CLASSCHANGE, // 1077-1078
	SP_HP_DRAIN_VALUE,SP_SP_DRAIN_VALUE, // 1079-1080
	SP_WEAPON_ATK,SP_WEAPON_ATK_RATE, // 1081-1082
	SP_DELAYRATE,	// 1083
	
	SP_RESTART_FULL_RECORVER=2000,SP_NO_CASTCANCEL,SP_NO_SIZEFIX,SP_NO_MAGIC_DAMAGE,SP_NO_WEAPON_DAMAGE,SP_NO_GEMSTONE, // 2000-2005
	SP_NO_CASTCANCEL2,SP_INFINITE_ENDURE,SP_UNBREAKABLE_WEAPON,SP_UNBREAKABLE_ARMOR, SP_UNBREAKABLE_HELM, // 2006-2010
	SP_UNBREAKABLE_SHIELD, SP_LONG_ATK_RATE, // 2011-2012

	SP_CRIT_ATK_RATE, SP_CRITICAL_ADDRACE, SP_NO_REGEN, SP_ADDEFF_WHENHIT, SP_AUTOSPELL_WHENHIT, // 2013-2017
	SP_SKILL_ATK, SP_UNSTRIPABLE, SP_ADD_DAMAGE_BY_CLASS, // 2018-2020
	SP_SP_GAIN_VALUE, SP_IGNORE_DEF_MOB, SP_HP_LOSS_RATE, SP_ADDRACE2, SP_HP_GAIN_VALUE, // 2021-2025
	SP_SUBSIZE, SP_DAMAGE_WHEN_UNEQUIP, SP_ADD_ITEM_HEAL_RATE, SP_LOSESP_WHEN_UNEQUIP, SP_EXP_ADDRACE,	// 2026-2030
	SP_SP_GAIN_RACE, SP_SUBRACE2, SP_ADDEFF_WHENHIT_SHORT,	// 2031-2033
	SP_UNSTRIPABLE_WEAPON,SP_UNSTRIPABLE_ARMOR,SP_UNSTRIPABLE_HELM,SP_UNSTRIPABLE_SHIELD  // 2034-2037
};

enum {
	LOOK_BASE,LOOK_HAIR,LOOK_WEAPON,LOOK_HEAD_BOTTOM,LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_HAIR_COLOR,LOOK_CLOTHES_COLOR,LOOK_SHIELD,LOOK_SHOES
};

// CELL
#define CELL_MASK		0x0f
#define CELL_NPC		0x80	// NPCÉZÉã
#define CELL_BASILICA	0x40	// BASILICAÉZÉã
#define CELL_MOONLIT	0x100
#define CELL_REGEN		0x200
/*
 * map_getcell()Ç≈égópÇ≥ÇÍÇÈÉtÉâÉO
 */
typedef enum { 
	CELL_CHKWALL=0,		// ï«(ÉZÉãÉ^ÉCÉv1)
	CELL_CHKWATER,		// êÖèÍ(ÉZÉãÉ^ÉCÉv3)
	CELL_CHKGROUND,		// ínñ è·äQï®(ÉZÉãÉ^ÉCÉv5)
	CELL_CHKPASS,		// í âﬂâ¬î\(ÉZÉãÉ^ÉCÉv1,5à»äO)
	CELL_CHKNOPASS,		// í âﬂïsâ¬(ÉZÉãÉ^ÉCÉv1,5)
	CELL_GETTYPE,		// ÉZÉãÉ^ÉCÉvÇï‘Ç∑
	CELL_CHKNPC=0x10,	// É^ÉbÉ`É^ÉCÉvÇÃNPC(ÉZÉãÉ^ÉCÉv0x80ÉtÉâÉO)
	CELL_CHKBASILICA,	// ÉoÉWÉäÉJ(ÉZÉãÉ^ÉCÉv0x40ÉtÉâÉO)
} cell_t;
// map_setcell()Ç≈égópÇ≥ÇÍÇÈÉtÉâÉO
enum {
	CELL_SETNPC=0x10,	// É^ÉbÉ`É^ÉCÉvÇÃNPCÇÉZÉbÉg
	CELL_SETBASILICA,	// ÉoÉWÉäÉJÇÉZÉbÉg
	CELL_CLRBASILICA,	// ÉoÉWÉäÉJÇÉNÉäÉA
};

struct chat_data {
	struct block_list bl;

	unsigned char pass[8];   /* password */
	unsigned char title[61]; /* room title MAX 60 */
	unsigned char limit;     /* join limit */
	unsigned char trigger;
	unsigned char users;     /* current users */
	unsigned char pub;       /* room attribute */
	struct map_session_data *usersd[20];
	struct block_list *owner_;
	struct block_list **owner;
	char npc_event[50];
};

extern struct map_data map[];
extern int map_num;
extern int autosave_interval;
extern int agit_flag;
extern int night_flag; // 0=day, 1=night [Yor]

// gat?÷ß
int map_getcell(int,int,int,cell_t);
int map_getcellp(struct map_data*,int,int,cell_t);
void map_setcell(int,int,int,int);
extern int map_read_flag; // 0: grf´’´°´§´Î 1: ´≠´„´√´∑´Â 2: ´≠´„´√´∑´Â(?ıÍ)
enum {
	READ_FROM_GAT, READ_FROM_AFM,
	READ_FROM_BITMAP, CREATE_BITMAP,
	READ_FROM_BITMAP_COMPRESSED, CREATE_BITMAP_COMPRESSED
};

extern char motd_txt[];
extern char help_txt[];

extern char talkie_mes[];

extern char wisp_server_name[];

// éIëSëÃèÓïÒ
void map_setusers(int);
int map_getusers(void);
// blockçÌèúä÷òA
int map_freeblock( void *bl );
int map_freeblock_lock(void);
int map_freeblock_unlock(void);
// blockä÷òA
int map_addblock(struct block_list *);
int map_delblock(struct block_list *);
void map_foreachinarea(int (*)(struct block_list*,va_list),int,int,int,int,int,int,...);
// -- moonsoul (added map_foreachincell)
void map_foreachincell(int (*)(struct block_list*,va_list),int,int,int,int,...);
void map_foreachinmovearea(int (*)(struct block_list*,va_list),int,int,int,int,int,int,int,int,...);
void map_foreachinpath(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int range,int type,...); // Celest
int map_countnearpc(int,int,int);
//blockä÷òAÇ…í«â¡
int map_count_oncell(int m,int x,int y);
struct skill_unit *map_find_skill_unit_oncell(struct block_list *,int x,int y,int skill_id,struct skill_unit *);
// àÍéûìIobjectä÷òA
int map_addobject(struct block_list *);
int map_delobject(int);
int map_delobjectnofree(int id);
void map_foreachobject(int (*)(struct block_list*,va_list),int,...);
//
int map_quit(struct map_session_data *);
// npc
int map_addnpc(int,struct npc_data *);

// è∞ÉAÉCÉeÉÄä÷òA
int map_clearflooritem_timer(int,unsigned int,int,int);
#define map_clearflooritem(id) map_clearflooritem_timer(0,0,id,1)
int map_addflooritem(struct item *,int,int,int,int,struct map_session_data *,struct map_session_data *,struct map_session_data *,int);
int map_searchrandfreecell(int,int,int,int);

// ÉLÉÉÉâidÅÅÅÑÉLÉÉÉâñº ïœä∑ä÷òA
void map_addchariddb(int charid,char *name);
void map_delchariddb(int charid);
int map_reqchariddb(struct map_session_data * sd,int charid);
char * map_charid2nick(int);
struct map_session_data * map_charid2sd(int);

struct map_session_data * map_id2sd(int);
struct block_list * map_id2bl(int);
int map_mapname2mapid(char*);
int map_mapname2ipport(char*,int*,int*);
int map_setipport(char *name,unsigned long ip,int port);
int map_eraseipport(char *name,unsigned long ip,int port);
int map_eraseallipport(void);
void map_addiddb(struct block_list *);
void map_deliddb(struct block_list *bl);
int map_foreachiddb(int (*)(void*,void*,va_list),...);
void map_addnickdb(struct map_session_data *);
struct map_session_data * map_nick2sd(char*);
int compare_item(struct item *a, struct item *b);

// ÇªÇÃëº
int map_check_dir(int s_dir,int t_dir);
int map_calc_dir( struct block_list *src,int x,int y);

// path.cÇÊÇË
int path_search(struct walkpath_data*,int,int,int,int,int,int);
int path_search_long(struct shootpath_data *,int,int,int,int,int);
int path_blownpos(int m,int x0,int y0,int dx,int dy,int count);

int map_who(int fd);

int cleanup_sub(struct block_list *bl, va_list ap);

void map_helpscreen(); // [Valaris]
int map_delmap(char *mapname);

extern char *INTER_CONF_NAME;
extern char *LOG_CONF_NAME;
extern char *MAP_CONF_NAME;
extern char *BATTLE_CONF_FILENAME;
extern char *ATCOMMAND_CONF_FILENAME;
extern char *CHARCOMMAND_CONF_FILENAME;
extern char *SCRIPT_CONF_NAME;
extern char *MSG_CONF_NAME;
extern char *GRF_PATH_FILENAME;

#ifndef TXT_ONLY

// MySQL
#ifdef __WIN32
#include <my_global.h>
#include <my_sys.h>
#endif
#include <mysql.h>

void char_online_check(void); // [Valaris]
void char_offline(struct map_session_data *sd);

extern MYSQL mmysql_handle;
extern char tmp_sql[65535];
extern MYSQL_RES* sql_res ;
extern MYSQL_ROW	sql_row ;

extern MYSQL lmysql_handle;
extern char tmp_lsql[65535];
extern MYSQL_RES* lsql_res ;
extern MYSQL_ROW	lsql_row ;

extern MYSQL mail_handle;
extern MYSQL_RES* 	mail_res ;
extern MYSQL_ROW	mail_row ;
extern char tmp_msql[65535];

extern int db_use_sqldbs;

extern char item_db_db[32];
extern char mob_db_db[32];
extern char login_db[32];

extern char login_db_level[32];
extern char login_db_account_id[32];

extern char gm_db[32];
extern char gm_db_level[32];
extern char gm_db_account_id[32];

extern int lowest_gm_level;
extern int read_gm_interval;

extern char char_db[32];
#endif /* not TXT_ONLY */

#endif
