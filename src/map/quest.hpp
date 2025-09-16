// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef QUEST_HPP
#define QUEST_HPP

#include <string>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/strlib.hpp>

#include "map.hpp"

class map_session_data;
enum e_size : uint8;

struct s_quest_dropitem {
	t_itemid nameid;
	uint16 count;
	uint16 rate;
	uint16 mob_id;
	//uint8 bound;
	//bool isAnnounced;
	//bool isGUID;
};

struct s_quest_objective {
	uint16 index;
	uint16 mob_id;
	uint16 count;
	uint16 min_level, max_level;
	e_race race;
	e_size size;
	e_element element;
	int16 mapid;
	std::string map_name;
	std::vector<uint16> mobs_allowed;
};

struct s_quest_db {
	int32 id;
	time_t time;
	bool time_at;
	int32 time_week;
	std::vector<std::shared_ptr<s_quest_objective>> objectives;
	std::vector<std::shared_ptr<s_quest_dropitem>> dropitem;
	std::string name;
};

// Questlog check types
enum e_quest_check_type : uint8 {
	HAVEQUEST, ///< Query the state of the given quest
	PLAYTIME,  ///< Check if the given quest has been completed or has yet to expire
	HUNTING,   ///< Check if the given hunting quest's requirements have been met
};

class QuestDatabase : public TypesafeYamlDatabase<uint32, s_quest_db> {
public:
	QuestDatabase() : TypesafeYamlDatabase("QUEST_DB", 3, 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;

	// Additional
	bool reload();
};

extern QuestDatabase quest_db;

int32 quest_pc_login(map_session_data *sd);

int32 quest_add(map_session_data *sd, int32 quest_id);
int32 quest_delete(map_session_data *sd, int32 quest_id);
int32 quest_change(map_session_data *sd, int32 qid1, int32 qid2);
int32 quest_update_objective_sub(block_list *bl, va_list ap);
void quest_update_objective(map_session_data *sd, mob_data* md);
int32 quest_update_status(map_session_data *sd, int32 quest_id, e_quest_state status);
int32 quest_check(map_session_data *sd, int32 quest_id, e_quest_check_type type);

std::shared_ptr<s_quest_db> quest_search(int32 quest_id);

void do_init_quest(void);
void do_final_quest(void);

#endif /* QUEST_HPP */
