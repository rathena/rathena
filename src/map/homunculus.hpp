// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef HOMUNCULUS_HPP
#define HOMUNCULUS_HPP

#include <string>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>

#include "mob.hpp"
#include "status.hpp" // struct status_data, struct status_change
#include "unit.hpp" // struct unit_data

#ifdef RENEWAL
	#define	HOMUN_LEVEL_STATWEIGHT_VALUE 0
	#define APPLY_HOMUN_LEVEL_STATWEIGHT()( \
		hom.str_value = hom.agi_value = \
		hom.vit_value = hom.int_value = \
		hom.dex_value = hom.luk_value = hom.level / 10 - HOMUN_LEVEL_STATWEIGHT_VALUE \
		)
#else
	#define APPLY_HOMUN_LEVEL_STATWEIGHT()
#endif

struct s_homun_exp_db {
	uint16 level;
	t_exp exp;
};

class HomExpDatabase : public TypesafeYamlDatabase<uint16, s_homun_exp_db> {
public:
	HomExpDatabase() : TypesafeYamlDatabase("HOMUN_EXP_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;

	// Additional
	t_exp get_nextexp(uint16 level);
};

struct s_hom_stats {
	unsigned int HP, SP;
	unsigned short str, agi, vit, int_, dex, luk;
};

/// Homunculus skill entry [Celest]
struct s_homun_skill_tree_entry {
	uint16 id;			///< Skill ID
	uint16 max;			///< Max level for this tree
	uint16 need_level;	///< Homunculus level required
	uint32 intimacy;	///< Intimacy required (n/100)
	bool evolution;		///< Require evolution to show on skill tree
	std::unordered_map<uint16, uint16> need; ///< Skills needed
};

struct s_homunculus_db {
	int base_class, evo_class;
	char name[NAME_LENGTH];
	struct s_hom_stats base, gmin, gmax, emin, emax;
	int foodID;
	uint16 baseASPD;
	int hungryDelay;
	e_element element;
	e_race race;
	e_size base_size, evo_size;
	std::vector<s_homun_skill_tree_entry> skill_tree;
};

enum e_hom_mode : uint8  { MH_MD_FIGHTING = 1, MH_MD_GRAPPLING };

enum e_hom_state : uint8 {
	HOM_ST_ACTIVE	= 0,
	HOM_ST_REST		= 1,
	HOM_ST_MORPH	= 2,
};

enum e_hom_state2 : uint8 {
	SP_ACK      = 0x0,
	SP_INTIMATE = 0x1,
	SP_HUNGRY   = 0x2,
};

struct homun_data {
	struct block_list bl;
	struct unit_data  ud;
	struct view_data *vd;
	struct status_data base_status, battle_status;
	status_change sc;
	struct regen_data regen;
	std::shared_ptr<s_homunculus_db> homunculusDB;	//[orn]
	struct s_homunculus homunculus;	//[orn]

	int masterteleport_timer;
	map_session_data *master; //pointer back to its master
	int hungry_timer;	//[orn]
	t_exp exp_next;
	std::vector<uint16> blockskill;	// [orn]
};

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

/// Homunculus class constants
enum e_homun_classid : uint16 {
	MER_LIF = 6001,
	MER_AMISTR,
	MER_FILIR,
	MER_VANILMIRTH,
	MER_LIF2,
	MER_AMISTR2,
	MER_FILIR2,
	MER_VANILMIRTH2,
	MER_LIF_H,
	MER_AMISTR_H,
	MER_FILIR_H,
	MER_VANILMIRTH_H,
	MER_LIF_H2,
	MER_AMISTR_H2,
	MER_FILIR_H2,
	MER_VANILMIRTH_H2,

