// $Id: pc.h,v 1.4 2004/09/25 05:32:18 MouseJstr Exp $

#ifndef _PC_H_
#define _PC_H_

#include "map.h"

#define OPTION_MASK 0xd7b8
#define CART_MASK 0x788
#define STATE_BLIND 0x10

#define MAX_SKILL_TREE 51

#define pc_setdead(sd) ((sd)->state.dead_sit = 1)
#define pc_setsit(sd) ((sd)->state.dead_sit = 2)
//#define pc_setstand(sd) ((sd)->state.dead_sit = 0)
#define pc_isdead(sd) ((sd)->state.dead_sit == 1)
#define pc_issit(sd) ((sd)->state.dead_sit == 2)
#define pc_setdir(sd,b,h) ((sd)->dir = (b) ,(sd)->head_dir = (h) )
#define pc_setchatid(sd,n) ((sd)->chatID = n)
#define pc_ishiding(sd) ((sd)->status.option&0x4006)
#define pc_iscarton(sd) ((sd)->status.option&CART_MASK)
#define pc_isfalcon(sd) ((sd)->status.option&0x0010)
#define pc_isriding(sd) ((sd)->status.option&0x0020)
#define pc_isinvisible(sd) ((sd)->status.option&0x0040)
#define pc_is50overweight(sd) (sd->weight*2 >= sd->max_weight) 
#define pc_is90overweight(sd) (sd->weight*10 >= sd->max_weight*9)

int pc_isGM(struct map_session_data *sd);
int pc_iskiller(struct map_session_data *src, struct map_session_data *target); // [MouseJstr]
int pc_getrefinebonus(int lv,int type);

int pc_counttargeted(struct map_session_data *sd,struct block_list *src,int target_lv);
int pc_setrestartvalue(struct map_session_data *sd,int type);
int pc_makesavestatus(struct map_session_data *);
int pc_setnewpc(struct map_session_data*,int,int,int,int,int,int);
int pc_authok(int, int, time_t, struct mmo_charstatus *);
int pc_authfail(int);

int pc_isequip(struct map_session_data *sd,int n);
int pc_equippoint(struct map_session_data *sd,int n);

int pc_break_equip(struct map_session_data *, unsigned short);
#define pc_breakweapon(sd)	(pc_break_equip(sd, EQP_WEAPON))
#define pc_breakarmor(sd)	(pc_break_equip(sd, EQP_ARMOR))
#define pc_breakshield(sd)	(pc_break_equip(sd, EQP_SHIELD))
#define pc_breakhelm(sd)	(pc_break_equip(sd, EQP_HELM))

int pc_checkskill(struct map_session_data *sd,int skill_id);
int pc_checkallowskill(struct map_session_data *sd);
int pc_checkequip(struct map_session_data *sd,int pos);

int pc_calc_skilltree(struct map_session_data *sd);
int pc_calc_skilltree_normalize_job(int c, struct map_session_data *sd);
int pc_clean_skilltree(struct map_session_data *sd);

int pc_checkoverhp(struct map_session_data*);
int pc_checkoversp(struct map_session_data*);

int pc_can_reach(struct map_session_data*,int,int);
int pc_walktoxy(struct map_session_data*,int,int);
int pc_stop_walking(struct map_session_data*,int);
int pc_movepos(struct map_session_data*,int,int);
int pc_setpos(struct map_session_data*,char*,int,int,int);
int pc_setsavepoint(struct map_session_data*,char*,int,int);
int pc_randomwarp(struct map_session_data *sd,int type);
int pc_memo(struct map_session_data *sd,int i);
int pc_randomwalk(struct map_session_data*,int tick);
int pc_remove_map(struct map_session_data *sd,int clrtype);

int pc_checkadditem(struct map_session_data*,int,int);
int pc_inventoryblank(struct map_session_data*);
int pc_search_inventory(struct map_session_data *sd,int item_id);
int pc_payzeny(struct map_session_data*,int);
int pc_additem(struct map_session_data*,struct item*,int);
int pc_getzeny(struct map_session_data*,int);
int pc_delitem(struct map_session_data*,int,int,int);
int pc_checkitem(struct map_session_data*);

int pc_cart_additem(struct map_session_data *sd,struct item *item_data,int amount);
int pc_cart_delitem(struct map_session_data *sd,int n,int amount,int type);
int pc_putitemtocart(struct map_session_data *sd,int idx,int amount);
int pc_getitemfromcart(struct map_session_data *sd,int idx,int amount);
int pc_cartitem_amount(struct map_session_data *sd,int idx,int amount);

int pc_takeitem(struct map_session_data*,struct flooritem_data*);
int pc_dropitem(struct map_session_data*,int,int);

int pc_checkweighticon(struct map_session_data *sd);

int pc_bonus(struct map_session_data*,int,int);
int pc_bonus2(struct map_session_data *sd,int,int,int);
int pc_bonus3(struct map_session_data *sd,int,int,int,int);
int pc_bonus4(struct map_session_data *sd,int,int,int,int,int);
int pc_skill(struct map_session_data*,int,int,int);

int pc_blockskill_start (struct map_session_data*,int,int);	// [celest]

int pc_insert_card(struct map_session_data *sd,int idx_card,int idx_equip);

int pc_item_identify(struct map_session_data *sd,int idx);
int pc_item_repair(struct map_session_data *sd,int idx); // [Celest]
int pc_item_refine(struct map_session_data *sd,int idx); // [Celest]
int pc_steal_item(struct map_session_data *sd,struct block_list *bl);
int pc_steal_coin(struct map_session_data *sd,struct block_list *bl);

int pc_modifybuyvalue(struct map_session_data*,int);
int pc_modifysellvalue(struct map_session_data*,int);

