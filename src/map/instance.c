// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/db.h"
#include "../common/malloc.h"

#include "clif.h"
#include "guild.h"
#include "instance.h"
#include "map.h"
#include "npc.h"
#include "party.h"
#include "pc.h"

#include <stdlib.h>

#define INSTANCE_INTERVAL	60000	// Interval used to check when an instance is to be destroyed (ms)

int instance_start = 0; // To keep the last index + 1 of normal map inserted in the map[ARRAY]

struct instance_data instance_data[MAX_INSTANCE_DATA];
struct eri *instance_maps_ers = NULL; ///< Array of maps per instance

static DBMap *InstanceDB; /// Instance DB: struct instance_db, key: id
static DBMap *InstanceNameDB; /// instance id, key: name

static struct {
	int id[MAX_INSTANCE_DATA];
	int count;
	int timer;
} instance_wait;

/*==========================================
 * Searches for an instance ID in the database
 *------------------------------------------*/
struct instance_db *instance_searchtype_db(unsigned short instance_id) {
	return (struct instance_db *)uidb_get(InstanceDB,instance_id);
}

static uint16 instance_name2id(const char *instance_name) {
	return (uint16)strdb_uiget(InstanceNameDB,instance_name);
}

/*==========================================
 * Searches for an instance name in the database
 *------------------------------------------*/
static struct instance_db *instance_searchname_db(const char *instance_name) {
	uint16 id = instance_name2id(instance_name);
	if (id == 0)
		return NULL;
	return (struct instance_db *)uidb_get(InstanceDB,id);
}

/**
 * Search for a sd of an Instance
 * @param instance_id: Instance ID
 * @param sd: Player data to attach
 * @param target: Target display type
 */
void instance_getsd(unsigned short instance_id, struct map_session_data **sd, enum send_target *target) {
	switch(instance_data[instance_id].mode) {
		case IM_NONE:
			(*sd) = NULL;
			(*target) = SELF;
			break;
		case IM_GUILD:
			(*sd) = guild_getavailablesd(guild_search(instance_data[instance_id].owner_id));
			(*target) = GUILD;
			break;
		case IM_PARTY:
			(*sd) = party_getavailablesd(party_search(instance_data[instance_id].owner_id));
			(*target) = PARTY;
			break;
		case IM_CHAR:
			(*sd) = map_id2sd(instance_data[instance_id].owner_id);
			(*target) = SELF;
			break;
	}
	return;
}

/*==========================================
 * Deletes an instance timer (Destroys instance)
 *------------------------------------------*/
static int instance_delete_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	instance_destroy(id);

	return 0;
}

/*==========================================
 * Create subscription timer
 *------------------------------------------*/
static int instance_subscription_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	int i, ret;
	unsigned short instance_id = instance_wait.id[0];
	struct map_session_data *sd = NULL;
	struct party_data *p = NULL;
	struct guild *g = NULL;
	enum instance_mode mode;

	if(instance_wait.count == 0 || instance_id == 0)
		return 0;

	// Check that maps have been added
	ret = instance_addmap(instance_id);
	mode = instance_data[instance_id].mode;

	switch(mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (ret == 0 && (sd = map_id2sd(instance_data[instance_id].owner_id)) != NULL) // If no maps are created, tell player to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		case IM_PARTY:
			if (ret == 0 && (p = party_search(instance_data[instance_id].owner_id)) != NULL) // If no maps are created, tell party to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		case IM_GUILD:
			if (ret == 0 && (g = guild_search(instance_data[instance_id].owner_id)) != NULL) // If no maps are created, tell guild to wait
				clif_instance_changewait(instance_id, 0xffff);
			break;
		default:
			return 0;
	}

	instance_wait.count--;
	memmove(&instance_wait.id[0],&instance_wait.id[1],sizeof(instance_wait.id[0])*instance_wait.count);
	memset(&instance_wait.id[instance_wait.count], 0, sizeof(instance_wait.id[0]));

	for(i = 0; i < instance_wait.count; i++) {
		if(	instance_data[instance_wait.id[i]].state == INSTANCE_IDLE &&
			((mode == IM_CHAR && sd != NULL) || (mode == IM_GUILD && g != NULL) || (mode == IM_PARTY && p != NULL))
		){
			clif_instance_changewait(instance_id, i + 1);
		}
	}

	if(instance_wait.count)
		instance_wait.timer = add_timer(gettick()+INSTANCE_INTERVAL, instance_subscription_timer, 0, 0);
	else
		instance_wait.timer = INVALID_TIMER;

	return 0;
}

