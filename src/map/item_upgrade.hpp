// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ITEM_UPGRADE_HPP
#define ITEM_UPGRADE_HPP

#include <string>
#include <vector>

#include "../common/cbasetypes.hpp"
#include "../common/database.hpp"
#include "../common/mmo.hpp"

#include "clif.hpp"
#include "pc.hpp" // struct map_session_data
#include "itemdb.hpp" // struct item_data, struct s_item_group_db
#include "script.hpp"

struct s_item_upgrade_db {
	t_itemid id;
	std::vector<t_itemid> targets;
	script_code *result;
	uint16 source_refine_min;
	uint16 source_refine_max;
	uint16 need_option_num;
	bool not_socket_enchant;
	bool delete_target_onsuccess;

	s_item_upgrade_db();
	~s_item_upgrade_db();

	bool targetExists(t_itemid target_id);
	bool checkRequirement(item *it, item_data *id);
	void setPlayerInfo(map_session_data *sd, uint16 target_index, item *it);
};

class ItemUpgradeDatabase : public TypesafeYamlDatabase<t_itemid, s_item_upgrade_db> {

public:
	ItemUpgradeDatabase() : TypesafeYamlDatabase("ITEM_UPGRADE_DB", 1) {
	}

	void clear();
	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node& node);
};

extern ItemUpgradeDatabase item_upgrade_db;

bool item_upgrade_open(map_session_data *sd, t_itemid itemid);
e_item_upgrade_result item_upgrade_submit(map_session_data *sd, t_itemid source_itemid, uint16 target_index);

void item_upgrade_read_db(void);
void item_upgrade_db_reload(void);

void do_init_item_upgrade(void);
void do_final_item_upgrade(void);

#endif /* ITEM_UPGRADE_HPP */
