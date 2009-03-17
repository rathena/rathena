// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"

#include "battleground.h"
#include "battle.h"
#include "clif.h"
#include "map.h"
#include "npc.h"
#include "pc.h"
#include "pet.h"
#include "homunculus.h"
#include "mercenary.h"

#include <string.h>
#include <stdio.h>

static DBMap* bg_team_db; // int bg_id -> struct battleground_data*
static unsigned int bg_team_counter = 0; // Next bg_id
struct guild bg_guild[2]; // Temporal fake guild information

struct battleground_data* bg_team_search(int bg_id)
{ // Search a BG Team using bg_id
	if( !bg_id ) return NULL;
	return (struct battleground_data *)idb_get(bg_team_db, bg_id);
}

struct map_session_data* bg_getavailablesd(struct battleground_data *bg)
{
	int i;
	nullpo_retr(NULL, bg);
	ARR_FIND(0, MAX_BG_MEMBERS, i, bg->members[i].sd != NULL);
	return( i < MAX_BG_MEMBERS ) ? bg->members[i].sd : NULL;
}

int bg_team_delete(int bg_id)
{ // Deletes BG Team from db
	int i;
	struct map_session_data *sd;
	struct battleground_data *bg = bg_team_search(bg_id);
	struct guild *g;

	if( bg == NULL ) return 0;
	for( i = 0; i < MAX_BG_MEMBERS; i++ )
	{
		if( (sd = bg->members[i].sd) == NULL )
			continue;

		bg_send_dot_remove(sd);
		sd->state.bg_id = 0;
		if( sd->status.guild_id && (g = guild_search(sd->status.guild_id)) != NULL )
		{
			clif_guild_belonginfo(sd,g);
			clif_guild_basicinfo(sd);
			clif_guild_allianceinfo(sd);
			clif_guild_memberlist(sd);
			clif_guild_skillinfo(sd);
		}
		else
			clif_bg_leave_single(sd, sd->status.name, "Leaving Battleground...");

		clif_charnameupdate(sd);
		clif_guild_emblem_area(&sd->bl);
	}
	idb_remove(bg_team_db, bg_id);
	return 1;
}

int bg_team_warp(int bg_id, unsigned short mapindex, short x, short y)
{ // Warps a Team
	int i;
	struct battleground_data *bg = bg_team_search(bg_id);
	if( bg == NULL ) return 0;
	for( i = 0; i < MAX_BG_MEMBERS; i++ )
		if( bg->members[i].sd != NULL ) pc_setpos(bg->members[i].sd, mapindex, x, y, 3);
	return 1;
}

int bg_send_dot_remove(struct map_session_data *sd)
{
	if( sd && sd->state.bg_id )
		clif_bg_xy_remove(sd);
	return 0;
}

int bg_team_join(int bg_id, struct map_session_data *sd)
{ // Player joins team
	int i;
	struct battleground_data *bg = bg_team_search(bg_id);
	struct map_session_data *pl_sd;

	if( bg == NULL || sd == NULL || sd->state.bg_id ) return 0;

	ARR_FIND(0, MAX_BG_MEMBERS, i, bg->members[i].sd == NULL);
	if( i == MAX_BG_MEMBERS ) return 0; // No free slots

	sd->state.bg_id = bg_id;
	bg->members[i].sd = sd;
	bg->members[i].x = sd->bl.x;
	bg->members[i].y = sd->bl.y;
	bg->count++;

	guild_send_dot_remove(sd);
	clif_bg_belonginfo(sd);
	// clif_bg_emblem(sd, bg->g);
	clif_charnameupdate(sd);

	for( i = 0; i < MAX_BG_MEMBERS; i++ )
	{
		if( (pl_sd = bg->members[i].sd) == NULL )
			continue;
		clif_guild_basicinfo(pl_sd);
		clif_bg_emblem(pl_sd, bg->g);
		clif_bg_memberlist(pl_sd);
		if( pl_sd != sd )
			clif_hpmeter_single(sd->fd, pl_sd->bl.id, pl_sd->battle_status.hp, pl_sd->battle_status.max_hp);
	}

	clif_guild_emblem_area(&sd->bl);
	clif_bg_hp(sd);
	clif_bg_xy(sd);

	return 1;
}

