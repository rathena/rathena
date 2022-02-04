// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "quest.hpp"

#include <stdlib.h>

#include "../common/cbasetypes.hpp"
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/strlib.hpp"
#include "../common/utilities.hpp"
#include "../common/utils.hpp"

#include "battle.hpp"
#include "chrif.hpp"
#include "clif.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "map.hpp"
#include "mob.hpp"
#include "party.hpp"
#include "pc.hpp"

static int split_exact_quest_time(char* modif_p, int* day, int* hour, int* minute, int *second);

const std::string QuestDatabase::getDefaultLocation() {
	return std::string(db_path) + "/quest_db.yml";
}

/**
 * Reads and parses an entry from the quest_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 QuestDatabase::parseBodyNode(const YAML::Node &node) {
	uint32 quest_id;

	if (!this->asUInt32(node, "Id", quest_id))
		return 0;

	std::shared_ptr<s_quest_db> quest = this->find(quest_id);
	bool exists = quest != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Title" }))
			return 0;

		quest = std::make_shared<s_quest_db>();
		quest->id = quest_id;
	}

	if (this->nodeExists(node, "Title")) {
		std::string name;

		if (!this->asString(node, "Title", name))
			return 0;

		quest->name = name;
	}

	if (this->nodeExists(node, "TimeLimit")) {
		std::string time;

		if (!this->asString(node, "TimeLimit", time))
			return 0;

		if (time.find("+") != std::string::npos) {
			double timediff = solve_time(const_cast<char *>(time.c_str()));

			if (timediff <= 0) {
				this->invalidWarning(node["TimeLimit"], "Incorrect TimeLimit format %s given, skipping.\n", time.c_str());
				return 0;
			}
			quest->time = static_cast<time_t>(timediff);
		}
		else {// '+' not found, set to specific time
			int32 day, hour, minute, second;

			if (split_exact_quest_time(const_cast<char *>(time.c_str()), &day, &hour, &minute, &second) == 0) {
				this->invalidWarning(node["TimeLimit"], "Incorrect TimeLimit format %s given, skipping.\n", time.c_str());
				return 0;
			}
			quest->time = day * 86400 + hour * 3600 + minute * 60 + second;
			quest->time_at = true;
		}

	} else {
		if (!exists) {
			quest->time = 0;
			quest->time_at = false;
		}
	}

	if (this->nodeExists(node, "Targets")) {
		const YAML::Node &targets = node["Targets"];

		for (const YAML::Node &targetNode : targets) {
			if (quest->objectives.size() >= MAX_QUEST_OBJECTIVES) {
				this->invalidWarning(targetNode, "Targets list exceeds the maximum of %d, skipping.\n", MAX_QUEST_OBJECTIVES);
				return 0;
			}

			if (!this->nodeExists(targetNode, "Mob") && !this->nodeExists(targetNode, "Id")) {
				this->invalidWarning(targetNode, "Missing Target 'Mob' or 'Id', skipping.\n");
				return 0;
			}

			std::shared_ptr<s_quest_objective> target;
			std::vector<std::shared_ptr<s_quest_objective>>::iterator it;
			uint16 index = 0, mob_id = 0;

			if (this->nodeExists(targetNode, "Mob")) {

				std::string mob_name;

				if (!this->asString(targetNode, "Mob", mob_name))
					return 0;

				std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname(mob_name.c_str());

				if (!mob) {
					this->invalidWarning(targetNode["Mob"], "Mob %s does not exist, skipping.\n", mob_name.c_str());
					return 0;
				}

				mob_id = mob->id;

				it = std::find_if(quest->objectives.begin(), quest->objectives.end(), [&](std::shared_ptr<s_quest_objective> const &v) {
					return (*v).mob_id == mob_id;
				});
			}
			else {
				if (!this->asUInt16(targetNode, "Id", index)) {
					this->invalidWarning(targetNode, "Missing 'Id', skipping.\n");
					return 0;
				}
				if (index == 0) {
					this->invalidWarning(targetNode["Id"], "'Id' can't be 0, skipping.\n");
					return 0;
				}

				it = std::find_if(quest->objectives.begin(), quest->objectives.end(), [&](std::shared_ptr<s_quest_objective> const &v) {
					return (*v).index == index;
				});
			}

			if (it != quest->objectives.end())
				target = (*it);
			else
				target = nullptr;

			bool targetExists = target != nullptr;

			if (!targetExists) {
				if (!this->nodeExists(targetNode, "Count")) {
					this->invalidWarning(targetNode["Count"], "Targets has no Count value specified, skipping.\n");
					return 0;
				}

				if (!this->nodeExists(targetNode, "Mob") && !this->nodeExists(targetNode, "MinLevel") && !this->nodeExists(targetNode, "MaxLevel") &&
						!this->nodeExists(targetNode, "Race") && !this->nodeExists(targetNode, "Size") && !this->nodeExists(targetNode, "Element") &&
						!this->nodeExists(targetNode, "Location") && !this->nodeExists(targetNode, "MapName")) {
					this->invalidWarning(targetNode, "Targets is missing required field, skipping.\n");
					return 0;
				}

				target = std::make_shared<s_quest_objective>();
				target->index = index;
				target->mob_id = mob_id;
				target->min_level = 0;
				target->max_level = 0;
				target->race = RC_ALL;
				target->size = SZ_ALL;
				target->element = ELE_ALL;
				target->mapid = -1;
				target->map_name = "";
			}

			if (!this->nodeExists(targetNode, "Mob")) {
				if (this->nodeExists(targetNode, "MinLevel")) {
					uint16 level;

					if (!this->asUInt16(targetNode, "MinLevel", level))
						return 0;

					target->min_level = level;
				}

				if (this->nodeExists(targetNode, "MaxLevel")) {
					uint16 level;

					if (!this->asUInt16(targetNode, "MaxLevel", level))
						return 0;

					if (target->min_level > level) {
						this->invalidWarning(targetNode["MaxLevel"], "%d's MinLevel is greater than MaxLevel. Defaulting MaxLevel to %d.\n", target->min_level, MAX_LEVEL);
						level = MAX_LEVEL;
					}

					target->max_level = level;
				}

				if (this->nodeExists(targetNode, "Race")) {
					std::string race;

					if (!this->asString(targetNode, "Race", race))
						return 0;

					std::string race_constant = "RC_" + race;
					int64 constant;

					if (!script_get_constant(race_constant.c_str(), &constant)) {
						this->invalidWarning(targetNode["Race"], "Invalid race %s, skipping.\n", race.c_str());
						return 0;
					}

					if (constant < RC_FORMLESS || constant > RC_ALL || constant == RC_PLAYER_HUMAN || constant == RC_PLAYER_DORAM) {
						this->invalidWarning(targetNode["Race"], "Unsupported race %s, skipping.\n", race.c_str());
						return 0;
					}

					target->race = static_cast<e_race>(constant);
				}

				if (this->nodeExists(targetNode, "Size")) {
					std::string size_;

					if (!this->asString(targetNode, "Size", size_))
						return 0;

					std::string size_constant = "Size_" + size_;
					int64 constant;

					if (!script_get_constant(size_constant.c_str(), &constant)) {
						this->invalidWarning(targetNode["Size"], "Invalid size type %s, skipping.\n", size_.c_str());
						return 0;
					}

					if (constant < SZ_SMALL || constant > SZ_ALL) {
						this->invalidWarning(targetNode["size"], "Unsupported size %s, skipping.\n", size_.c_str());
						return 0;
					}

					target->size = static_cast<e_size>(constant);
				}

				if (this->nodeExists(targetNode, "Element")) {
					std::string element;

					if (!this->asString(targetNode, "Element", element))
						return 0;

					std::string element_constant = "Ele_" + element;
					int64 constant;

					if (!script_get_constant(element_constant.c_str(), &constant)) {
						this->invalidWarning(targetNode["Element"], "Invalid element %s, skipping.\n", element.c_str());
						return 0;
					}

					if (constant < ELE_NEUTRAL || constant > ELE_ALL) {
						this->invalidWarning(targetNode["Element"], "Unsupported element %s, skipping.\n", element.c_str());
						return 0;
					}

					target->element = static_cast<e_element>(constant);
				}

				if (this->nodeExists(targetNode, "Location")) {
					std::string location;

					if (!this->asString(targetNode, "Location", location))
						return 0;

					uint16 mapindex = mapindex_name2idx(location.c_str(), nullptr);

					if (mapindex == 0) {
						this->invalidWarning(targetNode["Location"], "Map \"%s\" not found.\n", location.c_str());
						return 0;
					}

					target->mapid = map_mapindex2mapid(mapindex);
				}

				if (this->nodeExists(targetNode, "MapName")) {
					std::string map_name;

					if (!this->asString(targetNode, "MapName", map_name))
						return 0;

					target->map_name = map_name;
				}

				// if max_level is set, min_level is 1
				if (target->min_level == 0 && target->max_level > 0)
					target->min_level = 1;
			}

			if (this->nodeExists(targetNode, "Count")) {
				uint16 count;

				if (!this->asUInt16(targetNode, "Count", count))
					return 0;

				target->count = count;
			}

			quest->objectives.push_back(target);
		}
	}

	if (this->nodeExists(node, "Drops")) {
		const YAML::Node &drops = node["Drops"];

		for (const YAML::Node &dropNode : drops) {
			uint32 mob_id = 0; // Can be 0 which means all monsters

			if (this->nodeExists(dropNode, "Mob")) {
				std::string mob_name;

				if (!this->asString(dropNode, "Mob", mob_name))
					return 0;

				std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname(mob_name.c_str());

				if (!mob) {
					this->invalidWarning(dropNode["Mob"], "Mob %s does not exist, skipping.\n", mob_name.c_str());
					continue;
				}

				mob_id = mob->id;
			}

			//std::shared_ptr<s_quest_dropitem> target = util::vector_find(quest->dropitem, mob_id);
			std::shared_ptr<s_quest_dropitem> target;
			std::vector<std::shared_ptr<s_quest_dropitem>>::iterator it = std::find_if(quest->dropitem.begin(), quest->dropitem.end(), [&](std::shared_ptr<s_quest_dropitem> const &v) {
				return (*v).mob_id == mob_id;
			});

			if (it != quest->dropitem.end())
				target = (*it);
			else
				target = nullptr;

			bool targetExists = target != nullptr;

			if (!targetExists) {
				if (!this->nodeExists(dropNode, "Item")) {
					this->invalidWarning(dropNode["Item"], "Drops has no Item value specified, skipping.\n");
					continue;
				}

				if (!this->nodeExists(dropNode, "Rate")) {
					this->invalidWarning(dropNode["Item"], "Drops has no Rate value specified, skipping.\n");
					continue;
				}

				target = std::make_shared<s_quest_dropitem>();
				target->mob_id = mob_id;
			}

			if (this->nodeExists(dropNode, "Item")) {
				std::string item_name;

				if (!this->asString(dropNode, "Item", item_name))
					return 0;

				std::shared_ptr<item_data> item = item_db.search_aegisname( item_name.c_str() );

				if (!item) {
					this->invalidWarning(dropNode["Item"], "Item %s does not exist, skipping.\n", item_name.c_str());
					continue;
				}

				target->nameid = item->nameid;
			}

			if (this->nodeExists(dropNode, "Count")) {
				uint16 count;

				if (!this->asUInt16(dropNode, "Count", count))
					return 0;

				if (!itemdb_isstackable(target->nameid)) {
					this->invalidWarning(dropNode["Count"], "Item %s is not stackable, capping to 1.\n", itemdb_name(target->nameid));
					count = 1;
				}

				target->count = count;
			} else {
				if (!targetExists)
					target->count = 1;
			}

			if (this->nodeExists(dropNode, "Rate")) {
				uint16 rate;

				if (!this->asUInt16(dropNode, "Rate", rate))
					return 0;

				target->rate = rate;
			}

			quest->dropitem.push_back(target);
		}
	}

	if (!exists)
		this->put(quest_id, quest);

	return 1;
}


static int split_exact_quest_time(char* modif_p, int* day, int* hour, int* minute, int *second) {
	int d = -1, h = -1, mn = -1, s = -1;

	nullpo_retr(0, modif_p);

	while (modif_p[0] != '\0') {
		int value = atoi(modif_p);

		if (modif_p[0] == '-' || modif_p[0] == '+')
			modif_p++;
		while (modif_p[0] >= '0' && modif_p[0] <= '9')
			modif_p++;
		if (modif_p[0] == 's') {
			s = value;
			modif_p++;
		} else if (modif_p[0] == 'm' && modif_p[1] == 'n') {
			mn = value;
			modif_p = modif_p + 2;
		} else if (modif_p[0] == 'h') {
			h = value;
			modif_p++;
		} else if (modif_p[0] == 'd' || modif_p[0] == 'j') {
			d = value;
			modif_p++;
		} else if (modif_p[0] != '\0') {
			modif_p++;
		}
	}

	if (h < 0 || h > 23 || mn > 59 || s > 59)	// hour is required
		return 0;

	*day = max(0,d);
	*hour = h;
	*minute = max(0,mn);
	*second = max(0,s);

	return 1;
}

/**
 * Searches a quest by ID.
 * @param quest_id : ID to lookup
 * @return Quest entry or nullptr on failure
 */
