// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "achievement.hpp"

#include <array>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/cbasetypes.hpp"
#include "../common/database.hpp"
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/utilities.hpp"
#include "../common/utils.hpp"

#include "battle.hpp"
#include "chrif.hpp"
#include "clif.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "map.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "pc.hpp"
#include "script.hpp"
#include "status.hpp"

using namespace rathena;

void AchievementDatabase::clear(){
	TypesafeYamlDatabase::clear();
	this->achievement_mobs.clear();
}

const std::string AchievementDatabase::getDefaultLocation(){
	return std::string(db_path) + "/achievement_db.yml";
}

/**
 * Reads and parses an entry from the achievement_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 AchievementDatabase::parseBodyNode(const ryml::NodeRef& node){
	uint32 achievement_id;

	if( !this->asUInt32( node, "Id", achievement_id ) ){
		return 0;
	}

	std::shared_ptr<s_achievement_db> achievement = this->find( achievement_id );
	bool exists = achievement != nullptr;

	if( !exists ){
		if( !this->nodesExist( node, { "Name" } ) ){
			return 0;
		}

		achievement = std::make_shared<s_achievement_db>();
		achievement->achievement_id = achievement_id;
	}

	if( this->nodeExists( node, "Group" ) ){
		std::string group_name;

		if( !this->asString( node, "Group", group_name ) ){
			return 0;
		}

		std::string group_name_constant = "AG_" + group_name;
		int64 constant;

		if( !script_get_constant( group_name_constant.c_str(), &constant ) ){
			this->invalidWarning( node, "Invalid Group %s for achievement %d, skipping.\n", group_name.c_str(), achievement_id );
			return 0;
		}

		achievement->group = (e_achievement_group)constant;
	} else {
		if (!exists)
			achievement->group = AG_NONE;
	}

	if( this->nodeExists( node, "Name" ) ){
		std::string name;

		if( !this->asString( node, "Name", name ) ){
			return 0;
		}

		achievement->name = name;
	}

	if( this->nodeExists( node, "Targets" ) ){
		const auto& targets = node["Targets"];

		for( const auto& targetNode : targets ){
			uint16 targetId;

			if( !this->asUInt16( targetNode, "Id", targetId ) ){
				continue;
			}

			if( targetId >= MAX_ACHIEVEMENT_OBJECTIVES ){
				this->invalidWarning( targetNode["Id"], "Target Id is out of valid range [0,%d], skipping.\n", MAX_ACHIEVEMENT_OBJECTIVES );
				continue;
			}

			std::shared_ptr<achievement_target> target = rathena::util::map_find( achievement->targets, targetId );
			bool targetExists = target != nullptr;

			if( !targetExists ){
				if( !this->nodeExists( targetNode, "Count" ) && !this->nodeExists( targetNode, "Mob" ) ){
					this->invalidWarning( targetNode, "Target has no data specified, skipping.\n" );
					return 0;
				}

				target = std::make_shared<achievement_target>();
			}
			
			if( this->nodeExists( targetNode, "Count" ) ){
				uint32 count;

				if( !this->asUInt32( targetNode, "Count", count ) ){
					return 0;
				}

				if( count == 0 ){
					if( targetExists ){
						achievement->targets.erase( targetId );
						continue;
					}else{
						this->invalidWarning( targetNode["Count"], "Target count has to be > 0, skipping.\n" );
						return 0;
					}
				}

				target->count = count;
			}else{
				if( !targetExists ){
					target->count = 1;
				}
			}

			if( this->nodeExists( targetNode, "Mob" ) ){
				if( achievement->group != AG_BATTLE && achievement->group != AG_TAMING ){
					this->invalidWarning( targets, "Target Mob is only supported for targets in group AG_BATTLE or AG_TAMING, skipping.\n" );
					continue;
				}

				std::string mob_name;

				if( !this->asString( targetNode, "Mob", mob_name ) ){
					return 0;
				}

				std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname( mob_name.c_str() );

				if (mob == nullptr) {
					this->invalidWarning(targetNode["Mob"], "Target Mob %s does not exist, skipping.\n", mob_name.c_str());
					return 0;
				}

				uint32 mob_id = mob->id;

				if( !this->mobexists( mob_id ) ){
					this->achievement_mobs.push_back( mob_id );
				}

				target->mob = mob_id;
			}else{
				if( !targetExists ){
					target->mob = 0;
				}
			}

			achievement->targets[targetId] = target;
		}
	}

	if( this->nodeExists( node, "Condition" ) ){
		std::string condition;

		if( !this->asString( node, "Condition", condition ) ){
			return 0;
		}

		if( condition.find( "achievement_condition" ) == std::string::npos ){
			condition = "achievement_condition( " + condition + " );";
		}

		if( achievement->condition ){
			script_free_code( achievement->condition );
			achievement->condition = nullptr;
		}

		achievement->condition = parse_script( condition.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(node["Condition"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS );
	}else{
		if (!exists)
			achievement->condition = nullptr;
	}

	if( this->nodeExists( node, "Map" ) ){
		if( achievement->group != AG_CHATTING ){
			this->invalidWarning( node, "Map can only be used with the group AG_CHATTING, skipping.\n" );
			return 0;
		}

		std::string mapname;

		if( !this->asString( node, "Map", mapname ) ){
			return 0;
		}

		achievement->mapindex = map_mapname2mapid( mapname.c_str() );

		if( achievement->mapindex == -1 ){
			this->invalidWarning( node["Map"], "Map %s does not exist, skipping.\n", mapname.c_str() );
			return 0;
		}
	}else{
		if( !exists ){
			achievement->mapindex = -1;
		}
	}

	if( this->nodeExists( node, "Dependents" ) ){
		const auto& dependentNode = node["Dependents"];

		for( const auto& it : dependentNode ){
			auto id_str = it.key();
			uint32 dependent_achievement_id;
			c4::atou<uint32>(id_str, &dependent_achievement_id);
			bool active;

			if (!this->asBool(dependentNode, std::to_string(dependent_achievement_id), active))
				return 0;

			if (active) {
				if (std::find(achievement->dependent_ids.begin(), achievement->dependent_ids.end(), dependent_achievement_id) != achievement->dependent_ids.end()) {
					this->invalidWarning(dependentNode, "Dependent achievement %d is already part of the list, skipping.\n", dependent_achievement_id);
					continue;
				}

				if (achievement->dependent_ids.size() >= MAX_ACHIEVEMENT_DEPENDENTS) {
					this->invalidWarning(dependentNode, "Maximum amount (%d) of dependent achievements reached, skipping.\n", MAX_ACHIEVEMENT_DEPENDENTS);
					break;
				}

				achievement->dependent_ids.push_back(dependent_achievement_id);
			} else
				util::vector_erase_if_exists(achievement->dependent_ids, dependent_achievement_id);
		}
	}

	if( this->nodeExists( node, "Rewards" ) ){
		const auto& rewardNode = node["Rewards"];

		if( this->nodeExists( rewardNode, "Item" ) ){
			std::string item_name;

			if( !this->asString( rewardNode, "Item", item_name ) ){
				return 0;
			}

			std::shared_ptr<item_data> item = item_db.search_aegisname(item_name.c_str());

			if (item == nullptr) {
				this->invalidWarning(rewardNode["Item"], "Reward Item %s does not exist, skipping.\n", item_name.c_str());
				return 0;
			}

			achievement->rewards.nameid = item->nameid;
		}

		if( this->nodeExists( rewardNode, "Amount" ) ){
			uint16 amount;

			if( !this->asUInt16( rewardNode, "Amount", amount ) ){
				return 0;
			}

			achievement->rewards.amount = amount;
		} else {
			if (!exists)
				achievement->rewards.amount = 1;
		}

		if( this->nodeExists( rewardNode, "Script" ) ){
			std::string script;

			if( !this->asString( rewardNode, "Script", script ) ){
				return 0;
			}

			if( achievement->rewards.script ){
				script_free_code( achievement->rewards.script );
				achievement->rewards.script = nullptr;
			}

			achievement->rewards.script = parse_script( script.c_str(), this->getCurrentFile().c_str(), this->getLineNumber(rewardNode["Script"]), SCRIPT_IGNORE_EXTERNAL_BRACKETS );
		}else{
			if (!exists)
				achievement->rewards.script = nullptr;
		}

		if( this->nodeExists( rewardNode, "TitleId" ) ){
			uint32 title;

			if( !this->asUInt32( rewardNode, "TitleId", title ) ){
				return 0;
			}

			if (title < TITLE_BASE || title > TITLE_MAX) {
				this->invalidWarning(rewardNode["TitleId"], "Reward Title ID %u does not exist (%hu~%hu), skipping.\n", title, TITLE_BASE, TITLE_MAX);
				return 0;
			}

			achievement->rewards.title_id = title;
		} else {
			if (!exists)
				achievement->rewards.title_id = 0;
		}
	}

	if( this->nodeExists( node, "Score" ) ){
		uint32 score;

		if( !this->asUInt32( node, "Score", score ) ){
			return 0;
		}

		achievement->score = score;
	} else {
		if (!exists)
			achievement->score = 0;
	}

	if( !exists ){
		this->put( achievement_id, achievement );
	}

	return 1;
}

void AchievementDatabase::loadingFinished(){
	for (const auto &achit : *this) {
		const std::shared_ptr<s_achievement_db> ach = achit.second;

		for (auto dep = ach->dependent_ids.begin(); dep != ach->dependent_ids.end(); dep++) {
			if (!this->exists(*dep)) {
				ShowWarning("achievement_read_db: An invalid Dependent ID %d was given for Achievement %d. Removing from list.\n", *dep, ach->achievement_id);
				dep = ach->dependent_ids.erase(dep);

				if (dep == ach->dependent_ids.end()) {
					break;
				}
			}
		}

		ach->dependent_ids.shrink_to_fit();
	}
}

AchievementDatabase achievement_db;

/**
 * Searches for an achievement by monster ID
 * @param mob_id: Monster ID to lookup
 * @return True on success, false on failure
 */
