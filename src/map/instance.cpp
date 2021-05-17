// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "instance.hpp"

#include <stdlib.h>
#include <math.h>
#include <yaml-cpp/yaml.h>

#include "../common/cbasetypes.hpp"
#include "../common/db.hpp"
#include "../common/ers.hpp"  // ers_destroy
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"
#include "../common/utilities.hpp"

#include "clan.hpp"
#include "clif.hpp"
#include "guild.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "pc.hpp"

using namespace rathena;

/// Instance Idle Queue data
struct s_instance_wait {
	std::deque<int> id;
	int timer;
} instance_wait;

#define INSTANCE_INTERVAL	60000	// Interval used to check when an instance is to be destroyed (ms)

int16 instance_start = 0; // Instance MapID start
int instance_count = 1; // Total created instances

std::unordered_map<int, std::shared_ptr<s_instance_data>> instances;

const std::string InstanceDatabase::getDefaultLocation() {
	return std::string(db_path) + "/instance_db.yml";
}

/**
 * Reads and parses an entry from the instance_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 InstanceDatabase::parseBodyNode(const YAML::Node &node) {
	int32 instance_id = 0;

	if (!this->asInt32(node, "Id", instance_id))
		return 0;

	if (instance_id <= 0) {
		this->invalidWarning(node, "Instance Id is invalid. Valid range 1~%d, skipping.\n", INT_MAX);
		return 0;
	}

	std::shared_ptr<s_instance_db> instance = this->find(instance_id);
	bool exists = instance != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Name", "Enter" }))
			return 0;

		instance = std::make_shared<s_instance_db>();
		instance->id = instance_id;
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		for (const auto &instance : instance_db) {
			if (instance.second->name.compare(name) == 0) {
				this->invalidWarning(node["Name"], "Instance name %s already exists, skipping.\n", name.c_str());
				return 0;
			}
		}

		instance->name = name;
	}

	if (this->nodeExists(node, "TimeLimit")) {
		uint32 limit;

		if (!this->asUInt32(node, "TimeLimit", limit))
			return 0;

		instance->limit = limit;
	} else {
		if (!exists)
			instance->limit = 3600;
	}

	if (this->nodeExists(node, "IdleTimeOut")) {
		uint32 idle;

		if (!this->asUInt32(node, "IdleTimeOut", idle))
			return 0;

		instance->timeout = idle;
	} else {
		if (!exists)
			instance->timeout = 300;
	}

	if (this->nodeExists(node, "Destroyable")) {
		bool destroy;

		if (!this->asBool(node, "Destroyable", destroy))
			return 0;

		instance->destroyable = destroy;
	} else {
		if (!exists)
			instance->destroyable = true;
	}

	if (this->nodeExists(node, "Enter")) {
		const YAML::Node &enterNode = node["Enter"];

		if (!this->nodesExist(enterNode, { "Map", "X", "Y" }))
			return 0;

		if (this->nodeExists(enterNode, "Map")) {
			std::string map;

			if (!this->asString(enterNode, "Map", map))
				return 0;

			int16 m = map_mapname2mapid(map.c_str());

			if (m == -1) {
				this->invalidWarning(enterNode["Map"], "Map %s is not a valid map, skipping.\n", map.c_str());
				return 0;
			}

			instance->enter.map = m;
		}

		if (this->nodeExists(enterNode, "X")) {
			int16 x;

			if (!this->asInt16(enterNode, "X", x))
				return 0;

			instance->enter.x = x;
		}

		if (this->nodeExists(enterNode, "Y")) {
			int16 y;

			if (!this->asInt16(enterNode, "Y", y))
				return 0;

			instance->enter.y = y;
		}
	}

	if (this->nodeExists(node, "AdditionalMaps")) {
		const YAML::Node &mapNode = node["AdditionalMaps"];

		for (const auto &mapIt : mapNode) {
			std::string map = mapIt.first.as<std::string>();
			int16 m = map_mapname2mapid(map.c_str());

			if (m == instance->enter.map) {
				this->invalidWarning(mapNode, "Additional Map %s is already listed as the EnterMap.\n", map.c_str());
				continue;
			}

			if (m == -1) {
				this->invalidWarning(mapNode, "Additional Map %s is not a valid map, skipping.\n", map.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(mapNode, map, active))
				return 0;

			if (active)
				instance->maplist.push_back(m);
			else
				util::vector_erase_if_exists(instance->maplist, m);
		}
	}

	if (!exists)
		this->put(instance_id, instance);

	return 1;
}

InstanceDatabase instance_db;

/**
 * Searches for an instance name in the database
 * @param instance_name: Instance to search for
 * @return shared_ptr of instance or nullptr on failure
 */