/*==========================================
 * Adds timer back to members entering instance
 *------------------------------------------*/
static int instance_startkeeptimer(struct instance_data *im, unsigned short instance_id)
{
	struct instance_db *db;

	nullpo_retr(0, im);

	// No timer
	if(im->keep_timer != INVALID_TIMER)
		return 1;

	if((db = instance_searchtype_db(im->type)) == NULL)
		return 1;

	// Add timer
	im->keep_limit = (unsigned int)time(NULL) + db->limit;
	im->keep_timer = add_timer(gettick()+db->limit*1000, instance_delete_timer, instance_id, 0);

	switch(im->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_id2sd(im->owner_id) != NULL) // Notify player of the added instance timer
				clif_instance_status(instance_id, im->keep_limit, im->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(im->owner_id) != NULL) // Notify party of the added instance timer
				clif_instance_status(instance_id, im->keep_limit, im->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(im->owner_id) != NULL) // Notify guild of the added instance timer
				clif_instance_status(instance_id, im->keep_limit, im->idle_limit);
			break;
		default:
			return 1;
	}

	return 0;
}

/*==========================================
 * Creates idle timer
 * Default before instance destroy is 5 minutes
 *------------------------------------------*/
static int instance_startidletimer(struct instance_data *im, unsigned short instance_id)
{
	struct instance_db *db;

	nullpo_retr(1, im);

	// No current timer
	if(im->idle_timer != INVALID_TIMER)
		return 1;

	if ((db = instance_searchtype_db(im->type)) == NULL)
		return 1;

	// Add the timer
	im->idle_limit = (unsigned int)time(NULL) + db->timeout;
	im->idle_timer = add_timer(gettick() + db->timeout * 1000, instance_delete_timer, instance_id, 0);

	switch(im->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_id2sd(im->owner_id) != NULL && instance_searchtype_db(im->type) != NULL) // Notify player of added instance timer
				clif_instance_status(instance_id, im->keep_limit, im->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(im->owner_id) != NULL && instance_searchtype_db(im->type) != NULL) // Notify party of added instance timer
				clif_instance_status(instance_id, im->keep_limit, im->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(im->owner_id) != NULL && instance_searchtype_db(im->type) != NULL) // Notify guild of added instance timer
				clif_instance_status(instance_id, im->keep_limit, im->idle_limit);
			break;
		default:
			return 1;
	}

	return 0;
}

/*==========================================
 * Delete the idle timer
 *------------------------------------------*/
static int instance_stopidletimer(struct instance_data *im, unsigned short instance_id)
{
	nullpo_retr(0, im);
	
	// No timer
	if(im->idle_timer == INVALID_TIMER)
		return 1;

	// Delete the timer - Party has returned or instance is destroyed
	im->idle_limit = 0;
	delete_timer(im->idle_timer, instance_delete_timer);
	im->idle_timer = INVALID_TIMER;

	switch(im->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_id2sd(im->owner_id) != NULL) // Notify the player
				clif_instance_changestatus(instance_id, 0, im->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(im->owner_id) != NULL) // Notify the party
				clif_instance_changestatus(instance_id, 0, im->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(im->owner_id) != NULL) // Notify the guild
				clif_instance_changestatus(instance_id, 0, im->idle_limit);
			break;
		default:
			return 1;
	}

	return 0;
}

/*==========================================
 * Run the OnInstanceInit events for duplicated NPCs
 *------------------------------------------*/
static int instance_npcinit(struct block_list *bl, va_list ap)
{
	struct npc_data* nd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, nd = (struct npc_data *)bl);

	return npc_instanceinit(nd);
}

/*==========================================
 * Add an NPC to an instance
 *------------------------------------------*/
static int instance_addnpc_sub(struct block_list *bl, va_list ap)
{
	struct npc_data* nd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, nd = (struct npc_data *)bl);

	return npc_duplicate4instance(nd, va_arg(ap, int));
}

// Separate function used for reloading
void instance_addnpc(struct instance_data *im)
{
	int i;

	// First add the NPCs
	for(i = 0; i < im->cnt_map; i++)
		map_foreachinarea(instance_addnpc_sub, im->map[i]->src_m, 0, 0, map[im->map[i]->src_m].xs, map[im->map[i]->src_m].ys, BL_NPC, im->map[i]->m);

	// Now run their OnInstanceInit
	for(i = 0; i < im->cnt_map; i++)
		map_foreachinarea(instance_npcinit, im->map[i]->m, 0, 0, map[im->map[i]->m].xs, map[im->map[i]->m].ys, BL_NPC, im->map[i]->m);

}

