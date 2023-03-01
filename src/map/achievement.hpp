// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ACHIEVEMENT_HPP
#define ACHIEVEMENT_HPP

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <common/mmo.hpp>
#include <common/database.hpp>
#include <common/db.hpp>

class map_session_data;
struct block_list;

enum e_achievement_group {
	AG_NONE = 0,
	AG_ADD_FRIEND,
	AG_ADVENTURE,
	AG_BABY,
	AG_BATTLE,
	AG_CHATTING,
	AG_CHATTING_COUNT,
	AG_CHATTING_CREATE,
	AG_CHATTING_DYING,
	AG_EAT,
	AG_GET_ITEM,
	AG_GET_ZENY,
	AG_GOAL_ACHIEVE,
	AG_GOAL_LEVEL,
	AG_GOAL_STATUS,
	AG_JOB_CHANGE,
	AG_MARRY,
	AG_PARTY,
	AG_ENCHANT_FAIL,
	AG_ENCHANT_SUCCESS,
	AG_SPEND_ZENY,
	AG_TAMING,
	AG_MAX
};

enum e_achievement_info {
	ACHIEVEINFO_COUNT1 = 1,
	ACHIEVEINFO_COUNT2,
	ACHIEVEINFO_COUNT3,
	ACHIEVEINFO_COUNT4,
	ACHIEVEINFO_COUNT5,
	ACHIEVEINFO_COUNT6,
	ACHIEVEINFO_COUNT7,
	ACHIEVEINFO_COUNT8,
	ACHIEVEINFO_COUNT9,
	ACHIEVEINFO_COUNT10,
	ACHIEVEINFO_COMPLETE,
	ACHIEVEINFO_COMPLETEDATE,
	ACHIEVEINFO_GOTREWARD,
	ACHIEVEINFO_LEVEL,
	ACHIEVEINFO_SCORE,
	ACHIEVEINFO_MAX,
};

enum e_title_table : uint16 {
	TITLE_NONE = 0,
	TITLE_BASE = 1000,
	TITLE_MAX = 1046,
};

struct achievement_target {
	int mob;
	int count;
};

struct s_achievement_db {
	uint32 achievement_id;
	std::string name;
	enum e_achievement_group group;
	std::map<uint16, std::shared_ptr<achievement_target>> targets;
	std::vector<uint32> dependent_ids;
	struct script_code* condition;
	int16 mapindex;
	struct ach_reward {
		t_itemid nameid;
		unsigned short amount;
		struct script_code *script;
		uint32 title_id;
		ach_reward();
		~ach_reward();
	} rewards;
	int score;
	int has_dependent; // Used for quick updating of achievements that depend on others - this is their ID

	s_achievement_db();
	~s_achievement_db();
};

class AchievementDatabase : public TypesafeYamlDatabase<uint32, s_achievement_db>{
private:
	std::vector<uint32> achievement_mobs; // Avoids checking achievements on every mob killed

public:
	AchievementDatabase() : TypesafeYamlDatabase( "ACHIEVEMENT_DB", 2 ){

	}

	void clear() override;
	const std::string getDefaultLocation() override;
	uint64 parseBodyNode( const ryml::NodeRef& node ) override;
	void loadingFinished() override;

	// Additional
	bool mobexists(uint32 mob_id);
};

extern AchievementDatabase achievement_db;

struct s_achievement_level{
	uint16 level;
	uint16 points;
};

class AchievementLevelDatabase : public TypesafeYamlDatabase<uint16, s_achievement_level>{
public:
	AchievementLevelDatabase() : TypesafeYamlDatabase( "ACHIEVEMENT_LEVEL_DB", 1 ){

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode( const ryml::NodeRef& node ) override;
};

extern AchievementLevelDatabase achievement_level_db;

void achievement_get_reward(map_session_data *sd, int achievement_id, time_t rewarded);
struct achievement *achievement_add(map_session_data *sd, int achievement_id);
bool achievement_remove(map_session_data *sd, int achievement_id);
bool achievement_update_achievement(map_session_data *sd, int achievement_id, bool complete);
void achievement_check_reward(map_session_data *sd, int achievement_id);
void achievement_free(map_session_data *sd);
int achievement_check_progress(map_session_data *sd, int achievement_id, int type);
int *achievement_level(map_session_data *sd, bool flag);
bool achievement_check_condition(struct script_code* condition, map_session_data* sd);
void achievement_get_titles(uint32 char_id);
void achievement_update_objective(map_session_data *sd, enum e_achievement_group group, uint8 arg_count, ...);
int achievement_update_objective_sub(block_list *bl, va_list ap);
void achievement_read_db(void);
void achievement_db_reload(void);

void do_init_achievement(void);
void do_final_achievement(void);

#endif /* ACHIEVEMENT_HPP */
