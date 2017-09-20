// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "../common/yamlwrapper.h"

#include "achievement.h"
#include "chrif.h"
#include "clif.h"
#include "intif.h"
#include "itemdb.h"
#include "map.h"
#include "pc.h"
#include "script.h"
#include "status.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

static jmp_buf     av_error_jump;
static char*       av_error_msg;
static const char* av_error_pos;
static int         av_error_report;

static DBMap *achievement_db = NULL; // int achievement_id -> struct achievement_db *
static DBMap *achievementmobs_db = NULL; // Avoids checking achievements on every mob killed
static void achievement_db_free_sub(struct achievement_db *achievement, bool free);
struct achievement_db achievement_dummy;

/**
 * Searches an achievement by ID
 * @param achievement_id: ID to lookup
 * @return Achievement entry (equals to &achievement_dummy if the ID is invalid)
 */
struct achievement_db *achievement_search(int achievement_id)
{
	struct achievement_db *achievement = (struct achievement_db *)idb_get(achievement_db, achievement_id);

	if (!achievement)
		return &achievement_dummy;
	return achievement;
}

/**
 * Searches for an achievement by monster ID
 * @param mob_id: Monster ID to lookup
 * @return True on success, false on failure
 */
bool achievement_mobexists(int mob_id)
{
	if (!battle_config.feature_achievement)
		return false;
	return idb_exists(achievementmobs_db, mob_id);
}

/**
 * Add an achievement to the player's log
 * @param sd: Player data
 * @param achievement_id: Achievement to add
 * @return NULL on failure, achievement data on success
 */
struct achievement *achievement_add(struct map_session_data *sd, int achievement_id)
{
	struct achievement_db *adb = &achievement_dummy;
	int i, index;

	nullpo_retr(NULL, sd);

