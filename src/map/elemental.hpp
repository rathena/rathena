// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ELEMENTAL_HPP
#define ELEMENTAL_HPP

#include "../common/database.hpp"
#include "../common/mmo.hpp"
#include "../common/timer.hpp"

#include "status.hpp" // struct status_data, struct status_change
#include "unit.hpp" // struct unit_data

const t_tick MIN_ELETHINKTIME = 100;
#define MIN_ELEDISTANCE 2
#define MAX_ELEDISTANCE 5

#define EL_MODE_AGGRESSIVE (MD_CANMOVE|MD_AGGRESSIVE|MD_CANATTACK)
#define EL_MODE_ASSIST (MD_CANMOVE|MD_ASSIST)
#define EL_MODE_PASSIVE MD_CANMOVE

///Enum of Elemental Skill Mode
enum e_elemental_skillmode : uint8 {
	EL_SKILLMODE_PASSIVE    = 0x1,
	EL_SKILLMODE_ASSIST     = 0x2,
	EL_SKILLMODE_AGGRESSIVE = 0x4,
};

///Enum of Elemental ID
enum elemental_elementalid  : uint16 {
	ELEMENTALID_AGNI_S = 2114,
	ELEMENTALID_AGNI_M,
	ELEMENTALID_AGNI_L,
	ELEMENTALID_AQUA_S,
	ELEMENTALID_AQUA_M,
	ELEMENTALID_AQUA_L,
	ELEMENTALID_VENTUS_S,
	ELEMENTALID_VENTUS_M,
	ELEMENTALID_VENTUS_L,
	ELEMENTALID_TERA_S,
	ELEMENTALID_TERA_M,
	ELEMENTALID_TERA_L,
};

struct s_elemental_skill {
	uint16 id, lv;
};

struct s_elemental_db {
	int32 class_;
	std::string sprite, name;
	uint16 lv;
	uint16 range2, range3;
	status_data status;
	view_data vd;
	std::unordered_map<e_elemental_skillmode, std::shared_ptr<s_elemental_skill>> skill;	/// mode, skill
};

struct s_elemental_data {
	struct block_list bl;
	struct unit_data ud;
	struct view_data *vd;
	struct status_data base_status, battle_status;
	struct status_change sc;
	struct regen_data regen;

	std::shared_ptr<s_elemental_db> db;
	struct s_elemental elemental;

	int masterteleport_timer;
	struct map_session_data *master;
	int summon_timer;
	int skill_timer;

	t_tick last_thinktime, last_linktime, last_spdrain_time;
	short min_chase;
	int target_id, attacked_id;
};

class ElementalDatabase : public TypesafeYamlDatabase<int32, s_elemental_db> {
public:
	ElementalDatabase() : TypesafeYamlDatabase("ELEMENTAL_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node& node);
};

extern ElementalDatabase elemental_db;

struct view_data * elemental_get_viewdata(int class_);

int elemental_create(struct map_session_data *sd, int class_, unsigned int lifetime);
int elemental_data_received(struct s_elemental *ele, bool flag);
int elemental_save(struct s_elemental_data *ed);

int elemental_change_mode_ack(struct s_elemental_data *ed, e_elemental_skillmode skill_mode);
int elemental_change_mode(struct s_elemental_data *ed, e_mode mode);

void elemental_heal(struct s_elemental_data *ed, int hp, int sp);
int elemental_dead(struct s_elemental_data *ed);

int elemental_delete(struct s_elemental_data *ed);
void elemental_summon_stop(struct s_elemental_data *ed);

t_tick elemental_get_lifetime(struct s_elemental_data *ed);

int elemental_unlocktarget(struct s_elemental_data *ed);
bool elemental_skillnotok(uint16 skill_id, struct s_elemental_data *ed);
int elemental_set_target( struct map_session_data *sd, struct block_list *bl );
int elemental_clean_single_effect(struct s_elemental_data *ed, uint16 skill_id);
int elemental_clean_effect(struct s_elemental_data *ed);
int elemental_action(struct s_elemental_data *ed, struct block_list *bl, t_tick tick);
struct s_skill_condition elemental_skill_get_requirements(uint16 skill_id, uint16 skill_lv);

#define elemental_stop_walking(ed, type) unit_stop_walking(&(ed)->bl, type)
#define elemental_stop_attack(ed) unit_stop_attack(&(ed)->bl)

void read_elemental_skilldb(void);
void do_init_elemental(void);
void do_final_elemental(void);

#endif /* ELEMENTAL_HPP */