std::shared_ptr<s_instance_db> instance_search_db_name(const char *instance_name)
{
	for (const auto &it : instance_db) {
		if (!strcmp(it.second->name.c_str(), instance_name))
			return it.second;
	}

	return nullptr;
}

/**
 * Search for a sd of an Instance
 * @param instance_id: Instance ID
 * @param sd: Pointer to player data
 * @param target: Target display type
 */
void instance_getsd(int instance_id, struct map_session_data *&sd, enum send_target *target) {
	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);

	if (!idata) {
		sd = nullptr;
		return;
	}

	switch(idata->mode) {
		case IM_NONE:
			sd = nullptr;
			(*target) = SELF;
			break;
		case IM_GUILD:
			sd = guild_getavailablesd(guild_search(idata->owner_id));
			(*target) = GUILD;
			break;
		case IM_PARTY:
			sd = party_getavailablesd(party_search(idata->owner_id));
			(*target) = PARTY;
			break;
		case IM_CHAR:
			sd = map_charid2sd(idata->owner_id);
			(*target) = SELF;
			break;
		case IM_CLAN:
			sd = clan_getavailablesd(clan_search(idata->owner_id));
			(*target) = CLAN;
	}
	return;
}

/**
 * Deletes an instance timer (Destroys instance)
 */
static TIMER_FUNC(instance_delete_timer){
	instance_destroy(id);

	return 0;
}

/**
 * Create subscription timer
 */
static TIMER_FUNC(instance_subscription_timer){
	int instance_id = instance_wait.id[0];

	if (instance_id <= 0 || instance_wait.id.empty())
		return 0;

	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);

	if (!idata)
		return 0;

	struct map_session_data *sd;
	struct party_data *pd;
	struct guild *gd;
	struct clan *cd;
	e_instance_mode mode = idata->mode;
	int ret = instance_addmap(instance_id); // Check that maps have been added

	switch(mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (ret == 0 && (sd = map_charid2sd(idata->owner_id))) // If no maps are created, tell player to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		case IM_PARTY:
			if (ret == 0 && (pd = party_search(idata->owner_id))) // If no maps are created, tell party to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		case IM_GUILD:
			if (ret == 0 && (gd = guild_search(idata->owner_id))) // If no maps are created, tell guild to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		case IM_CLAN:
			if (ret == 0 && (cd = clan_search(idata->owner_id))) // If no maps are created, tell clan to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		default:
			return 0;
	}

	instance_wait.id.pop_front();

	for (int i = 0; i < instance_wait.id.size(); i++) {
		if (idata->state == INSTANCE_IDLE && ((mode == IM_CHAR && sd) || (mode == IM_GUILD && gd) || (mode == IM_PARTY && pd) || (mode == IM_CLAN && cd)))
			clif_instance_changewait(instance_id, i + 1);
	}

	if (!instance_wait.id.empty())
		instance_wait.timer = add_timer(gettick() + INSTANCE_INTERVAL, instance_subscription_timer, 0, 0);
	else
		instance_wait.timer = INVALID_TIMER;

	return 0;
}

/**
 * Adds timer back to members entering instance
 * @param idata: Instance data
 * @param instance_id: Instance ID to notify
 * @return True on success or false on failure
 */
bool instance_startkeeptimer(std::shared_ptr<s_instance_data> idata, int instance_id)
{
	// No timer
	if (!idata || idata->keep_timer != INVALID_TIMER)
		return false;

	std::shared_ptr<s_instance_db> db = instance_db.find(idata->id);

	if (!db)
		return false;

	// Add timer
	idata->keep_limit = static_cast<unsigned int>(time(nullptr)) + db->limit;
	idata->keep_timer = add_timer(gettick() + db->limit * 1000, instance_delete_timer, instance_id, 0);

	switch(idata->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_charid2sd(idata->owner_id)) // Notify player of the added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(idata->owner_id)) // Notify party of the added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(idata->owner_id)) // Notify guild of the added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_CLAN:
			if (clan_search(idata->owner_id)) // Notify clan of the added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		default:
			return false;
	}

	return true;
}

