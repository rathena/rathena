// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _HOMUNCULUS_H_
#define _HOMUNCULUS_H_

#include "status.h" // struct status_data, struct status_change
#include "unit.h" // struct unit_data

struct h_stats {
	unsigned int HP, SP;
	unsigned short str, agi, vit, int_, dex, luk;
};

struct s_homunculus_db {
	int base_class, evo_class;
	char name[NAME_LENGTH];
	struct h_stats base, gmin, gmax, emin, emax;
	int foodID ;
	int baseASPD ;
	long hungryDelay ;
	unsigned char element, race, base_size, evo_size;
};

extern struct s_homunculus_db homuncumlus_db[MAX_HOMUNCULUS_CLASS];
enum { HOMUNCULUS_CLASS, HOMUNCULUS_FOOD };
enum {
	SP_ACK 	= 0x00,
	SP_INTIMATE 	= 0x100,
	SP_HUNGRY 		= 0x200
};

struct homun_data {
	struct block_list bl;
	struct unit_data  ud;
	struct view_data *vd;
	struct status_data base_status, battle_status;
	struct status_change sc;
	struct regen_data regen;
	struct s_homunculus_db *homunculusDB;	//[orn]
	struct s_homunculus homunculus;	//[orn]

	struct map_session_data *master; //pointer back to its master
	int hungry_timer;	//[orn]
	unsigned int exp_next;
	char blockskill[MAX_SKILL];	// [orn]
};


#define homdb_checkid(id) (id >=  HM_CLASS_BASE && id <= HM_CLASS_MAX)

// merc_is_hom_alive(struct homun_data *)
#define merc_is_hom_active(x) (x && x->homunculus.vaporize != 1 && x->battle_status.hp > 0)
int do_init_merc(void);
int merc_hom_recv_data(int account_id, struct s_homunculus *sh, int flag); //albator
struct view_data* merc_get_hom_viewdata(int class_);
void merc_damage(struct homun_data *hd,struct block_list *src,int hp,int sp);
int merc_hom_dead(struct homun_data *hd, struct block_list *src);
void merc_hom_skillup(struct homun_data *hd,int skillnum);
int merc_hom_calc_skilltree(struct homun_data *hd) ;
int merc_hom_checkskill(struct homun_data *hd,int skill_id) ;
int merc_hom_gainexp(struct homun_data *hd,int exp) ;
int merc_hom_levelup(struct homun_data *hd) ;
int merc_hom_evolution(struct homun_data *hd) ;
void merc_hom_heal(struct homun_data *hd,int hp,int sp);
int merc_hom_vaporize(struct map_session_data *sd, int flag);
int merc_resurrect_homunculus(struct map_session_data *sd, unsigned char per, short x, short y);
void merc_hom_revive(struct homun_data *hd, unsigned int hp, unsigned int sp);
void merc_reset_stats(struct homun_data *hd);
int merc_hom_shuffle(struct homun_data *hd); // [Zephyrus]
void merc_save(struct homun_data *hd);
int merc_call_homunculus(struct map_session_data *sd);
int merc_create_homunculus_request(struct map_session_data *sd, int class_);
int search_homunculusDB_index(int key,int type);
int merc_menu(struct map_session_data *sd,int menunum);
int merc_hom_food(struct map_session_data *sd, struct homun_data *hd);
int merc_hom_hungry_timer_delete(struct homun_data *hd);
int merc_hom_change_name(struct map_session_data *sd,char *name);
int merc_hom_change_name_ack(struct map_session_data *sd, char* name, int flag);
#define merc_stop_walking(hd, type) unit_stop_walking(&(hd)->bl, type)
#define merc_stop_attack(hd) unit_stop_attack(&(hd)->bl)
int merc_hom_increase_intimacy(struct homun_data * hd, unsigned int value);
int merc_hom_decrease_intimacy(struct homun_data * hd, unsigned int value);
int merc_skill_tree_get_max(int id, int b_class);
void merc_hom_init_timers(struct homun_data * hd);
void merc_skill_reload(void);
void merc_reload(void);

#endif /* _HOMUNCULUS_H_ */
