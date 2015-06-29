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
#include "instance.h"
#include "map.h"
#include "npc.h"
#include "party.h"
#include "pc.h"

#include <stdlib.h>

#define INSTANCE_INTERVAL	60000	// Interval used to check when an instance is to be destroyed (ms)
#define INSTANCE_LIMIT		300000	// Idle timer before instance is destroyed (ms) : 5 Minute Default

int instance_start = 0; // To keep the last index + 1 of normal map inserted in the map[ARRAY]

struct instance_data instance_data[MAX_INSTANCE_DATA];

/// Instance DB entry struct
struct instance_db {
	unsigned short id; ///< Instance ID
	StringBuf *name; ///< Instance name
	unsigned int limit; ///< Duration limit
	struct {
		StringBuf *mapname; ///< Mapname, the limit should be MAP_NAME_LENGTH_EXT
		unsigned short x, y; ///< Map coordinates
	} enter;
	StringBuf **maplist; ///< Used maps in instance, the limit should be MAP_NAME_LENGTH_EXT
	uint8 maplist_count; ///< Number of used maps
};

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
static struct instance_db *instance_searchtype_db(unsigned short instance_id) {
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

/*==========================================
 * Deletes an instance timer (Destroys instance)
 *------------------------------------------*/
static int instance_delete_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	instance_destroy(id);

	return 0;
}

/*==========================================
 * Create subscription timer for party
 *------------------------------------------*/
static int instance_subscription_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	int i, ret;
	int instance_id = instance_wait.id[0];
	struct party_data *p;

	if(instance_wait.count == 0 || instance_id <= 0)
		return 0;

	// Check that maps have been added
	ret = instance_addmap(instance_id);

	// If no maps are created, tell party to wait
	if(ret == 0 && ( p = party_search( instance_data[instance_id].party_id ) ) != NULL)
		clif_instance_changewait( party_getavailablesd( p ), 0xffff, 1 );

	instance_wait.count--;
	memmove(&instance_wait.id[0],&instance_wait.id[1],sizeof(instance_wait.id[0])*instance_wait.count);
	memset(&instance_wait.id[instance_wait.count], 0, sizeof(instance_wait.id[0]));

	for(i = 0; i < instance_wait.count; i++) {
		if(	instance_data[instance_wait.id[i]].state == INSTANCE_IDLE &&
			( p = party_search( instance_data[instance_wait.id[i]].party_id ) ) != NULL
		){
			clif_instance_changewait( party_getavailablesd( p ), i + 1, 1 );
		}
	}

	if(instance_wait.count)
		instance_wait.timer = add_timer(gettick()+INSTANCE_INTERVAL, instance_subscription_timer, 0, 0);
	else
		instance_wait.timer = INVALID_TIMER;

	return 0;
}

/*==========================================
 * Adds timer back to party entering instance
 *------------------------------------------*/
static int instance_startkeeptimer(struct instance_data *im, short instance_id)
{
	struct instance_db *db;
	struct party_data *p;

	nullpo_retr(0, im);

	// No timer
	if(im->keep_timer != INVALID_TIMER)
		return 1;

	if((db = instance_searchtype_db(im->type)) == NULL)
		return 1;

	// Add timer
	im->keep_limit = (unsigned int)time(NULL) + db->limit;
	im->keep_timer = add_timer(gettick()+db->limit*1000, instance_delete_timer, instance_id, 0);

	// Notify party of the added instance timer
	if( ( p = party_search( im->party_id ) ) != NULL )
		clif_instance_status( party_getavailablesd( p ), StringBuf_Value(db->name), im->keep_limit, im->idle_limit, 1 );

	return 0;
}

/*==========================================
 * Creates idle timer
 * Default before instance destroy is 5 minutes
 *------------------------------------------*/
static int instance_startidletimer(struct instance_data *im, short instance_id)
{
	struct instance_db *db;
	struct party_data *p;

	nullpo_retr(1, im);

	// No current timer
	if(im->idle_timer != INVALID_TIMER)
		return 1;

	// Add the timer
	im->idle_limit = (unsigned int)time(NULL) + INSTANCE_LIMIT/1000;
	im->idle_timer = add_timer(gettick()+INSTANCE_LIMIT, instance_delete_timer, instance_id, 0);

	// Notify party of added instance timer
	if( ( p = party_search( im->party_id ) ) != NULL &&
		( db = instance_searchtype_db( im->type ) ) != NULL
		)
	{
		clif_instance_status( party_getavailablesd( p ), StringBuf_Value(db->name), im->keep_limit, im->idle_limit, 1 );
	}

	return 0;
}

/*==========================================
 * Delete the idle timer
 *------------------------------------------*/
