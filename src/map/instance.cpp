// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "instance.hpp"

#include <stdlib.h>
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

#include "clan.hpp"
#include "clif.hpp"
#include "guild.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "pc.hpp"

#define INSTANCE_INTERVAL	60000	// Interval used to check when an instance is to be destroyed (ms)

std::unordered_map<uint16, std::shared_ptr<s_instance_data>> instances;
std::unordered_map<uint16, std::shared_ptr<s_instance_db>> instance_db;

/**
 * Searches for an active instance
 * @param instance_id: ID to lookup
 * @return shared_ptr of instances on success or nullptr on failure
 */
std::shared_ptr<s_instance_data> instance_search(uint16 instance_id)
{
	return instances[instance_id];
}

/**
 * Searches an instance by ID in the database
 * @param instance_id: ID to lookup
 * @return True if instance exists or false if it doesn't
 */
bool instance_exists(uint16 instance_id)
{
	return instance_db.find(instance_id) != instance_db.end();
}

/**
 * Searches for an instance ID in the database
 * @param instance_id: Instance to search for
 * @return shared_ptr of instance on success or nullptr on failure
 */
std::shared_ptr<s_instance_db> instance_search_db(uint16 instance_id)
{
	return instance_db[instance_id];
}

/**
 * Searches for an instance name in the database
 * @param instance_name: Instance to search for
 * @return shared_ptr of instance
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
 * @param sd: Player data to attach
 * @param target: Target display type
 */
