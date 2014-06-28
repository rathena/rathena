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

enum { MH_MD_FIGHTING = 1, MH_MD_GRAPPLING };

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

/// Houmunculus ID
enum homun_mapid {
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

/// Homunculus type
enum homun_type {
	HT_REG		= 0x1,
	HT_EVO		= 0x2,
	HT_S		= 0x4,
	HT_INVALID	= -1,
};

/// Homunculus battle_config setting
enum homun_setting {
	HOMSET_NO_SUPPORT_SKILL				= 0x01, /// Cannot be targetted by support skills, except for their master
	HOMSET_NO_INSTANT_LAND_SKILL		= 0x02, /// Unit/land skill doesn't applied immediately
	HOMSET_FIRST_TARGET					= 0x04, /// Mobs will always go after them instead of players until attacked
	HOMSET_COPY_SPEED					= 0x08, /// Copy their master's speed on spawn/map-change
	HOMSET_DISPLAY_LUK					= 0x10, /// They display luk/3+1 instead of their actual critical in the stat window, by default they don't crit
	HOMSET_SAME_MATK					= 0x20, /// Their Min-Matk is always the same as their max
	HOMSET_RESET_REUSESKILL_VAPORIZED	= 0x40, /// Skill re-use delay is reset when they are vaporized.
	HOMSET_RESET_REUSESKILL_TELEPORTED	= 0x80, /// Skill re-use delay is reset when they are warped (by skill or item) with player.
};

/// Check Homunculus Class ID
#define homdb_checkid(id) (id >=  HM_CLASS_BASE && id <= HM_CLASS_MAX)

// merc_is_hom_alive(struct homun_data *)
#define hom_is_active(x) (x && x->homunculus.vaporize == HOM_ST_ACTIVE && x->battle_status.hp > 0)
int hom_recv_data(int account_id, struct s_homunculus *sh, int flag); //albator
struct view_data* hom_get_viewdata(int class_);
int hom_class2mapid(int hom_class);
enum homun_type hom_class2type(int class_);
void hom_damage(struct homun_data *hd);
int hom_dead(struct homun_data *hd);
void hom_skillup(struct homun_data *hd,uint16 skill_id);
int hom_calc_skilltree(struct homun_data *hd, int flag_evolve);
int hom_checkskill(struct homun_data *hd,uint16 skill_id);
int hom_gainexp(struct homun_data *hd,int exp);
int hom_levelup(struct homun_data *hd);
int hom_evolution(struct homun_data *hd);
int hom_mutate(struct homun_data *hd,int homun_id);
void hom_heal(struct homun_data *hd);
int hom_vaporize(struct map_session_data *sd, int flag);
int hom_ressurect(struct map_session_data *sd, unsigned char per, short x, short y);
void hom_revive(struct homun_data *hd, unsigned int hp, unsigned int sp);
void hom_reset_stats(struct homun_data *hd);
int hom_shuffle(struct homun_data *hd); // [Zephyrus]
void hom_save(struct homun_data *hd);
int hom_call(struct map_session_data *sd);
int hom_create_request(struct map_session_data *sd, int class_);
int hom_search(int key,int type);
int hom_menu(struct map_session_data *sd,int menunum);
int hom_food(struct map_session_data *sd, struct homun_data *hd);
int hom_hungry_timer_delete(struct homun_data *hd);
int hom_change_name(struct map_session_data *sd,char *name);
int hom_change_name_ack(struct map_session_data *sd, char* name, int flag);
#define hom_stop_walking(hd, type) unit_stop_walking(&(hd)->bl, type)
#define hom_stop_attack(hd) unit_stop_attack(&(hd)->bl)
int hom_increase_intimacy(struct homun_data * hd, unsigned int value);
int hom_decrease_intimacy(struct homun_data * hd, unsigned int value);
int hom_skill_tree_get_max(int id, int b_class);
void hom_init_timers(struct homun_data * hd);
void hom_reload_skill(void);
void hom_reload(void);

int hom_addspiritball(TBL_HOM *hd, int max);
int hom_delspiritball(TBL_HOM *hd, int count, int type);

uint8 hom_get_intimacy_grade(struct homun_data *hd);

void do_final_homunculus(void);
void do_init_homunculus(void);

#endif /* _HOMUNCULUS_H_ */