static int instance_stopidletimer(struct instance_data *im)
{
	struct party_data *p;

	nullpo_retr(0, im);
	
	// No timer
	if(im->idle_timer == INVALID_TIMER)
		return 1;

	// Delete the timer - Party has returned or instance is destroyed
	im->idle_limit = 0;
	delete_timer(im->idle_timer, instance_delete_timer);
	im->idle_timer = INVALID_TIMER;

	// Notify the party
	if( ( p = party_search( im->party_id ) ) != NULL )
		clif_instance_changestatus( party_getavailablesd( p ), 0, im->idle_limit, 1 );

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
		map_foreachinarea(instance_addnpc_sub, im->map[i].src_m, 0, 0, map[im->map[i].src_m].xs, map[im->map[i].src_m].ys, BL_NPC, im->map[i].m);

	// Now run their OnInstanceInit
	for(i = 0; i < im->cnt_map; i++)
		map_foreachinarea(instance_npcinit, im->map[i].m, 0, 0, map[im->map[i].m].xs, map[im->map[i].m].ys, BL_NPC, im->map[i].m);

}

/*--------------------------------------
 * name : instance name
 * Return value could be
 * -4 = no free instances | -3 = already exists | -2 = party not found | -1 = invalid type
 * On success return instance_id
 *--------------------------------------*/
int instance_create(int party_id, const char *name)
{
	short i;
	struct instance_db *db = instance_searchname_db(name);
	struct party_data *p = party_search(party_id);

	if(db == NULL)
		return -1;

	if( p == NULL )
		return -2;

	if( p->instance_id )
		return -3; // Party already instancing

	// Searching a Free Instance
	// 0 is ignored as this mean "no instance" on maps
	ARR_FIND(1, MAX_INSTANCE_DATA, i, instance_data[i].state == INSTANCE_FREE);
	if( i >= MAX_INSTANCE_DATA )
		return -4;

	instance_data[i].type = db->id;
	instance_data[i].state = INSTANCE_IDLE;
	instance_data[i].party_id = p->party.party_id;
	instance_data[i].keep_limit = 0;
	instance_data[i].keep_timer = INVALID_TIMER;
	instance_data[i].idle_limit = 0;
	instance_data[i].idle_timer = INVALID_TIMER;
	instance_data[i].vars = idb_alloc(DB_OPT_RELEASE_DATA);
	memset(instance_data[i].map, 0, sizeof(instance_data[i].map));

	p->instance_id = i;

	instance_wait.id[instance_wait.count++] = p->instance_id;

	clif_instance_create( party_getavailablesd( p ), name, instance_wait.count, 1);

	instance_subscription_timer(0,0,0,0);

	ShowInfo("[Instance] Created: %s.\n", name);

	return i;
}

/*--------------------------------------
 * Adds maps to the instance
 *--------------------------------------*/
int instance_addmap(short instance_id)
{
	int i, m;
	int cnt_map = 0;
	struct instance_data *im;
	struct instance_db *db;
	struct party_data *p;

	if(instance_id <= 0)
		return 0;

	im = &instance_data[instance_id];

	// If the instance isn't idle, we can't do anything
	if(im->state != INSTANCE_IDLE)
		return 0;

	if((db = instance_searchtype_db(im->type)) == NULL)
		return 0;

	// Set to busy, update timers
	im->state = INSTANCE_BUSY;
	im->idle_limit = (unsigned int)time(NULL) + INSTANCE_LIMIT/1000;
	im->idle_timer = add_timer(gettick()+INSTANCE_LIMIT, instance_delete_timer, instance_id, 0);

	// Add the maps
	for(i = 0; i < db->maplist_count; i++) {
		if(strlen(StringBuf_Value(db->maplist[i])) < 1)
			continue;
		else if( (m = map_addinstancemap(StringBuf_Value(db->maplist[i]), instance_id)) < 0) {
			// An error occured adding a map
			ShowError("instance_addmap: No maps added to instance %d.\n",instance_id);
			return 0;
		} else {
			im->map[cnt_map].m = m;
			im->map[cnt_map].src_m = map_mapname2mapid(StringBuf_Value(db->maplist[i]));
			cnt_map++;
		}
	}

	im->cnt_map = cnt_map;

	// Create NPCs on all maps
	instance_addnpc(im);

	// Inform party members of the created instance
	if( (p = party_search( im->party_id ) ) != NULL )
		clif_instance_status( party_getavailablesd( p ), StringBuf_Value(db->name), im->keep_limit, im->idle_limit, 1);

	return cnt_map;
}


/*==========================================
 * Returns an instance map ID using a map name
 * name : source map
 * instance_id : where to search
 * result : mapid of map "name" in this instance
 *------------------------------------------*/