bool AchievementDatabase::mobexists( uint32 mob_id ){
	if (!battle_config.feature_achievement)
		return false;

	auto it = std::find(this->achievement_mobs.begin(), this->achievement_mobs.end(), mob_id);

	return (it != this->achievement_mobs.end()) ? true : false;
}

const std::string AchievementLevelDatabase::getDefaultLocation(){
	return std::string(db_path) + "/achievement_level_db.yml";
}

uint64 AchievementLevelDatabase::parseBodyNode( const ryml::NodeRef& node ){
	if( !this->nodesExist( node, { "Level", "Points" } ) ){
		return 0;
	}

	uint16 level;

	if( !this->asUInt16( node, "Level", level ) ){
		return 0;
	}

	if( level == 0 ){
		this->invalidWarning( node, "Invalid achievement level %hu (minimum value: 1), skipping.\n", level );
		return 0;
	}

	// Make it zero based
	level -= 1;

	std::shared_ptr<s_achievement_level> ptr = this->find( level );
	bool exists = ptr != nullptr;

	if( !exists ){
		ptr = std::make_shared<s_achievement_level>();
		ptr->level = level;
	}

	uint16 points;

	if (!this->asUInt16(node, "Points", points)) {
		return 0;
	}

	ptr->points = points;

	if( !exists ){
		this->put( level, ptr );
	}

	return 1;
}