void instance_getsd(uint16 instance_id, struct map_session_data **sd, enum send_target *target) {
	auto idata = instance_search(instance_id);

	switch(idata->mode) {
		case IM_NONE:
			(*sd) = NULL;
			(*target) = SELF;
			break;
		case IM_GUILD:
			(*sd) = guild_getavailablesd(guild_search(idata->owner_id));
			(*target) = GUILD;
			break;
		case IM_PARTY:
			(*sd) = party_getavailablesd(party_search(idata->owner_id));
			(*target) = PARTY;
			break;
		case IM_CHAR:
			(*sd) = map_charid2sd(idata->owner_id);
			(*target) = SELF;
			break;
		case IM_CLAN:
			(*sd) = clan_getavailablesd(clan_search(idata->owner_id));
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
	int ret;
	uint16 instance_id = instance_wait.id[0];
	struct map_session_data *sd = NULL;
	struct party_data *pd = NULL;
	struct guild *gd = NULL;
	struct clan *cd = NULL;
	enum e_instance_mode mode;

	if (instance_wait.id.size() == 0 || instance_id == 0)
		return 0;

	// Check that maps have been added
	ret = instance_addmap(instance_id);

	auto idata = instance_search(instance_id);

	mode = idata->mode;

	switch(mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (ret == 0 && (sd = map_charid2sd(idata->owner_id)) != NULL) // If no maps are created, tell player to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		case IM_PARTY:
			if (ret == 0 && (pd = party_search(idata->owner_id)) != NULL) // If no maps are created, tell party to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		case IM_GUILD:
			if (ret == 0 && (gd = guild_search(idata->owner_id)) != NULL) // If no maps are created, tell guild to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		case IM_CLAN:
			if (ret == 0 && (cd = clan_search(idata->owner_id)) != NULL) // If no maps are created, tell clan to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		default:
			return 0;
	}

	instance_wait.id.pop_front();

	for(uint16 i = 0; i < instance_wait.id.size(); i++) {
		if (idata->state == INSTANCE_IDLE && ((mode == IM_CHAR && sd != NULL) || (mode == IM_GUILD && gd != NULL) || (mode == IM_PARTY && pd != NULL) || (mode == IM_CLAN && cd != NULL)))
			clif_instance_changewait(instance_id, i + 1);
	}

	if (!instance_wait.id.empty())
		instance_wait.timer = add_timer(gettick()+INSTANCE_INTERVAL, instance_subscription_timer, 0, 0);
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
bool instance_startkeeptimer(std::shared_ptr<s_instance_data> idata, uint16 instance_id)
{
	if (idata == nullptr)
		return false;

	// No timer
	if(idata->keep_timer != INVALID_TIMER)
		return false;

	auto db = instance_search_db(idata->id);

	if(db == nullptr)
		return false;

	// Add timer
	idata->keep_limit = (unsigned int)time(NULL) + db->limit;
	idata->keep_timer = add_timer(gettick()+db->limit*1000, instance_delete_timer, instance_id, 0);

	switch(idata->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_charid2sd(idata->owner_id) != NULL) // Notify player of the added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(idata->owner_id) != NULL) // Notify party of the added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(idata->owner_id) != NULL) // Notify guild of the added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_CLAN:
			if (clan_search(idata->owner_id) != NULL) // Notify clan of the added instance timer
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
bool instance_startidletimer(std::shared_ptr<s_instance_data> idata, uint16 instance_id)
{
	if (idata == nullptr)
		return false;

	// No current timer
	if(idata->idle_timer != INVALID_TIMER)
		return false;

	auto db = instance_search_db(idata->id);

	if (db == nullptr)
		return false;

	// Add the timer
	idata->idle_limit = (unsigned int)time(NULL) + db->timeout;
	idata->idle_timer = add_timer(gettick() + db->timeout * 1000, instance_delete_timer, instance_id, 0);

	switch(idata->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_charid2sd(idata->owner_id) != NULL && instance_search_db(idata->id) != NULL) // Notify player of added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(idata->owner_id) != NULL && instance_search_db(idata->id) != NULL) // Notify party of added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(idata->owner_id) != NULL && instance_search_db(idata->id) != NULL) // Notify guild of added instance timer
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_CLAN:
			if (clan_search(idata->owner_id) != NULL && instance_search_db(idata->id) != NULL) // Notify clan of added instance timer
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
bool instance_stopidletimer(std::shared_ptr<s_instance_data> idata, uint16 instance_id)
{
	if (idata == nullptr)
		return false;
	
	// No timer
	if(idata->idle_timer == INVALID_TIMER)
		return false;

	// Delete the timer - Party has returned or instance is destroyed
	idata->idle_limit = 0;
	delete_timer(idata->idle_timer, instance_delete_timer);
	idata->idle_timer = INVALID_TIMER;

	switch(idata->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_charid2sd(idata->owner_id) != NULL) // Notify the player
				clif_instance_changestatus(instance_id, 0, idata->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(idata->owner_id) != NULL) // Notify the party
				clif_instance_changestatus(instance_id, 0, idata->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(idata->owner_id) != NULL) // Notify the guild
				clif_instance_changestatus(instance_id, 0, idata->idle_limit);
			break;
		case IM_CLAN:
			if (clan_search(idata->owner_id) != NULL) // Notify the clan
				clif_instance_changestatus(instance_id, 0, idata->idle_limit);
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
	nullpo_retr(0, ap);
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
	nullpo_retr(0, ap);
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
	nullpo_retr(0, ap);
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
uint16 instance_create(int owner_id, const char *name, enum e_instance_mode mode) {
	auto db = instance_search_db_name(name);
	struct map_session_data *sd = NULL;
	struct party_data *pd = NULL;
	struct guild *gd = NULL;
	struct clan* cd = NULL;
	uint16 instance_id;

	if (db == nullptr) {
		ShowError("instance_create: Unknown instance %s creation was attempted.\n", name);
		return -1;
	}

	switch(mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if ((sd = map_charid2sd(owner_id)) == NULL) {
				ShowError("instance_create: Character %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (sd->instance_id)
				return -3; // Player already instancing
			break;
		case IM_PARTY:
			if ((pd = party_search(owner_id)) == NULL) {
				ShowError("instance_create: Party %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (pd->instance_id)
				return -3; // Party already instancing
			break;
		case IM_GUILD:
			if ((gd = guild_search(owner_id)) == NULL) {
				ShowError("instance_create: Guild %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (gd->instance_id)
				return -3; // Guild already instancing
			break;
		case IM_CLAN:
			if ((cd = clan_search(owner_id)) == NULL) {
				ShowError("instance_create: Clan %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (cd->instance_id)
				return -3; // Clan already instancing
			break;
		default:
			ShowError("instance_create: Unknown mode %u for owner_id %d and name %s.\n", mode, owner_id, name);
			return -2;
	}

	if (instances.size() > MAX_INSTANCE_DATA)
		return -4;

	instance_id = static_cast<uint16>(instances.size()) + 1;
	instances[instance_id] = std::make_shared<s_instance_data>();

	auto entry = instance_search(instance_id);

	entry->id = db->id;
	entry->state = INSTANCE_IDLE;
	entry->owner_id = owner_id;
	entry->mode = mode;
	entry->keep_limit = 0;
	entry->keep_timer = INVALID_TIMER;
	entry->idle_limit = 0;
	entry->idle_timer = INVALID_TIMER;
	entry->regs.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
	entry->regs.arrays = NULL;

	switch(mode) {
		case IM_CHAR:
			sd->instance_id = instance_id;
			break;
		case IM_PARTY:
			pd->instance_id = instance_id;
			break;
		case IM_GUILD:
			gd->instance_id = instance_id;
			break;
		case IM_CLAN:
			cd->instance_id = instance_id;
			break;
	}

	instance_wait.id.push_back(instance_id);
	clif_instance_create(instance_id, instance_wait.id.size());
	instance_subscription_timer(0,0,0,0);

	ShowInfo("[Instance] Created: %s (%hu).\n", name, instance_id);

	return instance_id;
}

/**
 * Adds maps to the instance
 * @param instance_id: Instance ID to add map to
 * @return 0 on failure or map count on success
 */
int instance_addmap(uint16 instance_id) {
	if (instance_id == 0)
		return 0;

	auto &idata = instance_search(instance_id);

	// If the instance isn't idle, we can't do anything
	if (idata->state != INSTANCE_IDLE)
		return 0;

	auto db = instance_search_db(idata->id);

	if (db == nullptr)
		return 0;

	// Set to busy, update timers
	idata->state = INSTANCE_BUSY;
	idata->idle_limit = (unsigned int)time(NULL) + db->timeout;
	idata->idle_timer = add_timer(gettick() + db->timeout * 1000, instance_delete_timer, instance_id, 0);

	int16 m;

	// Add initial map
	if ((m = map_addinstancemap(db->enter.map, instance_id)) < 0) {
		ShowError("instance_addmap: Failed to create initial map for instance '%s' (%hu).\n", db->name.c_str(), instance_id);
		return 0;
	}

	struct s_instance_map entry = {};

	entry.m = m;
	entry.src_m = db->enter.map;
	idata->map.push_back(entry);

	// Add extra maps (if any)
	for (const auto &it : db->maplist) {
		if ((m = map_addinstancemap(it, instance_id)) < 0) { // An error occured adding a map
			ShowError("instance_addmap: No maps added to instance '%s' (%hu).\n", db->name.c_str(), instance_id);
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
			if (map_charid2sd(idata->owner_id) != NULL) // Inform player of the created instance
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(idata->owner_id) != NULL) // Inform party members of the created instance
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(idata->owner_id) != NULL) // Inform guild members of the created instance
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		case IM_CLAN:
			if (clan_search(idata->owner_id) != NULL) // Inform clan members of the created instance
				clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);
			break;
		default:
			return 0;
	}

	return idata->map.size();
}

/**
 * Returns an instance map ID
 * @param m: Source map ID
 * @param instance_id: Instance to search
 * @return Map ID in this instance
 */
int16 instance_mapid(int16 m, uint16 instance_id)
{
	const char *iname;

	if(m < 0) {
		ShowError("instance_mapid: Map ID %d does not exist.\n", m);
		return -1;
	}

	if(instance_id == 0 || instance_id > MAX_INSTANCE_DATA)
		return -1;

	auto idata = instance_search(instance_id);

	if(idata->state != INSTANCE_BUSY)
		return -1;

	iname = map_mapid2mapname(m);

	for (const auto &it : idata->map) {
		if (it.src_m == m) {
			char alt_name[MAP_NAME_LENGTH];

			if ((strchr(iname, '@') == NULL) && strlen(iname) > 8) {
				memmove((void*)iname, iname + (strlen(iname) - 9), strlen(iname));
				snprintf(alt_name, sizeof(alt_name), "%hu#%s", instance_id, iname);
			}
			else
				snprintf(alt_name, sizeof(alt_name), "%.3hu%s", instance_id, iname);
			return map_mapname2mapid(alt_name);
		}
	}

	return m;
}

/**
 * Removes an instance, all its maps, and NPCs.
 * @param instance_id: Instance to remove
 * @return True on sucess or false on failure
 */
bool instance_destroy(uint16 instance_id)
{
	if (instance_id == 0 || instance_id > MAX_INSTANCE_DATA)
		return false;

	auto idata = instance_search(instance_id);

	if (idata->state == INSTANCE_FREE)
		return false;

	struct map_session_data *sd = NULL;
	struct party_data *pd = NULL;
	struct guild *gd = NULL;
	struct clan *cd = NULL;
	int type = 0;
	unsigned int now = (unsigned int)time(NULL);
	enum e_instance_mode mode;

	mode = idata->mode;
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
		for (const auto &it : instance_wait.id) {
			if(it == instance_id) {
				instance_wait.id.pop_front();

				for (uint16 i = 0; i < instance_wait.id.size(); i++) {
					if (instance_search(instance_wait.id[i])->state == INSTANCE_IDLE)
						if ((mode == IM_CHAR && sd) || (mode == IM_PARTY && pd) || (mode == IM_GUILD && gd) || (mode == IM_CLAN && cd))
							clif_instance_changewait(instance_id, i + 1);
				}

				if (!instance_wait.id.empty())
					instance_wait.timer = add_timer(gettick()+INSTANCE_INTERVAL, instance_subscription_timer, 0, 0);
				else
					instance_wait.timer = INVALID_TIMER;
				type = 0;
				break;
			}
		}
	} else {
		if(idata->keep_limit && idata->keep_limit <= now)
			type = 1;
		else if(idata->idle_limit && idata->idle_limit <= now)
			type = 2;
		else
			type = 3;

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
		if(type)
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

	ShowInfo("[Instance] Destroyed %hu.\n", instance_id);

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
enum e_instance_enter instance_enter(struct map_session_data *sd, uint16 instance_id, const char *name, short x, short y)
{
	nullpo_retr(IE_OTHER, sd);
	
	auto db = instance_search_db_name(name);

	if (db == nullptr) {
		ShowError("instance_enter: Unknown instance \"%s\".\n", name);
		return IE_OTHER;
	}

	struct party_data *pd = NULL;
	struct guild *gd = NULL;
	struct clan *cd = NULL;
	enum e_instance_mode mode;
	int16 m;
	auto idata = instance_search(instance_id);

	// If one of the two coordinates was not given or is below zero, we use the entry point from the database
	if( x < 0 || y < 0 ){
		x = db->enter.x;
		y = db->enter.y;
	}

	// Check if it is a valid instance
	if (instance_id == 0)
		mode = IM_PARTY;
	else
		mode = idata->mode;

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
			if ((pd = party_search(sd->status.party_id)) == NULL)
				return IE_NOMEMBER;
			if (pd->instance_id == 0) // Party must have an instance
				return IE_NOINSTANCE;
			if (idata->owner_id != pd->party.party_id)
				return IE_OTHER;
			break;
		case IM_GUILD:
			if (sd->status.guild_id == 0) // Character must be in instance guild
				return IE_NOMEMBER;
			if ((gd = guild_search(sd->status.guild_id)) == NULL)
				return IE_NOMEMBER;
			if (gd->instance_id == 0) // Guild must have an instance
				return IE_NOINSTANCE;
			if (idata->owner_id != gd->guild_id)
				return IE_OTHER;
			break;
		case IM_CLAN:
			if (sd->status.clan_id == 0) // Character must be in instance clan
				return IE_NOMEMBER;
			if ((cd = clan_search(sd->status.clan_id)) == NULL)
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

	// Does the instance match?
	if ((m = instance_mapid(db->enter.map, instance_id)) < 0)
		return IE_OTHER;

	if (pc_setpos(sd, map_id2index(m), x, y, CLR_OUTSIGHT))
		return IE_OTHER;

	// If there was an idle timer, let's stop it
	instance_stopidletimer(idata, instance_id);

	// Now we start the instance timer
	instance_startkeeptimer(idata, instance_id);

	return IE_OK;
}

/**
 * Request some info about the instance
 * @param sd: Player to display info to
 * @param instance_id: Instance to request
 * @return True on success or false on failure
 */
bool instance_reqinfo(struct map_session_data *sd, uint16 instance_id)
{
	nullpo_retr(false, sd);

	if(instance_id == 0 || instance_id > MAX_INSTANCE_DATA)
		return false;

	auto idata = instance_search(instance_id);

	if(instance_search_db(idata->id) == nullptr)
		return false;

	// Say it's created if instance is not busy
	if(idata->state == INSTANCE_IDLE) {
		for (uint16 i = 0; i < instance_wait.id.size(); i++) {
			if (instance_wait.id[i] == instance_id) {
				clif_instance_create(instance_id, i + 1);
				break;
			}
		}
	} else if(idata->state == INSTANCE_BUSY) // Give info on the instance if busy
		clif_instance_status(instance_id, idata->keep_limit, idata->idle_limit);

	return true;
}

/**
 * Add players to the instance (for timers) -- Unused?
 * @param instance_id: Instance to add
 * @return True on success or false on failure
 */
bool instance_addusers(uint16 instance_id)
{
	if(instance_id == 0 || instance_id > MAX_INSTANCE_DATA)
		return false;

	auto idata = instance_search(instance_id);

	if(idata->state != INSTANCE_BUSY)
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
bool instance_delusers(uint16 instance_id)
{
	int users = 0;

	if(instance_id == 0 || instance_id > MAX_INSTANCE_DATA)
		return false;

	auto idata = instance_search(instance_id);

	if(idata->state != INSTANCE_BUSY)
		return false;

	// If no one is in the instance, start the idle timer
	for (const auto &it : idata->map)
		users += max(map_getmapdata(it.m)->users,0);

	// We check the actual map.users before being updated, hence the 1
	// The instance should be empty if users are now <= 1
	if(users <= 1)
		instance_startidletimer(idata, instance_id);

	return true;
}

static void yaml_invalid_warning(const char* fmt, const YAML::Node &node, const std::string &file) {
	YAML::Emitter out;
	out << node;
	ShowWarning(fmt, file.c_str());
	ShowMessage("%s\n", out.c_str());
}

/**
* Reads and parses an entry from the instance_db.
* @param node: YAML node containing the entry.
* @param n: The sequential index of the current entry.
* @param source: The source YAML file.
* @return True on successful parse or false otherwise
*/
bool instance_read_db_sub(const YAML::Node &node, int n, const std::string &source)
{
	uint16 instance_id = 0;
	std::string name;
	bool existing = false;

	if (!node["ID"]) {
		yaml_invalid_warning("instance_read_db_sub: Missing ID field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
		return false;
	}
	try {
		instance_id = node["ID"].as<uint16>();
	} catch (...) {
		yaml_invalid_warning("instance_read_db_sub: Instance definition with invalid ID field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
		return false;
	}

	if (instance_id < 1 || instance_id > UINT16_MAX) {
		ShowWarning("instance_read_db_sub: Invalid instance ID %d in \"%s\", entry #%d (min: 1, max: %d), skipping.\n", instance_id, source.c_str(), n, UINT16_MAX);
		return false;
	}

	if (instance_exists(instance_id)) {
		if (source.find("import") != std::string::npos) // Import file read-in, free previous value and store new value
			existing = true;
		else { // Normal file read-in
			ShowWarning("instance_read_db_sub: Duplicate instance %d, skipping.\n", instance_id);
			return false;
		}
	}

	if (!node["Name"]) {
		ShowWarning("instance_read_db_sub: Missing instance name for instance %d in \"%s\", skipping.\n", instance_id, source.c_str());
		return false;
	}
	try {
		name = node["Name"].as<std::string>();

		if (instance_search_db_name(name.c_str()) != nullptr) {
			ShowWarning("instance_read_db_sub: Instance name %s already exists for instance %d in \"%s\", skipping.\n", name.c_str(), instance_id, source.c_str());
			return false;
		}
	} catch (...) {
		yaml_invalid_warning("instance_read_db_sub: Instance definition with invalid name field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
		return false;
	}

	if (!existing)
		instance_db[instance_id] = std::make_shared<s_instance_db>();
	auto entry = instance_search_db(instance_id);
	entry->id = instance_id;
	entry->name = name;

	if (!node["LimitTime"]) {
		ShowWarning("instance_read_db_sub: Missing instance limit time for instance %d in \"%s\", skipping.\n", instance_id, source.c_str());
		return false;
	}
	try {
		entry->limit = node["LimitTime"].as<unsigned int>();
	} catch (...) {
		yaml_invalid_warning("instance_read_db_sub: Instance definition with invalid limit time field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
		return false;
	}

	if (!node["IdleTimeOut"]) // Default to 300 seconds
		entry->timeout = 300;
	else {
		try {
			entry->timeout = node["IdleTimeOut"].as<unsigned int>();
		} catch (...) {
			yaml_invalid_warning("instance_read_db_sub: Instance definition with invalid idle time out field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	/*
	if (!node["Destroyable"]) // Default to true
		entry->destroyable = true;
	else {
		try {
			entry->destroyable = node["Destroyable"].as<bool>();
		} catch (...) {
			yaml_invalid_warning("instance_read_db_sub: Instance definition with invalid destroyable field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}
	*/

	if (!node["EnterAt"]) {
		ShowWarning("instance_read_db_sub: Missing instance enter location for instance %d in \"%s\", skipping.\n", instance_id, source.c_str());
		return false;
	}
	try {
		const YAML::Node enter_list = node["EnterAt"];
		std::string mapname;
		int16 m, x = -1, y = -1;

		mapname.clear();

		if (enter_list["Map"]) {
			try {
				mapname = enter_list["Map"].as<std::string>();
			} catch (...) {
				yaml_invalid_warning("instance_read_db_sub: Instance definition with invalid enter location map field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
				return false;
			}
		}
		if (enter_list["X"]) {
			try {
				x = enter_list["X"].as<int16>();
			} catch (...) {
				yaml_invalid_warning("instance_read_db_sub: Instance definition with invalid enter location x field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
				return false;
			}
		}
		if (enter_list["Y"]) {
			try {
				y = enter_list["Y"].as<int16>();
			} catch (...) {
				yaml_invalid_warning("instance_read_db_sub: Instance definition with invalid enter location y field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
				return false;
			}
		}

		if (mapname.size() == 0) {
			ShowWarning("instance_read_db_sub: Missing instance enter map location for instance %d in \"%s\", skipping.\n", instance_id, source.c_str());
			return false;
		}
		m = map_mapname2mapid(mapname.c_str());
		if (!m) {
			ShowWarning("instance_read_db_sub: Invalid enter map for instance %d in \"%s\", skipping.\n", instance_id, source.c_str());
			return false;
		}
		if (x == -1) {
			ShowWarning("instance_read_db_sub: Missing instance enter x location for instance %d in \"%s\", skipping.\n", instance_id, source.c_str());
			return false;
		}
		if (y == -1) {
			ShowWarning("instance_read_db_sub: Missing instance enter y location for instance %d in \"%s\", skipping.\n", instance_id, source.c_str());
			return false;
		}

		entry->enter.map = m;
		entry->enter.x = x;
		entry->enter.y = y;
	} catch(...) {
		yaml_invalid_warning("instance_read_db_sub: Instance definition with invalid enter location field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
		return false;
	}

	if (node["AdditionalMaps"]) {
		try {
			const YAML::Node map_list = node["AdditionalMaps"];

			if (map_list.IsSequence()) {
				for (uint16 i = 0; i < map_list.size(); i++) {
					std::string mapname;
					int16 m;

					mapname = map_list[i].as<std::string>();
					m = map_mapname2mapid(mapname.c_str());

					if (!m) {
						ShowWarning("instance_read_db_sub: Invalid additional map for instance %d in \"%s\", skipping.\n", instance_id, source.c_str());
						return false;
					}
					entry->maplist.push_back(m);
				}
			} else
				ShowWarning("instance_read_db_sub: Invalid additional map format for instance %d in \"%s\".\n", instance_id, source.c_str());
		} catch (...) {
			yaml_invalid_warning("instance_read_db_sub: Instance definition with invalid additional map field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	return true;
}

/**
 * Loads instances from the instance database
 */
void instance_readdb(void) {
	std::vector<std::string> directories = { std::string(db_path) + "/" + std::string(DBPATH),  std::string(db_path) + "/" + std::string(DBIMPORT) + "/" };
	static const std::string file_name("instance_db.yml");

	for (const auto &directory : directories) {
		std::string current_file = directory + file_name;
		YAML::Node config;
		int count = 0;

		try {
			config = YAML::LoadFile(current_file);
		} catch (...) {
			ShowError("Cannot read '" CL_WHITE "%s" CL_RESET "'.\n", current_file.c_str());
			return;
		}

		for (const auto &node : config["Instances"]) {
			if (node.IsDefined() && instance_read_db_sub(node, count, current_file))
				count++;
		}

		ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'\n", count, current_file.c_str());

	}

	return;
}

/**
 * Reload the instance database
 */
void instance_db_reload(void) {
	instance_db.clear();
	instance_readdb();
}

/**
 * Reloads the instance in runtime (reloadscript)
 */
void do_reload_instance(void)
{
	struct s_mapiterator* iter;
	struct map_session_data *sd;

	for (const auto &it : instances) {
		auto idata = instance_search(it.first);

		if (idata->map.empty())
			continue;
		else {
			// First we load the NPCs again
			instance_addnpc(idata);

			// Create new keep timer
			auto db = instance_search_db(idata->id);

			if (db != nullptr)
				idata->keep_limit = (unsigned int)time(NULL) + db->limit;
		}
	}

	// Reset player to instance beginning
	iter = mapit_getallusers();
	for (sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter)) {
		struct map_data *mapdata = map_getmapdata(sd->bl.m);

		if (sd && mapdata->instance_id) {
			struct party_data *pd = NULL;
			struct guild *gd = NULL;
			struct clan *cd = NULL;
			uint16 instance_id;

			auto idata = instance_search(map[sd->bl.m].instance_id);
			auto db = instance_search_db(idata->id);

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
				ShowError("do_reload_instance: Unexpected instance mode for instance %s (id=%u, mode=%u).\n", (db) ? db->name.c_str() : "Unknown", mapdata->instance_id, (uint8)idata->mode);
				continue;
			}
			if (db != nullptr && !instance_enter(sd, instance_id, db->name.c_str(), -1, -1)) { // All good
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
	instance_readdb();
	instance_wait.timer = -1;

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