/**
 * Creates an idle timer for an instance, default is 5 minutes
 * @param idata: Instance data
 * @param instance_id: Instance ID to notify
 * @param True on success or false on failure
 */
bool instance_startidletimer(std::shared_ptr<s_instance_data> idata, int instance_id)
{
	// No current timer
	if (!idata || idata->idle_timer != INVALID_TIMER)
		return false;

	std::shared_ptr<s_instance_db> db = instance_db.find(idata->id);

	if (!db)
		return false;

	// Add the timer
	idata->idle_limit = static_cast<unsigned int>(time(nullptr)) + db->timeout;
	idata->idle_timer = add_timer(gettick() + db->timeout * 1000, instance_delete_timer, instance_id, 0);

	switch(idata->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_charid2sd(idata->owner_id)) // Notify player of added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(idata->owner_id)) // Notify party of added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(idata->owner_id)) // Notify guild of added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_CLAN:
			if (clan_search(idata->owner_id)) // Notify clan of added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		default:
			return false;
	}

	return true;
}

/**
 * Remove the idle timer from an instance
 * @param idata: Instace data
 * @param instance_id: Instance ID to notify
 * @return True on success or false on failure
 */
bool instance_stopidletimer(std::shared_ptr<s_instance_data> idata, int instance_id)
{
	// No timer
	if (!idata || idata->idle_timer == INVALID_TIMER)
		return false;

	// Delete the timer - Party has returned or instance is destroyed
	idata->idle_limit = 0;
	delete_timer(idata->idle_timer, instance_delete_timer);
	idata->idle_timer = INVALID_TIMER;

	switch(idata->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_charid2sd(idata->owner_id)) // Notify the player
				clif_instance_changestatus(instance_id, IN_NOTIFY, idata->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(idata->owner_id)) // Notify the party
				clif_instance_changestatus(instance_id, IN_NOTIFY, idata->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(idata->owner_id)) // Notify the guild
				clif_instance_changestatus(instance_id, IN_NOTIFY, idata->idle_limit);
			break;
		case IM_CLAN:
			if (clan_search(idata->owner_id)) // Notify the clan
				clif_instance_changestatus(instance_id, IN_NOTIFY, idata->idle_limit);
			break;
		default:
			return false;
	}

	return true;
}

/**
 * Run the OnInstanceInit events for duplicated NPCs
 */
static int instance_npcinit(struct block_list *bl, va_list ap)
{
	struct npc_data* nd;

	nullpo_retr(0, bl);
	nullpo_retr(0, nd = (struct npc_data *)bl);

	return npc_instanceinit(nd);
}

/**
 * Run the OnInstanceDestroy events for duplicated NPCs
 */
static int instance_npcdestroy(struct block_list *bl, va_list ap)
{
	struct npc_data* nd;

	nullpo_retr(0, bl);
	nullpo_retr(0, nd = (struct npc_data *)bl);

	return npc_instancedestroy(nd);
}

/**
 * Update instance with new NPC
 */
static int instance_addnpc_sub(struct block_list *bl, va_list ap)
{
	struct npc_data* nd;

	nullpo_retr(0, bl);
	nullpo_retr(0, nd = (struct npc_data *)bl);

	return npc_duplicate4instance(nd, va_arg(ap, int));
}

/**
 * Add an NPC to an instance
 * @param idata: Instance data
 */
void instance_addnpc(std::shared_ptr<s_instance_data> idata)
{
	// First add the NPCs
	for (const auto &it : idata->map) {
		struct map_data *mapdata = map_getmapdata(it.m);

		map_foreachinallarea(instance_addnpc_sub, it.src_m, 0, 0, mapdata->xs, mapdata->ys, BL_NPC, it.m);
	}

	// Now run their OnInstanceInit
	for (const auto &it : idata->map) {
		struct map_data *mapdata = map_getmapdata(it.m);

		map_foreachinallarea(instance_npcinit, it.m, 0, 0, mapdata->xs, mapdata->ys, BL_NPC, it.m);
  }
}