AchievementLevelDatabase achievement_level_db;

/**
 * Add an achievement to the player's log
 * @param sd: Player data
 * @param achievement_id: Achievement to add
 * @return NULL on failure, achievement data on success
 */
struct achievement *achievement_add(struct map_session_data *sd, int achievement_id)
{
	int i, index;

	nullpo_retr(NULL, sd);

	std::shared_ptr<s_achievement_db> adb = achievement_db.find( achievement_id );

	if( adb == nullptr ){
		ShowError( "achievement_add: Achievement %d not found in DB.\n", achievement_id );
		return nullptr;
	}

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	if (i < sd->achievement_data.count) {
		ShowError("achievement_add: Character %d already has achievement %d.\n", sd->status.char_id, achievement_id);
		return NULL;
	}

	index = sd->achievement_data.incompleteCount;

	sd->achievement_data.count++;
	sd->achievement_data.incompleteCount++;
	RECREATE(sd->achievement_data.achievements, struct achievement, sd->achievement_data.count);

	// The character has some completed achievements, make room before them so that they will stay at the end of the array
	if (sd->achievement_data.incompleteCount != sd->achievement_data.count)
		memmove(&sd->achievement_data.achievements[index + 1], &sd->achievement_data.achievements[index], sizeof(struct achievement) * (sd->achievement_data.count - sd->achievement_data.incompleteCount));

	memset(&sd->achievement_data.achievements[index], 0, sizeof(struct achievement));

	sd->achievement_data.achievements[index].achievement_id = achievement_id;
	sd->achievement_data.achievements[index].score = adb->score;
	sd->achievement_data.save = true;

	clif_achievement_update(sd, &sd->achievement_data.achievements[index], sd->achievement_data.count - sd->achievement_data.incompleteCount);

