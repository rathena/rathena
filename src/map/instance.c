// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/version.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "../common/db.h"

#include "clif.h"
#include "instance.h"
#include "map.h"
#include "npc.h"
#include "party.h"
#include "pc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

int instance_start = 0; // To keep the last index + 1 of normal map inserted in the map[ARRAY]
struct s_instance instance[MAX_INSTANCE];

/*--------------------------------------
 * name : instance name
 * Return value could be
 * -4 = already exists | -3 = no free instances | -2 = missing parameter | -1 = invalid type
 * On success return instance_id
 *--------------------------------------*/
int instance_create(int party_id, const char *name)
{
	int i;
	struct party_data *p = NULL;

	if( !party_id || !name )
	{
		ShowError("map_instance_create: missing parameter.\n");
		return -2;
	}

	p = party_search(party_id);
	if( !p || p->instance_id )
		return -4; // Party already instancing

	// Searching a Free Instance
	// 0 is ignored as this mean "no instance" on maps
	ARR_FIND(1, MAX_INSTANCE, i, instance[i].state == INSTANCE_FREE);
	if( i == MAX_INSTANCE )
	{
		ShowError("map_instance_create: no free instances, consider increasing MAX_INSTANCE.\n");
		return -3;
	}

	instance[i].state = INSTANCE_IDLE;
	instance[i].instance_id = i;
	instance[i].idle_timer = INVALID_TIMER;
	instance[i].idle_timeout = instance[i].idle_timeoutval = 0;
	instance[i].progress_timer = INVALID_TIMER;
	instance[i].progress_timeout = instance[i].progress_timeoutval = 0;
	instance[i].users = 0;
	instance[i].party_id = party_id;
	instance[i].ivar = NULL;
	instance[i].svar = NULL;

	memcpy( instance[i].name, name, sizeof(instance[i].name) );
	memset( instance[i].map, 0x00, sizeof(instance[i].map) );
	p->instance_id = i;

	clif_instance(i, 1, 0); // Start instancing window
	ShowInfo("[Instance] Created: %s.\n", name);
	return i;
}

/*--------------------------------------
 * Add a map to the instance using src map "name"
 *--------------------------------------*/
int instance_add_map(const char *name, int instance_id, bool usebasename)
{
	int m = map_mapname2mapid(name), i, im = -1;
	size_t num_cell, size;

	if( m < 0 )
		return -1; // source map not found
		
	if( instance[instance_id].state == INSTANCE_FREE )
	{
		ShowError("instance_add_map: trying to attach '%s' map to non-existing instance %d.\n", name, instance_id);
		return -1;
	}
	if( instance[instance_id].num_map >= MAX_MAP_PER_INSTANCE )
	{
		ShowError("instance_add_map: trying to add '%s' map to instance %d (%s) failed. Please increase MAX_MAP_PER_INSTANCE.\n", name, instance_id, instance[instance_id].name);
		return -2;
	}
	if( map[m].instance_id != 0 )
	{ // Source map already belong to a Instance.
		ShowError("instance_add_map: trying to instance already instanced map %s.\n", name);
		return -4;
	}

	ARR_FIND( instance_start, map_num, i, !map[i].name[0] ); // Searching for a Free Map
	if( i < map_num ) im = i; // Unused map found (old instance)
	else if( map_num - 1 >= MAX_MAP_PER_SERVER )
	{ // No more free maps
		ShowError("instance_add_map: no more free space to create maps on this server.\n");
		return -5;
	}
	else im = map_num++; // Using next map index

	memcpy( &map[im], &map[m], sizeof(struct map_data) ); // Copy source map
	snprintf(map[im].name, MAP_NAME_LENGTH, (usebasename ? "%.3d#%s" : "%.3d%s"), instance_id, name); // Generate Name for Instance Map
	map[im].index = mapindex_addmap(-1, map[im].name); // Add map index

	if( !map[im].index )
	{
		map[im].name[0] = '\0';
		ShowError("instance_add_map: no more free map indexes.\n");
		return -3; // No free map index
	}	

	// Reallocate cells
	num_cell = map[im].xs * map[im].ys;
	CREATE( map[im].cell, struct mapcell, num_cell );
	memcpy( map[im].cell, map[m].cell, num_cell * sizeof(struct mapcell) );

	size = map[im].bxs * map[im].bys * sizeof(struct block_list*);
	map[im].block = (struct block_list**)aCalloc(size, 1);
	map[im].block_mob = (struct block_list**)aCalloc(size, 1);

	memset(map[im].npc, 0x00, sizeof(map[i].npc));
	map[im].npc_num = 0;

	memset(map[im].moblist, 0x00, sizeof(map[im].moblist));
	map[im].mob_delete_timer = INVALID_TIMER;

	map[im].m = im;
	map[im].instance_id = instance_id;
	map[im].instance_src_map = m;
	map[m].flag.src4instance = 1; // Flag this map as a src map for instances

	instance[instance_id].map[instance[instance_id].num_map++] = im; // Attach to actual instance
	map_addmap2db(&map[im]);

	return im;
}

