// $Id: mob.h,v 1.4 2004/09/25 05:32:18 MouseJstr Exp $
#ifndef _MOB_H_
#define _MOB_H_

#define MAX_RANDOMMONSTER 3

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
	char name[24],jname[24];
	int lv;
	int max_hp,max_sp;
	int base_exp,job_exp;
	int atk1,atk2;
	int def,mdef;
	int str,agi,vit,int_,dex,luk;
	int range,range2,range3;
	int size,race,element,mode;
	int speed,adelay,amotion,dmotion;
	int mexp,mexpper;
	struct { int nameid,p; } dropitem[8];
	struct { int nameid,p; } mvpitem[3];
	int view_class,sex;
	short hair,hair_color,weapon,shield,head_top,head_mid,head_buttom,option,clothes_color; // [Valaris]
	int equip; // [Valaris]
	int summonper[MAX_RANDOMMONSTER];
	int maxskill;
	struct mob_skill skill[MAX_MOBSKILL];
};
extern struct mob_db mob_db[];

enum {
	MST_TARGET			=	0,
	MST_SELF			=	1,
	MST_FRIEND			=	2,
	MST_AROUND5			=	3,
	MST_AROUND6			=	4,
	MST_AROUND7			=	5,
	MST_AROUND8			=	6,
	MST_AROUND1			=	7,
	MST_AROUND2			=	8,
	MST_AROUND3			=	9,
	MST_AROUND4			=	10,
	MST_AROUND			=	MST_AROUND4,

	MSC_ALWAYS			=	0x0000,
	MSC_MYHPLTMAXRATE	=	0x0001,
	MSC_FRIENDHPLTMAXRATE=	0x0010,
	MSC_MYSTATUSON		=	0x0020,
	MSC_MYSTATUSOFF		=	0x0021,
	MSC_FRIENDSTATUSON	=	0x0030,
	MSC_FRIENDSTATUSOFF	=	0x0031,

	MSC_ATTACKPCGT		=	0x0100,
	MSC_ATTACKPCGE		=	0x0101,
	MSC_SLAVELT			=	0x0110,
	MSC_SLAVELE			=	0x0111,
	MSC_CLOSEDATTACKED	=	0x1000,
	MSC_LONGRANGEATTACKED=	0x1001,
	MSC_SKILLUSED		=	0x1010,
	MSC_CASTTARGETED	=	0x1011,
};

enum {
	MSS_IDLE,	// ë“ã@
	MSS_WALK,	// à⁄ìÆ
	MSS_ATTACK,	// çUåÇ
	MSS_DEAD,	// éÄñS
	MSS_LOOT,	// ÉãÅ[Ég
	MSS_CHASE,	// ìÀåÇ
};

int mobdb_searchname(const char *str);
int mobdb_checkid(const int id);
int mob_once_spawn(struct map_session_data *sd,char *mapname,
	int x,int y,const char *mobname,int class,int amount,const char *event);
int mob_once_spawn_area(struct map_session_data *sd,char *mapname,
	int x0,int y0,int x1,int y1,
	const char *mobname,int class,int amount,const char *event);

int mob_spawn_guardian(struct map_session_data *sd,char *mapname,	// Spawning Guardians [Valaris]
	int x,int y,const char *mobname,int class,int amount,const char *event,int guardian);	// Spawning Guardians [Valaris]


int mob_walktoxy(struct mob_data *md,int x,int y,int easy);

int mob_target(struct mob_data *md,struct block_list *bl,int dist);
int mob_stop_walking(struct mob_data *md,int type);
int mob_stopattack(struct mob_data *);
int mob_spawn(int);
int mob_damage(struct block_list *,struct mob_data*,int,int);
int mob_changestate(struct mob_data *md,int state,int type);
int mob_heal(struct mob_data*,int);
int mob_exclusion_add(struct mob_data *md,int type,int id);
int mob_exclusion_check(struct mob_data *md,struct map_session_data *sd);
int mob_get_viewclass(int);
int mob_get_sex(int);
short mob_get_hair(int);
short mob_get_hair_color(int);
short mob_get_weapon(int);
short mob_get_shield(int);
short mob_get_head_top(int);
short mob_get_head_mid(int);
short mob_get_head_buttom(int);
short mob_get_clothes_color(int);	//player mob dye [Valaris]
int mob_get_equip(int); // mob equip [Valaris]
int do_init_mob(void);

int mob_delete(struct mob_data *md);
int mob_catch_delete(struct mob_data *md,int type);
int mob_timer_delete(int tid, unsigned int tick, int id, int data);

int mob_deleteslave(struct mob_data *md);

int mob_counttargeted(struct mob_data *md,struct block_list *src,int target_lv);

int mob_class_change(struct mob_data *md,int *value);
int mob_warp(struct mob_data *md,int m,int x,int y,int type);

int mobskill_use(struct mob_data *md,unsigned int tick,int event);
int mobskill_event(struct mob_data *md,int flag);
int mobskill_castend_id( int tid, unsigned int tick, int id,int data );
int mobskill_castend_pos( int tid, unsigned int tick, int id,int data );
int mob_summonslave(struct mob_data *md2,int *value,int amount,int flag);

int mob_gvmobcheck(struct map_session_data *sd, struct block_list *bl);
void mob_reload(void);

#endif