int bg_team_leave(struct map_session_data *sd, int flag)
{ // Single Player leaves team
	int i, bg_id;
	struct battleground_data *bg;
	struct map_session_data *pl_sd;
	struct guild *g;

	if( sd == NULL || !sd->state.bg_id )
		return 0;

	// Packets
	clif_bg_leave_single(sd, sd->status.name, "Leaving Battle...");
	bg_send_dot_remove(sd);
	bg_id = sd->state.bg_id;
	sd->state.bg_id = 0;

	if( sd->status.guild_id && (g = guild_search(sd->status.guild_id)) != NULL )
	{ // Refresh Guild Information
		clif_guild_belonginfo(sd, g);
		clif_guild_basicinfo(sd);
		clif_guild_allianceinfo(sd);
		clif_guild_memberlist(sd);
		clif_guild_skillinfo(sd);
		clif_guild_emblem(sd, g);
	}

	clif_charnameupdate(sd);
	clif_guild_emblem_area(&sd->bl);

	if( (bg = bg_team_search(bg_id)) == NULL )
		return 0;

	ARR_FIND(0, MAX_BG_MEMBERS, i, bg->members[i].sd == sd);
	if( i < MAX_BG_MEMBERS ) // Removes member from BG
		memset(&bg->members[i], 0, sizeof(bg->members[0]));

	bg->count--;
	for( i = 0; i < MAX_BG_MEMBERS; i++ )
	{ // Update other BG members
		if( (pl_sd = bg->members[i].sd) == NULL )
			continue;
		if( flag )
			clif_bg_expulsion_single(pl_sd, sd->status.name, "User has quit the game...");
		else
			clif_bg_leave_single(pl_sd, sd->status.name, "Leaving Battle...");

		clif_guild_basicinfo(pl_sd);
		clif_bg_emblem(pl_sd, bg->g);
		clif_bg_memberlist(pl_sd);
	}

	if( bg->logout_event[0] && flag )
		npc_event(sd, bg->logout_event, 0);

	return bg->count;
}

int bg_member_respawn(struct map_session_data *sd)
{ // Respawn after killed
	struct battleground_data *bg;
	if( sd == NULL || !pc_isdead(sd) || !sd->state.bg_id || (bg = bg_team_search(sd->state.bg_id)) == NULL )
		return 0;
	if( bg->mapindex == 0 )
		return 0; // Respawn not handled by Core
	pc_setpos(sd, bg->mapindex, bg->x, bg->y, 0);
	status_revive(&sd->bl, 1, 100);

	return 1; // Warped
}

struct guild* bg_guild_get(int bg_id)
{ // Return Fake Guild for BG Members
	struct battleground_data *bg = bg_team_search(bg_id);
	if( bg == NULL ) return NULL;
	return bg->g;
}

int bg_create(unsigned short mapindex, short rx, short ry, int guild_index, const char *ev, const char *dev)
{
	struct battleground_data *bg;
	bg_team_counter++;

	CREATE(bg, struct battleground_data, 1);
	bg->bg_id = bg_team_counter;
	bg->count = 0;
	bg->g = &bg_guild[guild_index];
	bg->mapindex = mapindex;
	bg->x = rx;
	bg->y = ry;
	safestrncpy(bg->logout_event, ev, sizeof(bg->logout_event));
	safestrncpy(bg->die_event, dev, sizeof(bg->die_event));

	memset(&bg->members, 0, sizeof(bg->members));
	idb_put(bg_team_db, bg_team_counter, bg);

	return bg->bg_id;
}