/**
 * Create an instance
 * @param owner_id: Owner block ID
 * @param name: Instance name
 * @param mode: Instance mode
 * @return -4 = no free instances | -3 = already exists | -2 = character/party/guild not found | -1 = invalid type | On success return instance_id
 */
int instance_create(int owner_id, const char *name, e_instance_mode mode) {
	std::shared_ptr<s_instance_db> db = instance_search_db_name(name);

	if (!db) {
		ShowError("instance_create: Unknown instance %s creation was attempted.\n", name);
		return -1;
	}

	struct map_session_data *sd = nullptr;
	struct party_data *pd;
	struct guild *gd;
	struct clan* cd;

	switch(mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (!(sd = map_charid2sd(owner_id))) {
				ShowError("instance_create: Character %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (sd->instance_id > 0)
				return -3; // Player already instancing
			break;
		case IM_PARTY:
			if (!(pd = party_search(owner_id))) {
				ShowError("instance_create: Party %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (pd->instance_id > 0)
				return -3; // Party already instancing
			break;
		case IM_GUILD:
			if (!(gd = guild_search(owner_id))) {
				ShowError("instance_create: Guild %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (gd->instance_id > 0)
				return -3; // Guild already instancing
			break;
		case IM_CLAN:
			if (!(cd = clan_search(owner_id))) {
				ShowError("instance_create: Clan %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (cd->instance_id > 0)
				return -3; // Clan already instancing
			break;
		default:
			ShowError("instance_create: Unknown mode %u for owner_id %d and name %s.\n", mode, owner_id, name);
			return -2;
	}

	if (instance_count <= 0)
		return -4;

	int instance_id = instance_count++;
	std::shared_ptr<s_instance_data> entry = std::make_shared<s_instance_data>();

	entry->id = db->id;
	entry->owner_id = owner_id;
	entry->mode = mode;
	entry->regs.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
	entry->regs.arrays = nullptr;
	instances.insert({ instance_id, entry });

	switch(mode) {
		case IM_CHAR:
			sd->instance_id = instance_id;
			break;
		case IM_PARTY:
			pd->instance_id = instance_id;
			int32 i;
			ARR_FIND(0, MAX_PARTY, i, pd->party.member[i].leader);

			if (i < MAX_PARTY)
				sd = map_charid2sd(pd->party.member[i].char_id);
			break;
		case IM_GUILD:
			gd->instance_id = instance_id;
			sd = map_charid2sd(gd->member[0].char_id);
			break;
		case IM_CLAN:
			cd->instance_id = instance_id;
			break;
	}

	if (sd != nullptr)
		sd->instance_mode = mode;

	instance_wait.id.push_back(instance_id);
	clif_instance_create(instance_id, instance_wait.id.size());
	instance_subscription_timer(0,0,0,0);

	ShowInfo("[Instance] Created: %s (%d)\n", name, instance_id);

	// Start the instance timer on instance creation
	instance_startkeeptimer(entry, instance_id);

	return instance_id;
}

/**
 * Adds maps to the instance
 * @param instance_id: Instance ID to add map to
 * @return 0 on failure or map count on success
 */
int instance_addmap(int instance_id) {
	if (instance_id <= 0)
		return 0;

	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);

	// If the instance isn't idle, we can't do anything
	if (idata->state != INSTANCE_IDLE)
		return 0;

	std::shared_ptr<s_instance_db> db = instance_db.find(idata->id);

	if (!db)
		return 0;

	// Set to busy, update timers
	idata->state = INSTANCE_BUSY;
	idata->idle_limit = static_cast<unsigned int>(time(nullptr)) + db->timeout;
	idata->idle_timer = add_timer(gettick() + db->timeout * 1000, instance_delete_timer, instance_id, 0);

	int16 m;

	// Add initial map
	if ((m = map_addinstancemap(db->enter.map, instance_id)) < 0) {
		ShowError("instance_addmap: Failed to create initial map for instance '%s' (%d).\n", db->name.c_str(), instance_id);
		return 0;
	}

	struct s_instance_map entry;

	entry.m = m;
	entry.src_m = db->enter.map;
	idata->map.push_back(entry);

	// Add extra maps (if any)
	for (const auto &it : db->maplist) {
		if ((m = map_addinstancemap(it, instance_id)) < 0) { // An error occured adding a map
			ShowError("instance_addmap: No maps added to instance '%s' (%d).\n", db->name.c_str(), instance_id);
			return 0;
		} else {
			entry.m = m;
			entry.src_m = it;
			idata->map.push_back(entry);
		}
	}

	// Create NPCs on all maps
	instance_addnpc(idata);

	switch(idata->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_charid2sd(idata->owner_id)) // Inform player of the created instance
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(idata->owner_id)) // Inform party members of the created instance
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(idata->owner_id)) // Inform guild members of the created instance
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_CLAN:
			if (clan_search(idata->owner_id)) // Inform clan members of the created instance
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		default:
			return 0;
	}

	return idata->map.size();
}

/**
 * Fills outname with the name of the instance map name
 * @param map_id: Mapid to use
 * @param instance_id: Instance id
 * @param outname: Pointer to allocated memory that will be filled in
 */
void instance_generate_mapname(int map_id, int instance_id, char outname[MAP_NAME_LENGTH]) {

#if MAX_MAP_PER_SERVER > 9999
	#error This algorithm is only safe for up to 9999 maps, change at your own risk.
#endif
	// Safe up to 9999 maps per map-server
	static const int prefix_length = 4;
	// Full map name length - prefix length - seperator character - zero termination
	static const int suffix_length = MAP_NAME_LENGTH - prefix_length - 1 - 1;
	static const int prefix_limit = static_cast<int>(pow(10, prefix_length));
	static const int suffix_limit = static_cast<int>(pow(10, suffix_length));
	safesnprintf(outname, MAP_NAME_LENGTH, "%0*u#%0*u", prefix_length, map_id % prefix_limit, suffix_length, instance_id % suffix_limit);
}

/**
 * Returns an instance map ID
 * @param m: Source map ID
 * @param instance_id: Instance to search
 * @return Map ID in this instance or -1 on failure
 */
int16 instance_mapid(int16 m, int instance_id)
{
	const char *name = map_mapid2mapname(m);

	if (name == nullptr) {
		ShowError("instance_mapid: Map ID %d does not exist.\n", m);
		return -1;
	}

	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);

	if(!idata || idata->state != INSTANCE_BUSY)
		return -1;

	for (const auto &it : idata->map) {
		if (it.src_m == m) {
			char alt_name[MAP_NAME_LENGTH];
			instance_generate_mapname(m, instance_id, alt_name);
			return map_mapname2mapid(alt_name);
		}
	}

	return m;
}

/**
 * Removes an instance, all its maps, and NPCs invoked by the client button.
 * @param sd: Player data
 */
void instance_destroy_command(map_session_data *sd) {
	nullpo_retv(sd);

	std::shared_ptr<s_instance_data> idata;
	int instance_id = 0;

	if (sd->instance_mode == IM_CHAR && sd->instance_id > 0) {
		idata = util::umap_find(instances, sd->instance_id);

		if (idata == nullptr)
			return;

		instance_id = sd->instance_id;
	} else if (sd->instance_mode == IM_PARTY && sd->status.party_id > 0) {
		party_data *pd = party_search(sd->status.party_id);

		if (pd == nullptr)
			return;

		idata = util::umap_find(instances, pd->instance_id);

		if (idata == nullptr)
			return;

		int32 i;

		ARR_FIND(0, MAX_PARTY, i, pd->data[i].sd == sd && pd->party.member[i].leader);

		if (i == MAX_PARTY) // Player is not party leader
			return;

		instance_id = pd->instance_id;
	} else if (sd->instance_mode == IM_GUILD && sd->guild != nullptr && sd->guild->instance_id > 0) {
		guild *gd = guild_search(sd->status.guild_id);

		if (gd == nullptr)
			return;

		idata = util::umap_find(instances, gd->instance_id);

		if (idata == nullptr)
			return;

		if (strcmp(sd->status.name, gd->master) != 0) // Player is not guild master
			return;

		instance_id = gd->instance_id;
	}

	if (instance_id == 0) // Checks above failed
		return;

	if (!instance_db.find(idata->id)->destroyable) // Instance is flagged as non-destroyable
		return;

	instance_destroy(instance_id);

	// Check for any other active instances and display their info
	if (sd->instance_id > 0)
		instance_reqinfo(sd, sd->instance_id);
	if (sd->status.party_id > 0) {
		party_data *pd = party_search(sd->status.party_id);

		if (pd == nullptr)
			return;

		if (pd->instance_id > 0)
			instance_reqinfo(sd, pd->instance_id);
	}
	if (sd->guild != nullptr && sd->guild->instance_id > 0) {
		guild *gd = guild_search(sd->status.guild_id);

		if (gd == nullptr)
			return;

		instance_reqinfo(sd, gd->instance_id);
	}
}

/**
 * Removes an instance, all its maps, and NPCs.
 * @param instance_id: Instance to remove
 * @return True on sucess or false on failure
 */
bool instance_destroy(int instance_id)
{
	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);

	if (!idata)
		return false;

	struct map_session_data *sd;
	struct party_data *pd;
	struct guild *gd;
	struct clan *cd;
	e_instance_mode mode = idata->mode;
	e_instance_notify type = IN_NOTIFY;

	switch(mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			sd = map_charid2sd(idata->owner_id);
			break;
		case IM_PARTY:
			pd = party_search(idata->owner_id);
			break;
		case IM_GUILD:
			gd = guild_search(idata->owner_id);
			break;
		case IM_CLAN:
			cd = clan_search(idata->owner_id);
			break;
	}

	if(idata->state == INSTANCE_IDLE) {
		for (auto instance_it = instance_wait.id.begin(); instance_it != instance_wait.id.end(); ++instance_it) {
			if (*instance_it == instance_id) {
				instance_wait.id.erase(instance_it);

				for (int i = 0; i < instance_wait.id.size(); i++) {
					if (util::umap_find(instances, instance_wait.id[i])->state == INSTANCE_IDLE)
						if ((mode == IM_CHAR && sd) || (mode == IM_PARTY && pd) || (mode == IM_GUILD && gd) || (mode == IM_CLAN && cd))
							clif_instance_changewait(instance_id, i + 1);
				}

				if (!instance_wait.id.empty())
					instance_wait.timer = add_timer(gettick() + INSTANCE_INTERVAL, instance_subscription_timer, 0, 0);
				else
					instance_wait.timer = INVALID_TIMER;
				break;
			}
		}
	} else {
		unsigned int now = static_cast<unsigned int>(time(nullptr));

		if(idata->keep_limit && idata->keep_limit <= now)
			type = IN_DESTROY_LIVE_TIMEOUT;
		else if(idata->idle_limit && idata->idle_limit <= now)
			type = IN_DESTROY_ENTER_TIMEOUT;
		else
			type = IN_DESTROY_USER_REQUEST;

		// Run OnInstanceDestroy on all NPCs in the instance
		for (const auto &it : idata->map) {
			struct map_data *mapdata = map_getmapdata(it.m);

			map_foreachinallarea(instance_npcdestroy, it.m, 0, 0, mapdata->xs, mapdata->ys, BL_NPC, it.m);
			map_delinstancemap(it.m);
		}
	}

	if(idata->keep_timer != INVALID_TIMER) {
		delete_timer(idata->keep_timer, instance_delete_timer);
		idata->keep_timer = INVALID_TIMER;
	}
	if(idata->idle_timer != INVALID_TIMER) {
		delete_timer(idata->idle_timer, instance_delete_timer);
		idata->idle_timer = INVALID_TIMER;
	}

	if (mode == IM_CHAR && sd)
		sd->instance_id = 0;
	else if (mode == IM_PARTY && pd)
		pd->instance_id = 0;
	else if (mode == IM_GUILD && gd)
		gd->instance_id = 0;
	else if (mode == IM_CLAN && cd)
		cd->instance_id = 0;

	if (mode != IM_NONE) {
		if(type != IN_NOTIFY)
			clif_instance_changestatus(instance_id, type, 0);
		else
			clif_instance_changewait(instance_id, 0xffff);
	}

	if( idata->regs.vars ) {
		db_destroy(idata->regs.vars);
		idata->regs.vars = NULL;
	}

	if( idata->regs.arrays )
		idata->regs.arrays->destroy(idata->regs.arrays, script_free_array_db);

	ShowInfo("[Instance] Destroyed: %s (%d)\n", instance_db.find(idata->id)->name.c_str(), instance_id);

	instances.erase(instance_id);

	return true;
}

/**
 * Warp a user into an instance
 * @param sd: Player to warp
 * @param instance_id: Instance to warp to
 * @param name: Map name
 * @param x: X coordinate
 * @param y: Y coordinate
 * @return e_instance_enter value
 */
e_instance_enter instance_enter(struct map_session_data *sd, int instance_id, const char *name, short x, short y)
{
	nullpo_retr(IE_OTHER, sd);
	
	std::shared_ptr<s_instance_db> db = instance_search_db_name(name);

	if (!db) {
		ShowError("instance_enter: Unknown instance \"%s\".\n", name);
		return IE_OTHER;
	}

	// If one of the two coordinates was not given or is below zero, we use the entry point from the database
	if (x < 0 || y < 0) {
		x = db->enter.x;
		y = db->enter.y;
	}

	std::shared_ptr<s_instance_data> idata = nullptr;
	struct party_data *pd;
	struct guild *gd;
	struct clan *cd;
	e_instance_mode mode;

	if (instance_id <= 0) // Default party checks will be used
		mode = IM_PARTY;
	else {
		if (!(idata = util::umap_find(instances, instance_id)))
			return IE_NOINSTANCE;
		mode = idata->mode;
	}

	switch(mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (sd->instance_id == 0) // Player must have an instance
				return IE_NOINSTANCE;
			if (idata->owner_id != sd->status.char_id)
				return IE_OTHER;
			break;
		case IM_PARTY:
			if (sd->status.party_id == 0) // Character must be in instance party
				return IE_NOMEMBER;
			if (!(pd = party_search(sd->status.party_id)))
				return IE_NOMEMBER;
			if (pd->instance_id == 0 || idata == nullptr) // Party must have an instance
				return IE_NOINSTANCE;
			if (idata->owner_id != pd->party.party_id)
				return IE_OTHER;
			break;
		case IM_GUILD:
			if (sd->status.guild_id == 0) // Character must be in instance guild
				return IE_NOMEMBER;
			if (!(gd = guild_search(sd->status.guild_id)))
				return IE_NOMEMBER;
			if (gd->instance_id == 0) // Guild must have an instance
				return IE_NOINSTANCE;
			if (idata->owner_id != gd->guild_id)
				return IE_OTHER;
			break;
		case IM_CLAN:
			if (sd->status.clan_id == 0) // Character must be in instance clan
				return IE_NOMEMBER;
			if (!(cd = clan_search(sd->status.clan_id)))
				return IE_NOMEMBER;
			if (cd->instance_id == 0) // Clan must have an instance
				return IE_NOINSTANCE;
			if (idata->owner_id != cd->id)
				return IE_OTHER;
			break;
	}

	if (idata->state != INSTANCE_BUSY)
		return IE_OTHER;
	if (idata->id != db->id)
		return IE_OTHER;

	int16 m;

	// Does the instance match?
	if ((m = instance_mapid(db->enter.map, instance_id)) < 0)
		return IE_OTHER;

	if (pc_setpos(sd, map_id2index(m), x, y, CLR_OUTSIGHT))
		return IE_OTHER;

	return IE_OK;
}

/**
 * Request some info about the instance
 * @param sd: Player to display info to
 * @param instance_id: Instance to request
 * @return True on success or false on failure
 */
bool instance_reqinfo(struct map_session_data *sd, int instance_id)
{
	nullpo_retr(false, sd);

	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);

	if (!idata || !instance_db.find(idata->id))
		return false;

	// Say it's created if instance is not busy
	if(idata->state == INSTANCE_IDLE) {
		for (int i = 0; i < instance_wait.id.size(); i++) {
			if (instance_wait.id[i] == instance_id) {
				clif_instance_create(instance_id, i + 1);
				sd->instance_mode = idata->mode;
				break;
			}
		}
	} else if (idata->state == INSTANCE_BUSY) { // Give info on the instance if busy
		int map_instance_id = map_getmapdata(sd->bl.m)->instance_id;
		if (map_instance_id == 0 || map_instance_id == instance_id) {
			clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			sd->instance_mode = idata->mode;
		}
	}

	return true;
}

/**
 * Add players to the instance (for timers) -- Unused?
 * @param instance_id: Instance to add
 * @return True on success or false on failure
 */
bool instance_addusers(int instance_id)
{
	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);

	if(!idata || idata->state != INSTANCE_BUSY)
		return false;

	// Stop the idle timer if we had one
	instance_stopidletimer(idata, instance_id);

	// Start the instance keep timer
	instance_startkeeptimer(idata, instance_id);

	return true;
}