std::shared_ptr<s_quest_db> quest_search(int quest_id)
{
	auto quest = quest_db.find(quest_id);

	if (!quest)
		return nullptr;

	return quest;
}

/**
 * Sends quest info to the player on login.
 * @param sd : Player's data
 * @return 0 in case of success, nonzero otherwise (i.e. the player has no quests)
 */
int quest_pc_login(struct map_session_data *sd)
{
	if (!sd->avail_quests)
		return 1;

	clif_quest_send_list(sd);

#if PACKETVER < 20141022
	clif_quest_send_mission(sd);

	//@TODO[Haru]: Is this necessary? Does quest_send_mission not take care of this?
	for (int i = 0; i < sd->avail_quests; i++)
		clif_quest_update_objective(sd, &sd->quest_log[i]);
#endif

	return 0;
}

/**
 * Determine a quest's time limit.
 * @param qi: Quest data
 * @return Time limit value
 */
static time_t quest_time(std::shared_ptr<s_quest_db> qi)
{
	if (!qi || qi->time < 0)
		return 0;

	if (!qi->time_at && qi->time > 0)
		return time(nullptr) + qi->time;
	else if (qi->time_at) {
		time_t t = time(nullptr);
		struct tm *lt = localtime(&t);
		uint32 time_today = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;

		if (time_today < (qi->time % 86400))
			return static_cast<time_t>(t + qi->time - time_today);
		else // Carry over to the next day
			return static_cast<time_t>(t + 86400 + qi->time - time_today);
	}

	return 0;
}