	if ((adb = achievement_search(achievement_id)) == &achievement_dummy) {
		ShowError("achievement_add: Achievement %d not found in DB.\n", achievement_id);
		return NULL;
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

	if (achievement_search(achievement_id) == &achievement_dummy) {
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
 * Checks to see if an achievement has a dependent, and if so, checks if that dependent is complete
 * @param sd: Player data
 * @param achievement_id: Achievement to check if it has a dependent
 * @return False on failure or not complete, true on complete or no dependents
 */
bool achievement_check_dependent(struct map_session_data *sd, int achievement_id)
{
	struct achievement_db *adb = &achievement_dummy;

	nullpo_retr(false, sd);

	adb = achievement_search(achievement_id);

	if (adb == &achievement_dummy)
		return false;

	// Check if the achievement has a dependent
	// If so, then do a check on all dependents to see if they're complete
	if (adb->dependent_count) {
		int i;

		for (i = 0; i < adb->dependent_count; i++) {
			struct achievement_db *adb_dep = achievement_search(adb->dependents[i].achievement_id);
			int j;

			if (adb_dep == &achievement_dummy)
				return false;

			ARR_FIND(0, sd->achievement_data.count, j, sd->achievement_data.achievements[j].achievement_id == adb->dependents[i].achievement_id && sd->achievement_data.achievements[j].completed > 0);
			if (j == sd->achievement_data.count)
				return false; // One of the dependent is not complete!
		}
	}

	return true;
}

/**
 * Check achievements that only have dependents and no other requirements
 * @return True if successful, false if not
 */
static int achievement_check_groups(DBKey key, DBData *data, va_list ap)
{
	struct achievement_db *ad;
	struct map_session_data *sd;
	int i;

	ad = (struct achievement_db *)db_data2ptr(data);
	sd = va_arg(ap, struct map_session_data *);

	if (ad == &achievement_dummy || sd == NULL)
		return 0;

	if (ad->group != AG_BATTLE && ad->group != AG_TAMING && ad->group != AG_ADVENTURE)
		return 0;

	if (ad->dependent_count == 0 || ad->condition)
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
	struct achievement_db *adb = &achievement_dummy;
	int i;

	nullpo_retr(false, sd);

	adb = achievement_search(achievement_id);

	if (adb == &achievement_dummy)
		return false;

	ARR_FIND(0, sd->achievement_data.incompleteCount, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	if (i == sd->achievement_data.incompleteCount)
		return false;

	if (sd->achievement_data.achievements[i].completed > 0)
		return false;

	// Finally we send the updated achievement to the client
	if (complete) {
		if (adb->target_count) { // Make sure all the objective targets are at their respective total requirement
			int k;

			for (k = 0; k < adb->target_count; k++)
				sd->achievement_data.achievements[i].count[k] = adb->targets[k].count;

			for (k = 1; k < adb->dependent_count; k++) {
				sd->achievement_data.achievements[i].count[k] = max(1, sd->achievement_data.achievements[i].count[k]);
			}
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
		achievement_db->foreach(achievement_db, achievement_check_groups, sd);
		ARR_FIND(sd->achievement_data.incompleteCount, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id); // Look for the index again, the position most likely changed
	}

	clif_achievement_update(sd, &sd->achievement_data.achievements[i], sd->achievement_data.count - sd->achievement_data.incompleteCount);
	sd->achievement_data.save = true; // Flag to save with the autosave interval

	if (sd->achievement_data.sendlist) {
		clif_achievement_list_all(sd);
		sd->achievement_data.sendlist = false;
	}

	return true;
}

/**
 * Get the reward of an achievement
 * @param sd: Player getting the reward
 * @param achievement_id: Achievement to get reward data
 */
void achievement_get_reward(struct map_session_data *sd, int achievement_id, time_t rewarded)
{
	struct achievement_db *adb = achievement_search(achievement_id);
	int i;

	nullpo_retv(sd);

	if( rewarded == 0 ){
		clif_achievement_reward_ack(sd->fd, 0, achievement_id);
		return;
	}

	if (adb == &achievement_dummy) {
		ShowError("achievement_reward: Inter server sent a reward claim for achievement %d not found in DB.\n", achievement_id);
		return;
	}

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);

	if (i == sd->achievement_data.count) {
		return;
	}

	// Only update in the cache, db was updated already
	sd->achievement_data.achievements[i].rewarded = rewarded;

	run_script(adb->rewards.script, 0, sd->bl.id, fake_nd->bl.id);
	if (adb->rewards.title_id) {
		RECREATE(sd->titles, int, sd->titleCount + 1);
		sd->titles[sd->titleCount] = adb->rewards.title_id;
		sd->titleCount++;
		sd->achievement_data.sendlist = true;
	}

	clif_achievement_reward_ack(sd->fd, 1, achievement_id);
	clif_achievement_update(sd, &sd->achievement_data.achievements[i], sd->achievement_data.count - sd->achievement_data.incompleteCount);
}

/**
 * Check if player has recieved an achievement's reward
 * @param sd: Player to get reward
 * @param achievement_id: Achievement to get reward data
 */
void achievement_check_reward(struct map_session_data *sd, int achievement_id)
{
	int i;
	struct achievement_db *adb = achievement_search(achievement_id);

	nullpo_retv(sd);

	if (adb == &achievement_dummy) {
		ShowError("achievement_reward: Trying to reward achievement %d not found in DB.\n", achievement_id);
		clif_achievement_reward_ack(sd->fd, 0, achievement_id);
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

	if( !intif_achievement_reward(sd,adb) ){
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
		sd->titles = NULL;
		sd->titleCount = 0;

		if (sd->achievement_data.count) {
			int i;

			for (i = 0; i < sd->achievement_data.count; i++) {
				struct achievement_db *adb = achievement_search(sd->achievement_data.achievements[i].achievement_id);

				if (adb && adb->rewards.title_id && sd->achievement_data.achievements[i].completed > 0) { // If the achievement has a title and is complete, give it to the player
					RECREATE(sd->titles, int, sd->titleCount + 1);
					sd->titles[sd->titleCount] = adb->rewards.title_id;
					sd->titleCount++;
				}
			}
		}
	}
}

/**
 * Frees the player's data for achievements and titles
 * @param sd: Player's session
 */
void achievement_free(struct map_session_data *sd)
{
	nullpo_retv(sd);

	if (sd->titleCount) {
		aFree(sd->titles);
		sd->titles = NULL;
		sd->titleCount = 0;
	}

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
 */
int *achievement_level(struct map_session_data *sd, bool flag)
{
	static int info[2];
	int i, old_level;
	const int score_table[MAX_ACHIEVEMENT_RANK] = { 18, 31, 49, 73, 135, 104, 140, 178, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000 }; //! TODO: Figure out the EXP required to level up from 8-20

	nullpo_retr(0, sd);

	sd->achievement_data.total_score = 0;
	old_level = sd->achievement_data.level;

	for (i = 0; i < sd->achievement_data.count; i++) {
		if (sd->achievement_data.achievements[i].completed > 0)
			sd->achievement_data.total_score += sd->achievement_data.achievements[i].score;
	}

	info[0] = 0;
	info[1] = 0;

	for (i = 0; i < MAX_ACHIEVEMENT_RANK; i++) {
		info[0] = info[1];
			
		if (i < ARRAYLENGTH(score_table))
			info[1] = score_table[i];
		else {
			info[0] = info[1];
			info[1] = info[1] + 500;
		}

		if (sd->achievement_data.total_score < info[1])
			break;
	}

	if (i == MAX_ACHIEVEMENT_RANK)
		i = 0;

	info[1] = info[1] - info[0]; // Right number
	info[0] = sd->achievement_data.total_score - info[0]; // Left number
	sd->achievement_data.level = i;

	if (flag == true && old_level != sd->achievement_data.level) {
		int achievement_id = 240000 + sd->achievement_data.level;

		if( achievement_add(sd, achievement_id) ){
			achievement_update_achievement(sd, achievement_id, true);
		}
	}

	return info;
}

/**
 * Update achievement objectives.
 * @see DBApply
 */
static int achievement_update_objectives(DBKey key, DBData *data, va_list ap)
{
	struct achievement_db *ad;
	struct map_session_data *sd;
	enum e_achievement_group group;
	struct achievement *entry = NULL;
	bool isNew = false, changed = false, complete = false;
	int i, k = 0, objective_count[MAX_ACHIEVEMENT_OBJECTIVES], update_count[MAX_ACHIEVEMENT_OBJECTIVES];

	ad = (struct achievement_db *)db_data2ptr(data);
	sd = va_arg(ap, struct map_session_data *);
	group = (enum e_achievement_group)va_arg(ap, int);
	memcpy(update_count, (int *)va_arg(ap, int *), sizeof(update_count));

	if (ad == NULL || sd == NULL)
		return 0;

	if (group <= AG_NONE || group >= AG_MAX)
		return 0;

	if (group != ad->group)
		return 0;

	memset(objective_count, 0, sizeof(objective_count)); // Current objectives count

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == ad->achievement_id);
	if (i == sd->achievement_data.count) { // Achievement isn't in player's log
		if (achievement_check_dependent(sd, ad->achievement_id) == false) // Check to see if dependents are complete before adding to player's log
			return 0;
		isNew = true;
	} else {
		entry = &sd->achievement_data.achievements[i];

		if (entry->completed > 0) // Player has completed the achievement
			return 0;

		memcpy(objective_count, entry->count, sizeof(objective_count));
	}

	switch (group) {
		case AG_ADD_FRIEND:
		case AG_BABY:
		case AG_CHAT_COUNT:
		case AG_CHAT_CREATE:
		case AG_CHAT_DYING:
		case AG_GET_ITEM:
		case AG_GET_ZENY:
		case AG_GOAL_LEVEL:
		case AG_GOAL_STATUS:
		case AG_JOB_CHANGE:
		case AG_MARRY:
		case AG_PARTY:
		case AG_REFINE_FAIL:
		case AG_REFINE_SUCCESS:
		case AG_SPEND_ZENY:
			if (group == AG_SPEND_ZENY) { // Achievement type is cummulative
				objective_count[0] += update_count[0];
				changed = true;
			}

			if (!ad->condition || achievement_check_condition(ad->condition, sd, update_count)) {
				changed = true;
				complete = true;
			}

			if (changed == false)
				break;

			if (isNew) {
				if ((entry = achievement_add(sd, ad->achievement_id)) == NULL)
					return 0; // Failed to add achievement, fall out
			}
			break;
		case AG_CHAT:
			if (!ad->target_count)
				break;

			if (ad->condition && !achievement_check_condition(ad->condition, sd, update_count)) // Parameters weren't met
				break;

			if (ad->mapindex > -1 && sd->bl.m != ad->mapindex)
				break;

			for (i = 0; i < ad->target_count; i++) {
				if (objective_count[i] < ad->targets[i].count)
					objective_count[i] += update_count[0];
			}

			changed = true;

			ARR_FIND(0, ad->target_count, k, objective_count[k] < ad->targets[k].count);
			if (k == ad->target_count)
				complete = true;

			if (isNew) {
				if ((entry = achievement_add(sd, ad->achievement_id)) == NULL)
					return 0; // Failed to add achievement, fall out
			}
			break;
		case AG_BATTLE:
		case AG_TAMING:
			ARR_FIND(0, ad->target_count, k, ad->targets[k].mob == update_count[0]);
			if (k == ad->target_count)
				break; // Mob wasn't found

			for (k = 0; k < ad->target_count; k++) {
				if (ad->targets[k].mob == update_count[0] && objective_count[k] < ad->targets[k].count) {
					objective_count[k]++;
					changed = true;
				}
			}

			ARR_FIND(0, ad->target_count, k, objective_count[k] < ad->targets[k].count);
			if (k == ad->target_count)
				complete = true;

			if (isNew) {
				if ((entry = achievement_add(sd, ad->achievement_id)) == NULL)
					return 0; // Failed to add achievement, fall out
			}
			break;
	}

	if (changed) {
		memcpy(entry->count, objective_count, sizeof(objective_count));
		achievement_update_achievement(sd, ad->achievement_id, complete);
	}

	return 1;
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
	if (sd) {
		va_list ap;
		int i, count[MAX_ACHIEVEMENT_OBJECTIVES];

		if (!battle_config.feature_achievement)
			return;

		memset(count, 0, sizeof(count)); // Clear out array before setting values

		va_start(ap, arg_count);
		for (i = 0; i < arg_count; i++)
			count[i] = va_arg(ap, int);
		va_end(ap);

		switch(group) {
			case AG_CHAT: //! TODO: Not sure how this works officially
			case AG_GOAL_ACHIEVE:
				// These have no objective use right now.
				break;
			default:
				achievement_db->foreach(achievement_db, achievement_update_objectives, sd, (int)group, count);
				break;
		}
	}
}

/*==========================================
 * Achievement condition parsing section
 *------------------------------------------*/
static void disp_error_message2(const char *mes,const char *pos,int report)
{
	av_error_msg = aStrdup(mes);
	av_error_pos = pos;
	av_error_report = report;
	longjmp(av_error_jump, 1);
}
#define disp_error_message(mes,pos) disp_error_message2(mes,pos,1)

/**
 * Checks the condition of an achievement.
 * @param condition: Achievement condition
 * @param sd: Player data
 * @param count: Script arguments
 * @return The result of the condition.
 */
long long achievement_check_condition(struct av_condition *condition, struct map_session_data *sd, int *count)
{
	long long left = 0;
	long long right = 0;

	// Reduce the recursion, almost all calls will be C_PARAM, C_NAME or C_ARG
	if (condition->left) {
		if (condition->left->op == C_NAME || condition->left->op == C_INT)
			left = condition->left->value;
		else if (condition->left->op == C_PARAM)
			left = pc_readparam(sd, (int)condition->left->value);
		else if (condition->left->op == C_ARG && condition->left->value < MAX_ACHIEVEMENT_OBJECTIVES)
			left = count[condition->left->value];
		else
			left = achievement_check_condition(condition->left, sd, count);
	}

	if (condition->right) {
		if (condition->right->op == C_NAME || condition->right->op == C_INT)
			right = condition->right->value;
		else if (condition->right->op == C_PARAM)
			right = pc_readparam(sd, (int)condition->right->value);
		else if (condition->right->op == C_ARG && condition->right->value < MAX_ACHIEVEMENT_OBJECTIVES)
			right = count[condition->right->value];
		else
			right = achievement_check_condition(condition->right, sd, count);
	}

	switch(condition->op) {
		case C_NOP:
			return false;
		case C_NAME:
		case C_INT:
			return condition->value;
		case C_PARAM:
			return pc_readparam(sd, (int)condition->value);
		case C_LOR: 
			return left || right;
		case C_LAND:
			return left && right;
		case C_LE:
			return left <= right;
		case C_LT:
			return left < right;
		case C_GE:
			return left >= right;
		case C_GT:
			return left > right;
		case C_EQ:
			return left == right;
		case C_NE:
			return left != right;
		case C_XOR:
			return left ^ right;
		case C_OR:
			return left || right;
		case C_AND:
			return left & right;
		case C_ADD:
			return left + right;
		case C_SUB:
			return left - right;
		case C_MUL:
			return left * right;
		case C_DIV:
			return left / right;
		case C_MOD:
			return left % right;
		case C_NEG:
			return -left;
		case C_LNOT:
			return !left;
		case C_NOT:
			return ~left;
		case C_R_SHIFT:
			return left >> right;
		case C_L_SHIFT:
			return left << right;
		case C_ARG:
			if (condition->value < MAX_ACHIEVEMENT_OBJECTIVES)
				return count[condition->value];

			return false;
		default:
			ShowError("achievement_check_condition: unexpected operator: %d\n", condition->op);
			return false;
	}

	return false;
}

static const char *skip_word(const char *p)
{
	while (ISALNUM(*p) || *p == '_')
		++p;

	if (*p == '$') // String
		p++;

	return p;
}

const char *av_parse_simpleexpr(const char *p, struct av_condition *parent)
{
	long long i;

	p = skip_space(p);

	if(*p == ';' || *p == ',')
		disp_error_message("av_parse_simpleexpr: unexpected character.", p);
	if(*p == '(') {
		p = av_parse_subexpr(p + 1, -1, parent);
		p = skip_space(p);

		if (*p != ')')
			disp_error_message("av_parse_simpleexpr: unmatched ')'", p);
		++p;
	} else if(is_number(p)) {
		char *np;

		while(*p == '0' && ISDIGIT(p[1]))
			p++;
		i = strtoll(p, &np, 0);

		if (i < INT_MIN) {
			i = INT_MIN;
			disp_error_message("av_parse_simpleexpr: underflow detected, capping value to INT_MIN.", p);
		} else if (i > INT_MAX) {
			i = INT_MAX;
			disp_error_message("av_parse_simpleexpr: underflow detected, capping value to INT_MAX.", p);
		}

		parent->op = C_INT;
		parent->value = i;
		p = np;
	} else {
		int v, len;
		char * word;

		if (skip_word(p) == p)
			disp_error_message("av_parse_simpleexpr: unexpected character.", p);

		len = skip_word(p) - p;

		if (len == 0)
			disp_error_message("av_parse_simpleexpr: invalid word. A word consists of undercores and/or alphanumeric characters.", p);

		word = (char*)aMalloc(len + 1);
		memcpy(word, p, len);
		word[len] = 0;

		if (script_get_parameter(word, &v))
			parent->op = C_PARAM;
		else if (script_get_constant(word, &v)) {
			if (word[0] == 'b' && ISUPPER(word[1])) // Consider b* variables as parameters (because they... are?)
				parent->op = C_PARAM;
			else
				parent->op = C_NAME;
		} else {
			if (word[0] == 'A' && word[1] == 'R' && word[2] == 'G' && ISDIGIT(word[3])) { // Special constants used to set temporary variables
				parent->op = C_ARG;
				v = atoi(word + 3);
			} else {
				aFree(word);
				disp_error_message("av_parse_simpleexpr: invalid constant.", p);
			}
		}

		aFree(word);
		parent->value = v;
		p = skip_word(p);
	}

	return p;
}

const char* av_parse_subexpr(const char* p, int limit, struct av_condition *parent)
{
	int op, opl, len;

	p = skip_space(p);

	CREATE(parent->left, struct av_condition, 1);

	if ((op = C_NEG, *p == '-') || (op = C_LNOT, *p == '!') || (op = C_NOT, *p == '~')) { // Unary - ! ~ operators
		p = av_parse_subexpr(p + 1, 11, parent->left);
		parent->op = op;
	} else
		p = av_parse_simpleexpr(p, parent->left);

	p = skip_space(p);

	while((
			((op=C_ADD,opl=9,len=1,*p=='+') && p[1]!='+') ||
			((op=C_SUB,opl=9,len=1,*p=='-') && p[1]!='-') ||
			(op=C_MUL,opl=10,len=1,*p=='*') ||
			(op=C_DIV,opl=10,len=1,*p=='/') ||
			(op=C_MOD,opl=10,len=1,*p=='%') ||
			(op=C_LAND,opl=2,len=2,*p=='&' && p[1]=='&') ||
			(op=C_AND,opl=5,len=1,*p=='&') ||
			(op=C_LOR,opl=1,len=2,*p=='|' && p[1]=='|') ||
			(op=C_OR,opl=3,len=1,*p=='|') ||
			(op=C_XOR,opl=4,len=1,*p=='^') ||
			(op=C_EQ,opl=6,len=2,*p=='=' && p[1]=='=') ||
			(op=C_NE,opl=6,len=2,*p=='!' && p[1]=='=') ||
			(op=C_R_SHIFT,opl=8,len=2,*p=='>' && p[1]=='>') ||
			(op=C_GE,opl=7,len=2,*p=='>' && p[1]=='=') ||
			(op=C_GT,opl=7,len=1,*p=='>') ||
			(op=C_L_SHIFT,opl=8,len=2,*p=='<' && p[1]=='<') ||
			(op=C_LE,opl=7,len=2,*p=='<' && p[1]=='=') ||
			(op=C_LT,opl=7,len=1,*p=='<')) && opl>limit) {
		p += len;

		if (parent->right) { // Chain conditions
			struct av_condition *condition = NULL;
			CREATE(condition, struct av_condition, 1);
			condition->op = parent->op;
			condition->left = parent->left;
			condition->right = parent->right;
			parent->left = condition;
			parent->right = NULL;
		}

		CREATE(parent->right, struct av_condition, 1);
		p = av_parse_subexpr(p, opl, parent->right);
		parent->op = op;
		p = skip_space(p);
	}

	if (parent->op == C_NOP && parent->right == NULL) { // Move the node up
		struct av_condition *temp = parent->left;

		parent->right = parent->left->right;
		parent->op = parent->left->op;
		parent->value = parent->left->value;
		parent->left = parent->left->left;

		aFree(temp);
	}

	return p;
}

/**
 * Parses a condition from a script.
 * @param p: The script buffer.
 * @param file: The file being parsed.
 * @param line: The current achievement line number.
 * @return The parsed achievement condition.
 */
struct av_condition *parse_condition(const char *p, const char *file, int line)
{
	struct av_condition *condition = NULL;

	if (setjmp(av_error_jump) != 0) {
		if (av_error_report)
			script_error(p,file,line,av_error_msg,av_error_pos);
		aFree(av_error_msg);
		if (condition)
			achievement_script_free(condition);
		return NULL;
	}

	switch(*p) {
		case ')': case ';': case ':': case '[': case ']': case '}':
			disp_error_message("parse_condition: unexpected character.", p);
	}

	condition = (struct av_condition *) aCalloc(1, sizeof(struct av_condition));
	av_parse_subexpr(p, -1, condition);

	return condition;
}

/**
 * Reads and parses an entry from the achievement_db.
 * @param wrapper: The YAML wrapper containing the entry.
 * @param n: The sequential index of the current entry.
 * @param source: The source YAML file.
 * @return The parsed achievement entry or NULL in case of error.
 */
struct achievement_db *achievement_read_db_sub(yamlwrapper *wrapper, int n, const char *source)
{
	struct achievement_db *entry = NULL;
	yamlwrapper *t = NULL;
	yamliterator *it;
	enum e_achievement_group group = AG_NONE;
	int score = 0, achievement_id = 0;
	char *group_char = NULL, *name = NULL, *condition = NULL, *mapname = NULL;

	if (!yaml_node_is_defined(wrapper, "ID")) {
		ShowWarning("achievement_read_db_sub: Missing ID in \"%s\", entry #%d, skipping.\n", source, n);
		return NULL;
	} else
		achievement_id = yaml_get_int(wrapper, "ID");
	if (achievement_id < 1 || achievement_id > INT_MAX) {
		ShowWarning("achievement_read_db_sub: Invalid achievement ID %d in \"%s\", entry #%d (min: 1, max: %d), skipping.\n", achievement_id, source, n, INT_MAX);
		return NULL;
	}

	if (!yaml_node_is_defined(wrapper, "Group")) {
		ShowWarning("achievement_read_db_sub: Missing group for achievement %d in \"%s\", skipping.\n", achievement_id, source);
		return NULL;
	} else
		group_char = yaml_get_c_string(wrapper, "Group");
	if (!script_get_constant(group_char, (int *)&group)) {
		ShowWarning("achievement_read_db_sub: Invalid group %s for achievement %d in \"%s\", skipping.\n", group_char, achievement_id, source);
		return NULL;
	}
	aFree(group_char);

	if (!yaml_node_is_defined(wrapper, "Name")) {
		ShowWarning("achievement_read_db_sub: Missing achievement name for achievement %d in \"%s\", skipping.\n", name, achievement_id, source);
		return NULL;
	} else
		name = yaml_get_c_string(wrapper, "Name");

	CREATE(entry, struct achievement_db, 1);
	entry->achievement_id = achievement_id;
	entry->group = group;
	safestrncpy(entry->name, name, sizeof(entry->name));
	aFree(name);
	entry->mapindex = -1;

	if (yaml_node_is_defined(wrapper, "Target") && (t = yaml_get_subnode(wrapper, "Target")) && (it = yaml_get_iterator(t)) && yaml_iterator_is_valid(it)) {
		yamlwrapper *tt = NULL;

		for (tt = yaml_iterator_first(it); yaml_iterator_has_next(it) && entry->target_count < MAX_ACHIEVEMENT_OBJECTIVES; tt = yaml_iterator_next(it)) {
			int mobid = 0, count = 0;

			if (yaml_node_is_defined(tt, "MobID") && (mobid = yaml_get_int(tt, "MobID")) && !mobdb_exists(mobid)) { // The mob ID field is not required
				ShowError("achievement_read_db_sub: Invalid mob ID %d for achievement %d in \"%s\", skipping.\n", mobid, achievement_id, source);
				continue;
			}
			if (yaml_node_is_defined(tt, "Count") && (!(count = yaml_get_int(tt, "Count")) || count <= 0)) {
				ShowError("achievement_read_db_sub: Invalid count %d for achievement %d in \"%s\", skipping.\n", count, achievement_id, source);
				continue;
			}
			if (mobid && group == AG_BATTLE && !idb_exists(achievementmobs_db, mobid)) {
				struct achievement_mob *entrymob = NULL;

				CREATE(entrymob, struct achievement_mob, 1);
				idb_put(achievementmobs_db, mobid, entrymob);
			}

			RECREATE(entry->targets, struct achievement_target, entry->target_count + 1);
			entry->targets[entry->target_count].mob = mobid;
			entry->targets[entry->target_count].count = count;
			entry->target_count++;
			yaml_destroy_wrapper(tt);
		}
		yaml_iterator_destroy(it);
	}

	if (yaml_node_is_defined(wrapper, "Condition") && (condition = yaml_get_c_string(wrapper, "Condition"))){
		entry->condition = parse_condition(condition, source, n);
		aFree(condition);
	}

	if (yaml_node_is_defined(wrapper, "Map") && (mapname = yaml_get_c_string(wrapper, "Map"))) {
		if (group != AG_CHAT)
			ShowWarning("achievement_read_db_sub: The map argument can only be used with the group AG_CHATTING (achievement %d in \"%s\"), skipping.\n", achievement_id, source);
		else {
			entry->mapindex = map_mapname2mapid(mapname);

			if (entry->mapindex == -1)
				ShowWarning("achievement_read_db_sub: Invalid map name %s for achievement %d in \"%s\".\n", mapname, achievement_id, source);
		}
		aFree(mapname);
	}

	if (yaml_node_is_defined(wrapper, "Dependent") && (t = yaml_get_subnode(wrapper, "Dependent")) && (it = yaml_get_iterator(t))) {
		if (yaml_iterator_is_valid(it)) {
			yamlwrapper *tt = NULL;

			for (tt = yaml_iterator_first(it); yaml_iterator_has_next(it) && entry->dependent_count < MAX_ACHIEVEMENT_DEPENDENTS; tt = yaml_iterator_next(it)) {
				RECREATE(entry->dependents, struct achievement_dependent, entry->dependent_count + 1);
				entry->dependents[entry->dependent_count].achievement_id = yaml_as_int(tt);
				entry->dependent_count++;
				yaml_destroy_wrapper(tt);
			}
			yaml_iterator_destroy(it);
		} else
			ShowWarning("achievement_read_db_sub: Invalid dependent format for achievement %d in \"%s\".\n", achievement_id, source);
	}

	if (yaml_node_is_defined(wrapper, "Reward") && (t = yaml_get_subnode(wrapper, "Reward"))) {
		char *script_char = NULL;
		int nameid = 0, amount = 0, titleid = 0;

		if (yaml_node_is_defined(t, "ItemID") && (nameid = yaml_get_int(t, "ItemID"))) {
			if (itemdb_exists(nameid)) {
				entry->rewards.nameid = nameid;
				entry->rewards.amount = 1; // Default the amount to 1
			} else if (nameid && !itemdb_exists(nameid)) {
				ShowWarning("achievement_read_db_sub: Invalid reward item ID %hu for achievement %d in \"%s\". Setting to 0.\n", nameid, achievement_id, source);
				entry->rewards.nameid = nameid = 0;
			}

			if (yaml_node_is_defined(t, "Amount") && (amount = yaml_get_int(t, "Amount")) && amount > 0 && nameid)
				entry->rewards.amount = amount;
		}
		if (yaml_node_is_defined(t, "Script") && (script_char = yaml_get_c_string(t, "Script"))){
			entry->rewards.script = parse_script(script_char, source, achievement_id, SCRIPT_IGNORE_EXTERNAL_BRACKETS);
			aFree(script_char);
		}
		if (yaml_node_is_defined(t, "TitleID") && (titleid = yaml_get_int(t, "TitleID")) && titleid > 0)
			entry->rewards.title_id = titleid;
	}

	if ((score = yaml_get_int(wrapper, "Score")) && score > 0)
		entry->score = score;

	return entry;
}

/**
 * Loads achievements from the achievement db.
 */
void achievement_read_db(void)
{
	yamlwrapper *adb = NULL, *adb_sub = NULL;
	yamliterator *it;
	int i = 0;
	const char *dbsubpath[] = {
		"",
		"/"DBIMPORT"/",
		//add other path here
	};

	for (i = 0; i < ARRAYLENGTH(dbsubpath); i++) {
		char filepath[256];
		int count = 0;

		if (!i)
			sprintf(filepath, "%s/%s%s%s", db_path, DBPATH, dbsubpath[i], "achievement_db.yml");
		else
			sprintf(filepath, "%s%s%s", db_path, dbsubpath[i], "achievement_db.yml");

		if ((adb = yaml_load_file(filepath)) == NULL) {
			ShowError("Failed to read '%s'.\n", filepath);
			continue;
		}

		if (!yaml_node_is_defined(adb, "Achievements"))
			continue; // Skip if base structure isn't defined
		adb_sub = yaml_get_subnode(adb, "Achievements");
		it = yaml_get_iterator(adb_sub);
		if (yaml_iterator_is_valid(it)) {
			yamlwrapper *id = NULL;

			for (id = yaml_iterator_first(it); yaml_iterator_has_next(it); id = yaml_iterator_next(it)) {
				struct achievement_db *duplicate = &achievement_dummy, *entry = achievement_read_db_sub(id, count, filepath);

				if (!entry) {
					ShowWarning("achievement_read_db: Failed to parse achievement entry %d.\n", count);
					continue;
				}
				if ((duplicate = achievement_search(entry->achievement_id)) != &achievement_dummy) {
					if (!i) { // Normal file read-in
						ShowWarning("achievement_read_db: Duplicate achievement %d.\n", entry->achievement_id);
						achievement_db_free_sub(entry, false);
						continue;
					}
					else // Import file read-in, free previous value and store new value
						achievement_db_free_sub(duplicate, false);
				}
				yaml_destroy_wrapper(id);
				idb_put(achievement_db, entry->achievement_id, entry);
				count++;
			}
		}
		yaml_destroy_wrapper(adb_sub);
		yaml_iterator_destroy(it);

		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filepath);
	}

	return;
}

/**
 * Recursive method to free an achievement condition
 * @param condition: Condition to clear
 */
void achievement_script_free(struct av_condition *condition) 
{
	if (condition->left) {
		achievement_script_free(condition->left);
		condition->left = NULL;
	}

	if (condition->right) {
		achievement_script_free(condition->right);
		condition->right = NULL;
	}

	aFree(condition);
}

/**
 * Clear achievement single entry
 * @param achievement: Achievement to clear
 * @param free: Will free achievement from memory
 */
void achievement_db_free_sub(struct achievement_db *achievement, bool free)
{
	if (achievement->targets) {
		aFree(achievement->targets);
		achievement->targets = NULL;
		achievement->target_count = 0;
	}
	if (achievement->condition) {
		achievement_script_free(achievement->condition);
		achievement->condition = NULL;
	}
	if (achievement->dependents) {
		aFree(achievement->dependents);
		achievement->dependents = NULL;
		achievement->dependent_count = 0;
	}
	if (achievement->rewards.script) {
		script_free_code(achievement->rewards.script);
		achievement->rewards.script = NULL;
	}
	if (free)
		aFree(achievement);
}

/**
 * Clears the achievement database for shutdown or reload.
 */
static int achievement_db_free(DBKey key, DBData *data, va_list ap)
{
	struct achievement_db *achievement = (struct achievement_db *)db_data2ptr(data);

	if (!achievement)
		return 0;

	achievement_db_free_sub(achievement, true);
	return 1;
}

static int achievementmobs_db_free(DBKey key, DBData *data, va_list ap)
{
	struct achievementmobs_db *achievement = (struct achievementmobs_db *)db_data2ptr(data);

	if (!achievement)
		return 0;

	aFree(achievement);
	return 1;
}

void achievement_db_reload(void)
{
	if (!battle_config.feature_achievement)
		return;
	achievementmobs_db->clear(achievementmobs_db, achievementmobs_db_free);
	achievement_db->clear(achievement_db, achievement_db_free);
	achievement_read_db();
}

void do_init_achievement(void)
{
	if (!battle_config.feature_achievement)
		return;
	memset(&achievement_dummy, 0, sizeof(achievement_dummy));
	achievement_db = idb_alloc(DB_OPT_BASE);
	achievementmobs_db = idb_alloc(DB_OPT_BASE);
	achievement_read_db();
}

void do_final_achievement(void)
{
	if (!battle_config.feature_achievement)
		return;
	achievementmobs_db->destroy(achievementmobs_db, achievementmobs_db_free);
	achievement_db->destroy(achievement_db, achievement_db_free);
}