/*--------------------------------------
 * m : source map of this instance
 * party_id : source party of this instance
 * type : result (0 = map id | 1 = instance id)
 *--------------------------------------*/
int instance_map2imap(int m, int instance_id)
{
 	int i;
	for( i = 0; i < instance[instance_id].num_map; i++ )
 	{
		if( instance[instance_id].map[i] && map[instance[instance_id].map[i]].instance_src_map == m )
			return instance[instance_id].map[i];
 	}
 	return -1;
}

/*--------------------------------------
 * m : source map
 * instance_id : where to search
 * result : mapid of map "m" in this instance
 *--------------------------------------*/
int instance_mapid2imapid(int m, int instance_id)
{
	int i, max;
	if( map[m].flag.src4instance == 0 )
		return m; // not instances found for this map
	else if( map[m].instance_id )
	{ // This map is a instance, not a src map instance
		ShowError("map_instance_mapid2imapid: already instanced (%d / %d)\n", m, instance_id);
		return -1;
	}

	if( instance_id <= 0 )
		return -1;

	max = instance[instance_id].num_map;

	for( i = 0; i < max; i++ )
		if( map[instance[instance_id].map[i]].instance_src_map == m )
			return instance[instance_id].map[i];

	return -1;
}

/*--------------------------------------
 * map_instance_map_npcsub
 * Used on Init instance. Duplicates each script on source map
 *--------------------------------------*/
int instance_map_npcsub(struct block_list* bl, va_list args)
{
	struct npc_data* nd = (struct npc_data*)bl;
	int m = va_arg(args, int); // Destination Map

	npc_duplicate4instance(nd, m);
	return 1;
}

/*--------------------------------------
 * Init all map on the instance. Npcs are created here
 *--------------------------------------*/
void instance_init(int instance_id)
{
	int i;

	if( !instance_id )
		return; // nothing to do

	for( i = 0; i < instance[instance_id].num_map; i++ )
		map_foreachinmap(instance_map_npcsub, map[instance[instance_id].map[i]].instance_src_map, BL_NPC, instance[instance_id].map[i]);

	instance[instance_id].state = INSTANCE_BUSSY;
	ShowInfo("[Instance] Initialized %s.\n", instance[instance_id].name);
}

/*--------------------------------------
 * Used on instance deleting process.
 * Warps all players on each instance map to its save points.
 *--------------------------------------*/
int instance_del_load(struct map_session_data* sd, va_list args)
{
	int m = va_arg(args,int);
	if( !sd || sd->bl.m != m )
		return 0;

	pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, 0);
	return 1;
}

/*--------------------------------------
 * Removes a simple instance map
 *--------------------------------------*/
void instance_del_map(int m)
{
	int sm, i;
	if( m <= 0 || !map[m].instance_id )
	{
		ShowError("Tried to remove non-existing instance map (%d)\n", m);
		return;
	}

	sm = map[m].instance_src_map;
	map_foreachpc(instance_del_load, m);
	map_foreachinmap(cleanup_sub, m, BL_ALL);

	if( map[m].mob_delete_timer != INVALID_TIMER )
		delete_timer(map[m].mob_delete_timer, map_removemobs_timer);

	mapindex_removemap( map[m].index );

	// Free memory
	aFree(map[m].cell);
	aFree(map[m].block);
	aFree(map[m].block_mob);

	// Remove from instance
	for( i = 0; i < instance[map[m].instance_id].num_map; i++ )
	{
		if( instance[map[m].instance_id].map[i] == m )
		{
			instance[map[m].instance_id].num_map--;
			for( ; i < instance[map[m].instance_id].num_map; i++ )
				instance[map[m].instance_id].map[i] = instance[map[m].instance_id].map[i+1];
			i = -1;
			break;
		}
	}
	if( i == instance[map[m].instance_id].num_map )
		ShowError("map_instance_del: failed to remove %s from instance list (%s): %d\n", map[m].name, instance[map[m].instance_id].name, m);

	map_removemapdb(&map[m]);
	memset(&map[m], 0x00, sizeof(map[0]));
}

/*--------------------------------------
 * Used for instance variables. Clean each variable from memory.
 *--------------------------------------*/
void instance_destroy_freesvar(void *key, void *data, va_list args)
{
	if( data ) aFree(data);
}

/*--------------------------------------
 * Timer to destroy instance by process or idle
 *--------------------------------------*/
int instance_destroy_timer(int tid, unsigned int tick, int id, intptr data)
{
	instance_destroy(id);
	return 0;
}

/*--------------------------------------
 * Removes a instance, all its maps and npcs.
 *--------------------------------------*/