/*--------------------------------------
 * name : instance name
 * Return value could be
 * -4 = no free instances | -3 = already exists | -2 = character/party/guild not found | -1 = invalid type
 * On success return instance_id
 *--------------------------------------*/
int instance_create(int owner_id, const char *name, enum instance_mode mode) {
	struct instance_db *db = instance_searchname_db(name);
	struct map_session_data *sd = NULL;
	struct party_data *p = NULL;
	struct guild *g = NULL;
	unsigned short i;

	nullpo_retr(-1, db);

	switch(mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if ((sd = map_id2sd(owner_id)) == NULL) {
				ShowError("instance_create: character %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (sd->instance_id)
				return -3; // Player already instancing
			break;
		case IM_PARTY:
			if ((p = party_search(owner_id)) == NULL) {
				ShowError("instance_create: party %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (p->instance_id)
				return -3; // Party already instancing
			break;
		case IM_GUILD:
			if ((g = guild_search(owner_id)) == NULL) {
				ShowError("instance_create: guild %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			if (g->instance_id)
				return -3; // Guild already instancing
			break;
		default:
			ShowError("instance_create: unknown mode %u for owner_id %d and name %s.\n", mode, owner_id, name);
			return -2;
	}

	// Searching a Free Instance
	// 0 is ignored as this means "no instance" on maps
	ARR_FIND(1, MAX_INSTANCE_DATA, i, instance_data[i].state == INSTANCE_FREE);
	if( i >= MAX_INSTANCE_DATA )
		return -4;

	instance_data[i].type = db->id;
	instance_data[i].state = INSTANCE_IDLE;
	instance_data[i].owner_id = owner_id;
	instance_data[i].mode = mode;
	instance_data[i].keep_limit = 0;
	instance_data[i].keep_timer = INVALID_TIMER;
	instance_data[i].idle_limit = 0;
	instance_data[i].idle_timer = INVALID_TIMER;
	instance_data[i].regs.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
	instance_data[i].regs.arrays = NULL;
	instance_data[i].cnt_map = 0;

	switch(mode) {
		case IM_CHAR:
			sd->instance_id = i;
			break;
		case IM_PARTY:
			p->instance_id = i;
			break;
		case IM_GUILD:
			g->instance_id = i;
			break;
	}

	instance_wait.id[instance_wait.count++] = i;

	clif_instance_create(i, instance_wait.count);

	instance_subscription_timer(0,0,0,0);

	ShowInfo("[Instance] Created: %s (%hu).\n", name, i);

	return i;
}

/*--------------------------------------
 * Adds maps to the instance
 *--------------------------------------*/
int instance_addmap(unsigned short instance_id) {
	int i, m;
	struct instance_data *im;
	struct instance_db *db;
	struct s_instance_map *entry;

	if (instance_id == 0)
		return 0;

	im = &instance_data[instance_id];

	// If the instance isn't idle, we can't do anything
	if (im->state != INSTANCE_IDLE)
		return 0;

	if ((db = instance_searchtype_db(im->type)) == NULL)
		return 0;

	// Set to busy, update timers
	im->state = INSTANCE_BUSY;
	im->idle_limit = (unsigned int)time(NULL) + db->timeout;
	im->idle_timer = add_timer(gettick() + db->timeout * 1000, instance_delete_timer, instance_id, 0);

	// Add the maps
	if (db->maplist_count > MAX_MAP_PER_INSTANCE) {
		ShowError("instance_addmap: Too many maps (%d) created for a single instance '%s' (%hu).\n", db->maplist_count, StringBuf_Value(db->name), instance_id);
		return 0;
	}

	// Add initial map
	if ((m = map_addinstancemap(StringBuf_Value(db->enter.mapname), instance_id)) < 0) {
		ShowError("instance_addmap: Failed to create initial map for instance '%s' (%hu).\n", StringBuf_Value(db->name), instance_id);
		return 0;
	}
	entry = ers_alloc(instance_maps_ers, struct s_instance_map);
	entry->m = m;
	entry->src_m = map_mapname2mapid(StringBuf_Value(db->enter.mapname));
	RECREATE(im->map, struct s_instance_map *, im->cnt_map + 1);
	im->map[im->cnt_map++] = entry;

	// Add extra maps (if any)
	for(i = 0; i < db->maplist_count; i++) {
		if(strlen(StringBuf_Value(db->maplist[i])) < 1)
			continue;
		else if( (m = map_addinstancemap(StringBuf_Value(db->maplist[i]), instance_id)) < 0) {
			// An error occured adding a map
			ShowError("instance_addmap: No maps added to instance '%s' (%hu).\n", StringBuf_Value(db->name), instance_id);
			return 0;
		} else {
			entry = ers_alloc(instance_maps_ers, struct s_instance_map);
			entry->m = m;
			entry->src_m = map_mapname2mapid(StringBuf_Value(db->maplist[i]));
			RECREATE(im->map, struct s_instance_map *, im->cnt_map + 1);
			im->map[im->cnt_map++] = entry;
		}
	}

	// Create NPCs on all maps
	instance_addnpc(im);

	switch(im->mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (map_id2sd(im->owner_id) != NULL) // Inform player of the created instance
				clif_instance_status(instance_id, im->keep_limit, im->idle_limit);
			break;
		case IM_PARTY:
			if (party_search(im->owner_id) != NULL) // Inform party members of the created instance
				clif_instance_status(instance_id, im->keep_limit, im->idle_limit);
			break;
		case IM_GUILD:
			if (guild_search(im->owner_id) != NULL) // Inform guild members of the created instance
				clif_instance_status(instance_id, im->keep_limit, im->idle_limit);
			break;
		default:
			return 0;
	}

	return im->cnt_map;
}


/*==========================================
 * Returns an instance map ID using a map name
 * name : source map
 * instance_id : where to search
 * result : mapid of map "name" in this instance
 *------------------------------------------*/
int16 instance_mapname2mapid(const char *name, unsigned short instance_id)
{
	struct instance_data *im;
	int16 m = map_mapname2mapid(name);
	char iname[MAP_NAME_LENGTH];
	int i;

	if(m < 0) {
		ShowError("instance_mapname2mapid: map name %s does not exist.\n",name);
		return m;
	}

	strcpy(iname,name);

	if(instance_id == 0 || instance_id > MAX_INSTANCE_DATA)
		return m;

	im = &instance_data[instance_id];
	if(im->state != INSTANCE_BUSY)
		return m;

	for(i = 0; i < im->cnt_map; i++)
		if(im->map[i]->src_m == m) {
			char alt_name[MAP_NAME_LENGTH];
			if((strchr(iname,'@') == NULL) && strlen(iname) > 8) {
				memmove(iname, iname+(strlen(iname)-9), strlen(iname));
				snprintf(alt_name, sizeof(alt_name),"%hu#%s", instance_id, iname);
			} else
				snprintf(alt_name, sizeof(alt_name),"%.3hu%s", instance_id, iname);
			return map_mapname2mapid(alt_name);
		}

	return m;
}

/*==========================================
 * Removes a instance, all its maps and npcs.
 *------------------------------------------*/
int instance_destroy(unsigned short instance_id)
{
	struct instance_data *im;
	struct map_session_data *sd = NULL;
	struct party_data *p = NULL;
	struct guild *g = NULL;
	int i, type = 0;
	unsigned int now = (unsigned int)time(NULL);
	enum instance_mode mode;

	if(instance_id == 0 || instance_id > MAX_INSTANCE_DATA)
		return 1;

	im = &instance_data[instance_id];

	if(im->state == INSTANCE_FREE)
		return 1;

	mode = im->mode;
	switch(mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			sd = map_id2sd(im->owner_id);
			break;
		case IM_PARTY:
			p = party_search(im->owner_id);
			break;
		case IM_GUILD:
			g = guild_search(im->owner_id);
			break;
	}

	if(im->state == INSTANCE_IDLE) {
		for(i = 0; i < instance_wait.count; i++) {
			if(instance_wait.id[i] == instance_id) {
				instance_wait.count--;
				memmove(&instance_wait.id[i],&instance_wait.id[i+1],sizeof(instance_wait.id[0])*(instance_wait.count-i));
				memset(&instance_wait.id[instance_wait.count], 0, sizeof(instance_wait.id[0]));

				for(i = 0; i < instance_wait.count; i++)
					if(instance_data[instance_wait.id[i]].state == INSTANCE_IDLE)
						if ((mode == IM_CHAR && sd) || (mode == IM_PARTY && p) || (mode == IM_GUILD && g))
							clif_instance_changewait(instance_id, i + 1);

				if(instance_wait.count)
					instance_wait.timer = add_timer(gettick()+INSTANCE_INTERVAL, instance_subscription_timer, 0, 0);
				else
					instance_wait.timer = INVALID_TIMER;
				type = 0;
				break;
			}
		}
	} else {
		if(im->keep_limit && im->keep_limit <= now)
			type = 1;
		else if(im->idle_limit && im->idle_limit <= now)
			type = 2;
		else
			type = 3;

		for(i = 0; i < im->cnt_map; i++) {
			map_delinstancemap(im->map[i]->m);
			ers_free(instance_maps_ers, im->map[i]);
		}
	}

	if(im->keep_timer != INVALID_TIMER) {
		delete_timer(im->keep_timer, instance_delete_timer);
		im->keep_timer = INVALID_TIMER;
	}
	if(im->idle_timer != INVALID_TIMER) {
		delete_timer(im->idle_timer, instance_delete_timer);
		im->idle_timer = INVALID_TIMER;
	}

	if (mode == IM_CHAR && sd)
		sd->instance_id = 0;
	else if (mode == IM_PARTY && p)
		p->instance_id = 0;
	else if (mode == IM_GUILD && g)
		g->instance_id = 0;

	if (mode != IM_NONE) {
		if(type)
			clif_instance_changestatus(instance_id, type, 0);
		else
			clif_instance_changewait(instance_id, 0xffff);
	}

	if( im->regs.vars ) {
		db_destroy(im->regs.vars);
		im->regs.vars = NULL;
	}

	if( im->regs.arrays )
		instance_data[instance_id].regs.arrays->destroy(instance_data[instance_id].regs.arrays, script_free_array_db);

	ShowInfo("[Instance] Destroyed %hu.\n", instance_id);

	memset(&instance_data[instance_id], 0, sizeof(instance_data[instance_id]));

	return 0;
}

/*==========================================
 * Allows a user to enter an instance
 *------------------------------------------*/
int instance_enter(struct map_session_data *sd, unsigned short instance_id, const char *name)
{
	struct instance_db *db = instance_searchname_db(name);

	nullpo_retr(-1, sd);

	if (db == NULL)
		return 2;

	return instance_enter_position(sd, instance_id, name, db->enter.x, db->enter.y);
}

/*==========================================
 * Warp a user into instance
 *------------------------------------------*/
int instance_enter_position(struct map_session_data *sd, unsigned short instance_id, const char *name, short x, short y)
{
	struct instance_data *im = &instance_data[instance_id];
	struct instance_db *db = instance_searchname_db(name);
	struct party_data *p = NULL;
	struct guild *g = NULL;
	int16 m;

	nullpo_retr(-1, sd);
	nullpo_retr(3, db);

	switch(instance_data[instance_id].mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (sd->instance_id == 0) // Player must have an instance
				return 2;
			if (im->owner_id != sd->status.account_id)
				return 3;
			break;
		case IM_PARTY:
			if (sd->status.party_id == 0) // Character must be in instance party
				return 1;
			if ((p = party_search(sd->status.party_id)) == NULL)
				return 1;
			if (p->instance_id == 0) // Party must have an instance
				return 2;
			if (im->owner_id != p->party.party_id)
				return 3;
			break;
		case IM_GUILD:
			if (sd->status.guild_id == 0) // Character must be in instance guild
				return 1;
			if ((g = guild_search(sd->status.guild_id)) == NULL)
				return 1;
			if (g->instance_id == 0) // Guild must have an instance
				return 2;
			if (im->owner_id != g->guild_id)
				return 3;
			break;
	}

	if (im->state != INSTANCE_BUSY)
		return 3;
	if (im->type != db->id)
		return 3;

	// Does the instance match?
	if ((m = instance_mapname2mapid(StringBuf_Value(db->enter.mapname), instance_id)) < 0)
		return 3;

	if (pc_setpos(sd, map_id2index(m), x, y, CLR_OUTSIGHT))
		return 3;

	// If there was an idle timer, let's stop it
	instance_stopidletimer(im, instance_id);

	// Now we start the instance timer
	instance_startkeeptimer(im, instance_id);

	return 0;
}

/*==========================================
 * Request some info about the instance
 *------------------------------------------*/
int instance_reqinfo(struct map_session_data *sd, unsigned short instance_id)
{
	struct instance_data *im;

	nullpo_retr(1, sd);

	if(instance_id == 0 || instance_id > MAX_INSTANCE_DATA)
		return 1;

	im = &instance_data[instance_id];

	if(instance_searchtype_db(im->type) == NULL)
		return 1;

	// Say it's created if instance is not busy
	if(im->state == INSTANCE_IDLE) {
		int i;

		for(i = 0; i < instance_wait.count; i++) {
			if(instance_wait.id[i] == instance_id) {
				clif_instance_create(instance_id, i + 1);
				break;
			}
		}
	} else if(im->state == INSTANCE_BUSY) // Give info on the instance if busy
		clif_instance_status(instance_id, im->keep_limit, im->idle_limit);

	return 0;
}

/*==========================================
 * Add players to the instance (for timers)
 *------------------------------------------*/
int instance_addusers(unsigned short instance_id)
{
	struct instance_data *im;

	if(instance_id == 0 || instance_id > MAX_INSTANCE_DATA)
		return 1;

	im = &instance_data[instance_id];
	if(im->state != INSTANCE_BUSY)
		return 1;

	// Stop the idle timer if we had one
	instance_stopidletimer(im, instance_id);

	// Start the instance keep timer
	instance_startkeeptimer(im, instance_id);

	return 0;
}

/*==========================================
 * Delete players from the instance (for timers)
 *------------------------------------------*/
int instance_delusers(unsigned short instance_id)
{
	struct instance_data *im;
	int i, idle = 0;

	if(instance_id == 0 || instance_id > MAX_INSTANCE_DATA)
		return 1;

	im = &instance_data[instance_id];
	if(im->state != INSTANCE_BUSY)
		return 1;

	// If no one is in the instance, start the idle timer
	for(i = 0; im->map[i]->m && i > im->cnt_map; i++)
		if(map[im->map[i]->m].users > 1) // We check this before the actual map.users are updated, hence the 1
			idle++;

	if(!idle) // If idle wasn't added to, we know no one was in the instance
		instance_startidletimer(im, instance_id);

	return 0;
}

static bool instance_db_free_sub(struct instance_db *db);

/*==========================================
 * Read the instance_db.txt file
 *------------------------------------------*/
static bool instance_readdb_sub(char* str[], int columns, int current)
{
	uint8 i;
	int id = atoi(str[0]);
	struct instance_db *db;
	bool isNew = false;

	if (!id || id >  USHRT_MAX) {
		ShowError("instance_readdb_sub: Cannot add instance with ID '%d'. Valid ID is 1 ~ %d.\n", id, USHRT_MAX);
		return false;
	}

	if (mapindex_name2id(str[4]) == 0) {
		ShowError("instance_readdb_sub: Invalid map '%s' as entrance map.\n", str[4]);
		return false;
	}

	if (!(db = (struct instance_db *)uidb_get(InstanceDB, id))) {
		CREATE(db, struct instance_db, 1);
		db->id = id;
		db->name = StringBuf_Malloc();
		db->enter.mapname = StringBuf_Malloc();
		isNew = true;
	}
	else {
		StringBuf_Clear(db->name);
		StringBuf_Clear(db->enter.mapname);
		if (db->maplist_count) {
			for (i = 0; i < db->maplist_count; i++)
				StringBuf_Free(db->maplist[i]);
			aFree(db->maplist);
			db->maplist = NULL;
		}
	}

	StringBuf_AppendStr(db->name, str[1]);
	db->limit = atoi(str[2]);
	db->timeout = atoi(str[3]);
	StringBuf_AppendStr(db->enter.mapname, str[4]);
	db->enter.x = atoi(str[5]);
	db->enter.y = atoi(str[6]);

	//Instance maps
	for (i = 7; i < columns; i++) {
		if (strlen(str[i])) {
			if (mapindex_name2id(str[i]) == 0) {
				ShowWarning("instance_readdb_sub: Invalid map '%s' in maplist, skipping...\n", str[i]);
				continue;
			}
			RECREATE(db->maplist, StringBuf *, db->maplist_count+1);
			db->maplist[db->maplist_count] = StringBuf_Malloc();
			StringBuf_AppendStr(db->maplist[db->maplist_count], str[i]);
			db->maplist_count++;
		}
	}

	if (isNew) {
		uidb_put(InstanceDB, id, db);
		strdb_uiput(InstanceNameDB, StringBuf_Value(db->name), id);
	}
	return true;
}

/**
 * Free InstanceDB single entry
 * @param db Instance Db entry
 **/
static bool instance_db_free_sub(struct instance_db *db) {
	if (!db)
		return 1;
	StringBuf_Free(db->name);
	StringBuf_Free(db->enter.mapname);
	if (db->maplist_count) {
		uint8 i;
		for (i = 0; i < db->maplist_count; i++)
			StringBuf_Free(db->maplist[i]);
		aFree(db->maplist);
	}
	aFree(db);
	return 0;
}

/**
 * Free InstanceDB entries
 **/
static int instance_db_free(DBKey key, DBData *data, va_list ap) {
	struct instance_db *db = (struct instance_db *)db_data2ptr(data);
	return instance_db_free_sub(db);
}

/**
 * Read instance_db.txt files
 **/
void instance_readdb(void) {
	const char* filename[] = { DBPATH"instance_db.txt", "import/instance_db.txt"};
	int f;

	for (f = 0; f<ARRAYLENGTH(filename); f++) {
		sv_readdb(db_path, filename[f], ',', 7, 7+MAX_MAP_PER_INSTANCE, -1, &instance_readdb_sub, f);
	}
}

/**
 * Reload Instance DB
 **/
void instance_reload(void) {
	InstanceDB->clear(InstanceDB, instance_db_free);
	db_clear(InstanceNameDB);
	instance_readdb();
}

/*==========================================
 * Reloads the instance in runtime (reloadscript)
 *------------------------------------------*/
void do_reload_instance(void)
{
	struct instance_data *im;
	struct instance_db *db;
	struct s_mapiterator* iter;
	struct map_session_data *sd;
	unsigned short i;

	for( i = 1; i < MAX_INSTANCE_DATA; i++ ) {
		im = &instance_data[i];
		if(!im->cnt_map)
			continue;
		else {
			// First we load the NPCs again
			instance_addnpc(im);

			// Create new keep timer
			if((db = instance_searchtype_db(im->type)) != NULL)
				im->keep_limit = (unsigned int)time(NULL) + db->limit;
		}
	}

	// Reset player to instance beginning
	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
		if(sd && map[sd->bl.m].instance_id) {
			struct party_data *p = NULL;
			struct guild *g = NULL;

			if (instance_data[map[sd->bl.m].instance_id].mode == IM_PARTY && (!(p = party_search(sd->status.party_id)) || p->instance_id != map[sd->bl.m].instance_id)) // Someone not in party is on instance map
				continue;
			if (instance_data[map[sd->bl.m].instance_id].mode == IM_GUILD && (!(g = guild_search(sd->status.guild_id)) || g->instance_id != map[sd->bl.m].instance_id)) // Someone not in guild is on instance map
				continue;
			im = &instance_data[p->instance_id];
			if((db = instance_searchtype_db(im->type)) != NULL && !instance_enter(sd, i, StringBuf_Value(db->name))) { // All good
				clif_displaymessage(sd->fd, msg_txt(sd,515)); // Instance has been reloaded
				instance_reqinfo(sd,p->instance_id);
			} else // Something went wrong
				ShowError("do_reload_instance: Error setting character at instance start: character_id=%d instance=%s.\n",sd->status.char_id,StringBuf_Value(db->name));
		}
	mapit_free(iter);
}

void do_init_instance(void) {
	InstanceDB = uidb_alloc(DB_OPT_BASE);
	InstanceNameDB = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA,0);

	instance_readdb();
	memset(instance_data, 0, sizeof(instance_data));
	memset(&instance_wait, 0, sizeof(instance_wait));
	instance_wait.timer = -1;

	instance_maps_ers = ers_new(sizeof(struct s_instance_map),"instance.c::instance_maps_ers", ERS_OPT_NONE);

	add_timer_func_list(instance_delete_timer,"instance_delete_timer");
	add_timer_func_list(instance_subscription_timer,"instance_subscription_timer");
}

void do_final_instance(void) {
	int i;

	ers_destroy(instance_maps_ers);
	for( i = 1; i < MAX_INSTANCE_DATA; i++ )
		instance_destroy(i);

	InstanceDB->destroy(InstanceDB, instance_db_free);
	db_destroy(InstanceNameDB);
}