/**
 * Delete players from the instance (for timers)
 * @param instance_id: Instance to remove
 * @return True on success or false on failure
 */
bool instance_delusers(int instance_id)
{
	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);

	if(!idata || idata->state != INSTANCE_BUSY)
		return false;

	int users = 0;

	// If no one is in the instance, start the idle timer
	for (const auto &it : idata->map)
		users += max(map_getmapdata(it.m)->users,0);

	// We check the actual map.users before being updated, hence the 1
	// The instance should be empty if users are now <= 1
	if(users <= 1)
		instance_startidletimer(idata, instance_id);

	return true;
}

/**
 * Reloads the instance in runtime (reloadscript)
 */
void do_reload_instance(void)
{
	for (const auto &it : instances) {
		std::shared_ptr<s_instance_data> idata = it.second;

		if (!idata || idata->map.empty())
			continue;
		else {
			// First we load the NPCs again
			instance_addnpc(idata);

			// Create new keep timer
			std::shared_ptr<s_instance_db> db = instance_db.find(idata->id);

			if (db)
				idata->keep_limit = static_cast<unsigned int>(time(nullptr)) + db->limit;
		}
	}

	// Reset player to instance beginning
	struct s_mapiterator *iter = mapit_getallusers();
	struct map_session_data *sd;

	for (sd = (TBL_PC *)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC *)mapit_next(iter)) {
		struct map_data *mapdata = map_getmapdata(sd->bl.m);

		if (sd && mapdata->instance_id > 0) {
			struct party_data *pd;
			struct guild *gd;
			struct clan *cd;
			int instance_id;
			std::shared_ptr<s_instance_data> idata = util::umap_find(instances, map[sd->bl.m].instance_id);
			std::shared_ptr<s_instance_db> db = instance_db.find(idata->id);

			switch (idata->mode) {
				case IM_NONE:
					continue;
				case IM_CHAR:
					if (sd->instance_id != mapdata->instance_id) // Someone who is not instance owner is on instance map
						continue;
					instance_id = sd->instance_id;
					break;
				case IM_PARTY:
					if ((!(pd = party_search(sd->status.party_id)) || pd->instance_id != mapdata->instance_id)) // Someone not in party is on instance map
						continue;
					instance_id = pd->instance_id;
					break;
				case IM_GUILD:
					if (!(gd = guild_search(sd->status.guild_id)) || gd->instance_id != mapdata->instance_id) // Someone not in guild is on instance map
						continue;
					instance_id = gd->instance_id;
					break;
				case IM_CLAN:
					if (!(cd = clan_search(sd->status.clan_id)) || cd->instance_id != mapdata->instance_id) // Someone not in clan is on instance map
						continue;
					instance_id = cd->instance_id;
					break;
				default:
					ShowError("do_reload_instance: Unexpected instance mode for instance %s (id=%d, mode=%u).\n", (db) ? db->name.c_str() : "Unknown", mapdata->instance_id, (uint8)idata->mode);
					continue;
			}
			if (db && instance_enter(sd, instance_id, db->name.c_str(), -1, -1) == IE_OK) { // All good
				clif_displaymessage(sd->fd, msg_txt(sd, 515)); // Instance has been reloaded
				instance_reqinfo(sd, instance_id);
			} else // Something went wrong
				ShowError("do_reload_instance: Error setting character at instance start: character_id=%d instance=%s.\n", sd->status.char_id, db->name.c_str());
		}
	}
	mapit_free(iter);
}

/**
 * Initializes the instance database
 */
void do_init_instance(void) {
	instance_start = map_num;
	instance_db.load();
	instance_wait.timer = INVALID_TIMER;

	add_timer_func_list(instance_delete_timer,"instance_delete_timer");
	add_timer_func_list(instance_subscription_timer,"instance_subscription_timer");
}

/**
 * Finalizes the instances and instance database
 */
void do_final_instance(void) {
	for (const auto &it : instances)
		instance_destroy(it.first);
}
