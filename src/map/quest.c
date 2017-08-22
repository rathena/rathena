// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/socket.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"

#include "itemdb.h"
#include "map.h"
#include "pc.h"
#include "party.h"
#include "quest.h"
#include "chrif.h"
#include "intif.h"

#include <stdlib.h>

static DBMap *questdb;
static void questdb_free_sub(struct quest_db *quest, bool free);

/**
 * Searches a quest by ID.
 * @param quest_id : ID to lookup
 * @return Quest entry (equals to &quest_dummy if the ID is invalid)
 */
struct quest_db *quest_search(int quest_id)
{
	struct quest_db *quest = (struct quest_db *)idb_get(questdb, quest_id);
	if (!quest)
		return &quest_dummy;
	return quest;
}

/**
 * Sends quest info to the player on login.
 * @param sd : Player's data
 * @return 0 in case of success, nonzero otherwise (i.e. the player has no quests)
 */
int quest_pc_login(TBL_PC *sd)
{
#if PACKETVER < 20141022
	int i;
#endif

	if( sd->avail_quests == 0 )
		return 1;

	clif_quest_send_list(sd);

#if PACKETVER < 20141022
	clif_quest_send_mission(sd);

	//@TODO[Haru]: Is this necessary? Does quest_send_mission not take care of this?
	for( i = 0; i < sd->avail_quests; i++ )
		clif_quest_update_objective(sd, &sd->quest_log[i], 0);
#endif

	return 0;
}

/**
 * Adds a quest to the player's list.
 * New quest will be added as Q_ACTIVE.
 * @param sd : Player's data
 * @param quest_id : ID of the quest to add.
 * @return 0 in case of success, nonzero otherwise
 */
