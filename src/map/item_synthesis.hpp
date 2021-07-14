// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ITEM_SYNTHESIS_HPP
#define ITEM_SYNTHESIS_HPP

#include <string>
#include <vector>

#include "../common/cbasetypes.hpp"
#include "../common/database.hpp"
#include "../common/mmo.hpp"

#include "clif.hpp"
#include "pc.hpp" // struct map_session_data
#include "itemdb.hpp" // struct item_data, struct s_item_group_db
#include "script.hpp"

#define MAX_SYNTHESIS_SOURCES 10 // Client's limit for source is 10

struct s_item_synthesis_list {
	uint16 index;
	uint16 amount;
};

struct s_item_synthesis_source {
	t_itemid nameid;
	uint16 amount;

	s_item_synthesis_source();
};

struct s_item_synthesis_db {
	t_itemid id;
	uint16 source_needed;
	std::vector<s_item_synthesis_source> sources;
	script_code *reward;
	uint16 source_refine_min;
	uint16 source_refine_max;

	s_item_synthesis_db();
	~s_item_synthesis_db();

	bool sourceExists(t_itemid source_id);
	bool checkRequirement(map_session_data *sd, const std::vector<s_item_synthesis_list> items);
	bool deleteRequirement(map_session_data *sd, const std::vector<s_item_synthesis_list> items);
};

class ItemSynthesisDatabase : public TypesafeYamlDatabase<t_itemid, s_item_synthesis_db> {

public:
	ItemSynthesisDatabase() : TypesafeYamlDatabase("ITEM_SYNTHESIS_DB", 1) {
	}

	void clear();
	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node& node);
};

extern ItemSynthesisDatabase item_synthesis_db;

bool item_synthesis_open(map_session_data *sd, t_itemid itemid);
e_item_synthesis_result item_synthesis_submit(map_session_data *sd, t_itemid itemid, const std::vector<s_item_synthesis_list> items);

void item_synthesis_read_db(void);
void item_synthesis_db_reload(void);

void do_init_item_synthesis(void);
void do_final_item_synthesis(void);

#endif /* ITEM_SYNTHESIS_HPP */
