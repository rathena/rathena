// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "collection.hpp"

#include <map>
#include <string>

#include <stdlib.h>

#include <yaml-cpp/yaml.h>

#include "../common/db.hpp"
#include "../common/ers.hpp"
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"
#include "../common/utilities.hpp"
#include "../common/utils.hpp"

#include "battle.hpp"
#include "chrif.hpp"
#include "clif.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "mob.hpp"

using namespace rathena;

const std::string CollectionDatabase::getDefaultLocation(){
	return std::string(db_path) + "/collection_db.yml";
}

uint64 CollectionDatabase::parseBodyNode( const YAML::Node &node ){
	std::string item_name;

	if (!this->asString(node, "ConsumeItem", item_name))
		return 0;

	std::shared_ptr<item_data> item = item_db.search_aegisname(item_name.c_str());

	if (item == nullptr) {
		this->invalidWarning(node["Collection"], "Item %s does not exist and cannot be used.\n", item_name.c_str());
		return 0;
	}

	std::shared_ptr<s_collection_db> collection = this->find(item->nameid);
	bool exists = collection != nullptr;

	if (!exists) {
		// Check mandatory nodes
		if (!this->nodesExist(node, { "Target", "RewardItemGroup" })) {
			return 0;
		}

		collection = std::make_shared<s_collection_db>();
		collection->ConsumeID = item->nameid;
	}

	if (this->nodeExists(node, "Target")) {
		const YAML::Node& monsterNode = node["Target"];

		for (const auto& target : monsterNode) {
			std::string mob_name = target.first.as<std::string>(); 
			std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname(mob_name.c_str());

			if (mob == nullptr) {
				this->invalidWarning(node["Mob"], "Mob %s does not exist and cannot be used as a pet.\n", mob_name.c_str());
				return 0;
			}

			uint32 mob_id = mob->id;

			bool active;

			if (!this->asBool(monsterNode, mob_name, active))
				return 0;

			if (active)
				collection->MobID.push_back(mob_id);
			else
				util::vector_erase_if_exists(collection->MobID, mob_id);
		}
	}

	if (this->nodeExists(node, "CaptureRate")) {
		uint16 rate;

		if (!this->asUInt16(node, "CaptureRate", rate)) {
			return 0;
		}

		if (rate > 10000) {
			this->invalidWarning(node["CaptureRate"], "CaptureRate %hu exceeds maximum of 10000. Capping...\n", rate);
			rate = 10000;
		}

		collection->CaptureRate = rate;
	} else {
		if (!exists) {
			collection->CaptureRate = 10000;
		}
	}

	if (this->nodeExists(node, "RewardItemGroup")) {
		std::string group_name;

		if (!this->asString(node, "RewardItemGroup", group_name)) {
			return 0;
		}

		std::string group_name_constant = "IG_" + group_name;
		int64 constant;

		if (!script_get_constant(group_name_constant.c_str(), &constant) || constant < IG_BLUEBOX) {
			if (strncasecmp(group_name.c_str(), "IG_", 3) != 0)
				this->invalidWarning(node["RewardItemGroup"], "Invalid collection %s.\n", group_name.c_str());
			else
				this->invalidWarning(node["RewardItemGroup"], "Invalid collection %s. Note that 'IG_' is automatically appended to the group name.\n", group_name.c_str());
			return 0;
		}

		collection->GroupID = static_cast<uint16>(constant);
	}

	if (!exists) {
		this->put(item->nameid, collection);
	}
	return 1;
}

bool CollectionDatabase::reload(){
	if( !TypesafeYamlDatabase::reload() ){
		return false;
	}

	return true;
}

CollectionDatabase collection_db;

/**
 * Display the success/failure roulette wheel when trying to catch monster.
 * @param sd : player requesting
 * @param target_class : monster ID of monster to catch
 * @return 0
 */
int collection_catch_process1(struct map_session_data *sd, t_itemid item_id)
{
	nullpo_ret(sd);

	sd->catch_collection_class = item_id;
	clif_catch_collection_process(sd);

	return 0;
}

/**
 * Begin the actual catching process of a monster.
 * @param sd : player requesting
 * @param target_id : monster ID of monster to catch
 * @return 0:success, 1:failure
 */
int collection_catch_process2(struct map_session_data* sd, uint32 target_id)
{
	nullpo_retr(1, sd);

	std::shared_ptr<s_collection_db> collection = collection_db.find(sd->catch_collection_class);

	struct mob_data* md = (struct mob_data*)map_id2bl(target_id);

	if (!md || md->bl.type != BL_MOB || md->bl.prev == NULL) {	// Invalid inputs/state, abort capture
		clif_collection_roulette(sd, 0);
		sd->catch_collection_class = COLLECTION_CATCH_FAIL;
		sd->itemid = 0;
		sd->itemindex = -1;
		return 1;
	}

	// Monster ID not in array
	if (!collection) {
		clif_emotion(&md->bl, ET_ANGER);
		clif_collection_roulette(sd, 0);
		sd->catch_collection_class = COLLECTION_CATCH_FAIL;
		return 1;
	}

	// If the target is a valid monster, we have a few exceptions
	if (collection) {
		if (sd->catch_collection_class == COLLECTION_CATCH_UNIVERSAL && !status_has_mode(&md->status, MD_STATUSIMMUNE)) {
			sd->catch_collection_class = md->mob_id;
		}
		else if (sd->catch_collection_class == COLLECTION_CATCH_UNIVERSAL_ITEM && sd->itemid == collection->ConsumeID) {
			sd->catch_collection_class = md->mob_id;
		}
	}

	// Monster ID not in array
	if (std::find(collection->MobID.begin(), collection->MobID.end(), md->mob_id) == collection->MobID.end()) {
		clif_emotion(&md->bl, ET_ANGER);
		clif_collection_roulette(sd, 0);
		sd->catch_collection_class = COLLECTION_CATCH_FAIL;
		return 1;
	}

	// Is it should use the same formula as monster catch rate?
	int collection_catch_rate = (collection->CaptureRate+ (sd->status.base_level - md->level)*30 + sd->battle_status.luk*20)*(200 - get_percentage(md->status.hp, md->status.max_hp))/100;

	if(collection_catch_rate < 1)
		collection_catch_rate = 1;

	if(rnd()%10000 < collection_catch_rate) {
		unit_remove_map(&md->bl,CLR_OUTSIGHT);
		status_kill(&md->bl);
		clif_collection_roulette(sd,1);
		itemdb_group.pc_get_itemgroup(collection->GroupID, 1, sd);
	} else {
		clif_collection_roulette(sd,0);
		sd->catch_collection_class = COLLECTION_CATCH_FAIL;
	}

	return 0;
}

/**
 * Initialize collection data.
 */
void do_init_collection(void)
{
	collection_db.load();
}

/**
 * Destroy collection data.
 */
void do_final_collection(void)
{
	collection_db.clear();
}