int quest_add(TBL_PC *sd, int quest_id)
{
	int n;
	struct quest_db *qi = quest_search(quest_id);

	if( qi == &quest_dummy ) {
		ShowError("quest_add: quest %d not found in DB.\n", quest_id);
		return -1;
	}

	if( quest_check(sd, quest_id, HAVEQUEST) >= 0 ) {
		ShowError("quest_add: Character %d already has quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	n = sd->avail_quests; //Insertion point

	sd->num_quests++;
	sd->avail_quests++;
	RECREATE(sd->quest_log, struct quest, sd->num_quests);

	//The character has some completed quests, make room before them so that they will stay at the end of the array
	if( sd->avail_quests != sd->num_quests )
		memmove(&sd->quest_log[n + 1], &sd->quest_log[n], sizeof(struct quest) * (sd->num_quests-sd->avail_quests));

	memset(&sd->quest_log[n], 0, sizeof(struct quest));

	sd->quest_log[n].quest_id = qi->id;
	if (qi->time) {
		if (qi->time_type == 0)
			sd->quest_log[n].time = (unsigned int)(time(NULL) + qi->time);
		else {	// quest time limit at HH:MM
			int time_today;
			time_t t;
			struct tm * lt;

			t = time(NULL);
			lt = localtime(&t);
			time_today = (lt->tm_hour) * 3600 + (lt->tm_min) * 60 + (lt->tm_sec);
			if (time_today < qi->time)
				sd->quest_log[n].time = (unsigned int)(time(NULL) + qi->time - time_today);
			else	// next day
				sd->quest_log[n].time = (unsigned int)(time(NULL) + 86400 + qi->time - time_today);
		}
	}
	sd->quest_log[n].state = Q_ACTIVE;

	sd->save_quest = true;

	clif_quest_add(sd, &sd->quest_log[n]);
	clif_quest_update_objective(sd, &sd->quest_log[n], 0);

	if( save_settings&CHARSAVE_QUEST )
		chrif_save(sd, CSAVE_NORMAL);

	return 0;
}

/**
 * Replaces a quest in a player's list with another one.
 * @param sd : Player's data
 * @param qid1 : Current quest to replace
 * @param qid2 : New quest to add
 * @return 0 in case of success, nonzero otherwise
 */
int quest_change(TBL_PC *sd, int qid1, int qid2)
{
	int i;
	struct quest_db *qi = quest_search(qid2);

	if( qi == &quest_dummy ) {
		ShowError("quest_change: quest %d not found in DB.\n", qid2);
		return -1;
	}

	if( quest_check(sd, qid2, HAVEQUEST) >= 0 ) {
		ShowError("quest_change: Character %d already has quest %d.\n", sd->status.char_id, qid2);
		return -1;
	}

	if( quest_check(sd, qid1, HAVEQUEST) < 0 ) {
		ShowError("quest_change: Character %d doesn't have quest %d.\n", sd->status.char_id, qid1);
		return -1;
	}

	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == qid1);
	if( i == sd->avail_quests ) {
		ShowError("quest_change: Character %d has completed quest %d.\n", sd->status.char_id, qid1);
		return -1;
	}

	memset(&sd->quest_log[i], 0, sizeof(struct quest));
	sd->quest_log[i].quest_id = qi->id;

	if (qi->time) {
		if (qi->time_type == 0)
			sd->quest_log[i].time = (unsigned int)(time(NULL) + qi->time);
		else {	// quest time limit at HH:MM
			int time_today;
			time_t t;
			struct tm * lt;

			t = time(NULL);
			lt = localtime(&t);
			time_today = (lt->tm_hour) * 3600 + (lt->tm_min) * 60 + (lt->tm_sec);
			if (time_today < qi->time)
				sd->quest_log[i].time = (unsigned int)(time(NULL) + qi->time - time_today);
			else	// next day
				sd->quest_log[i].time = (unsigned int)(time(NULL) + 86400 + qi->time - time_today);
		}
	}

	sd->quest_log[i].state = Q_ACTIVE;

	sd->save_quest = true;

	clif_quest_delete(sd, qid1);
	clif_quest_add(sd, &sd->quest_log[i]);
	clif_quest_update_objective(sd, &sd->quest_log[i], 0);

	if( save_settings&CHARSAVE_QUEST )
		chrif_save(sd, CSAVE_NORMAL);

	return 0;
}

/**
 * Removes a quest from a player's list
 * @param sd : Player's data
 * @param quest_id : ID of the quest to remove
 * @return 0 in case of success, nonzero otherwise
 */
int quest_delete(TBL_PC *sd, int quest_id)
{
	int i;

	//Search for quest
	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if( i == sd->num_quests ) {
		ShowError("quest_delete: Character %d doesn't have quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	if( sd->quest_log[i].state != Q_COMPLETE )
		sd->avail_quests--;

	if( i < --sd->num_quests ) //Compact the array
		memmove(&sd->quest_log[i], &sd->quest_log[i + 1], sizeof(struct quest) * (sd->num_quests - i));

	if( sd->num_quests == 0 ) {
		aFree(sd->quest_log);
		sd->quest_log = NULL;
	} else
		RECREATE(sd->quest_log, struct quest, sd->num_quests);

	sd->save_quest = true;

	clif_quest_delete(sd, quest_id);

	if( save_settings&CHARSAVE_QUEST )
		chrif_save(sd, CSAVE_NORMAL);

	return 0;
}

/**
 * Map iterator subroutine to update quest objectives for a party after killing a monster.
 * @see map_foreachinrange
 * @param ap : Argument list, expecting:
 *   int Party ID
 *   int Mob ID
 */
int quest_update_objective_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd;
	int mob_id, party_id;

	nullpo_ret(bl);
	nullpo_ret(sd = (struct map_session_data *)bl);

	party_id = va_arg(ap,int);
	mob_id = va_arg(ap,int);

	if( !sd->avail_quests )
		return 0;
	if( sd->status.party_id != party_id )
		return 0;

	quest_update_objective(sd, mob_id);

	return 1;
}

/**
 * Updates the quest objectives for a character after killing a monster, including the handling of quest-granted drops.
 * @param sd : Character's data
 * @param mob_id : Monster ID
 */
void quest_update_objective(TBL_PC *sd, int mob_id)
{
	int i, j;

	for( i = 0; i < sd->avail_quests; i++ ) {
		struct quest_db *qi = NULL;

		if( sd->quest_log[i].state == Q_COMPLETE ) // Skip complete quests
			continue;

		qi = quest_search(sd->quest_log[i].quest_id);

		for( j = 0; j < qi->objectives_count; j++ ) {
			if( qi->objectives[j].mob == mob_id && sd->quest_log[i].count[j] < qi->objectives[j].count )  {
				sd->quest_log[i].count[j]++;
				sd->save_quest = true;
				clif_quest_update_objective(sd, &sd->quest_log[i], mob_id);
			}
		}

		// process quest-granted extra drop bonuses
		for (j = 0; j < qi->dropitem_count; j++) {
			struct quest_dropitem *dropitem = &qi->dropitem[j];
			struct item item;
			int temp;

			if (dropitem->mob_id != 0 && dropitem->mob_id != mob_id)
				continue;
			// TODO: Should this be affected by server rates?
			if (dropitem->rate < 10000 && rnd()%10000 >= dropitem->rate)
				continue;
			if (!itemdb_exists(dropitem->nameid))
				continue;

			memset(&item,0,sizeof(item));
			item.nameid = dropitem->nameid;
			item.identify = itemdb_isidentified(dropitem->nameid);
			item.amount = dropitem->count;
//#ifdef BOUND_ITEMS
//			item.bound = dropitem->bound;
//#endif
//			if (dropitem->isGUID)
//				item.unique_id = pc_generate_unique_id(sd);
			if ((temp = pc_additem(sd, &item, 1, LOG_TYPE_QUEST)) != 0) // Failed to obtain the item
				clif_additem(sd, 0, 0, temp);
//			else if (dropitem->isAnnounced || itemdb_exists(dropitem->nameid)->flag.broadcast)
//				intif_broadcast_obtain_special_item(sd, dropitem->nameid, dropitem->mob_id, ITEMOBTAIN_TYPE_MONSTER_ITEM);
		}
	}
}

/**
 * Updates a quest's state.
 * Only status of active and inactive quests can be updated. Completed quests can't (for now).
 * @param sd : Character's data
 * @param quest_id : Quest ID to update
 * @param qs : New quest state
 * @return 0 in case of success, nonzero otherwise
 * @author [Inkfish]
 */
int quest_update_status(TBL_PC *sd, int quest_id, enum quest_state status)
{
	int i;

	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == quest_id);
	if( i == sd->avail_quests ) {
		ShowError("quest_update_status: Character %d doesn't have quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	sd->quest_log[i].state = status;
	sd->save_quest = true;

	if( status < Q_COMPLETE ) {
		clif_quest_update_status(sd, quest_id, status == Q_ACTIVE ? true : false);
		return 0;
	}

	// The quest is complete, so it needs to be moved to the completed quests block at the end of the array.
	if( i < (--sd->avail_quests) ) {
		struct quest tmp_quest;

		memcpy(&tmp_quest, &sd->quest_log[i], sizeof(struct quest));
		memcpy(&sd->quest_log[i], &sd->quest_log[sd->avail_quests], sizeof(struct quest));
		memcpy(&sd->quest_log[sd->avail_quests], &tmp_quest, sizeof(struct quest));
	}

	clif_quest_delete(sd, quest_id);

	if( save_settings&CHARSAVE_QUEST )
		chrif_save(sd, CSAVE_NORMAL);

	return 0;
}

/**
 * Queries quest information for a character.
 * @param sd : Character's data
 * @param quest_id : Quest ID
 * @param type : Check type
 * @return -1 if the quest was not found, otherwise it depends on the type:
 *   HAVEQUEST: The quest's state
 *   PLAYTIME:  2 if the quest's timeout has expired
 *              1 if the quest was completed
 *              0 otherwise
 *   HUNTING:   2 if the quest has not been marked as completed yet, and its objectives have been fulfilled
 *              1 if the quest's timeout has expired
 *              0 otherwise
 */
int quest_check(TBL_PC *sd, int quest_id, enum quest_check_type type)
{
	int i;

	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if( i == sd->num_quests )
		return -1;

	switch( type ) {
		case HAVEQUEST:
			if (sd->quest_log[i].state == Q_INACTIVE) // Player has the quest but it's in the inactive state; send it as Q_ACTIVE.
				return 1;
			return sd->quest_log[i].state;
		case PLAYTIME:
			return (sd->quest_log[i].time < (unsigned int)time(NULL) ? 2 : sd->quest_log[i].state == Q_COMPLETE ? 1 : 0);
		case HUNTING:
			if( sd->quest_log[i].state == Q_INACTIVE || sd->quest_log[i].state == Q_ACTIVE ) {
				int j;
				struct quest_db *qi = quest_search(sd->quest_log[i].quest_id);

				ARR_FIND(0, qi->objectives_count, j, sd->quest_log[i].count[j] < qi->objectives[j].count);
				if( j == qi->objectives_count )
					return 2;
				if( sd->quest_log[i].time < (unsigned int)time(NULL) )
					return 1;
			}
			return 0;
		default:
			ShowError("quest_check_quest: Unknown parameter %d",type);
			break;
	}

	return -1;
}

/**
 * Loads quests from the quest db.yml
 * @return Number of loaded quests, or -1 if the file couldn't be read.
 */
void quest_read_ymldb(void)
{
	const char* dbsubpath[] = {
		DBPATH,
		DBIMPORT"/",
	};
	uint8 f;
	for (f = 0; f < ARRAYLENGTH(dbsubpath); f++) {
		char filename[256];

		struct quest_db *quest = NULL;
		int quest_id = 0;
		uint16 total_quest = 0;
		int64 iterator_size = 0;

		sprintf(filename, "%s/%s%s", db_path, dbsubpath[f], "quest_db.yml");
		yamlwrapper* root_node = yaml_load_file(filename);
		yamlwrapper* quests_list = NULL;

		if (root_node == NULL) {
			ShowError("Can't read %s\n", filename);
			continue;
		}
		if (!yaml_node_is_defined(root_node, "Quest_list")) {
			ShowError("Missing 'Quest_list' in \"%s\", skipping.\n", filename);
			yaml_destroy_wrapper(root_node);
			continue;
		}
		quests_list = yaml_get_subnode(root_node, "Quest_list");
		yamliterator* quest_iterator = yaml_get_iterator(quests_list);

		if (yaml_iterator_is_valid(quest_iterator)) {
			yamlwrapper *id = NULL;

			iterator_size = yaml_iterator_size(quest_iterator);
			for (id = yaml_iterator_first(quest_iterator); yaml_iterator_has_next(quest_iterator); id = yaml_iterator_next(quest_iterator)) {
				yamlwrapper* hunt = NULL;
				yamlwrapper* hunt_for_item = NULL;
				yamlwrapper *tmp_node = NULL;
				yamliterator *tmp_it;

				// quest id
				if (!yaml_node_is_defined(id, "Quest_Infos")) {
					ShowError("Missing Quest info in '%s', entry #%d, skipping.\n", filename, total_quest);
					yaml_destroy_wrapper(id);
					continue;
				}
				tmp_node = yaml_get_subnode(id, "Quest_Infos");
				tmp_it = yaml_get_iterator(tmp_node);
				if (!yaml_iterator_is_valid(tmp_it)) {
					ShowWarning("quest_db: Invalid Quest_Infos format in '%s', entry #%d, skipping.\n", filename, total_quest);
					yaml_destroy_wrapper(id);
					yaml_destroy_wrapper(tmp_node);
					continue;
				}
				if (yaml_iterator_size(tmp_it) > 2)
					ShowDebug("quest_db: Invalid Quest_Infos size in '%s, entry #%d.\n", quest_id, filename, total_quest);
				yamlwrapper *wrapper_id = yaml_iterator_first(tmp_it);
				quest_id = yaml_as_int(wrapper_id);
				// if (yaml_iterator_has_next(tmp_it)) {
					// yamlwrapper *wrapper_name = yaml_iterator_next(tmp_it);
					// char* quest_name = yaml_as_c_string(tmp_it);
					// yaml_destroy_wrapper(wrapper_name);
				// }
				yaml_destroy_wrapper(wrapper_id);
				yaml_destroy_wrapper(tmp_node);
				yaml_iterator_destroy(tmp_it);

				if (quest_id < 0 || quest_id >= INT_MAX) {
					ShowError("Invalid quest ID '%d' in '%s'' (min: 0, max: %d.), skipping.\n", quest_id, filename, INT_MAX);
					yaml_destroy_wrapper(id);
					continue;
				}

				if (!(quest = (struct quest_db *)idb_get(questdb, quest_id)))
					CREATE(quest, struct quest_db, 1);
				else {
					if (quest->objectives) {
						aFree(quest->objectives);
						quest->objectives = NULL;
						quest->objectives_count = 0;
					}
					if (quest->dropitem) {
						aFree(quest->dropitem);
						quest->dropitem = NULL;
						quest->dropitem_count = 0;
					}
				}

				if (quest->id) {
					ShowWarning("Duplicate quest ID '%d' in '%s'.\n", quest_id, filename);
					yaml_destroy_wrapper(id);
					continue;
				}

				// TimeLimit
				if (yaml_node_is_defined(id, "TimeLimit")) {
					quest->time = yaml_get_int(id, "TimeLimit");
					quest->time_type = 0;
				}
				else if (yaml_node_is_defined(id, "TimeLimit_HourMin")) {
					char* time_limit_value = yaml_get_c_string(id, "TimeLimit_HourMin");
					if (strchr(time_limit_value,':') == NULL) {
						ShowWarning("Invalid TimeLimit_HourMin format for quest ID '%d' in '%s'.\n", quest_id, filename);
						yaml_destroy_wrapper(id);
						continue;
					}
					unsigned char hour, min;

					hour = atoi(time_limit_value);
					time_limit_value = strchr(time_limit_value,':');
					*time_limit_value ++= 0;
					min = atoi(time_limit_value);

					quest->time = hour * 3600 + min * 60;
					quest->time_type = 1;
				}

				// hunt objectives
				if (yaml_node_is_defined(id, "Hunt") && (tmp_node = yaml_get_subnode(id, "Hunt"))) {
					tmp_it = yaml_get_iterator(tmp_node);
					if (!yaml_iterator_is_valid(tmp_it)) {
						ShowWarning("quest_db: Invalid Hunt format for quest id %d in '%s', skipping.\n", quest_id, filename);
						yaml_destroy_wrapper(id);
						yaml_destroy_wrapper(tmp_node);
						continue;
					}
					if (yaml_iterator_size(tmp_it) % 2) {
						ShowWarning("quest_db: Invalid Hunt size for quest id %d in '%s, skipping.\n", quest_id, filename);
						yaml_destroy_wrapper(id);
						yaml_destroy_wrapper(tmp_node);
						continue;
					}
					yamlwrapper *wrapper_hunt = NULL;
					yamlwrapper *wrapper_count = NULL;

					for (wrapper_hunt = yaml_iterator_first(tmp_it); yaml_iterator_has_next(tmp_it) && quest->objectives_count < MAX_QUEST_OBJECTIVES; wrapper_hunt = yaml_iterator_next(tmp_it)) {
						wrapper_count = yaml_iterator_next(tmp_it);
						uint16 mob_id = yaml_as_int(wrapper_hunt);
						if (mob_id && mobdb_exists(mob_id) == NULL)
							ShowWarning("Invalid monster ID '%d' for quest id %d in '%s.\n", mob_id, quest_id, filename);
						else {
							RECREATE(quest->objectives, struct quest_objective, quest->objectives_count+1);
							quest->objectives[quest->objectives_count].mob = mob_id;
							quest->objectives[quest->objectives_count].count = yaml_as_int(wrapper_count);
							quest->objectives_count++;
						}
						yaml_destroy_wrapper(wrapper_hunt);
						yaml_destroy_wrapper(wrapper_count);
					}
					yaml_iterator_destroy(tmp_it);
					yaml_destroy_wrapper(tmp_node);
				}

				// drop objectives
				if (yaml_node_is_defined(id, "Hunt_For_Item") && (tmp_node = yaml_get_subnode(id, "Hunt_For_Item"))) {
					tmp_it = yaml_get_iterator(tmp_node);
					if (!yaml_iterator_is_valid(tmp_it)) {
						ShowWarning("quest_db: Invalid Hunt_For_Item format for quest id %d in '%s, skipping.\n", quest_id, filename);
						yaml_destroy_wrapper(id);
						yaml_destroy_wrapper(tmp_node);
						continue;
					}
					if (yaml_iterator_size(tmp_it) % 3) {
						ShowWarning("quest_db: Invalid Hunt_For_Item size for quest id %d in '%s, skipping.\n", quest_id, filename);
						yaml_destroy_wrapper(id);
						yaml_destroy_wrapper(tmp_node);
						continue;
					}
					yamlwrapper *wrapper_hunt = NULL;
					yamlwrapper *wrapper_nameid = NULL;
					yamlwrapper *wrapper_rate = NULL;

					for (wrapper_hunt = yaml_iterator_first(tmp_it); yaml_iterator_has_next(tmp_it) && quest->objectives_count < MAX_QUEST_OBJECTIVES; wrapper_hunt = yaml_iterator_next(tmp_it), wrapper_nameid = yaml_iterator_next(tmp_it), wrapper_rate = yaml_iterator_next(tmp_it)) {
						wrapper_nameid = yaml_iterator_next(tmp_it);
						wrapper_rate = yaml_iterator_next(tmp_it);
						uint16 nameid = yaml_as_int(wrapper_nameid);
						if (nameid && itemdb_exists(nameid) == NULL)
							ShowWarning("Invalid item reward '%d' for quest id %d in '%s.\n", nameid, quest_id, filename);
						else {
							uint16 mob_id = yaml_as_int(wrapper_hunt);
							if (mob_id && mobdb_exists(mob_id) == NULL)
								ShowWarning("Invalid monster ID '%d' for quest id %d in '%s.\n", mob_id, quest_id, filename);
							else {
								uint16 rate = yaml_as_int(wrapper_rate);
								RECREATE(quest->dropitem, struct quest_dropitem, quest->dropitem_count+1);
								quest->dropitem[quest->dropitem_count].mob_id = mob_id;
								quest->dropitem[quest->dropitem_count].nameid = nameid;
								quest->dropitem[quest->dropitem_count].count = 1;
								quest->dropitem[quest->dropitem_count].rate = rate;
								quest->dropitem_count++;
							}
						}
						yaml_destroy_wrapper(wrapper_hunt);
						yaml_destroy_wrapper(wrapper_nameid);
						yaml_destroy_wrapper(wrapper_rate);
					}
					yaml_iterator_destroy(tmp_it);
					yaml_destroy_wrapper(tmp_node);
				}

				if (!quest->id) {
					quest->id = quest_id;
					idb_put(questdb, quest->id, quest);
				}
				yaml_destroy_wrapper(id);
				total_quest++;
			}
		}
		yaml_iterator_destroy(quest_iterator);
		yaml_destroy_wrapper(quests_list);
		yaml_destroy_wrapper(root_node);

		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"/"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", total_quest, iterator_size, filename);
	}
}

/**
 * Loads Quest DB
 */
static void quest_read_db(void)
{
	quest_read_ymldb();
}

/**
 * Map iterator to ensures a player has no invalid quest log entries.
 * Any entries that are no longer in the db are removed.
 * @see map_foreachpc
 * @param sd : Character's data
 * @param ap : Ignored
 */
int quest_reload_check_sub(struct map_session_data *sd, va_list ap)
{
	int i, j;

	nullpo_ret(sd);

	j = 0;
	for( i = 0; i < sd->num_quests; i++ ) {
		struct quest_db *qi = quest_search(sd->quest_log[i].quest_id);

		if( qi == &quest_dummy ) { //Remove no longer existing entries
			if( sd->quest_log[i].state != Q_COMPLETE ) //And inform the client if necessary
				clif_quest_delete(sd, sd->quest_log[i].quest_id);
			continue;
		}

		if( i != j ) {
			//Move entries if there's a gap to fill
			memcpy(&sd->quest_log[j], &sd->quest_log[i], sizeof(struct quest));
		}

		j++;
	}

	sd->num_quests = j;
	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].state == Q_COMPLETE);
	sd->avail_quests = i;

	return 1;
}

/**
 * Clear quest single entry
 * @param quest
 * @param free Will free quest from memory
 **/
static void questdb_free_sub(struct quest_db *quest, bool free)
{
	if (quest->objectives) {
		aFree(quest->objectives);
		quest->objectives = NULL;
		quest->objectives_count = 0;
	}
	if (quest->dropitem) {
		aFree(quest->dropitem);
		quest->dropitem = NULL;
		quest->dropitem_count = 0;
	}
	if (&quest->name)
		StringBuf_Destroy(&quest->name);
	if (free)
		aFree(quest);
}

/**
 * Clears the quest database for shutdown or reload.
 */
static int questdb_free(DBKey key, DBData *data, va_list ap)
{
	struct quest_db *quest = (struct quest_db *)db_data2ptr(data);

	if (!quest)
		return 0;

	questdb_free_sub(quest, true);
	return 1;
}

/**
 * Initializes the quest interface.
 */
void do_init_quest(void)
{
	questdb = idb_alloc(DB_OPT_BASE);
	quest_read_db();
}

/**
 * Finalizes the quest interface before shutdown.
 */
void do_final_quest(void)
{
	memset(&quest_dummy, 0, sizeof(quest_dummy));
	questdb->destroy(questdb, questdb_free);
}

/**
 * Reloads the quest database.
 */
void do_reload_quest(void)
{
	memset(&quest_dummy, 0, sizeof(quest_dummy));
	questdb->clear(questdb, questdb_free);

	quest_read_db();

	//Update quest data for players, to ensure no entries about removed quests are left over.
	map_foreachpc(&quest_reload_check_sub);
}
