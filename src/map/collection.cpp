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

#include "achievement.hpp"
#include "battle.hpp"
#include "chrif.hpp"
#include "clif.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "pc.hpp"

using namespace rathena;

const std::string CollectionDatabase::getDefaultLocation(){
	return std::string(db_path) + "/collection_db.yml";
}

uint64 CollectionDatabase::parseBodyNode( const YAML::Node &node ){
	std::string mob_name;

	if( !this->asString( node, "Mob", mob_name ) ){
		return 0;
	}

	std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname( mob_name.c_str() );

	if( mob == nullptr ){
		this->invalidWarning( node["Mob"], "Mob %s does not exist and cannot be used as a collection.\n", mob_name.c_str() );
		return 0;
	}

	uint16 mob_id = mob->id;

	std::shared_ptr<s_collection_db> collection = this->find( mob_id );
	bool exists = collection != nullptr;

	if( !exists ){
		// Check mandatory nodes
		if( !this->nodesExist( node, { "ConsumeItem", "RewardItemGroup" } ) ){
			return 0;
		}

		collection = std::make_shared<s_collection_db>();
		collection->class_ = mob_id;
	}

	if( this->nodeExists( node, "ConsumeItem" ) ){
		std::string item_name;

		if( !this->asString( node, "ConsumeItem", item_name ) ){
			return 0;
		}

		std::shared_ptr<item_data> item = item_db.search_aegisname( item_name.c_str() );

		if( item == nullptr ){
			this->invalidWarning( node["ConsumeItem"], "Consume item %s does not exist.\n", item_name.c_str() );
			return 0;
		}

		collection->ConsumeID = item->nameid;
	} else {
		if (!exists) {
			collection->ConsumeID = 0;
		}
	}

	if( this->nodeExists( node, "CaptureRate" ) ){
		uint16 rate;

		if( !this->asUInt16( node, "CaptureRate", rate ) ){
			return 0;
		}

		if( rate > 10000 ){
			this->invalidWarning( node["CaptureRate"], "CaptureRate %hu exceeds maximum of 10000. Capping...\n", rate );
			rate = 10000;
		}

		collection->CaptureRate = rate;
	} else {
		if (!exists) {
			collection->CaptureRate = 10000;
		}
	}

	if( this->nodeExists( node, "RewardItemGroup" ) ){
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
	} else {
		if (!exists) {
			collection->GroupID = 0;
		}
	}

	if (!exists) {
		this->put(mob_id, collection);
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
 * Search monster collection database for given value and type.
 * @param key : value to search for
 * @param type : collection type to search for (Catch)
 * @return Collection DB pointer on success, NULL on failure
 */
std::shared_ptr<s_collection_db> collection_db_search( int key, enum e_collection_itemtype type ){
	for( auto &pair : collection_db ){
		std::shared_ptr<s_collection_db> collection = pair.second;

		switch(type) {
			case COLLECTION_CATCH:		if(collection->ConsumeID == key) return collection; break;
			default:
				ShowError( "collection_db_search: Unsupported type %d\n", type );
				return nullptr;
		}
	}

	return nullptr;
}

/**
 * Display the success/failure roulette wheel when trying to catch monster.
 * @param sd : player requesting
 * @param target_class : monster ID of monster to catch
 * @return 0
 */
int collection_catch_process1(struct map_session_data *sd,int target_id)
{
	nullpo_ret(sd);

	sd->catch_target_class = target_id;
	clif_catch_collection_process(sd);

	return 0;
}

/**
 * Begin the actual catching process of a monster.
 * @param sd : player requesting
 * @param target_id : monster ID of monster to catch
 * @return 0:success, 1:failure
 */
int collection_catch_process2(struct map_session_data* sd, int target_id)
{
	struct mob_data* md;
	int collection_catch_rate = 0;

	nullpo_retr(1, sd);

	md = (struct mob_data*)map_id2bl(target_id);

	if (!md || md->bl.type != BL_MOB || md->bl.prev == NULL) { // Invalid inputs/state, abort capture.
		clif_collection_roulette(sd, 0);
		sd->catch_target_class = COLLECTION_CATCH_FAIL;
		sd->itemid = 0;
		sd->itemindex = -1;
		return 1;
	}

	std::shared_ptr<s_collection_db> collection = collection_db.find(md->mob_id);
	// If the target is a valid monster, we have a few exceptions
	if (collection) {
		if (sd->catch_target_class == COLLECTION_CATCH_UNIVERSAL && !status_has_mode(&md->status, MD_STATUSIMMUNE)) {
			sd->catch_target_class = md->mob_id;
		}
		else if (sd->catch_target_class == COLLECTION_CATCH_UNIVERSAL_ITEM && sd->itemid == collection->ConsumeID) {
			sd->catch_target_class = md->mob_id;
		}
	}

	if (sd->catch_target_class != md->mob_id || !collection) {
		clif_emotion(&md->bl, ET_ANGER);
		clif_collection_roulette(sd, 0);
		sd->catch_target_class = COLLECTION_CATCH_FAIL;
		return 1;
	}

	// Is it should use the same formula as monster catch rate?
	collection_catch_rate = (collection->CaptureRate+ (sd->status.base_level - md->level)*30 + sd->battle_status.luk*20)*(200 - get_percentage(md->status.hp, md->status.max_hp))/100;

	if(collection_catch_rate < 1)
		collection_catch_rate = 1;

	if(rnd()%10000 < collection_catch_rate) {
		unit_remove_map(&md->bl,CLR_OUTSIGHT);
		status_kill(&md->bl);
		clif_collection_roulette(sd,1);
		itemdb_group.pc_get_itemgroup(collection->GroupID, 1, sd);
	} else {
		clif_collection_roulette(sd,0);
		sd->catch_target_class = COLLECTION_CATCH_FAIL;
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