int pc_attack(struct map_session_data*,int,int);
int pc_stopattack(struct map_session_data*);

int pc_follow(struct map_session_data*, int); // [MouseJstr]

int pc_checkbaselevelup(struct map_session_data *sd);
int pc_checkjoblevelup(struct map_session_data *sd);
int pc_gainexp(struct map_session_data*,int,int);
int pc_nextbaseexp(struct map_session_data *);
int pc_nextbaseafter(struct map_session_data *); // [Valaris]
int pc_nextjobexp(struct map_session_data *);
int pc_nextjobafter(struct map_session_data *); // [Valaris]
int pc_need_status_point(struct map_session_data *,int);
int pc_statusup(struct map_session_data*,int);
int pc_statusup2(struct map_session_data*,int,int);
int pc_skillup(struct map_session_data*,int);
int pc_allskillup(struct map_session_data*);
int pc_resetlvl(struct map_session_data*,int type);
int pc_resetstate(struct map_session_data*);
int pc_resetskill(struct map_session_data*);
int pc_equipitem(struct map_session_data*,int,int);
int pc_unequipitem(struct map_session_data*,int,int);
int pc_checkitem(struct map_session_data*);
int pc_useitem(struct map_session_data*,int);

int pc_damage(struct block_list *,struct map_session_data*,int);
int pc_heal(struct map_session_data *,int,int);
int pc_itemheal(struct map_session_data *sd,int hp,int sp);
int pc_percentheal(struct map_session_data *sd,int,int);
int pc_jobchange(struct map_session_data *,int, int);
int pc_setoption(struct map_session_data *,int);
int pc_setcart(struct map_session_data *sd,int type);
int pc_setfalcon(struct map_session_data *sd);
int pc_setriding(struct map_session_data *sd);
int pc_changelook(struct map_session_data *,int,int);
int pc_equiplookall(struct map_session_data *sd);

int pc_readparam(struct map_session_data*,int);
int pc_setparam(struct map_session_data*,int,int);
int pc_readreg(struct map_session_data*,int);
int pc_setreg(struct map_session_data*,int,int);
char *pc_readregstr(struct map_session_data *sd,int reg);
int pc_setregstr(struct map_session_data *sd,int reg,char *str);
int pc_readglobalreg(struct map_session_data*,char*);
int pc_setglobalreg(struct map_session_data*,char*,int);
int pc_readaccountreg(struct map_session_data*,char*);
int pc_setaccountreg(struct map_session_data*,char*,int);
int pc_readaccountreg2(struct map_session_data*,char*);
int pc_setaccountreg2(struct map_session_data*,char*,int);

int pc_addeventtimer(struct map_session_data *sd,int tick,const char *name);
int pc_deleventtimer(struct map_session_data *sd,const char *name);
int pc_cleareventtimer(struct map_session_data *sd);
int pc_addeventtimercount(struct map_session_data *sd,const char *name,int tick);

int pc_calc_pvprank(struct map_session_data *sd);
int pc_calc_pvprank_timer(int tid,unsigned int tick,int id,int data);

int pc_ismarried(struct map_session_data *sd);
int pc_marriage(struct map_session_data *sd,struct map_session_data *dstsd);
int pc_divorce(struct map_session_data *sd);
int pc_adoption(struct map_session_data *sd,struct map_session_data *dstsd,struct map_session_data *jasd);
struct map_session_data *pc_get_partner(struct map_session_data *sd);
struct map_session_data *pc_get_father(struct map_session_data *sd);
struct map_session_data *pc_get_mother(struct map_session_data *sd);
struct map_session_data *pc_get_child(struct map_session_data *sd);

int pc_set_gm_level(int account_id, int level);
void pc_setstand(struct map_session_data *sd);
int pc_break_equip(struct map_session_data *sd, unsigned short where);
int pc_candrop(struct map_session_data *sd,int item_id);

struct pc_base_job{
	int job; //E‹ÆA‚½‚¾‚µ“]¶E‚â—{qE‚Ìê‡‚ÍŒ³‚ÌE‹Æ‚ğ•Ô‚·(”pƒvƒŠ¨ƒvƒŠ)
	int type; //ƒmƒr 0, ˆêŸE 1, “ñŸE 2, ƒXƒpƒmƒr 3
	int upper; //’Êí 0, “]¶ 1, —{q 2
};

struct pc_base_job pc_calc_base_job(int b_class);//“]¶‚â—{qE‚ÌŒ³‚ÌE‹Æ‚ğ•Ô‚·
int pc_calc_base_job2(int b_class);	// Celest
int pc_calc_upper(int b_class);

struct skill_tree_entry {
	short id;
	unsigned char max;
	unsigned char joblv;
	struct {
		short id;
		unsigned char lv;
	} need[5];
}; // Celest
extern struct skill_tree_entry skill_tree[3][25][MAX_SKILL_TREE];

int pc_read_gm_account(int fd);
int pc_setinvincibletimer(struct map_session_data *sd,int);
int pc_delinvincibletimer(struct map_session_data *sd);
int pc_addspiritball(struct map_session_data *sd,int,int);
int pc_delspiritball(struct map_session_data *sd,int,int);

int pc_eventtimer(int tid,unsigned int tick,int id,int data); // for npc_dequeue 

int pc_readdb(void);
int do_init_pc(void);
void do_final_pc(void);

enum {ADDITEM_EXIST,ADDITEM_NEW,ADDITEM_OVERAMOUNT};

// timer for night.day
extern int day_timer_tid;
extern int night_timer_tid;
int map_day_timer(int,unsigned int,int,int); // by [yor]
int map_night_timer(int,unsigned int,int,int); // by [yor]

#endif