	// Homunculus S
	MER_EIRA = 6048,
	MER_BAYERI,
	MER_SERA,
	MER_DIETER,
	MER_ELEANOR,
};

/// Homunculus type
enum homun_type : int8 {
	HT_REG		= 0x1,
	HT_EVO		= 0x2,
	HT_S		= 0x4,
	HT_INVALID	= -1,
};

/// Homunculus battle_config setting
enum homun_setting : uint8 {
	HOMSET_NO_SUPPORT_SKILL				= 0x01, /// Cannot be targetted by support skills, except for their master
	HOMSET_NO_INSTANT_LAND_SKILL		= 0x02, /// Unit/land skill doesn't applied immediately
	HOMSET_FIRST_TARGET					= 0x04, /// Mobs will always go after them instead of players until attacked
	HOMSET_COPY_SPEED					= 0x08, /// Copy their master's speed on spawn/map-change
	HOMSET_DISPLAY_LUK					= 0x10, /// They display luk/3+1 instead of their actual critical in the stat window, by default they don't crit
	HOMSET_SAME_MATK					= 0x20, /// Their Min-Matk is always the same as their max
};

enum e_homun_grade : uint8 {
	HOMGRADE_HATE_WITH_PASSION = 0,
	HOMGRADE_HATE,
	HOMGRADE_AWKWARD,
	HOMGRADE_SHY,
	HOMGRADE_NEUTRAL,
	HOMGRADE_CORDIAL,
	HOMGRADE_LOYAL,
};

class HomunculusDatabase : public TypesafeYamlDatabase<int32, s_homunculus_db> {
private:
	bool parseStatusNode(const std::string &nodeName, const std::string &subNodeName, const ryml::NodeRef &node, s_hom_stats &bonus);

public:
	HomunculusDatabase() : TypesafeYamlDatabase("HOMUNCULUS_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const ryml::NodeRef& node);

	// Additional
	std::shared_ptr<s_homunculus_db> homun_search(int32 class_);
};

extern HomunculusDatabase homunculus_db;

/// Check Homunculus Class ID
#define homdb_checkid(id) ((id) >=  HM_CLASS_BASE && (id) <= HM_CLASS_MAX)

// merc_is_hom_alive(struct homun_data *)
#define hom_is_active(x) ((x) && (x)->homunculus.vaporize == HOM_ST_ACTIVE && (x)->battle_status.hp > 0)
int hom_recv_data(uint32 account_id, struct s_homunculus *sh, int flag); //albator
struct view_data* hom_get_viewdata(int class_);
int hom_class2mapid(int hom_class);
enum homun_type hom_class2type(int class_);
void hom_damage(struct homun_data *hd);
int hom_dead(struct homun_data *hd);
void hom_skillup(struct homun_data *hd,uint16 skill_id);
void hom_calc_skilltree(homun_data *hd);
short hom_checkskill(struct homun_data *hd,uint16 skill_id);
uint16 hom_skill_get_min_level(int class_, uint16 skill_id);
void hom_gainexp(struct homun_data *hd,t_exp exp);
int hom_levelup(struct homun_data *hd);
int hom_evolution(struct homun_data *hd);
int hom_mutate(struct homun_data *hd,int homun_id);
void hom_heal(struct homun_data *hd);
int hom_vaporize(map_session_data *sd, int flag);
int hom_ressurect(map_session_data *sd, unsigned char per, short x, short y);
void hom_revive(struct homun_data *hd, unsigned int hp, unsigned int sp);
void hom_reset_stats(struct homun_data *hd);
int hom_shuffle(struct homun_data *hd); // [Zephyrus]
void hom_save(struct homun_data *hd);
bool hom_call(map_session_data *sd);
bool hom_create_request(map_session_data *sd, int class_);
void hom_menu(map_session_data *sd,int type);
int hom_food(map_session_data *sd, struct homun_data *hd);
int hom_hungry_timer_delete(struct homun_data *hd);
int hom_change_name(map_session_data *sd,char *name);
void hom_change_name_ack(map_session_data *sd, char* name, int flag);
#define hom_stop_walking(hd, type) unit_stop_walking(&(hd)->bl, type)
#define hom_stop_attack(hd) unit_stop_attack(&(hd)->bl)
int hom_increase_intimacy(struct homun_data * hd, unsigned int value);
int hom_decrease_intimacy(struct homun_data * hd, unsigned int value);
int hom_skill_tree_get_max(int skill_id, int b_class);
void hom_init_timers(struct homun_data * hd);
void hom_reload(void);

void hom_addspiritball(TBL_HOM *hd, int max);
void hom_delspiritball(TBL_HOM *hd, int count, int type);

uint8 hom_get_intimacy_grade(struct homun_data *hd);
uint32 hom_intimacy_grade2intimacy(enum e_homun_grade grade);
enum e_homun_grade hom_intimacy_intimacy2grade(uint32 intimacy);

short hom_skill_get_index(uint16 skill_id);

void do_final_homunculus(void);
void do_init_homunculus(void);

#endif /* HOMUNCULUS_HPP */
