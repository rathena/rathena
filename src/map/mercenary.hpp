// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MERCENARY_HPP
#define MERCENARY_HPP

#include "../common/cbasetypes.hpp"

#include "status.hpp" // struct status_data, struct status_change
#include "unit.hpp" // struct unit_data

// number of cells that a mercenary can walk to from it's master before being warped
#define MAX_MER_DISTANCE 15

enum e_MercGuildType {
	NONE_MERC_GUILD = -1,
	ARCH_MERC_GUILD,
	SPEAR_MERC_GUILD,
	SWORD_MERC_GUILD,
};

enum MERID {
	MERID_MER_ARCHER01 = 6017,
	MERID_MER_ARCHER10 = 6026,
	MERID_MER_LANCER01,
	MERID_MER_LANCER10 = 6036,
	MERID_MER_SWORDMAN01,
	MERID_MER_SWORDMAN10 = 6046
};

struct s_mercenary_db {
	int32 class_;
	std::string sprite, name;
	uint16 lv;
	uint16 range2, range3;
	status_data status;
	view_data vd;
	std::unordered_map<uint16, uint16> skill;
};

struct s_mercenary_data {
	struct block_list bl;
	struct unit_data ud;
	struct view_data *vd;
	struct status_data base_status, battle_status;
	struct status_change sc;
	struct regen_data regen;

	std::shared_ptr<s_mercenary_db> db;
	struct s_mercenary mercenary;
	std::vector<uint16> blockskill;

	int masterteleport_timer;
	struct map_session_data *master;
	int contract_timer;

	unsigned devotion_flag : 1;
};

struct view_data * mercenary_get_viewdata(uint16 class_);

class MercenaryDatabase : public TypesafeYamlDatabase<int32, s_mercenary_db> {
public:
	MercenaryDatabase() : TypesafeYamlDatabase("MERCENARY_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node& node);
};

extern MercenaryDatabase mercenary_db;

bool mercenary_create(struct map_session_data *sd, uint16 class_, unsigned int lifetime);
bool mercenary_recv_data(struct s_mercenary *merc, bool flag);
void mercenary_save(struct s_mercenary_data *md);

void mercenary_heal(struct s_mercenary_data *md, int hp, int sp);
bool mercenary_dead(struct s_mercenary_data *md);

int mercenary_delete(struct s_mercenary_data *md, int reply);
void mercenary_contract_stop(struct s_mercenary_data *md);

t_tick mercenary_get_lifetime(struct s_mercenary_data *md);
enum e_MercGuildType mercenary_get_guild(struct s_mercenary_data *md);
int mercenary_get_faith(struct s_mercenary_data *md);
void mercenary_set_faith(struct s_mercenary_data *md, int value);
int mercenary_get_calls(struct s_mercenary_data *md);
void mercenary_set_calls(struct s_mercenary_data *md, int value);
void mercenary_kills(struct s_mercenary_data *md);

uint16 mercenary_checkskill(struct s_mercenary_data *md, uint16 skill_id);

void do_init_mercenary(void);
void do_final_mercenary(void);

#endif /* MERCENARY_HPP */