	return &sd->achievement_data.achievements[index];
}

/**
 * Removes an achievement from a player's log
 * @param sd: Player's data
 * @param achievement_id: Achievement to remove
 * @return True on success, false on failure
 */
bool achievement_remove(struct map_session_data *sd, int achievement_id)
{
	struct achievement dummy;
	int i;

	nullpo_retr(false, sd);

	if (!achievement_db.exists(achievement_id)) {
		ShowError("achievement_delete: Achievement %d not found in DB.\n", achievement_id);
		return false;
	}

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	if (i == sd->achievement_data.count) {
		ShowError("achievement_delete: Character %d doesn't have achievement %d.\n", sd->status.char_id, achievement_id);
		return false;
	}

	if (!sd->achievement_data.achievements[i].completed)
		sd->achievement_data.incompleteCount--;

	if (i != sd->achievement_data.count - 1)
		memmove(&sd->achievement_data.achievements[i], &sd->achievement_data.achievements[i + 1], sizeof(struct achievement) * (sd->achievement_data.count - 1 - i));

	sd->achievement_data.count--;
	if( sd->achievement_data.count == 0 ){
		aFree(sd->achievement_data.achievements);
		sd->achievement_data.achievements = NULL;
	}else{
		RECREATE(sd->achievement_data.achievements, struct achievement, sd->achievement_data.count);
	}
	sd->achievement_data.save = true;

	// Send a removed fake achievement
	memset(&dummy, 0, sizeof(struct achievement));
	dummy.achievement_id = achievement_id;
	clif_achievement_update(sd, &dummy, sd->achievement_data.count - sd->achievement_data.incompleteCount);

	return true;
}

/**
 * Lambda function that checks for completed achievements
 * @param sd: Player data
 * @param achievement_id: Achievement to check if it's complete
 * @return True on completed, false if not
 */
static bool achievement_done(struct map_session_data *sd, int achievement_id) {
	for (int i = 0; i < sd->achievement_data.count; i++) {
		if (sd->achievement_data.achievements[i].achievement_id == achievement_id && sd->achievement_data.achievements[i].completed > 0)
			return true;
	}

	return false;
}

/**
 * Checks to see if an achievement has a dependent, and if so, checks if that dependent is complete
 * @param sd: Player data
 * @param achievement_id: Achievement to check if it has a dependent
 * @return False on failure or not complete, true on complete or no dependents
 */
bool achievement_check_dependent(struct map_session_data *sd, int achievement_id)
{
	nullpo_retr(false, sd);

	std::shared_ptr<s_achievement_db> adb = achievement_db.find( achievement_id );

	if( adb == nullptr ){
		return false;
	}

	// Check if the achievement has a dependent
	// If so, then do a check on all dependents to see if they're complete
	for (const auto &depit : adb->dependent_ids) {
		if (!achievement_done(sd, depit))
			return false; // One of the dependent is not complete!
	}

	return true;
}

/**
 * Check achievements that only have dependents and no other requirements
 * @param sd: Player to update
 * @param sd: Achievement to compare for completed dependents
 * @return True if successful, false if not
 */
static int achievement_check_groups(struct map_session_data *sd, struct s_achievement_db *ad)
{
	int i;

	if (ad == NULL || sd == NULL)
		return 0;

	if (ad->group != AG_BATTLE && ad->group != AG_TAMING && ad->group != AG_ADVENTURE)
		return 0;

	if (ad->dependent_ids.empty() || ad->condition)
		return 0;

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == ad->achievement_id);
	if (i == sd->achievement_data.count) { // Achievement isn't in player's log
		if (achievement_check_dependent(sd, ad->achievement_id) == true) {
			achievement_add(sd, ad->achievement_id);
			achievement_update_achievement(sd, ad->achievement_id, true);
		}
	}

	return 1;
}

/**
 * Update an achievement
 * @param sd: Player to update
 * @param achievement_id: Achievement ID of the achievement to update
 * @param complete: Complete state of an achievement
 * @return True if successful, false if not
 */
