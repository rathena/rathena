// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef QUEST_HPP
#define QUEST_HPP

#include <string>

#include "../common/cbasetypes.hpp"
#include "../common/database.hpp"
#include "../common/strlib.hpp"

#include "map.hpp"

struct map_session_data;
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
};

struct s_quest_db {
	int32 id;
	time_t time;
	bool time_at;
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
	QuestDatabase() : TypesafeYamlDatabase("QUEST_DB", 2, 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node& node);
	bool reload();
};

extern QuestDatabase quest_db;

int quest_pc_login(struct map_session_data *sd);

int quest_add(struct map_session_data *sd, int quest_id);
int quest_delete(struct map_session_data *sd, int quest_id);
int quest_change(struct map_session_data *sd, int qid1, int qid2);
int quest_update_objective_sub(struct block_list *bl, va_list ap);
void quest_update_objective(struct map_session_data *sd, struct mob_data* md);
int quest_update_status(struct map_session_data *sd, int quest_id, e_quest_state status);
int quest_check(struct map_session_data *sd, int quest_id, e_quest_check_type type);

std::shared_ptr<s_quest_db> quest_search(int quest_id);

void do_init_quest(void);
void do_final_quest(void);

#endif /* QUEST_HPP */