/**
 * Adds a quest to the player's list.
 * New quest will be added as Q_ACTIVE.
 * @param sd : Player's data
 * @param quest_id : ID of the quest to add.
 * @return 0 in case of success, nonzero otherwise
 */
int quest_add(struct map_session_data *sd, int quest_id)
{
	std::shared_ptr<s_quest_db> qi = quest_search(quest_id);

	if (!qi) {
		ShowError("quest_add: quest %d not found in DB.\n", quest_id);
		return -1;
	}

	if (quest_check(sd, quest_id, HAVEQUEST) >= 0) {
		ShowError("quest_add: Character %d already has quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	int n = sd->avail_quests; //Insertion point

	sd->num_quests++;
	sd->avail_quests++;
	RECREATE(sd->quest_log, struct quest, sd->num_quests);

	//The character has some completed quests, make room before them so that they will stay at the end of the array
	if (sd->avail_quests != sd->num_quests)
		memmove(&sd->quest_log[n + 1], &sd->quest_log[n], sizeof(struct quest) * (sd->num_quests-sd->avail_quests));

	sd->quest_log[n] = {};
	sd->quest_log[n].quest_id = qi->id;
	sd->quest_log[n].time = (uint32)quest_time(qi);
	sd->quest_log[n].state = Q_ACTIVE;
	sd->save_quest = true;

	clif_quest_add(sd, &sd->quest_log[n]);
	clif_quest_update_objective(sd, &sd->quest_log[n]);

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
int quest_change(struct map_session_data *sd, int qid1, int qid2)
{
	std::shared_ptr<s_quest_db> qi = quest_search(qid2);

	if (!qi) {
		ShowError("quest_change: quest %d not found in DB.\n", qid2);
		return -1;
	}

	if (quest_check(sd, qid2, HAVEQUEST) >= 0) {
		ShowError("quest_change: Character %d already has quest %d.\n", sd->status.char_id, qid2);
		return -1;
	}

	if (quest_check(sd, qid1, HAVEQUEST) < 0) {
		ShowError("quest_change: Character %d doesn't have quest %d.\n", sd->status.char_id, qid1);
		return -1;
	}

	int i;

	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == qid1);
	if (i == sd->avail_quests) {
		ShowError("quest_change: Character %d has completed quest %d.\n", sd->status.char_id, qid1);
		return -1;
	}

	sd->quest_log[i] = {};
	sd->quest_log[i].quest_id = qi->id;
	sd->quest_log[i].time = (uint32)quest_time(qi);
	sd->quest_log[i].state = Q_ACTIVE;
	sd->save_quest = true;

	clif_quest_delete(sd, qid1);
	clif_quest_add(sd, &sd->quest_log[i]);
	clif_quest_update_objective(sd, &sd->quest_log[i]);

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
int quest_delete(struct map_session_data *sd, int quest_id)
{
	int i;

	//Search for quest
	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if (i == sd->num_quests) {
		ShowError("quest_delete: Character %d doesn't have quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	if (sd->quest_log[i].state != Q_COMPLETE)
		sd->avail_quests--;

	if (i < --sd->num_quests) //Compact the array
		memmove(&sd->quest_log[i], &sd->quest_log[i + 1], sizeof(struct quest) * (sd->num_quests - i));

	if (sd->num_quests == 0) {
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
 *   int Mob Level
 *   int Mob Race
 *   int Mob Size
 *   int Mob Element
 */
int quest_update_objective_sub(struct block_list *bl, va_list ap)
{
	nullpo_ret(bl);

	struct map_session_data *sd = BL_CAST(BL_PC, bl);

	nullpo_ret(sd);

	if( !sd->avail_quests )
		return 0;
	
	if( sd->status.party_id != va_arg(ap, int))
		return 0;

	quest_update_objective(sd, va_arg(ap, struct mob_data*));

	return 1;
}

/**
 * Updates the quest objectives for a character after killing a monster, including the handling of quest-granted drops.
 * @param sd: Character's data
 * @param mob_id: Monster ID
 * @param mob_level: Monster Level
 * @param mob_race: Monster Race
 * @param mob_size: Monster Size
 * @param mob_element: Monster Element
 */
void quest_update_objective(struct map_session_data *sd, struct mob_data* md)
{
	nullpo_retv(sd);

	for (int i = 0; i < sd->avail_quests; i++) {
		if (sd->quest_log[i].state == Q_COMPLETE) // Skip complete quests
			continue;

		std::shared_ptr<s_quest_db> qi = quest_search(sd->quest_log[i].quest_id);
		if (!qi)
			continue;

		// Process quest objectives
		for (int j = 0; j < qi->objectives.size(); j++) {
			uint8 objective_check = 0; // Must pass all 6 checks

			if (qi->objectives[j]->mob_id == md->mob_id)
				objective_check = 6;
			else if (qi->objectives[j]->mob_id == 0) {
				if (qi->objectives[j]->min_level == 0 || qi->objectives[j]->min_level <= md->level)
					objective_check++;
				if (qi->objectives[j]->max_level == 0 || qi->objectives[j]->max_level >= md->level)
					objective_check++;
				if (qi->objectives[j]->race == RC_ALL || qi->objectives[j]->race == md->status.race)
					objective_check++;
				if (qi->objectives[j]->size == SZ_ALL || qi->objectives[j]->size == md->status.size)
					objective_check++;
				if (qi->objectives[j]->element == ELE_ALL || qi->objectives[j]->element == md->status.def_ele)
					objective_check++;
				if (qi->objectives[j]->mapid < 0 || (qi->objectives[j]->mapid == sd->bl.m && md->spawn != nullptr))
					objective_check++;
				else if (qi->objectives[j]->mapid >= 0) {
					struct map_data *mapdata = map_getmapdata(sd->bl.m);

					if (mapdata->instance_id && mapdata->instance_src_map == qi->objectives[j]->mapid)
						objective_check++;
				}
			}

			if (objective_check == 6 && sd->quest_log[i].count[j] < qi->objectives[j]->count)  {
				sd->quest_log[i].count[j]++;
				sd->save_quest = true;
				clif_quest_update_objective(sd, &sd->quest_log[i]);
			}
		}

		// Process quest-granted extra drop bonuses
		for (const auto &it : qi->dropitem) {
			if (it->mob_id != 0 && it->mob_id != md->mob_id)
				continue;
			if (it->rate < 10000 && rnd()%10000 >= it->rate)
				continue; // TODO: Should this be affected by server rates?
			if (!itemdb_exists(it->nameid))
				continue;

			struct item entry = {};

			entry.nameid = it->nameid;
			entry.identify = itemdb_isidentified(it->nameid);
			entry.amount = it->count;
//#ifdef BOUND_ITEMS
//			entry.bound = it->bound;
//#endif
//			if (it.isGUID)
//				item.unique_id = pc_generate_unique_id(sd);
			
			e_additem_result result;

			if ((result = pc_additem(sd, &entry, 1, LOG_TYPE_QUEST)) != ADDITEM_SUCCESS) // Failed to obtain the item
				clif_additem(sd, 0, 0, result);
//			else if (it.isAnnounced || itemdb_exists(it.nameid)->flag.broadcast)
//				intif_broadcast_obtain_special_item(sd, it.nameid, it.mob_id, ITEMOBTAIN_TYPE_MONSTER_ITEM);
		}
	}
	pc_show_questinfo(sd);
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
int quest_update_status(struct map_session_data *sd, int quest_id, e_quest_state status)
{
	int i;

	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == quest_id);
	if (i == sd->avail_quests) {
		ShowError("quest_update_status: Character %d doesn't have quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	sd->quest_log[i].state = status;
	sd->save_quest = true;

	if (status < Q_COMPLETE) {
		clif_quest_update_status(sd, quest_id, status == Q_ACTIVE ? true : false);
		return 0;
	}

	// The quest is complete, so it needs to be moved to the completed quests block at the end of the array.
	if (i < (--sd->avail_quests)) {
		struct quest tmp_quest;

		memcpy(&tmp_quest, &sd->quest_log[i], sizeof(struct quest));
		memcpy(&sd->quest_log[i], &sd->quest_log[sd->avail_quests], sizeof(struct quest));
		memcpy(&sd->quest_log[sd->avail_quests], &tmp_quest, sizeof(struct quest));
	}

	clif_quest_delete(sd, quest_id);

	if (save_settings&CHARSAVE_QUEST)
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
int quest_check(struct map_session_data *sd, int quest_id, e_quest_check_type type)
{
	int i;

	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if (i == sd->num_quests)
		return -1;

	switch (type) {
		case HAVEQUEST:
			if (sd->quest_log[i].state == Q_INACTIVE) // Player has the quest but it's in the inactive state; send it as Q_ACTIVE.
				return 1;
			return sd->quest_log[i].state;
		case PLAYTIME:
			return (sd->quest_log[i].time < (unsigned int)time(nullptr) ? 2 : sd->quest_log[i].state == Q_COMPLETE ? 1 : 0);
		case HUNTING:
			if (sd->quest_log[i].state == Q_INACTIVE || sd->quest_log[i].state == Q_ACTIVE) {
				int j;
				std::shared_ptr<s_quest_db> qi = quest_search(sd->quest_log[i].quest_id);

				ARR_FIND(0, qi->objectives.size(), j, sd->quest_log[i].count[j] < qi->objectives[j]->count);
				if (j == qi->objectives.size())
					return 2;
				if (sd->quest_log[i].time < (unsigned int)time(nullptr))
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
 * Map iterator to ensures a player has no invalid quest log entries.
 * Any entries that are no longer in the db are removed.
 * @see map_foreachpc
 * @param sd : Character's data
 * @param ap : Ignored
 */
static int quest_reload_check_sub(struct map_session_data *sd, va_list ap)
{
	nullpo_ret(sd);

	int i, j = 0;

	for (i = 0; i < sd->num_quests; i++) {
		std::shared_ptr<s_quest_db> qi = quest_search(sd->quest_log[i].quest_id);

		if (!qi) { //Remove no longer existing entries
			if (sd->quest_log[i].state != Q_COMPLETE) //And inform the client if necessary
				clif_quest_delete(sd, sd->quest_log[i].quest_id);
			continue;
		}

		if (i != j) {
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

bool QuestDatabase::reload() {
	if (!TypesafeYamlDatabase::reload())
		return false;

	// Update quest data for players, to ensure no entries about removed quests are left over.
	map_foreachpc(&quest_reload_check_sub);
	return true;
}

QuestDatabase quest_db;

/**
 * Initializes the quest interface.
 */
void do_init_quest(void)
{
	quest_db.load();
}

/**
 * Finalizes the quest interface before shutdown.
 */
void do_final_quest(void)
{
	quest_db.clear();
}