bool achievement_update_achievement(struct map_session_data *sd, int achievement_id, bool complete)
{
	int i;

	nullpo_retr(false, sd);

	std::shared_ptr<s_achievement_db> adb = achievement_db.find( achievement_id );

	if( adb == nullptr ){
		return false;
	}

	ARR_FIND(0, sd->achievement_data.incompleteCount, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	if (i == sd->achievement_data.incompleteCount)
		return false;

	if (sd->achievement_data.achievements[i].completed > 0)
		return false;

	// Finally we send the updated achievement to the client
	if (complete) {
		if (!adb->targets.empty()) { // Make sure all the objective targets are at their respective total requirement
			for (const auto &it : adb->targets)
				sd->achievement_data.achievements[i].count[it.first] = it.second->count;
		}

		sd->achievement_data.achievements[i].completed = time(NULL);

		if (i < (--sd->achievement_data.incompleteCount)) { // The achievement needs to be moved to the completed achievements block at the end of the array
			struct achievement tmp_ach;

			memcpy(&tmp_ach, &sd->achievement_data.achievements[i], sizeof(struct achievement));
			memcpy(&sd->achievement_data.achievements[i], &sd->achievement_data.achievements[sd->achievement_data.incompleteCount], sizeof(struct achievement));
			memcpy(&sd->achievement_data.achievements[sd->achievement_data.incompleteCount], &tmp_ach, sizeof(struct achievement));
		}

		achievement_level(sd, true); // Re-calculate achievement level
		// Check dependents
		for (auto &ach : achievement_db)
			achievement_check_groups(sd, ach.second.get());
		ARR_FIND(sd->achievement_data.incompleteCount, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id); // Look for the index again, the position most likely changed
	}

	clif_achievement_update(sd, &sd->achievement_data.achievements[i], sd->achievement_data.count - sd->achievement_data.incompleteCount);
	sd->achievement_data.save = true; // Flag to save with the autosave interval

	return true;
}

/**
 * Get the reward of an achievement
 * @param sd: Player getting the reward
 * @param achievement_id: Achievement to get reward data
 */
void achievement_get_reward(struct map_session_data *sd, int achievement_id, time_t rewarded)
{
	int i;

	nullpo_retv(sd);

	std::shared_ptr<s_achievement_db> adb = achievement_db.find( achievement_id );

	if( adb == nullptr ){
		ShowError( "achievement_reward: Inter server sent a reward claim for achievement %d not found in DB.\n", achievement_id );
		return;
	}

	if (rewarded == 0) {
		clif_achievement_reward_ack(sd->fd, 0, achievement_id);
		return;
	}

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	if (i == sd->achievement_data.count)
		return;

	// Only update in the cache, db was updated already
	sd->achievement_data.achievements[i].rewarded = rewarded;
	sd->achievement_data.save = true;

	run_script(adb->rewards.script, 0, sd->bl.id, fake_nd->bl.id);
	if (adb->rewards.title_id) {
		sd->titles.push_back(adb->rewards.title_id);
		clif_achievement_list_all(sd);
	}else{
		clif_achievement_reward_ack(sd->fd, 1, achievement_id);
		clif_achievement_update(sd, &sd->achievement_data.achievements[i], sd->achievement_data.count - sd->achievement_data.incompleteCount);
	}
}

/**
 * Check if player has recieved an achievement's reward
 * @param sd: Player to get reward
 * @param achievement_id: Achievement to get reward data
 */
void achievement_check_reward(struct map_session_data *sd, int achievement_id)
{
	int i;

	nullpo_retv(sd);

	std::shared_ptr<s_achievement_db> adb = achievement_db.find( achievement_id );

	if( adb == nullptr ){
		ShowError( "achievement_reward: Trying to reward achievement %d not found in DB.\n", achievement_id );
		clif_achievement_reward_ack( sd->fd, 0, achievement_id );
		return;
	}

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	if (i == sd->achievement_data.count) {
		clif_achievement_reward_ack(sd->fd, 0, achievement_id);
		return;
	}

	if (sd->achievement_data.achievements[i].rewarded > 0 || sd->achievement_data.achievements[i].completed == 0) {
		clif_achievement_reward_ack(sd->fd, 0, achievement_id);
		return;
	}

	if (!intif_achievement_reward(sd, adb.get())) {
		clif_achievement_reward_ack(sd->fd, 0, achievement_id);
	}
}

/**
 * Return all titles to a player based on completed achievements
 * @param char_id: Character ID requesting
 */
void achievement_get_titles(uint32 char_id)
{
	struct map_session_data *sd = map_charid2sd(char_id);

	if (sd) {
		sd->titles.clear();

		if (sd->achievement_data.count) {
			for (int i = 0; i < sd->achievement_data.count; i++) {
				std::shared_ptr<s_achievement_db> adb = achievement_db.find( sd->achievement_data.achievements[i].achievement_id );

				// If the achievement has a title and is complete, give it to the player
				if( adb != nullptr && adb->rewards.title_id && sd->achievement_data.achievements[i].completed > 0 ){
					sd->titles.push_back( adb->rewards.title_id );
				}
			}
		}
	}
}

/**
 * Frees the player's data for achievements
 * @param sd: Player's session
 */
void achievement_free(struct map_session_data *sd)
{
	nullpo_retv(sd);

	if (sd->achievement_data.count) {
		aFree(sd->achievement_data.achievements);
		sd->achievement_data.achievements = NULL;
		sd->achievement_data.count = sd->achievement_data.incompleteCount = 0;
	}
}

/**
 * Get an achievement's progress information
 * @param sd: Player to check achievement progress
 * @param achievement_id: Achievement progress to check
 * @param type: Type to return
 * @return The type's data, -1 if player doesn't have achievement, -2 on failure/incorrect type
 */
int achievement_check_progress(struct map_session_data *sd, int achievement_id, int type)
{
	int i;

	nullpo_retr(-2, sd);

	// Achievement ID is not needed so skip the lookup
	if (type == ACHIEVEINFO_LEVEL)
		return sd->achievement_data.level;
	else if (type == ACHIEVEINFO_SCORE)
		return sd->achievement_data.total_score;

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	if (i == sd->achievement_data.count)
		return -1;

	if (type >= ACHIEVEINFO_COUNT1 && type <= ACHIEVEINFO_COUNT10)
		return sd->achievement_data.achievements[i].count[type - 1];
	else if (type == ACHIEVEINFO_COMPLETE)
		return sd->achievement_data.achievements[i].completed > 0;
	else if (type == ACHIEVEINFO_COMPLETEDATE)
		return (int)sd->achievement_data.achievements[i].completed;
	else if (type == ACHIEVEINFO_GOTREWARD)
		return sd->achievement_data.achievements[i].rewarded > 0;
	return -2;
}

/**
 * Calculate a player's achievement level
 * @param sd: Player to check achievement level
 * @param flag: If the call should attempt to give the AG_GOAL_ACHIEVE achievement
 * @return Rollover and TNL EXP or 0 on failure
 */
int *achievement_level(struct map_session_data *sd, bool flag)
{
	nullpo_retr(nullptr, sd);

	sd->achievement_data.total_score = 0;

	for (int i = 0; i < sd->achievement_data.count; i++) { // Recount total score
		if (sd->achievement_data.achievements[i].completed > 0)
			sd->achievement_data.total_score += sd->achievement_data.achievements[i].score;
	}

	int left_score, right_score, old_level = sd->achievement_data.level;

	for( sd->achievement_data.level = 0; /* Break condition's inside the loop */; sd->achievement_data.level++ ){
		std::shared_ptr<s_achievement_level> level = achievement_level_db.find( sd->achievement_data.level );

		if( level != nullptr && sd->achievement_data.total_score > level->points ){
			std::shared_ptr<s_achievement_level> next_level = achievement_level_db.find( sd->achievement_data.level + 1 );

			// Check if there is another level
			if( next_level == nullptr ){
				std::shared_ptr<s_achievement_level> level = achievement_level_db.find( sd->achievement_data.level );

				left_score = sd->achievement_data.total_score - level->points;
				right_score = 0;

				// Increase the level for client side display
				sd->achievement_data.level++;
				break;
			}else{
				// Enough points for this level, check the next one
				continue;
			}
		}

		if( sd->achievement_data.level == 0 ){
			left_score = sd->achievement_data.total_score;
			if( level == nullptr ){
				right_score = 0;
			}else{
				right_score = level->points;
			}
			break;
		}else{
			std::shared_ptr<s_achievement_level> previous_level = achievement_level_db.find( sd->achievement_data.level - 1 );

			left_score = sd->achievement_data.total_score - previous_level->points;
			right_score = level->points - previous_level->points;
			break;
		}
	}

	static int info[2];

	info[0] = left_score; // Left number
	info[1] = right_score; // Right number

	if (flag && old_level != sd->achievement_data.level) { // Give AG_GOAL_ACHIEVE
		achievement_update_objective( sd, AG_GOAL_ACHIEVE, 0 );
	}

	return info;
}

bool achievement_check_condition( struct script_code* condition, struct map_session_data* sd ){
	// Save the old script the player was attached to
	struct script_state* previous_st = sd->st;

	// Only if there was an old script
	if( previous_st != nullptr ){
		// Detach the player from the current script
		script_detach_rid(previous_st);
	}

	run_script( condition, 0, sd->bl.id, fake_nd->bl.id );

	struct script_state* st = sd->st;

	int value = 0;

	if( st != nullptr ){
		value = script_getnum( st, 2 );

		script_free_state(st);
	}

	// If an old script is present
	if( previous_st != nullptr ){
		// Because of detach the RID will be removed, so we need to restore it
		previous_st->rid = sd->bl.id;

		// Reattach the player to it, so that the limitations of that script kick back in
		script_attach_state( previous_st );
	}

	return value != 0;
}

/**
 * Check to see if an achievement's target count is complete
 * @param ad: Achievement data
 * @param current_count: Current target data
 * @return True if all target values meet the requirements or false otherwise
 */
static bool achievement_target_complete(std::shared_ptr<s_achievement_db> ad, std::array<int, MAX_ACHIEVEMENT_OBJECTIVES> current_count) {
	return std::find_if(ad->targets.begin(), ad->targets.end(),
		[current_count](const std::pair<uint16, std::shared_ptr<achievement_target>> &target) -> bool {
		return current_count[target.first] < target.second->count;
	}) == ad->targets.end();
}

/**
 * Update achievement objectives.
 * @param sd: Player to update
 * @param ad: Achievement data to compare for completion
 * @param group: Achievement group to update
 * @param update_count: Objective values from event
 * @return 1 on success and false on failure
 */
static bool achievement_update_objectives(struct map_session_data *sd, std::shared_ptr<struct s_achievement_db> ad, enum e_achievement_group group, const std::array<int, MAX_ACHIEVEMENT_OBJECTIVES> &update_count)
{
	if (!ad || !sd)
		return false;

	if (group <= AG_NONE || group >= AG_MAX)
		return false;

	if (group != ad->group)
		return false;

	struct achievement *entry = NULL;
	bool isNew = false, changed = false, complete = false;
	std::array<int, MAX_ACHIEVEMENT_OBJECTIVES> current_count = {}; // Player's current objective values
	int i;

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == ad->achievement_id);
	if (i == sd->achievement_data.count) { // Achievement isn't in player's log
		if (!achievement_check_dependent(sd, ad->achievement_id)) // Check to see if dependents are complete before adding to player's log
			return false;

		isNew = true;
	} else {
		entry = &sd->achievement_data.achievements[i];

		if (entry->completed > 0) // Player has completed the achievement
			return false;

		memcpy(current_count.data(), entry->count, sizeof(current_count));
	}

	switch (group) {
		case AG_ADD_FRIEND:
		case AG_BABY:
		case AG_CHATTING_COUNT:
		case AG_CHATTING_CREATE:
		case AG_CHATTING_DYING:
		case AG_GET_ITEM:
		case AG_GET_ZENY:
		case AG_GOAL_LEVEL:
		case AG_GOAL_STATUS:
		case AG_JOB_CHANGE:
		case AG_MARRY:
		case AG_PARTY:
		case AG_ENCHANT_FAIL:
		case AG_ENCHANT_SUCCESS:
			if (!ad->condition)
				return false;

			if (!achievement_check_condition(ad->condition, sd)) // Parameters weren't met
				return false;

			changed = true;
			complete = true;
			break;
		case AG_SPEND_ZENY:
			if (ad->targets.empty() || !ad->condition)
				return false;

			for (const auto &it : ad->targets) {
				if (current_count[it.first] < it.second->count)
					current_count[it.first] += update_count[it.first];
			}

			if (!achievement_check_condition(ad->condition, sd)) // Parameters weren't met
				return false;

			changed = true;

			if (achievement_target_complete(ad, current_count))
				complete = true;
			break;
		case AG_BATTLE:
		case AG_TAMING:
			if (ad->targets.empty())
				return false;

			for (const auto &it : ad->targets) {
				if (it.second->mob == update_count[0] && current_count[it.first] < it.second->count) { // Here update_count[0] contains the killed/tamed monster ID
					current_count[it.first]++;
					changed = true;
				}
			}

			if (!changed)
				return false;

			if (achievement_target_complete(ad, current_count))
				complete = true;
			break;
		case AG_GOAL_ACHIEVE:
			if (!achievement_check_condition(ad->condition, sd)) // Parameters weren't met
				return false;

			changed = true;
			complete = true;
			break;
		/*
		case AG_CHATTING:
			if (ad->targets.empty())
				return false;

			if (ad->mapindex > -1 && sd->bl.m != ad->mapindex)
				return false;

			for (const auto &it : ad->targets) {
				if (current_count[it.first] < it.second->count) {
					current_count[it.first]++;
					changed = true;
				}
			}

			if (!changed)
				return false;

			if (achievement_target_complete(ad, current_count))
				complete = true;
			break;
		*/
	}

	if( isNew ){
		// Always add the achievement if it was completed
		bool hasCounter = complete;

		// If it was not completed
		if( !hasCounter ){
			// Check if it has a counter
			for( int counter : current_count ){
				if( counter != 0 ){
					hasCounter = true;
					break;
				}
			}
		}

		if( hasCounter ){
			if( !( entry = achievement_add( sd, ad->achievement_id ) ) ){
				return false; // Failed to add achievement
			}
		}else{
			changed = false;
		}
	}

	if (changed) {
		memcpy(entry->count, current_count.data(), sizeof(current_count));
		achievement_update_achievement(sd, ad->achievement_id, complete);
	}

	return true;
}