void instance_destroy(int instance_id)
{
	int last = 0, type;
	struct party_data *p;
	time_t now = time(NULL);

	if( !instance_id || instance[instance_id].state == INSTANCE_FREE )
		return; // nothing to do

	if( instance[instance_id].progress_timeout && instance[instance_id].progress_timeout <= now )
		type = 1;
	else if( instance[instance_id].idle_timeout && instance[instance_id].idle_timeout <= now )
		type = 2;
	else
		type = 3;

	clif_instance(instance_id, 5, type); // Report users this instance has been destroyed

	while( instance[instance_id].num_map && last != instance[instance_id].map[0] )
	{ // Remove all maps from instance
		last = instance[instance_id].map[0];
		instance_del_map( instance[instance_id].map[0] );
	}

	if( instance[instance_id].ivar )
		linkdb_final( &instance[instance_id].ivar ); // Remove numeric vars

	if( instance[instance_id].svar )
	{ // Remove string vars
		linkdb_foreach( &instance[instance_id].svar, instance_destroy_freesvar );
		linkdb_final( &instance[instance_id].svar );
	}

	if( instance[instance_id].progress_timer != INVALID_TIMER )
		delete_timer( instance[instance_id].progress_timer, instance_destroy_timer);
	if( instance[instance_id].idle_timer != INVALID_TIMER )
		delete_timer( instance[instance_id].idle_timer, instance_destroy_timer);

	instance[instance_id].ivar = NULL;
	instance[instance_id].svar = NULL;

	if( instance[instance_id].party_id && (p = party_search(instance[instance_id].party_id)) != NULL )
		p->instance_id = 0; // Update Party information

	ShowInfo("[Instance] Destroyed %s.\n", instance[instance_id].name);
	memset( &instance[instance_id], 0x00, sizeof(instance[0]) );

	instance[instance_id].state = INSTANCE_FREE;
}

/*--------------------------------------
 * Checks if there are users in the instance or not to start idle timer
 *--------------------------------------*/
void instance_check_idle(int instance_id)
{
	bool idle = true;
	time_t now = time(NULL);

	if( !instance_id || instance[instance_id].idle_timeoutval == 0 )
		return;

	if( instance[instance_id].users )
		idle = false;

	if( instance[instance_id].idle_timer != INVALID_TIMER && !idle )
	{
		delete_timer(instance[instance_id].idle_timer, instance_destroy_timer);
		instance[instance_id].idle_timer = INVALID_TIMER;
		instance[instance_id].idle_timeout = 0;
		clif_instance(instance_id, 3, 0); // Notify instance users normal instance expiration
	}
	else if( instance[instance_id].idle_timer == INVALID_TIMER && idle )
	{
		instance[instance_id].idle_timeout = now + instance[instance_id].idle_timeoutval;
		instance[instance_id].idle_timer = add_timer( gettick() + (unsigned int)instance[instance_id].idle_timeoutval * 1000, instance_destroy_timer, instance_id, 0);
		clif_instance(instance_id, 4, 0); // Notify instance users it will be destroyed of no user join it again in "X" time
	}
}

/*--------------------------------------
 * Set instance Timers
 *--------------------------------------*/
void instance_set_timeout(int instance_id, unsigned int progress_timeout, unsigned int idle_timeout)
{
	time_t now = time(0);

	if( !instance_id )
		return;
		
	if( instance[instance_id].progress_timer != INVALID_TIMER )
		delete_timer( instance[instance_id].progress_timer, instance_destroy_timer);
	if( instance[instance_id].idle_timer != INVALID_TIMER )
		delete_timer( instance[instance_id].idle_timer, instance_destroy_timer);

	if( progress_timeout )
	{
		instance[instance_id].progress_timeoutval = progress_timeout;
		instance[instance_id].progress_timeout = now + progress_timeout;
		instance[instance_id].progress_timer = add_timer( gettick() + progress_timeout * 1000, instance_destroy_timer, instance_id, 0);
	}
	else
	{
		instance[instance_id].progress_timeoutval = 0;
		instance[instance_id].progress_timeout = 0;
		instance[instance_id].progress_timer = INVALID_TIMER;
	}
		
	if( idle_timeout )
	{
		instance[instance_id].idle_timeoutval = idle_timeout;
		instance[instance_id].idle_timer = INVALID_TIMER;
		instance_check_idle(instance_id);
	}
	else
	{
		instance[instance_id].idle_timeoutval = 0;
		instance[instance_id].idle_timeout = 0;
		instance[instance_id].idle_timer = INVALID_TIMER;
	}

	if( instance[instance_id].idle_timer == INVALID_TIMER && instance[instance_id].progress_timer != INVALID_TIMER )
		clif_instance(instance_id, 3, 0);
}

/*--------------------------------------
 * Checks if sd in on a instance and should be kicked from it
 *--------------------------------------*/
void instance_check_kick(struct map_session_data *sd)
{
	int m = sd->bl.m;

	clif_instance_leave(sd->fd);
	if( map[m].instance_id )
	{ // User was on the instance map
		if( map[m].save.map )
			pc_setpos(sd, map[m].save.map, map[m].save.x, map[m].save.y, 3);
		else
			pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, 3);
	}
}

void do_init_instance(void)
{
	memset(instance, 0x00, sizeof(instance));
	add_timer_func_list(instance_destroy_timer, "instance_destroy_timer");
}
