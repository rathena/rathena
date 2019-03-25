// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ACHIEVEMENT_HPP
#define ACHIEVEMENT_HPP

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../common/mmo.hpp"
#include "../common/database.hpp"
#include "../common/db.hpp"

struct map_session_data;
struct block_list;

enum e_achievement_group {
	AG_NONE = 0,
	AG_ADD_FRIEND,
	AG_ADVENTURE,
	AG_BABY,
	AG_BATTLE,
	AG_CHAT,
	AG_CHAT_COUNT,
	AG_CHAT_CREATE,
	AG_CHAT_DYING,
	AG_EAT,
	AG_GET_ITEM,
	AG_GET_ZENY,
	AG_GOAL_ACHIEVE,
	AG_GOAL_LEVEL,
	AG_GOAL_STATUS,
	AG_HEAR,
	AG_JOB_CHANGE,
	AG_MARRY,
	AG_PARTY,
	AG_REFINE_FAIL,
	AG_REFINE_SUCCESS,
	AG_SEE,
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

struct achievement_target {
	int mob;
	int count;
};

struct s_achievement_db {
	uint32 achievement_id;
	std::string name;
	enum e_achievement_group group;
	std::unordered_map<uint16, std::shared_ptr<achievement_target>> targets;
	std::vector<uint32> dependent_ids;
	struct script_code* condition;
	int16 mapindex;
	struct ach_reward {
		unsigned short nameid, amount;
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
	// Avoids checking achievements on every mob killed
	std::vector<uint32> achievement_mobs;

public:
	AchievementDatabase() : TypesafeYamlDatabase( "ACHIEVEMENT_DB", 1 ){

	}

	void clear();
	const std::string getDefaultLocation();
	uint64 parseBodyNode( const YAML::Node& node );

	// Additional
	bool mobexists(uint32 mob_id);
};

extern AchievementDatabase achievement_db;

void achievement_get_reward(struct map_session_data *sd, int achievement_id, time_t rewarded);
struct achievement *achievement_add(struct map_session_data *sd, int achievement_id);
bool achievement_remove(struct map_session_data *sd, int achievement_id);
bool achievement_update_achievement(struct map_session_data *sd, int achievement_id, bool complete);
void achievement_check_reward(struct map_session_data *sd, int achievement_id);
void achievement_free(struct map_session_data *sd);
int achievement_check_progress(struct map_session_data *sd, int achievement_id, int type);
int *achievement_level(struct map_session_data *sd, bool flag);
void achievement_get_titles(uint32 char_id);
void achievement_update_objective(struct map_session_data *sd, enum e_achievement_group group, uint8 arg_count, ...);
void achievement_read_db(void);
void achievement_db_reload(void);

void do_init_achievement(void);
void do_final_achievement(void);

#endif /* ACHIEVEMENT_HPP */