int instance_mapname2mapid(const char *name, short instance_id)
{
	struct instance_data *im;
	int m = map_mapname2mapid(name);
	char iname[MAP_NAME_LENGTH];
	int i;

	if(m < 0) {
		ShowError("instance_mapname2mapid: map name %s does not exist.\n",name);
		return m;
	}

	strcpy(iname,name);

	if(instance_id <= 0 || instance_id > MAX_INSTANCE_DATA)
		return m;

	im = &instance_data[instance_id];
	if(im->state != INSTANCE_BUSY)
		return m;

	for(i = 0; i < MAX_MAP_PER_INSTANCE; i++)
		if(im->map[i].src_m == m) {
			char alt_name[MAP_NAME_LENGTH];
			if((strchr(iname,'@') == NULL) && strlen(iname) > 8) {
				memmove(iname, iname+(strlen(iname)-9), strlen(iname));
				snprintf(alt_name, sizeof(alt_name),"%d#%s", instance_id, iname);
			} else
				snprintf(alt_name, sizeof(alt_name),"%.3d%s", instance_id, iname);
			return map_mapname2mapid(alt_name);
		}

	return m;
}

/*==========================================
 * Removes a instance, all its maps and npcs.
 *------------------------------------------*/
int instance_destroy(short instance_id)
{
	struct instance_data *im;
	struct party_data *p;
	int i, type = 0;
	unsigned int now = (unsigned int)time(NULL);

	if(instance_id <= 0 || instance_id > MAX_INSTANCE_DATA)
		return 1;

	im = &instance_data[instance_id];

	if(im->state == INSTANCE_FREE)
		return 1;

	if(im->state == INSTANCE_IDLE) {
		for(i = 0; i < instance_wait.count; i++) {
			if(instance_wait.id[i] == instance_id) {
				instance_wait.count--;
				memmove(&instance_wait.id[i],&instance_wait.id[i+1],sizeof(instance_wait.id[0])*(instance_wait.count-i));
				memset(&instance_wait.id[instance_wait.count], 0, sizeof(instance_wait.id[0]));

				for(i = 0; i < instance_wait.count; i++)
					if(instance_data[instance_wait.id[i]].state == INSTANCE_IDLE)
						if((p = party_search(instance_data[instance_wait.id[i]].party_id)) != NULL)
							clif_instance_changewait( party_getavailablesd( p ), i+1, 1);

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

		for(i = 0; i < MAX_MAP_PER_INSTANCE; i++)
			map_delinstancemap(im->map[i].m);
	}

	if(im->keep_timer != INVALID_TIMER) {
		delete_timer(im->keep_timer, instance_delete_timer);
		im->keep_timer = INVALID_TIMER;
	}
	if(im->idle_timer != INVALID_TIMER) {
		delete_timer(im->idle_timer, instance_delete_timer);
		im->idle_timer = INVALID_TIMER;
	}

	if((p = party_search(im->party_id))) {
		p->instance_id = 0;

		if(type)
			clif_instance_changestatus( party_getavailablesd( p ), type, 0, 1 );
		else
			clif_instance_changewait( party_getavailablesd( p ), 0xffff, 1 );
	}

	if( im->vars ) {
		db_destroy(im->vars);
		im->vars = NULL;
	}

	ShowInfo("[Instance] Destroyed %d.\n", instance_id);

	memset(&instance_data[instance_id], 0, sizeof(instance_data[instance_id]));

	return 0;
}

/*==========================================
 * Allows a user to enter an instance
 *------------------------------------------*/
int instance_enter(struct map_session_data *sd, const char *name)
{
	struct instance_db *db = instance_searchname_db(name);

	if(db == NULL)
		return 3;

	return instance_enter_position(sd, name, db->enter.x, db->enter.y);
}

/*==========================================
 * Warp a user into instance
 *------------------------------------------*/
int instance_enter_position(struct map_session_data *sd, const char *name, short x, short y)
{
	struct instance_data *im;
	struct instance_db *db = instance_searchname_db(name);
	struct party_data *p;
	int m;

	nullpo_retr(-1, sd);

	// Character must be in instance party
	if(sd->status.party_id == 0)
		return 1;
	if((p = party_search(sd->status.party_id)) == NULL)
		return 1;

	// Party must have an instance
	if(p->instance_id == 0)
		return 2;

	if(db == NULL)
		return 3;

	im = &instance_data[p->instance_id];
	if(im->party_id != p->party.party_id)
		return 3;
	if(im->state != INSTANCE_BUSY)
		return 3;
	if(im->type != db->id)
		return 3;

	// Does the instance match?
	if((m = instance_mapname2mapid(StringBuf_Value(db->enter.mapname), p->instance_id)) < 0)
		return 3;

	if(pc_setpos(sd, map_id2index(m), x, y, CLR_OUTSIGHT))
		return 3;

	// If there was an idle timer, let's stop it
	instance_stopidletimer(im);

	// Now we start the instance timer
	instance_startkeeptimer(im, p->instance_id);

	return 0;
}

/*==========================================
 * Request some info about the instance
 *------------------------------------------*/
int instance_reqinfo(struct map_session_data *sd, short instance_id)
{
	struct instance_data *im;
	struct instance_db *db;

	nullpo_retr(1, sd);

	if(instance_id <= 0 || instance_id > MAX_INSTANCE_DATA)
		return 1;

	im = &instance_data[instance_id];

	if((db = instance_searchtype_db(im->type)) == NULL)
		return 1;

	// Say it's created if instance is not busy
	if(im->state == INSTANCE_IDLE) {
		int i;

		for(i = 0; i < instance_wait.count; i++) {
			if(instance_wait.id[i] == instance_id) {
				clif_instance_create(sd, StringBuf_Value(db->name), i+1, 0);
				break;
			}
		}
	} else if(im->state == INSTANCE_BUSY) // Give info on the instance if busy
		clif_instance_status(sd, StringBuf_Value(db->name), im->keep_limit, im->idle_limit, 0);

	return 0;
}

/*==========================================
 * Add players to the instance (for timers)
 *------------------------------------------*/
int instance_addusers(short instance_id)
{
	struct instance_data *im;

	if(instance_id <= 0 || instance_id > MAX_INSTANCE_DATA)
		return 1;

	im = &instance_data[instance_id];
	if(im->state != INSTANCE_BUSY)
		return 1;

	// Stop the idle timer if we had one
	instance_stopidletimer(im);

	// Start the instance keep timer
	instance_startkeeptimer(im, instance_id);

	return 0;
}

/*==========================================
 * Delete players from the instance (for timers)
 *------------------------------------------*/
int instance_delusers(short instance_id)
{
	struct instance_data *im;
	int i, idle = 0;

	if(instance_id <= 0 || instance_id > MAX_INSTANCE_DATA)
		return 1;

	im = &instance_data[instance_id];
	if(im->state != INSTANCE_BUSY)
		return 1;

	// If no one is in the instance, start the idle timer
	for(i = 0; im->map[i].m && i < MAX_MAP_PER_INSTANCE; i++)
		if(map[im->map[i].m].users > 1) // We check this before the actual map.users are updated, hence the 1
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
	bool isNew = false, defined = false;

	if (!id || id >  USHRT_MAX) {
		ShowError("instance_readdb_sub: Cannot add instance with ID '%d'. Valid ID is 1 ~ %d.\n", id, USHRT_MAX);
		return false;
	}

	if (mapindex_name2id(str[3]) == 0) {
		ShowError("instance_readdb_sub: Invalid map '%s' as entrance map.\n", str[3]);
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
	StringBuf_AppendStr(db->enter.mapname, str[3]);
	db->enter.x = atoi(str[4]);
	db->enter.y = atoi(str[5]);

	//Instance maps
	for (i = 6; i < columns; i++) {
		if (strlen(str[i])) {
			if (mapindex_name2id(str[i]) == 0) {
				ShowWarning("instance_readdb_sub: Invalid map '%s' in maplist, skipping...\n", str[i]);
				continue;
			}
			RECREATE(db->maplist, StringBuf *, db->maplist_count+1);
			db->maplist[db->maplist_count] = StringBuf_Malloc();
			if (strcmpi(str[i], str[3]) == 0)
				defined = true;
			StringBuf_AppendStr(db->maplist[db->maplist_count], str[i]);
			db->maplist_count++;
		}
	}

	if (!defined) {
		ShowError("instance_readdb_sub: The entrance map is not defined in instance map list.\n");
		instance_db_free_sub(db);
		if (!isNew)
			uidb_remove(InstanceDB,id);
		return false;
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
	int i;

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
			struct party_data *p;
			if(!(p = party_search(sd->status.party_id)) || p->instance_id != map[sd->bl.m].instance_id) // Someone not in party is on instance map
				continue;
			im = &instance_data[p->instance_id];
			if((db = instance_searchtype_db(im->type)) != NULL && !instance_enter(sd,StringBuf_Value(db->name))) { // All good
				clif_displaymessage(sd->fd, msg_txt(sd,515)); // Instance has been reloaded
				instance_reqinfo(sd,p->instance_id);
			} else // Something went wrong
				ShowError("do_reload_instance: Error setting character at instance start: character_id=%d instance=%s.\n",sd->status.char_id,db->name);
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

	add_timer_func_list(instance_delete_timer,"instance_delete_timer");
	add_timer_func_list(instance_subscription_timer,"instance_subscription_timer");
}

void do_final_instance(void) {
	int i;

	for( i = 1; i < MAX_INSTANCE_DATA; i++ )
		instance_destroy(i);

	InstanceDB->destroy(InstanceDB, instance_db_free);
	db_destroy(InstanceNameDB);
}