/**
 * Update achievement objective count.
 * @param sd: Player data
 * @param group: Achievement enum type
 * @param sp_value: SP parameter value
 * @param arg_count: va_arg count
 */
void achievement_update_objective(struct map_session_data *sd, enum e_achievement_group group, uint8 arg_count, ...)
{
	if (!battle_config.feature_achievement)
		return;

	if (sd) {
		va_list ap;
		std::array<int, MAX_ACHIEVEMENT_OBJECTIVES> count = {};

		va_start(ap, arg_count);
		for (int i = 0; i < arg_count; i++){
			std::string name = "ARG" + std::to_string(i);

			count[i] = va_arg(ap, int);

			pc_setglobalreg( sd, add_str( name.c_str() ), (int)count[i] );
		}
		va_end(ap);

		for (auto &ach : achievement_db)
			achievement_update_objectives(sd, ach.second, group, count);

		// Remove variables that might have been set
		for (int i = 0; i < arg_count; i++){
			std::string name = "ARG" + std::to_string(i);

			pc_setglobalreg( sd, add_str( name.c_str() ), 0 );
		}
	}
}

/**
 * Map iterator subroutine to update achievement objectives for a party after killing a monster.
 * @see map_foreachinrange
 * @param ap: Argument list, expecting:
 *   int Party ID
 *   int Mob ID
 */
