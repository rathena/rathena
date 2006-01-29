// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MOB_H_
#define _MOB_H_

#define MAX_RANDOMMONSTER 3
#define MAX_MOB_RACE_DB 6
#define MAX_MOB_DB 10000
	/* Change this to increase the table size in your mob_db to accomodate
		a larger mob database. Be sure to note that IDs 4001 to 4048 are reserved for advanced/baby/expanded classes.
	*/

// These define the range of available IDs for clones. [Valaris]
#define MOB_CLONE_START 9001
#define MOB_CLONE_END 10000

struct mob_skill {
	short state;
	short skill_id,skill_lv;
	short permillage;
	int casttime,delay;
	short cancel;
	short cond1,cond2;
	short target;
	int val[5];
	short emotion;
};

struct mob_db {
	char name[NAME_LENGTH],jname[NAME_LENGTH];
	short lv;
	int max_hp,max_sp;
	int base_exp,job_exp;
	int atk1,atk2;
	int def,mdef;
	int str,agi,vit,int_,dex,luk;
	int range,range2,range3;
	int size,race,element,mode;
	short race2;	// celest
	int speed,adelay,amotion,dmotion;
	int mexp,mexpper;
	struct { int nameid,p; } dropitem[10]; //8 -> 10 Lupus
	struct { int nameid,p; } mvpitem[3];
	int view_class,sex;
	short hair,hair_color,weapon,shield,head_top,head_mid,head_buttom,option,clothes_color; // [Valaris]
	int equip; // [Valaris]
	int summonper[MAX_RANDOMMONSTER];
	int maxskill;
	struct mob_skill skill[MAX_MOBSKILL];
};

enum {
	MST_TARGET =	0,
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
	MST_AROUND			=	MST_AROUND4,

	MSC_ALWAYS			=	0x0000,
	MSC_MYHPLTMAXRATE,
	MSC_FRIENDHPLTMAXRATE,
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
	MSC_SKILLUSED	,
	MSC_CASTTARGETED,
	MSC_RUDEATTACKED,
	MSC_MASTERHPLTMAXRATE,
	MSC_MASTERATTACKED,
	MSC_ALCHEMIST,
};

//Mob skill states.
enum {
	MSS_IDLE,
	MSS_WALK,
	MSS_LOOT,
	MSS_DEAD,
	MSS_BERSERK, //Aggressive mob attacking
	MSS_ANGRY,   //Mob retaliating from being attacked.
	MSS_RUSH,    //Mob following a player after being attacked.
	MSS_FOLLOW,  //Mob following a player without being attacked.
};

struct mob_db* mob_db(int class_);
int mobdb_searchname(const char *str);
int mobdb_checkid(const int id);
int mob_once_spawn(struct map_session_data *sd,char *mapname,
	int x,int y,const char *mobname,int class_,int amount,const char *event);
int mob_once_spawn_area(struct map_session_data *sd,char *mapname,
	int x0,int y0,int x1,int y1,
	const char *mobname,int class_,int amount,const char *event);

int mob_spawn_guardian(struct map_session_data *sd,char *mapname,	// Spawning Guardians [Valaris]
	int x,int y,const char *mobname,int class_,int amount,const char *event,int guardian);	// Spawning Guardians [Valaris]
int mob_guardian_guildchange(struct block_list *bl,va_list ap); //Change Guardian's ownership. [Skotlex]

int mob_walktoxy(struct mob_data *md,int x,int y,int easy);
int mob_randomwalk(struct mob_data *md,int tick);
int mob_can_move(struct mob_data *md);

int mob_target(struct mob_data *md,struct block_list *bl,int dist);
int mob_unlocktarget(struct mob_data *md,int tick);
int mob_stop_walking(struct mob_data *md,int type);
int mob_stopattack(struct mob_data *);
int mob_spawn(int);
int mob_setdelayspawn(int);
int mob_damage(struct block_list *,struct mob_data*,int,int);
int mob_changestate(struct mob_data *md,int state,int type);
int mob_heal(struct mob_data*,int);

//Defines to speed up search.
#define mob_get_viewclass(class_) mob_db(class_)->view_class
#define mob_get_sex(class_) mob_db(class_)->sex
#define mob_get_hair(class_) mob_db(class_)->hair
#define mob_get_hair_color(class_) mob_db(class_)->hair_color
#define mob_get_weapon(class_) mob_db(class_)->weapon
#define mob_get_shield(class_) mob_db(class_)->shield
#define mob_get_head_top(class_) mob_db(class_)->head_top
#define mob_get_head_mid(class_) mob_db(class_)->head_mid
#define mob_get_head_buttom(class_) mob_db(class_)->head_buttom
#define mob_get_clothes_color(class_) mob_db(class_)->clothes_color
#define mob_get_equip(class_) mob_db(class_)->equip

int do_init_mob(void);
int do_final_mob(void);

void mob_unload(struct mob_data *md);
int mob_remove_map(struct mob_data *md, int type);
int mob_delete(struct mob_data *md);
int mob_timer_delete(int tid, unsigned int tick, int id, int data);

int mob_deleteslave(struct mob_data *md);


int mob_random_class (int *value, size_t count);
int mob_get_random_id(int type, int flag, int lv);
int mob_class_change(struct mob_data *md,int class_);
int mob_warp(struct mob_data *md,int m,int x,int y,int type);
int mob_warpslave(struct block_list *bl, int range);

int mobskill_use(struct mob_data *md,unsigned int tick,int event);
int mobskill_event(struct mob_data *md,int flag);
int mobskill_castend_id( int tid, unsigned int tick, int id,int data );
int mobskill_castend_pos( int tid, unsigned int tick, int id,int data );
int mob_summonslave(struct mob_data *md2,int *value,int amount,int skill_id);
int mob_countslave(struct block_list *bl);

int mob_is_clone(int class_);

int mob_clone_spawn(struct map_session_data *sd, char *map, int x, int y, const char *event, int master_id, int mode, int flag, unsigned int duration);
int mob_clone_delete(int class_);

void mob_reload(void);

#endif
