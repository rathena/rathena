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

extern struct s_homunculus_db homunculus_db[MAX_HOMUNCULUS_CLASS];
enum { HOMUNCULUS_CLASS, HOMUNCULUS_FOOD };

enum { MH_MD_FIGHTING=1, MH_MD_GRAPPLING };

enum {
	HOM_ST_ACTIVE	= 0,
	HOM_ST_REST		= 1,
	HOM_ST_MORPH	= 2,
};

enum {
	SP_ACK      = 0x0,
	SP_INTIMATE = 0x1,
	SP_HUNGRY   = 0x2,
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

	int masterteleport_timer;
	struct map_session_data *master; //pointer back to its master
	int hungry_timer;	//[orn]
	unsigned int exp_next;
	char blockskill[MAX_SKILL];	// [orn]
};

#define MAX_HOM_SKILL_REQUIRE 5
struct homun_skill_tree_entry {
	short id;
	unsigned char max;
	unsigned char joblv;
	short intimacylv;
	struct {
		short id;
		unsigned char lv;
	} need[MAX_HOM_SKILL_REQUIRE];
}; // Celest

#define HOM_EVO 0x100 //256
#define HOM_S 0x200 //512
#define HOM_REG 0x1000 //4096

enum {
// Normal Homunculus
	MAPID_LIF = HOM_REG|0x0,
	MAPID_AMISTR,
	MAPID_FILIR,
	MAPID_VANILMIRTH,
// Evolved Homunulus
	MAPID_LIF_E = HOM_REG|HOM_EVO|0x0,
	MAPID_AMISTR_E,
	MAPID_FILIR_E,
	MAPID_VANILMIRTH_E,
// Homunculus S
	MAPID_EIRA = HOM_S|0x0,
	MAPID_BAYERI,
	MAPID_SERA,
	MAPID_DIETER,
	MAPID_ELANOR,
};
enum homun_type {
	HT_REG	= 0x1,
	HT_EVO	= 0x2,
	HT_S	= 0x4,
};

#define homdb_checkid(id) (id >=  HM_CLASS_BASE && id <= HM_CLASS_MAX)

// merc_is_hom_alive(struct homun_data *)
#define merc_is_hom_active(x) (x && x->homunculus.vaporize == HOM_ST_ACTIVE && x->battle_status.hp > 0)
int do_init_merc(void);
int merc_hom_recv_data(int account_id, struct s_homunculus *sh, int flag); //albator
struct view_data* merc_get_hom_viewdata(int class_);
int hom_class2mapid(int hom_class);
enum homun_type hom_class2type(int class_);
void merc_damage(struct homun_data *hd);
int merc_hom_dead(struct homun_data *hd);
void merc_hom_skillup(struct homun_data *hd,uint16 skill_id);
int merc_hom_calc_skilltree(struct homun_data *hd, int flag_evolve);
int merc_hom_checkskill(struct homun_data *hd,uint16 skill_id);
int merc_hom_gainexp(struct homun_data *hd,int exp);
int merc_hom_levelup(struct homun_data *hd);
int merc_hom_evolution(struct homun_data *hd);
int hom_mutate(struct homun_data *hd,int homun_id);
void merc_hom_heal(struct homun_data *hd);
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

int hom_addspiritball(TBL_HOM *hd, int max);
int hom_delspiritball(TBL_HOM *hd, int count, int type);

#endif /* _HOMUNCULUS_H_ */