int achievement_update_objective_sub(block_list *bl, va_list ap)
{
	map_session_data *sd;
	int mob_id, party_id;

	nullpo_ret(bl);
	nullpo_ret(sd = (map_session_data *)bl);

	party_id = va_arg(ap, int);
	mob_id = va_arg(ap, int);

	if (sd->achievement_data.achievements == nullptr)
		return 0;
	if (sd->status.party_id != party_id)
		return 0;

	achievement_update_objective(sd, AG_BATTLE, 1, mob_id);

	return 1;
}

/**
 * Reloads the achievement database
 */
void achievement_db_reload(void)
{
	do_final_achievement();
	do_init_achievement();
}

/**
 * Initializes the achievement database
 */
void do_init_achievement(void)
{
	if (!battle_config.feature_achievement)
		return;
	achievement_db.load();
	achievement_level_db.load();
}

/**
 * Finalizes the achievement database
 */
void do_final_achievement(void){
	achievement_db.clear();
	achievement_level_db.clear();
}

/**
 * Achievement constructor
 */
s_achievement_db::s_achievement_db()
	: achievement_id(0)
	, name("")
	, group()
	, targets()
	, dependent_ids()
	, condition(nullptr)
	, mapindex(-1)
	, rewards()
	, score(0)
	, has_dependent(0)
{}

/**
* Achievement deconstructor
*/
s_achievement_db::~s_achievement_db()
{
	if (condition)
		script_free_code(condition);
}

/**
 * Achievement reward constructor
 */
s_achievement_db::ach_reward::ach_reward()
	: nameid(0)
	, amount(0)
	, script(nullptr)
	, title_id(0)
{}

/**
 * Achievement reward deconstructor
 */
s_achievement_db::ach_reward::~ach_reward()
{
	if (script)
		script_free_code(script);
}