int bg_team_get_id(struct block_list *bl)
{
	nullpo_retr(0, bl);
	switch( bl->type )
	{
		case BL_PC:
			return ((TBL_PC*)bl)->state.bg_id;
		case BL_PET:
			if( ((TBL_PET*)bl)->msd )
				return ((TBL_PET*)bl)->msd->state.bg_id;
			break;
		case BL_MOB:
		{
			struct map_session_data *msd;
			struct mob_data *md = (TBL_MOB*)bl;
			if( md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL )
				return msd->state.bg_id;
			return md->state.bg_id;
		}
		case BL_HOM:
			if( ((TBL_HOM*)bl)->master )
				return ((TBL_HOM*)bl)->master->state.bg_id;
			break;
		case BL_MER:
			if( ((TBL_MER*)bl)->master )
				return ((TBL_MER*)bl)->master->state.bg_id;
			break;
		case BL_SKILL:
			return ((TBL_SKILL*)bl)->group->bg_id;
	}

	return 0;
}

int bg_send_message(struct map_session_data *sd, const char *mes, int len)
{
	struct battleground_data *bg;

	nullpo_retr(0, sd);
	if( sd->state.bg_id == 0 || (bg = bg_team_search(sd->state.bg_id)) == NULL )
		return 0;
	clif_bg_message(bg, sd->status.name, mes, len);
	return 0;
}

int bg_send_xy_timer_sub(DBKey key, void *data, va_list ap)
{
	struct battleground_data *bg = (struct battleground_data *)data;
	struct map_session_data *sd;
	int i;
	nullpo_retr(0, bg);
	for( i = 0; i < MAX_BG_MEMBERS; i++ )
	{
		if( (sd = bg->members[i].sd) == NULL )
			continue;
		if( sd->bl.x != bg->members[i].x || sd->bl.y != bg->members[i].y )
		{ // xy update
			bg->members[i].x = sd->bl.x;
			bg->members[i].y = sd->bl.y;
			clif_bg_xy(sd);
		}
	}
	return 0;
}

int bg_send_xy_timer(int tid, unsigned int tick, int id, intptr data)
{
	bg_team_db->foreach(bg_team_db, bg_send_xy_timer_sub, tick);
	return 0;
}

void bg_guild_build_data(void)
{
	int i, j;
	memset(&bg_guild, 0, sizeof(bg_guild));
	for( i = 1; i <= 2; i++ )
	{ // Emblem Data
		FILE* fp = NULL;
		char path[256];

		j = i - 1;
		sprintf(path, "%s/bgemblem_%d.ebm", db_path, i);
		if( (fp = fopen(path, "rb")) == NULL )
			continue;

		bg_guild[j].emblem_id = 1;
		fseek(fp, 0, SEEK_END);
		bg_guild[j].emblem_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fread(&bg_guild[j].emblem_data, 1, bg_guild[j].emblem_len, fp);
		fclose(fp);

		ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"' emblem data file.\n", path);
	}

	// Guild Data - Guillaume
	strncpy(bg_guild[0].name, "Blue Team", NAME_LENGTH);
	strncpy(bg_guild[0].master, "General Guillaume Marollo", NAME_LENGTH);
	bg_guild[0].guild_id = battle_config.bg_guild_id1;
	bg_guild[0].guild_lv = 1;
	strncpy(bg_guild[0].position[0].name, "Blue Team", NAME_LENGTH);
	bg_guild[0].max_member = MAX_BG_MEMBERS;

	// Guild Data - Croix
	strncpy(bg_guild[1].name, "Red Team", NAME_LENGTH);
	strncpy(bg_guild[1].master, "Prince Croix Marollo", NAME_LENGTH);
	bg_guild[1].guild_id = battle_config.bg_guild_id2;
	bg_guild[1].guild_lv = 1;
	strncpy(bg_guild[1].position[0].name, "Red Team", NAME_LENGTH);
	bg_guild[1].max_member = MAX_BG_MEMBERS;
}

void do_init_battleground(void)
{
	bg_team_db = idb_alloc(DB_OPT_RELEASE_DATA);
	add_timer_func_list(bg_send_xy_timer, "bg_send_xy_timer");
	add_timer_interval(gettick() + battle_config.bg_update_interval, bg_send_xy_timer, 0, 0, battle_config.bg_update_interval);
	bg_guild_build_data();
}

void do_final_battleground(void)
{
	bg_team_db->destroy(bg_team_db, NULL);
}
