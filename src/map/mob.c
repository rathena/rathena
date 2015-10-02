// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/db.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/ers.h"
#include "../common/random.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "../common/socket.h"

#include "map.h"
#include "path.h"
#include "clif.h"
#include "intif.h"
#include "pc.h"
#include "pet.h"
#include "homunculus.h"
#include "mercenary.h"
#include "elemental.h"
#include "party.h"
#include "quest.h"

#include <stdlib.h>
#include <math.h>

#define ACTIVE_AI_RANGE 2	//Distance added on top of 'AREA_SIZE' at which mobs enter active AI mode.

#define IDLE_SKILL_INTERVAL 10	//Active idle skills should be triggered every 1 second (1000/MIN_MOBTHINKTIME)

// Probability for mobs far from players from doing their IDLE skill. (rate of 1000 minute)
// in Aegis, this is 100% for mobs that have been activated by players and none otherwise.
#define MOB_LAZYSKILLPERC(md) (md->state.spotted?1000:0)
// Move probability for mobs away from players (rate of 1000 minute)
// in Aegis, this is 100% for mobs that have been activated by players and none otherwise.
#define MOB_LAZYMOVEPERC(md) (md->state.spotted?1000:0)
#define MOB_MAX_DELAY (24*3600*1000)
#define MAX_MINCHASE 30	//Max minimum chase value to use for mobs.
#define RUDE_ATTACKED_COUNT 2	//After how many rude-attacks should the skill be used?
#define MAX_MOB_CHAT 50 //Max Skill's messages

// On official servers, monsters will only seek targets that are closer to walk to than their
// search range. The search range is affected depending on if the monster is walking or not.
// On some maps there can be a quite long path for just walking two cells in a direction and
// the client does not support displaying walk paths that are longer than 14 cells, so this
// option reduces position lag in such situation. But doing a complex search for every possible
// target, might be CPU intensive.
// Disable this to make monsters not do any path search when looking for a target (old behavior).
#define ACTIVEPATHSEARCH

//Dynamic mob database, allows saving of memory when there's big gaps in the mob_db [Skotlex]
//!NOTE: Mob ID is used also as index in mob db
struct mob_db *mob_db_data[MAX_MOB_DB+1];
struct mob_db *mob_dummy = NULL;	//Dummy mob to be returned when a non-existant one is requested.

struct mob_db *mob_db(int mob_id) { if (mob_id < 0 || mob_id > MAX_MOB_DB || mob_db_data[mob_id] == NULL) return mob_dummy; return mob_db_data[mob_id]; }

//Dynamic mob chat database
struct mob_chat *mob_chat_db[MAX_MOB_CHAT+1];
struct mob_chat *mob_chat(short id) { if(id<=0 || id>MAX_MOB_CHAT || mob_chat_db[id]==NULL) return (struct mob_chat*)NULL; return mob_chat_db[id]; }

//Dynamic item drop ratio database for per-item drop ratio modifiers overriding global drop ratios.
#define MAX_ITEMRATIO_MOBS 10
struct s_mob_item_drop_ratio {
	unsigned short nameid;
	int drop_ratio;
	unsigned short mob_id[MAX_ITEMRATIO_MOBS];
};
static DBMap *mob_item_drop_ratio;

/// Mob skill struct for temporary storage
struct s_mob_skill {
	int16 mob_id; ///< Monster ID. -1 boss types, -2 normal types, -3 all monsters
	struct mob_skill skill[MAX_MOBSKILL]; ///< Skills
	uint8 count; ///< Number of skills
};
static DBMap *mob_skill_db; /// Monster skill temporary db. s_mob_skill -> mobid

struct mob_db *mobdb_exists(uint16 mob_id) {
	struct mob_db *db = mob_db(mob_id);

	if (db == mob_dummy)
		return NULL;
	return db;
}

static struct eri *item_drop_ers; //For loot drops delay structures.
static struct eri *item_drop_list_ers;

struct s_randomsummon_entry {
	uint16 mob_id;
	uint32 rate;
};

struct s_randomsummon_group {
	uint8 random_id;
	struct s_randomsummon_entry *list;
	uint16 count;
};

static DBMap *mob_summon_db; /// Random Summon DB. struct s_randomsummon_group -> group_id

//Defines the Manuk/Splendide mob groups for the status reductions [Epoque]
const int mob_manuk[8] = { MOBID_TATACHO, MOBID_CENTIPEDE, MOBID_NEPENTHES, MOBID_HILLSRION, MOBID_HARDROCK_MOMMOTH, MOBID_G_TATACHO, MOBID_G_HILLSRION, MOBID_CENTIPEDE_LARVA };
const int mob_splendide[5] = { MOBID_TENDRILRION, MOBID_CORNUS, MOBID_NAGA, MOBID_LUCIOLA_VESPA, MOBID_PINGUICULA };

/*==========================================
 * Local prototype declaration   (only required thing)
 *------------------------------------------*/
static int mob_makedummymobdb(int);
static int mob_spawn_guardian_sub(int tid, unsigned int tick, int id, intptr_t data);
int mob_skill_id2skill_idx(int mob_id,uint16 skill_id);

/*==========================================
 * Mob is searched with a name.
 *------------------------------------------*/
int mobdb_searchname(const char *str)
{
	int i;
	for(i=0;i<=MAX_MOB_DB;i++){
		struct mob_db *mob = mob_db(i);
		if(mob == mob_dummy) //Skip dummy mobs.
			continue;
		if(strcmpi(mob->name,str)==0 || strcmpi(mob->jname,str)==0 || strcmpi(mob->sprite,str)==0)
			return i;
	}
	return 0;
}
static int mobdb_searchname_array_sub(struct mob_db* mob, const char *str)
{
	if (mob == mob_dummy)
		return 1;
	if(!mob->base_exp && !mob->job_exp && mob->spawn[0].qty < 1)
		return 1; // Monsters with no base/job exp and no spawn point are, by this criteria, considered "slave mobs" and excluded from search results
	if(stristr(mob->jname,str))
		return 0;
	if(stristr(mob->name,str))
		return 0;
	return strcmpi(mob->jname,str);
}

/**
 * Create and display a tombstone on the map
 * @author [GreenBox]
 * @param md : the mob to create a tombstone for
 * @param killer : name of who has killed the mob
 * @param time : time at wich the killed happen
 */
void mvptomb_create(struct mob_data *md, char *killer, time_t time)
{
	struct npc_data *nd;

	if ( md->tomb_nid )
		mvptomb_destroy(md);

	CREATE(nd, struct npc_data, 1);

	nd->bl.id = md->tomb_nid = npc_get_new_npc_id();

	nd->ud.dir = md->ud.dir;
	nd->bl.m = md->bl.m;
	nd->bl.x = md->bl.x;
	nd->bl.y = md->bl.y;
	nd->bl.type = BL_NPC;

	safestrncpy(nd->name, msg_txt(NULL,656), sizeof(nd->name));

	nd->class_ = 565;
	nd->speed = 200;
	nd->subtype = NPCTYPE_TOMB;

	nd->u.tomb.md = md;
	nd->u.tomb.kill_time = time;

	if (killer)
		safestrncpy(nd->u.tomb.killer_name, killer, NAME_LENGTH);
	else
		nd->u.tomb.killer_name[0] = '\0';

	map_addnpc(nd->bl.m, nd);
	if(map_addblock(&nd->bl))
		return;
	status_set_viewdata(&nd->bl, nd->class_);
	status_change_init(&nd->bl);
	unit_dataset(&nd->bl);
	clif_spawn(&nd->bl);

}

/** Destroys MVP Tomb
* @param md
*/
void mvptomb_destroy(struct mob_data *md) {
	struct npc_data *nd;

	if ( (nd = map_id2nd(md->tomb_nid)) ) {
		int16 m = nd->bl.m, i;
		clif_clearunit_area(&nd->bl,CLR_OUTSIGHT);
		map_delblock(&nd->bl);

		ARR_FIND( 0, map[m].npc_num, i, map[m].npc[i] == nd );
		if( !(i == map[m].npc_num) ) {
			map[m].npc_num--;
			map[m].npc[i] = map[m].npc[map[m].npc_num];
			map[m].npc[map[m].npc_num] = NULL;
		}
		map_deliddb(&nd->bl);
		aFree(nd);
	}

	md->tomb_nid = 0;
}

/*==========================================
 * Founds up to N matches. Returns number of matches [Skotlex]
 *------------------------------------------*/
int mobdb_searchname_array(struct mob_db** data, int size, const char *str)
{
	int count = 0, i;
	struct mob_db* mob;
	for(i=0;i<=MAX_MOB_DB;i++){
		mob = mob_db(i);
		if (mob == mob_dummy || mob_is_clone(i) ) //keep clones out (or you leak player stats)
			continue;
		if (!mobdb_searchname_array_sub(mob, str)) {
			if (count < size)
				data[count] = mob;
			count++;
		}
	}
	return count;
}

/*==========================================
 * Id Mob is checked.
 *------------------------------------------*/
int mobdb_checkid(const int id)
{
	if (mob_db(id) == mob_dummy)
		return 0;
	if (mob_is_clone(id)) //checkid is used mostly for random ID based code, therefore clone mobs are out of the question.
		return 0;
	return id;
}

/*==========================================
 * Returns the view data associated to this mob class.
 *------------------------------------------*/
struct view_data * mob_get_viewdata(int mob_id)
{
	if (mob_db(mob_id) == mob_dummy)
		return 0;
	return &mob_db(mob_id)->vd;
}
/*==========================================
 * Cleans up mob-spawn data to make it "valid"
 *------------------------------------------*/
int mob_parse_dataset(struct spawn_data *data)
{
	size_t len;

	if ((!mobdb_checkid(data->id) && !mob_is_clone(data->id)) || !data->num)
		return 0;

	if( ( len = strlen(data->eventname) ) > 0 )
	{
		if( data->eventname[len-1] == '"' )
			data->eventname[len-1] = '\0'; //Remove trailing quote.
		if( data->eventname[0] == '"' ) //Strip leading quotes
			memmove(data->eventname, data->eventname+1, len-1);
	}

	if(strcmp(data->name,"--en--")==0)
		safestrncpy(data->name, mob_db(data->id)->name, sizeof(data->name));
	else if(strcmp(data->name,"--ja--")==0)
		safestrncpy(data->name, mob_db(data->id)->jname, sizeof(data->name));

	return 1;
}
/*==========================================
 * Generates the basic mob data using the spawn_data provided.
 *------------------------------------------*/
struct mob_data* mob_spawn_dataset(struct spawn_data *data)
{
	struct mob_data *md = (struct mob_data*)aCalloc(1, sizeof(struct mob_data));
	md->bl.id= npc_get_new_npc_id();
	md->bl.type = BL_MOB;
	md->bl.m = data->m;
	md->bl.x = data->x;
	md->bl.y = data->y;
	md->mob_id = data->id;
	md->state.boss = data->state.boss;
	md->db = mob_db(md->mob_id);
	if (data->level > 0 && data->level <= MAX_LEVEL)
		md->level = data->level;
	memcpy(md->name, data->name, NAME_LENGTH);
	if (data->state.ai)
		md->special_state.ai = data->state.ai;
	if (data->state.size)
		md->special_state.size = data->state.size;
	if (data->eventname[0] && strlen(data->eventname) >= 4)
		memcpy(md->npc_event, data->eventname, 50);
	if(md->db->status.mode&MD_LOOTER)
		md->lootitems = (struct s_mob_lootitem *)aCalloc(LOOTITEM_SIZE,sizeof(struct s_mob_lootitem));
	md->spawn_timer = INVALID_TIMER;
	md->deletetimer = INVALID_TIMER;
	md->skill_idx = -1;
	status_set_viewdata(&md->bl, md->mob_id);
	status_change_init(&md->bl);
	unit_dataset(&md->bl);

	map_addiddb(&md->bl);
	return md;
}

/*==========================================
 * Fetches a random mob_id [Skotlex]
 * type: Where to fetch from:
 * 0: dead branch list
 * 1: poring list
 * 2: bloody branch list
 * flag:
 * &1 : Apply the summon success chance found in the list (otherwise get any monster from the db)
 * &2 : Apply a monster check level.
 * &4 : Selected monster should not be a boss type (MD_BOSS) (except from MOBG_Bloody_Dead_Branch /swt)
 * &8 : Selected monster must have normal spawn.
 * &16: Selected monster should not be a plant type (MD_PLANT)
 * lv: Mob level to check against
 *------------------------------------------*/
int mob_get_random_id(int type, int flag, int lv)
{
	struct mob_db *mob;
	int i = 0, mob_id = 0, rand = 0;
	struct s_randomsummon_group *msummon = (struct s_randomsummon_group *)idb_get(mob_summon_db, type);
	struct s_randomsummon_entry *entry = NULL;

	if (type == MOBG_Bloody_Dead_Branch)
		flag &= ~4;
	
	if (!msummon) {
		ShowError("mob_get_random_id: Invalid type (%d) of random monster.\n", type);
		return 0;
	}
	if (!msummon->count) {
		ShowError("mob_get_random_id: Random monster type %d is not defined.\n", type);
		return 0;
	}

	do {
		rand = rnd()%msummon->count;
		entry = &msummon->list[rand];
		mob_id = entry->mob_id;
		mob = mob_db(mob_id);
	} while ((rand == 0 || // Skip default first
		mob == mob_dummy ||
		mob_is_clone(mob_id) ||
		(flag&0x01 && (entry->rate < 1000000 && entry->rate <= rnd() % 1000000)) ||
		(flag&0x02 && lv < mob->lv) ||
		(flag&0x04 && mob->status.mode&MD_BOSS) ||
		(flag&0x08 && mob->spawn[0].qty < 1) ||
		(flag&0x10 && mob->status.mode&MD_PLANT)
	) && (i++) < MAX_MOB_DB && msummon->count > 1);

	if (i >= MAX_MOB_DB && &msummon->list[0])  // no suitable monster found, use fallback for given list
		mob_id = msummon->list[0].mob_id;
	return mob_id;
}

/*==========================================
 * Kill Steal Protection [Zephyrus]
 *------------------------------------------*/
bool mob_ksprotected (struct block_list *src, struct block_list *target)
{
	struct block_list *s_bl, *t_bl;
	struct map_session_data
		*sd,    // Source
		*t_sd;  // Mob Target
	struct mob_data *md;
	unsigned int tick = gettick();

	if( !battle_config.ksprotection )
		return false; // KS Protection Disabled

	if( !(md = BL_CAST(BL_MOB,target)) )
		return false; // Tarjet is not MOB

	if( (s_bl = battle_get_master(src)) == NULL )
		s_bl = src;

	if( !(sd = BL_CAST(BL_PC,s_bl)) )
		return false; // Master is not PC

	t_bl = map_id2bl(md->target_id);
	if( !t_bl || (s_bl = battle_get_master(t_bl)) == NULL )
		s_bl = t_bl;

	t_sd = BL_CAST(BL_PC,s_bl);

	do {
		struct status_change_entry *sce;
		struct map_session_data *pl_sd; // Owner
		char output[128];
		
		if( map[md->bl.m].flag.allowks || map_flag_ks(md->bl.m) )
			return false; // Ignores GVG, PVP and AllowKS map flags

		if( md->db->mexp || md->master_id )
			return false; // MVP, Slaves mobs ignores KS

		if( (sce = md->sc.data[SC_KSPROTECTED]) == NULL )
			break; // No KS Protected

		if( sd->bl.id == sce->val1 || // Same Owner
			(sce->val2 == 2 && sd->status.party_id && sd->status.party_id == sce->val3) || // Party KS allowed
			(sce->val2 == 3 && sd->status.guild_id && sd->status.guild_id == sce->val4) ) // Guild KS allowed
			break;

		if( t_sd && (
			(sce->val2 == 1 && sce->val1 != t_sd->bl.id) ||
			(sce->val2 == 2 && sce->val3 && sce->val3 != t_sd->status.party_id) ||
			(sce->val2 == 3 && sce->val4 && sce->val4 != t_sd->status.guild_id)) )
			break;

		if( (pl_sd = map_id2sd(sce->val1)) == NULL || pl_sd->bl.m != md->bl.m )
			break;

		if( !pl_sd->state.noks )
			return false; // No KS Protected, but normal players should be protected too

		// Message to KS
		if( DIFF_TICK(sd->ks_floodprotect_tick, tick) <= 0 )
		{
			sprintf(output, "[KS Warning!! - Owner : %s]", pl_sd->status.name);
			clif_disp_onlyself(sd, output, strlen(output));

			sd->ks_floodprotect_tick = tick + 2000;
		}

		// Message to Owner
		if( DIFF_TICK(pl_sd->ks_floodprotect_tick, tick) <= 0 )
		{
			sprintf(output, "[Watch out! %s is trying to KS you!]", sd->status.name);
			clif_disp_onlyself(pl_sd, output, strlen(output));

			pl_sd->ks_floodprotect_tick = tick + 2000;
		}

		return true;
	} while(0);

	status_change_start(src, target, SC_KSPROTECTED, 10000, sd->bl.id, sd->state.noks, sd->status.party_id, sd->status.guild_id, battle_config.ksprotection, SCSTART_NOAVOID);

	return false;
}

struct mob_data *mob_once_spawn_sub(struct block_list *bl, int16 m, int16 x, int16 y, const char *mobname, int mob_id, const char *event, unsigned int size, unsigned int ai)
{
	struct spawn_data data;

	memset(&data, 0, sizeof(struct spawn_data));
	data.m = m;
	data.num = 1;
	data.id = mob_id;
	data.state.size = size;
	data.state.ai = ai;

	if (mobname)
		safestrncpy(data.name, mobname, sizeof(data.name));
	else
		if (battle_config.override_mob_names == 1)
			strcpy(data.name, "--en--");
		else
			strcpy(data.name, "--ja--");

	if (event)
		safestrncpy(data.eventname, event, sizeof(data.eventname));

	// Locate spot next to player.
	if (bl && (x < 0 || y < 0))
		map_search_freecell(bl, m, &x, &y, 1, 1, 0);

	// if none found, pick random position on map
	if (x <= 0 || x >= map[m].xs || y <= 0 || y >= map[m].ys)
		map_search_freecell(NULL, m, &x, &y, -1, -1, 1);

	data.x = x;
	data.y = y;

	if (!mob_parse_dataset(&data))
		return NULL;

	return mob_spawn_dataset(&data);
}

/*==========================================
 * Spawn a single mob on the specified coordinates.
 *------------------------------------------*/
int mob_once_spawn(struct map_session_data* sd, int16 m, int16 x, int16 y, const char* mobname, int mob_id, int amount, const char* event, unsigned int size, unsigned int ai)
{
	struct mob_data* md = NULL;
	int count, lv;

	if (m < 0 || amount <= 0)
		return 0; // invalid input

	lv = (sd) ? sd->status.base_level : 255;

	for (count = 0; count < amount; count++)
	{
		int c = (mob_id >= 0) ? mob_id : mob_get_random_id(-mob_id - 1, (battle_config.random_monster_checklv) ? 3 : 1, lv);
		md = mob_once_spawn_sub((sd) ? &sd->bl : NULL, m, x, y, mobname, c, event, size, ai);

		if (!md)
			continue;

		if (mob_id == MOBID_EMPERIUM)
		{
			struct guild_castle* gc = guild_mapindex2gc(map[m].index);
			struct guild* g = (gc) ? guild_search(gc->guild_id) : NULL;
			if (gc)
			{
				md->guardian_data = (struct guardian_data*)aCalloc(1, sizeof(struct guardian_data));
				md->guardian_data->castle = gc;
				md->guardian_data->number = MAX_GUARDIANS;
				md->guardian_data->guild_id = gc->guild_id;
				if (g)
				{
					md->guardian_data->emblem_id = g->emblem_id;
					memcpy(md->guardian_data->guild_name, g->name, NAME_LENGTH);
				}
				else if (gc->guild_id) //Guild not yet available, retry in 5.
					add_timer(gettick()+5000,mob_spawn_guardian_sub,md->bl.id,md->guardian_data->guild_id);
			}
		}	// end addition [Valaris]

		mob_spawn(md);

		if (mob_id < 0 && battle_config.dead_branch_active)
			//Behold Aegis's masterful decisions yet again...
			//"I understand the "Aggressive" part, but the "Can Move" and "Can Attack" is just stupid" - Poki#3
			sc_start4(NULL,&md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE|MD_CANATTACK|MD_CANMOVE|MD_ANGRY, 0, 60000);
	}

	return (md) ? md->bl.id : 0; // id of last spawned mob
}

/*==========================================
 * Spawn mobs in the specified area.
 *------------------------------------------*/
int mob_once_spawn_area(struct map_session_data* sd, int16 m, int16 x0, int16 y0, int16 x1, int16 y1, const char* mobname, int mob_id, int amount, const char* event, unsigned int size, unsigned int ai)
{
	int i, max, id = 0;
	int lx = -1, ly = -1;

	if (m < 0 || amount <= 0)
		return 0; // invalid input

	// normalize x/y coordinates
	if (x0 > x1)
		swap(x0, x1);
	if (y0 > y1)
		swap(y0, y1);

	// choose a suitable max. number of attempts
	max = (y1 - y0 + 1)*(x1 - x0 + 1)*3;
	if (max > 1000)
		max = 1000;

	// spawn mobs, one by one
	for (i = 0; i < amount; i++)
	{
		int x, y;
		int j = 0;

		// find a suitable map cell
		do {
			x = rnd()%(x1-x0+1)+x0;
			y = rnd()%(y1-y0+1)+y0;
			j++;
		} while (map_getcell(m,x,y,CELL_CHKNOPASS) && j < max);

		if (j == max)
		{// attempt to find an available cell failed
			if (lx == -1 && ly == -1)
				return 0; // total failure

			// fallback to last good x/y pair
			x = lx;
			y = ly;
		}

		// record last successful coordinates
		lx = x;
		ly = y;

		id = mob_once_spawn(sd, m, x, y, mobname, mob_id, 1, event, size, ai);
	}

	return id; // id of last spawned mob
}
/*==========================================
 * Set a Guardian's guild data [Skotlex]
 *------------------------------------------*/
static int mob_spawn_guardian_sub(int tid, unsigned int tick, int id, intptr_t data)
{	//Needed because the guild_data may not be available at guardian spawn time.
	struct block_list* bl = map_id2bl(id);
	struct mob_data* md;
	struct guild* g;
	int guardup_lv;

	if (bl == NULL) //It is possible mob was already removed from map when the castle has no owner. [Skotlex]
		return 0;

	if (bl->type != BL_MOB)
	{
		ShowError("mob_spawn_guardian_sub: Block error!\n");
		return 0;
	}

	md = (struct mob_data*)bl;
	nullpo_ret(md->guardian_data);
	g = guild_search((int)data);

	if (g == NULL)
	{	//Liberate castle, if the guild is not found this is an error! [Skotlex]
		ShowError("mob_spawn_guardian_sub: Couldn't load guild %d!\n", (int)data);
		if (md->mob_id == MOBID_EMPERIUM)
		{	//Not sure this is the best way, but otherwise we'd be invoking this for ALL guardians spawned later on.
			md->guardian_data->guild_id = 0;
			if (md->guardian_data->castle->guild_id) //Free castle up.
			{
				ShowNotice("Clearing ownership of castle %d (%s)\n", md->guardian_data->castle->castle_id, md->guardian_data->castle->castle_name);
				guild_castledatasave(md->guardian_data->castle->castle_id, 1, 0);
			}
		} else {
			if (md->guardian_data->number >= 0 && md->guardian_data->number < MAX_GUARDIANS && md->guardian_data->castle->guardian[md->guardian_data->number].visible)
				guild_castledatasave(md->guardian_data->castle->castle_id, 10+md->guardian_data->number,0);
			unit_free(&md->bl,CLR_OUTSIGHT); //Remove guardian.
		}
		return 0;
	}
	guardup_lv = guild_checkskill(g,GD_GUARDUP);
	md->guardian_data->emblem_id = g->emblem_id;
	memcpy(md->guardian_data->guild_name, g->name, NAME_LENGTH);
	md->guardian_data->guardup_lv = guardup_lv;
	if( guardup_lv )
		status_calc_mob(md, SCO_NONE); //Give bonuses.
	return 0;
}

/*==========================================
 * Summoning Guardians [Valaris]
 *------------------------------------------*/
int mob_spawn_guardian(const char* mapname, short x, short y, const char* mobname, int mob_id, const char* event, int guardian, bool has_index)
{
	struct mob_data *md=NULL;
	struct spawn_data data;
	struct guild *g=NULL;
	struct guild_castle *gc;
	int16 m;
	memset(&data, 0, sizeof(struct spawn_data));
	data.num = 1;

	m=map_mapname2mapid(mapname);

	if(m<0)
	{
		ShowWarning("mob_spawn_guardian: Map [%s] not found.\n", mapname);
		return 0;
	}
	data.m = m;
	data.num = 1;
	if(mob_id<=0) {
		mob_id = mob_get_random_id(-mob_id-1, 1, 99);
		if (!mob_id) return 0;
	}

	data.id = mob_id;

	if( !has_index )
	{
		guardian = -1;
	}
	else if( guardian < 0 || guardian >= MAX_GUARDIANS )
	{
		ShowError("mob_spawn_guardian: Invalid guardian index %d for guardian %d (castle map %s)\n", guardian, mob_id, map[m].name);
		return 0;
	}

	if((x<=0 || y<=0) && !map_search_freecell(NULL, m, &x, &y, -1,-1, 1))
	{
		ShowWarning("mob_spawn_guardian: Couldn't locate a spawn cell for guardian class %d (index %d) at castle map %s\n",mob_id, guardian, map[m].name);
		return 0;
	}
	data.x = x;
	data.y = y;
	safestrncpy(data.name, mobname, sizeof(data.name));
	safestrncpy(data.eventname, event, sizeof(data.eventname));
	if (!mob_parse_dataset(&data))
		return 0;

	gc=guild_mapname2gc(map[m].name);
	if (gc == NULL)
	{
		ShowError("mob_spawn_guardian: No castle set at map %s\n", map[m].name);
		return 0;
	}
	if (!gc->guild_id)
		ShowWarning("mob_spawn_guardian: Spawning guardian %d on a castle with no guild (castle map %s)\n", mob_id, map[m].name);
	else
		g = guild_search(gc->guild_id);

	if( has_index && gc->guardian[guardian].id )
  	{	//Check if guardian already exists, refuse to spawn if so.
		struct mob_data *md2 = (TBL_MOB*)map_id2bl(gc->guardian[guardian].id);
		if (md2 && md2->bl.type == BL_MOB &&
			md2->guardian_data && md2->guardian_data->number == guardian)
		{
			ShowError("mob_spawn_guardian: Attempted to spawn guardian in position %d which already has a guardian (castle map %s)\n", guardian, map[m].name);
			return 0;
		}
	}

	md = mob_spawn_dataset(&data);
	md->guardian_data = (struct guardian_data*)aCalloc(1, sizeof(struct guardian_data));
	md->guardian_data->number = guardian;
	md->guardian_data->guild_id = gc->guild_id;
	md->guardian_data->castle = gc;
	if( has_index )
	{// permanent guardian
		gc->guardian[guardian].id = md->bl.id;
	}
	else
	{// temporary guardian
		int i;
		ARR_FIND(0, gc->temp_guardians_max, i, gc->temp_guardians[i] == 0);
		if( i == gc->temp_guardians_max )
		{
			++(gc->temp_guardians_max);
			RECREATE(gc->temp_guardians, int, gc->temp_guardians_max);
		}
		gc->temp_guardians[i] = md->bl.id;
	}
	if (g)
	{
		md->guardian_data->emblem_id = g->emblem_id;
		memcpy (md->guardian_data->guild_name, g->name, NAME_LENGTH);
		md->guardian_data->guardup_lv = guild_checkskill(g,GD_GUARDUP);
	} else if (md->guardian_data->guild_id)
		add_timer(gettick()+5000,mob_spawn_guardian_sub,md->bl.id,md->guardian_data->guild_id);
	mob_spawn(md);

	return md->bl.id;
}

/*==========================================
 * Summoning BattleGround [Zephyrus]
 *------------------------------------------*/
int mob_spawn_bg(const char* mapname, short x, short y, const char* mobname, int mob_id, const char* event, unsigned int bg_id)
{
	struct mob_data *md = NULL;
	struct spawn_data data;
	int16 m;

	if( (m = map_mapname2mapid(mapname)) < 0 )
	{
		ShowWarning("mob_spawn_bg: Map [%s] not found.\n", mapname);
		return 0;
	}

	memset(&data, 0, sizeof(struct spawn_data));
	data.m = m;
	data.num = 1;
	if( mob_id <= 0 )
	{
		mob_id = mob_get_random_id(-mob_id-1,1,99);
		if( !mob_id ) return 0;
	}

	data.id = mob_id;
	if( (x <= 0 || y <= 0) && !map_search_freecell(NULL, m, &x, &y, -1,-1, 1) )
	{
		ShowWarning("mob_spawn_bg: Couldn't locate a spawn cell for guardian class %d (bg_id %d) at map %s\n",mob_id, bg_id, map[m].name);
		return 0;
	}

	data.x = x;
	data.y = y;
	safestrncpy(data.name, mobname, sizeof(data.name));
	safestrncpy(data.eventname, event, sizeof(data.eventname));
	if( !mob_parse_dataset(&data) )
		return 0;

	md = mob_spawn_dataset(&data);
	mob_spawn(md);
	md->bg_id = bg_id; // BG Team ID

	return md->bl.id;
}

/*==========================================
 * Reachability to a Specification ID existence place
 * state indicates type of 'seek' mob should do:
 * - MSS_LOOT: Looking for item, path must be easy.
 * - MSS_RUSH: Chasing attacking player, path is complex
 * - MSS_FOLLOW: Initiative/support seek, path is complex
 *------------------------------------------*/
int mob_can_reach(struct mob_data *md,struct block_list *bl,int range, int state)
{
	int easy = 0;

	nullpo_ret(md);
	nullpo_ret(bl);
	switch (state) {
		case MSS_RUSH:
		case MSS_FOLLOW:
			easy = 0; //(battle_config.mob_ai&0x1?0:1);
			break;
		case MSS_LOOT:
		default:
			easy = 1;
			break;
	}
	return unit_can_reach_bl(&md->bl, bl, range, easy, NULL, NULL);
}

/*==========================================
 * Links nearby mobs (supportive mobs)
 *------------------------------------------*/
int mob_linksearch(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	int mob_id;
	struct block_list *target;
	unsigned int tick;

	nullpo_ret(bl);
	md=(struct mob_data *)bl;
	mob_id = va_arg(ap, int);
	target = va_arg(ap, struct block_list *);
	tick=va_arg(ap, unsigned int);

	if (md->mob_id == mob_id && DIFF_TICK(md->last_linktime, tick) < MIN_MOBLINKTIME
		&& !md->target_id)
	{
		md->last_linktime = tick;
		if( mob_can_reach(md,target,md->db->range2, MSS_FOLLOW) ){	// Reachability judging
			md->target_id = target->id;
			md->min_chase=md->db->range3;
			return 1;
		}
	}

	return 0;
}

/*==========================================
 * mob spawn with delay (timer function)
 *------------------------------------------*/
int mob_delayspawn(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list* bl = map_id2bl(id);
	struct mob_data* md = BL_CAST(BL_MOB, bl);

	if( md )
	{
		if( md->spawn_timer != tid )
		{
			ShowError("mob_delayspawn: Timer mismatch: %d != %d\n", tid, md->spawn_timer);
			return 0;
		}
		md->spawn_timer = INVALID_TIMER;
		mob_spawn(md);
	}
	return 0;
}

/*==========================================
 * spawn timing calculation
 *------------------------------------------*/
int mob_setdelayspawn(struct mob_data *md)
{
	unsigned int spawntime, mode;
	struct mob_db *db;

	if (!md->spawn) //Doesn't has respawn data!
		return unit_free(&md->bl,CLR_DEAD);

	spawntime = md->spawn->delay1; //Base respawn time
	if (md->spawn->delay2) //random variance
		spawntime+= rnd()%md->spawn->delay2;

	//Apply the spawn delay fix [Skotlex]
	db = mob_db(md->spawn->id);
	mode = db->status.mode;
	if (mode & MD_BOSS) {	//Bosses
		if (battle_config.boss_spawn_delay != 100) {
			// Divide by 100 first to prevent overflows
			//(precision loss is minimal as duration is in ms already)
			spawntime = spawntime/100*battle_config.boss_spawn_delay;
		}
	} else if (mode&MD_PLANT) {	//Plants
		if (battle_config.plant_spawn_delay != 100) {
			spawntime = spawntime/100*battle_config.plant_spawn_delay;
		}
	} else if (battle_config.mob_spawn_delay != 100) {	//Normal mobs
		spawntime = spawntime/100*battle_config.mob_spawn_delay;
	}

	if (spawntime < 5000) //Monsters should never respawn faster than within 5 seconds
		spawntime = 5000;

	if( md->spawn_timer != INVALID_TIMER )
		delete_timer(md->spawn_timer, mob_delayspawn);
	md->spawn_timer = add_timer(gettick()+spawntime, mob_delayspawn, md->bl.id, 0);
	return 0;
}

int mob_count_sub(struct block_list *bl, va_list ap) {
    int mobid[10], i;
    ARR_FIND(0, 10, i, (mobid[i] = va_arg(ap, int)) == 0); //fetch till 0
    if (mobid[0]) { //if there one let's check it otherwise go backward
        TBL_MOB *md = BL_CAST(BL_MOB, bl);
        ARR_FIND(0, 10, i, md->mob_id == mobid[i]);
        return (i < 10) ? 1 : 0;
    }
    return 1; //backward compatibility
}

/**
 * Mob spawning. Initialization is also variously here. (Spawn a mob in a map)
 * @param md : mob data to spawn
 * @return 0:spawned, 1:delayed, 2:error
 */
int mob_spawn (struct mob_data *md)
{
	int i=0;
	unsigned int tick = gettick();
	int c =0;

	md->last_thinktime = tick;
	if (md->bl.prev != NULL)
		unit_remove_map(&md->bl,CLR_RESPAWN);
	else
	if (md->spawn && md->mob_id != md->spawn->id)
	{
		md->mob_id = md->spawn->id;
		status_set_viewdata(&md->bl, md->mob_id);
		md->db = mob_db(md->mob_id);
		memcpy(md->name,md->spawn->name,NAME_LENGTH);
	}

	if (md->spawn) { //Respawn data
		md->bl.m = md->spawn->m;
		md->bl.x = md->spawn->x;
		md->bl.y = md->spawn->y;

		if( (md->bl.x == 0 && md->bl.y == 0) || md->spawn->xs || md->spawn->ys )
		{	//Monster can be spawned on an area.
			if( !map_search_freecell(&md->bl, -1, &md->bl.x, &md->bl.y, md->spawn->xs, md->spawn->ys, battle_config.no_spawn_on_player?4:0) )
			{ // retry again later
				if( md->spawn_timer != INVALID_TIMER )
					delete_timer(md->spawn_timer, mob_delayspawn);
				md->spawn_timer = add_timer(tick+5000,mob_delayspawn,md->bl.id,0);
				return 1;
			}
		}
		else if( battle_config.no_spawn_on_player > 99 && map_foreachinrange(mob_count_sub, &md->bl, AREA_SIZE, BL_PC) )
		{ // retry again later (players on sight)
			if( md->spawn_timer != INVALID_TIMER )
				delete_timer(md->spawn_timer, mob_delayspawn);
			md->spawn_timer = add_timer(tick+5000,mob_delayspawn,md->bl.id,0);
			return 1;
		}
	}

	memset(&md->state, 0, sizeof(md->state));
	status_calc_mob(md, SCO_FIRST);
	md->attacked_id = 0;
	md->target_id = 0;
	md->move_fail_count = 0;
	md->ud.state.attack_continue = 0;
	md->ud.target_to = 0;
	md->ud.dir = 0;
	if( md->spawn_timer != INVALID_TIMER ) {
		delete_timer(md->spawn_timer, mob_delayspawn);
		md->spawn_timer = INVALID_TIMER;
	}

//	md->master_id = 0;
	md->master_dist = 0;

	md->state.aggressive = md->status.mode&MD_ANGRY?1:0;
	md->state.skillstate = MSS_IDLE;
	md->next_walktime = tick+rnd()%1000+MIN_RANDOMWALKTIME;
	md->last_linktime = tick;
	md->dmgtick = tick - 5000;
	md->last_pcneartime = 0;

	for (i = 0, c = tick-MOB_MAX_DELAY; i < MAX_MOBSKILL; i++)
		md->skilldelay[i] = c;

	memset(md->dmglog, 0, sizeof(md->dmglog));
	md->tdmg = 0;

	if (md->lootitems)
		memset(md->lootitems, 0, sizeof(*md->lootitems));

	md->lootitem_count = 0;

	if(md->db->option)
		// Added for carts, falcons and pecos for cloned monsters. [Valaris]
		md->sc.option = md->db->option;

	// MvP tomb [GreenBox]
	if ( md->tomb_nid )
		mvptomb_destroy(md);

	if(map_addblock(&md->bl))
		return 2;
	if( map[md->bl.m].users )
		clif_spawn(&md->bl);
	skill_unit_move(&md->bl,tick,1);
	mobskill_use(md, tick, MSC_SPAWN);
	return 0;
}

/*==========================================
 * Determines if the mob can change target. [Skotlex]
 *------------------------------------------*/
static int mob_can_changetarget(struct mob_data* md, struct block_list* target, int mode)
{
	// if the monster was provoked ignore the above rule [celest]
	if(md->state.provoke_flag)
	{
		if (md->state.provoke_flag == target->id)
			return 1;
		else if (!(battle_config.mob_ai&0x4))
			return 0;
	}

	switch (md->state.skillstate) {
		case MSS_BERSERK:
			if (!(mode&MD_CHANGETARGET_MELEE))
				return 0;
			return (battle_config.mob_ai&0x4 || check_distance_bl(&md->bl, target, 3));
		case MSS_RUSH:
			return (mode&MD_CHANGETARGET_CHASE);
		case MSS_FOLLOW:
		case MSS_ANGRY:
		case MSS_IDLE:
		case MSS_WALK:
		case MSS_LOOT:
			return 1;
		default:
			return 0;
	}
}

/*==========================================
 * Determination for an attack of a monster
 *------------------------------------------*/
int mob_target(struct mob_data *md,struct block_list *bl,int dist)
{
	nullpo_ret(md);
	nullpo_ret(bl);

	// Nothing will be carried out if there is no mind of changing TAGE by TAGE ending.
	if(md->target_id && !mob_can_changetarget(md, bl, status_get_mode(&md->bl)))
		return 0;

	if(!status_check_skilluse(&md->bl, bl, 0, 0))
		return 0;

	md->target_id = bl->id;	// Since there was no disturbance, it locks on to target.
	if (md->state.provoke_flag && bl->id != md->state.provoke_flag)
		md->state.provoke_flag = 0;
	md->min_chase=dist+md->db->range3;
	if(md->min_chase>MAX_MINCHASE)
		md->min_chase=MAX_MINCHASE;
	return 0;
}

/*==========================================
 * The ?? routine of an active monster
 *------------------------------------------*/
static int mob_ai_sub_hard_activesearch(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	struct block_list **target;
	int mode;
	int dist;

	nullpo_ret(bl);
	md=va_arg(ap,struct mob_data *);
	target= va_arg(ap,struct block_list**);
	mode= va_arg(ap,int);

	//If can't seek yet, not an enemy, or you can't attack it, skip.
	if ((*target) == bl || !status_check_skilluse(&md->bl, bl, 0, 0))
		return 0;

	if ((mode&MD_TARGETWEAK) && status_get_lv(bl) >= md->level-5)
		return 0;

	if(battle_check_target(&md->bl,bl,BCT_ENEMY)<=0)
		return 0;

	switch (bl->type)
	{
	case BL_PC:
		if (((TBL_PC*)bl)->state.gangsterparadise &&
			!(status_get_mode(&md->bl)&MD_BOSS))
			return 0; //Gangster paradise protection.
	default:
		if (battle_config.hom_setting&HOMSET_FIRST_TARGET &&
			(*target) && (*target)->type == BL_HOM && bl->type != BL_HOM)
			return 0; //For some reason Homun targets are never overriden.

		dist = distance_bl(&md->bl, bl);
		if(
			((*target) == NULL || !check_distance_bl(&md->bl, *target, dist)) &&
			battle_check_range(&md->bl,bl,md->db->range2)
		) { //Pick closest target?
#ifdef ACTIVEPATHSEARCH
			struct walkpath_data wpd;
			if (!path_search(&wpd, md->bl.m, md->bl.x, md->bl.y, bl->x, bl->y, 0, CELL_CHKWALL)) // Count walk path cells
				return 0;
			//Standing monsters use range2, walking monsters use range3
			if ((md->ud.walktimer == INVALID_TIMER && wpd.path_len > md->db->range2)
				|| (md->ud.walktimer != INVALID_TIMER && wpd.path_len > md->db->range3))
				return 0;
#endif
			(*target) = bl;
			md->target_id=bl->id;
			md->min_chase= dist + md->db->range3;
			if(md->min_chase>MAX_MINCHASE)
				md->min_chase=MAX_MINCHASE;
			return 1;
		}
		break;
	}
	return 0;
}

/*==========================================
 * chase target-change routine.
 *------------------------------------------*/
static int mob_ai_sub_hard_changechase(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	struct block_list **target;

	nullpo_ret(bl);
	md=va_arg(ap,struct mob_data *);
	target= va_arg(ap,struct block_list**);

	//If can't seek yet, not an enemy, or you can't attack it, skip.
	if ((*target) == bl ||
		battle_check_target(&md->bl,bl,BCT_ENEMY)<=0 ||
	  	!status_check_skilluse(&md->bl, bl, 0, 0))
		return 0;

	if(battle_check_range (&md->bl, bl, md->status.rhw.range))
	{
		(*target) = bl;
		md->target_id=bl->id;
		md->min_chase= md->db->range3;
	}
	return 1;
}

/*==========================================
 * finds nearby bg ally for guardians looking for users to follow.
 *------------------------------------------*/
static int mob_ai_sub_hard_bg_ally(struct block_list *bl,va_list ap) {
	struct mob_data *md;
	struct block_list **target;

	nullpo_ret(bl);
	md=va_arg(ap,struct mob_data *);
	target= va_arg(ap,struct block_list**);

	if( status_check_skilluse(&md->bl, bl, 0, 0) && battle_check_target(&md->bl,bl,BCT_ENEMY)<=0 ) {
		(*target) = bl;
	}
	return 1;
}

/*==========================================
 * loot monster item search
 *------------------------------------------*/
static int mob_ai_sub_hard_lootsearch(struct block_list *bl,va_list ap)
{
	struct mob_data* md;
	struct block_list **target;
	int dist;

	md = va_arg(ap,struct mob_data *);
	target = va_arg(ap,struct block_list**);

	dist = distance_bl(&md->bl, bl);
	if (mob_can_reach(md,bl,dist+1, MSS_LOOT) && (
		(*target) == NULL ||
		(battle_config.monster_loot_search_type && md->target_id > bl->id) ||
		(!battle_config.monster_loot_search_type && !check_distance_bl(&md->bl, *target, dist)) // New target closer than previous one.
		))
	{
		(*target) = bl;
		md->target_id = bl->id;
		md->min_chase = md->db->range3;
	}
	else if (!battle_config.monster_loot_search_type)
		mob_stop_walking(md, 1); // Stop walking immediately if item is no longer on the ground.
	return 0;
}

static int mob_warpchase_sub(struct block_list *bl,va_list ap) {
	struct block_list *target;
	struct npc_data **target_nd;
	struct npc_data *nd;
	int *min_distance;
	int cur_distance;

	target= va_arg(ap, struct block_list*);
	target_nd= va_arg(ap, struct npc_data**);
	min_distance= va_arg(ap, int*);

	nd = (TBL_NPC*) bl;

	if(nd->subtype != NPCTYPE_WARP)
		return 0; //Not a warp

	if(nd->u.warp.mapindex != map[target->m].index)
		return 0; //Does not lead to the same map.

	cur_distance = distance_blxy(target, nd->u.warp.x, nd->u.warp.y);
	if (cur_distance < *min_distance)
	{	//Pick warp that leads closest to target.
		*target_nd = nd;
		*min_distance = cur_distance;
		return 1;
	}
	return 0;
}
/*==========================================
 * Processing of slave monsters
 *------------------------------------------*/
static int mob_ai_sub_hard_slavemob(struct mob_data *md,unsigned int tick)
{
	struct block_list *bl;

	bl=map_id2bl(md->master_id);

	if (!bl || status_isdead(bl)) {
		status_kill(&md->bl);
		return 1;
	}
	if (bl->prev == NULL)
		return 0; //Master not on a map? Could be warping, do not process.

	if(status_get_mode(&md->bl)&MD_CANMOVE)
	{	//If the mob can move, follow around. [Check by Skotlex]
		int old_dist;

		// Distance with between slave and master is measured.
		old_dist=md->master_dist;
		md->master_dist=distance_bl(&md->bl, bl);

		// Since the master was in near immediately before, teleport is carried out and it pursues.
		if(bl->m != md->bl.m ||
			(old_dist<10 && md->master_dist>18) ||
			md->master_dist > MAX_MINCHASE
		){
			md->master_dist = 0;
			unit_warp(&md->bl,bl->m,bl->x,bl->y,CLR_TELEPORT);
			return 1;
		}

		if(md->target_id) //Slave is busy with a target.
			return 0;

		// Approach master if within view range, chase back to Master's area also if standing on top of the master.
		if((md->master_dist>MOB_SLAVEDISTANCE || md->master_dist == 0) &&
			unit_can_move(&md->bl))
		{
			short x = bl->x, y = bl->y;
			mob_stop_attack(md);
			if(map_search_freecell(&md->bl, bl->m, &x, &y, MOB_SLAVEDISTANCE, MOB_SLAVEDISTANCE, 1)
				&& unit_walktoxy(&md->bl, x, y, 0))
				return 1;
		}
	} else if (bl->m != md->bl.m && map_flag_gvg(md->bl.m)) {
		//Delete the summoned mob if it's in a gvg ground and the master is elsewhere. [Skotlex]
		status_kill(&md->bl);
		return 1;
	}

	//Avoid attempting to lock the master's target too often to avoid unnecessary overload. [Skotlex]
	if (DIFF_TICK(md->last_linktime, tick) < MIN_MOBLINKTIME && !md->target_id)
  	{
		struct unit_data *ud = unit_bl2ud(bl);
		md->last_linktime = tick;

		if (ud) {
			struct block_list *tbl=NULL;
			if (ud->target && ud->state.attack_continue)
				tbl=map_id2bl(ud->target);
			else if (ud->skilltarget) {
				tbl = map_id2bl(ud->skilltarget);
				//Required check as skilltarget is not always an enemy. [Skotlex]
				if (tbl && battle_check_target(&md->bl, tbl, BCT_ENEMY) <= 0)
					tbl = NULL;
			}
			if (tbl && status_check_skilluse(&md->bl, tbl, 0, 0)) {
				md->target_id=tbl->id;
				md->min_chase=md->db->range3+distance_bl(&md->bl, tbl);
				if(md->min_chase>MAX_MINCHASE)
					md->min_chase=MAX_MINCHASE;
				return 1;
			}
		}
	}
	return 0;
}

/*==========================================
 * A lock of target is stopped and mob moves to a standby state.
 * This also triggers idle skill/movement since the AI can get stuck
 * when trying to pick new targets when the current chosen target is
 * unreachable.
 *------------------------------------------*/
int mob_unlocktarget(struct mob_data *md, unsigned int tick)
{
	nullpo_ret(md);

	switch (md->state.skillstate) {
	case MSS_WALK:
		if (md->ud.walktimer != INVALID_TIMER)
			break;
		//Because it is not unset when the mob finishes walking.
		md->state.skillstate = MSS_IDLE;
	case MSS_IDLE:
		// Idle skill.
		if (!(++md->ud.walk_count%IDLE_SKILL_INTERVAL) && mobskill_use(md, tick, -1))
			break;
		//Random walk.
		if (!md->master_id &&
			DIFF_TICK(md->next_walktime, tick) <= 0 &&
			!mob_randomwalk(md,tick))
			//Delay next random walk when this one failed.
			md->next_walktime = tick+rnd()%1000;
		break;
	default:
		mob_stop_attack(md);
		mob_stop_walking(md,1); //Stop chasing.
		md->state.skillstate = MSS_IDLE;
		if(battle_config.mob_ai&0x8) //Walk instantly after dropping target
			md->next_walktime = tick+rnd()%1000;
		else
			md->next_walktime = tick+rnd()%1000+MIN_RANDOMWALKTIME;
		break;
	}
	if (md->target_id) {
		md->target_id=0;
		md->ud.target_to = 0;
		unit_set_target(&md->ud, 0);
	}
	if(battle_config.official_cell_stack_limit > 0
		&& map_count_oncell(md->bl.m, md->bl.x, md->bl.y, BL_CHAR|BL_NPC, 1) > battle_config.official_cell_stack_limit) {
		unit_walktoxy(&md->bl, md->bl.x, md->bl.y, 8);
	}

	return 0;
}
/*==========================================
 * Random walk
 *------------------------------------------*/
int mob_randomwalk(struct mob_data *md,unsigned int tick)
{
	const int retrycount=20;
	int i,c,d;
	int speed;

	nullpo_ret(md);

	if(DIFF_TICK(md->next_walktime,tick)>0 ||
	   (status_get_mode(&md->bl)&MD_NORANDOM_WALK) ||
	   !unit_can_move(&md->bl) ||
	   !(status_get_mode(&md->bl)&MD_CANMOVE))
		return 0;

	d =12-md->move_fail_count;
	if(d<5) d=5;
	if(d>7) d=7;
	for(i=0;i<retrycount;i++){	// Search of a movable place
		int r=rnd();
		int x=r%(d*2+1)-d;
		int y=r/(d*2+1)%(d*2+1)-d;
		x+=md->bl.x;
		y+=md->bl.y;

		if(((x != md->bl.x) || (y != md->bl.y)) && map_getcell(md->bl.m,x,y,CELL_CHKPASS) && unit_walktoxy(&md->bl,x,y,8)){
			break;
		}
	}
	if(i==retrycount){
		md->move_fail_count++;
		if(md->move_fail_count>1000){
			ShowWarning("MOB can't move. random spawn %d, class = %d, at %s (%d,%d)\n",md->bl.id,md->mob_id,map[md->bl.m].name, md->bl.x, md->bl.y);
			md->move_fail_count=0;
			mob_spawn(md);
		}
		return 0;
	}
	speed=status_get_speed(&md->bl);
	for(i=c=0;i<md->ud.walkpath.path_len;i++){	// The next walk start time is calculated.
		if(md->ud.walkpath.path[i]&1)
			c+=speed*MOVE_DIAGONAL_COST/MOVE_COST;
		else
			c+=speed;
	}
	md->state.skillstate=MSS_WALK;
	md->move_fail_count=0;
	md->next_walktime = tick+rnd()%1000+MIN_RANDOMWALKTIME+c;
	return 1;
}

int mob_warpchase(struct mob_data *md, struct block_list *target)
{
	struct npc_data *warp = NULL;
	int distance = AREA_SIZE;
	if (!(target && battle_config.mob_ai&0x40 && battle_config.mob_warp&1))
		return 0; //Can't warp chase.

	if (target->m == md->bl.m && check_distance_bl(&md->bl, target, AREA_SIZE))
		return 0; //No need to do a warp chase.

	if (md->ud.walktimer != INVALID_TIMER &&
		map_getcell(md->bl.m,md->ud.to_x,md->ud.to_y,CELL_CHKNPC))
		return 1; //Already walking to a warp.

	//Search for warps within mob's viewing range.
	map_foreachinrange (mob_warpchase_sub, &md->bl,
		md->db->range2, BL_NPC, target, &warp, &distance);

	if (warp && unit_walktobl(&md->bl, &warp->bl, 1, 1))
		return 1;
	return 0;
}

/*==========================================
 * AI of MOB whose is near a Player
 *------------------------------------------*/
static bool mob_ai_sub_hard(struct mob_data *md, unsigned int tick)
{
	struct block_list *tbl = NULL, *abl = NULL;
	int mode;
	int view_range, can_move;

	if(md->bl.prev == NULL || md->status.hp == 0)
		return false;

	if (DIFF_TICK(tick, md->last_thinktime) < MIN_MOBTHINKTIME)
		return false;

	md->last_thinktime = tick;

	if (md->ud.skilltimer != INVALID_TIMER)
		return false;

	// Abnormalities
	if(( md->sc.opt1 > 0 && md->sc.opt1 != OPT1_STONEWAIT && md->sc.opt1 != OPT1_BURNING && md->sc.opt1 != OPT1_CRYSTALIZE )
	   || md->sc.data[SC_BLADESTOP] || md->sc.data[SC__MANHOLE] || md->sc.data[SC_CURSEDCIRCLE_TARGET]) {//Should reset targets.
		md->target_id = md->attacked_id = 0;
		return false;
	}

	if (md->sc.count && md->sc.data[SC_BLIND])
		view_range = 3;
	else
		view_range = md->db->range2;
	mode = status_get_mode(&md->bl);

	can_move = (mode&MD_CANMOVE) && unit_can_move(&md->bl);

	if (md->target_id)
	{	//Check validity of current target. [Skotlex]
		tbl = map_id2bl(md->target_id);
		if (!tbl || tbl->m != md->bl.m ||
			(md->ud.attacktimer == INVALID_TIMER && !status_check_skilluse(&md->bl, tbl, 0, 0)) ||
			(md->ud.walktimer != INVALID_TIMER && !(battle_config.mob_ai&0x1) && !check_distance_bl(&md->bl, tbl, md->min_chase)) ||
			(
				tbl->type == BL_PC &&
				((((TBL_PC*)tbl)->state.gangsterparadise && !(mode&MD_BOSS)) ||
				((TBL_PC*)tbl)->invincible_timer != INVALID_TIMER)
		)) {	//No valid target
			if (mob_warpchase(md, tbl))
				return true; //Chasing this target.
			if(md->ud.walktimer != INVALID_TIMER && (!can_move || md->ud.walkpath.path_pos <= battle_config.mob_chase_refresh)
				&& (tbl || md->ud.walkpath.path_pos == 0))
				return true; //Walk at least "mob_chase_refresh" cells before dropping the target unless target is non-existent
			mob_unlocktarget(md, tick); //Unlock target
			tbl = NULL;
		}
	}

	// Check for target change.
	if( md->attacked_id && mode&MD_CANATTACK )
	{
		if( md->attacked_id == md->target_id )
		{	//Rude attacked check.
			if( !battle_check_range(&md->bl, tbl, md->status.rhw.range)
			&&  ( //Can't attack back and can't reach back.
					(!can_move && DIFF_TICK(tick, md->ud.canmove_tick) > 0 && (battle_config.mob_ai&0x2 || (md->sc.data[SC_SPIDERWEB] && md->sc.data[SC_SPIDERWEB]->val1)
						|| md->sc.data[SC_BITE] || md->sc.data[SC_VACUUM_EXTREME] || md->sc.data[SC_THORNSTRAP]
						|| md->sc.data[SC__MANHOLE] // Not yet confirmed if boss will teleport once it can't reach target.
						|| md->walktoxy_fail_count > 0)
					)
					|| !mob_can_reach(md, tbl, md->min_chase, MSS_RUSH)
				)
			&&  md->state.attacked_count++ >= RUDE_ATTACKED_COUNT
			&&  !mobskill_use(md, tick, MSC_RUDEATTACKED) // If can't rude Attack
			&&  can_move && unit_escape(&md->bl, tbl, rnd()%10 +1)) // Attempt escape
			{	//Escaped
				md->attacked_id = 0;
				return true;
			}
		}
		else
		if( (abl = map_id2bl(md->attacked_id)) && (!tbl || mob_can_changetarget(md, abl, mode)) )
		{
			int dist;
			if( md->bl.m != abl->m || abl->prev == NULL
				|| (dist = distance_bl(&md->bl, abl)) >= MAX_MINCHASE // Attacker longer than visual area
				|| battle_check_target(&md->bl, abl, BCT_ENEMY) <= 0 // Attacker is not enemy of mob
				|| (battle_config.mob_ai&0x2 && !status_check_skilluse(&md->bl, abl, 0, 0)) // Cannot normal attack back to Attacker
				|| (!battle_check_range(&md->bl, abl, md->status.rhw.range) // Not on Melee Range and ...
				&& ( // Reach check
					(!can_move && DIFF_TICK(tick, md->ud.canmove_tick) > 0 && (battle_config.mob_ai&0x2 || (md->sc.data[SC_SPIDERWEB] && md->sc.data[SC_SPIDERWEB]->val1)
						|| md->sc.data[SC_BITE] || md->sc.data[SC_VACUUM_EXTREME] || md->sc.data[SC_THORNSTRAP]
						|| md->sc.data[SC__MANHOLE] // Not yet confirmed if boss will teleport once it can't reach target.
						|| md->walktoxy_fail_count > 0)
					)
					|| !mob_can_reach(md, abl, dist+md->db->range3, MSS_RUSH)
				   )
				) )
			{ // Rude attacked
				if (md->state.attacked_count++ >= RUDE_ATTACKED_COUNT
				&& !mobskill_use(md, tick, MSC_RUDEATTACKED) && can_move
				&& !tbl && unit_escape(&md->bl, abl, rnd()%10 +1))
				{	//Escaped.
					//TODO: Maybe it shouldn't attempt to run if it has another, valid target?
					md->attacked_id = 0;
					return true;
				}
			}
			else
			if (!(battle_config.mob_ai&0x2) && !status_check_skilluse(&md->bl, abl, 0, 0))
			{
				//Can't attack back, but didn't invoke a rude attacked skill...
			}
			else
			{ //Attackable
				if (!tbl || dist < md->status.rhw.range || !check_distance_bl(&md->bl, tbl, dist)
					|| battle_gettarget(tbl) != md->bl.id)
				{	//Change if the new target is closer than the actual one
					//or if the previous target is not attacking the mob. [Skotlex]
					md->target_id = md->attacked_id; // set target
					if (md->state.attacked_count)
					  md->state.attacked_count--; //Should we reset rude attack count?
					md->min_chase = dist+md->db->range3;
					if(md->min_chase>MAX_MINCHASE)
						md->min_chase=MAX_MINCHASE;
					tbl = abl; //Set the new target
				}
			}
		}

		//Clear it since it's been checked for already.
		md->attacked_id = 0;
	}

	// Processing of slave monster
	if (md->master_id > 0 && mob_ai_sub_hard_slavemob(md, tick))
		return true;

	// Scan area for targets
	if (!tbl && can_move && mode&MD_LOOTER && md->lootitems && DIFF_TICK(tick, md->ud.canact_tick) > 0 &&
		(md->lootitem_count < LOOTITEM_SIZE || battle_config.monster_loot_type != 1))
	{	// Scan area for items to loot, avoid trying to loot if the mob is full and can't consume the items.
		map_foreachinshootrange (mob_ai_sub_hard_lootsearch, &md->bl, view_range, BL_ITEM, md, &tbl);
	}

	if ((!tbl && mode&MD_AGGRESSIVE) || md->state.skillstate == MSS_FOLLOW)
	{
		map_foreachinrange (mob_ai_sub_hard_activesearch, &md->bl, view_range, DEFAULT_ENEMY_TYPE(md), md, &tbl, mode);
	}
	else
	if (mode&MD_CHANGECHASE && (md->state.skillstate == MSS_RUSH || md->state.skillstate == MSS_FOLLOW))
	{
		int search_size;
		search_size = view_range<md->status.rhw.range ? view_range:md->status.rhw.range;
		map_foreachinrange (mob_ai_sub_hard_changechase, &md->bl, search_size, DEFAULT_ENEMY_TYPE(md), md, &tbl);
	}

	if (!tbl) { //No targets available.
		if (mode&MD_ANGRY && !md->state.aggressive)
			md->state.aggressive = 1; //Restore angry state when no targets are available.

		/* bg guardians follow allies when no targets nearby */
		if( md->bg_id && mode&MD_CANATTACK ) {
			if( md->ud.walktimer != INVALID_TIMER )
				return true;/* we are already moving */
			map_foreachinrange (mob_ai_sub_hard_bg_ally, &md->bl, view_range, BL_PC, md, &tbl, mode);
			if( tbl ) {
				if( distance_blxy(&md->bl, tbl->x, tbl->y) <= 3 || unit_walktobl(&md->bl, tbl, 1, 1) )
					return true;/* we're moving or close enough don't unlock the target. */
			}
		}

		//This handles triggering idle/walk skill.
		mob_unlocktarget(md, tick);
		return true;
	}

	//Target exists, attack or loot as applicable.
	if (tbl->type == BL_ITEM)
	{	//Loot time.
		struct flooritem_data *fitem;
		if (md->ud.target == tbl->id && md->ud.walktimer != INVALID_TIMER)
			return true; //Already locked.
		if (md->lootitems == NULL)
		{	//Can't loot...
			mob_unlocktarget(md, tick);
			return true;
		}
		if (!check_distance_bl(&md->bl, tbl, 0))
		{	//Still not within loot range.
			if (!(mode&MD_CANMOVE))
			{	//A looter that can't move? Real smart.
				mob_unlocktarget(md, tick);
				return true;
			}
			if (!can_move) //Stuck. Wait before walking.
				return true;
			md->state.skillstate = MSS_LOOT;
			if (!unit_walktobl(&md->bl, tbl, 0, 1))
				mob_unlocktarget(md, tick); //Can't loot...
			return true;
		}
		//Within looting range.
		if (md->ud.attacktimer != INVALID_TIMER)
			return true; //Busy attacking?

		fitem = (struct flooritem_data *)tbl;
		//Logs items, taken by (L)ooter Mobs [Lupus]
		log_pick_mob(md, LOG_TYPE_LOOT, fitem->item.amount, &fitem->item);

		if (md->lootitem_count < LOOTITEM_SIZE) {
			memcpy (&md->lootitems[md->lootitem_count].item, &fitem->item, sizeof(md->lootitems[0].item));
			md->lootitems[md->lootitem_count].mob_id = fitem->mob_id;
			md->lootitem_count++;
		} else {	//Destroy first looted item...
			if (md->lootitems[0].item.card[0] == CARD0_PET)
				intif_delete_petdata(MakeDWord(md->lootitems[0].item.card[1],md->lootitems[0].item.card[2]));
			memmove(&md->lootitems[0], &md->lootitems[1], (LOOTITEM_SIZE-1)*sizeof(md->lootitems[0]));
			memcpy (&md->lootitems[LOOTITEM_SIZE-1].item, &fitem->item, sizeof(md->lootitems[0].item));
			md->lootitems[LOOTITEM_SIZE-1].mob_id = fitem->mob_id;
		}

		if (pcdb_checkid(md->vd->class_))
		{	//Give them walk act/delay to properly mimic players. [Skotlex]
			clif_takeitem(&md->bl,tbl);
			md->ud.canact_tick = tick + md->status.amotion;
			unit_set_walkdelay(&md->bl, tick, md->status.amotion, 1);
		}
		//Clear item.
		map_clearflooritem(tbl);
		mob_unlocktarget(md, tick);
		return true;
	}

	//Attempt to attack.
	//At this point we know the target is attackable, we just gotta check if the range matches.
	if (battle_check_range(&md->bl, tbl, md->status.rhw.range) && !(md->sc.option&OPTION_HIDE))
	{	//Target within range and able to use normal attack, engage
		if (md->ud.target != tbl->id || md->ud.attacktimer == INVALID_TIMER) 
		{ //Only attack if no more attack delay left
			if(tbl->type == BL_PC)
				mob_log_damage(md, tbl, 0); //Log interaction (counts as 'attacker' for the exp bonus)

			if( !(mode&MD_RANDOMTARGET) )
				unit_attack(&md->bl,tbl->id,1);
			else { // Attack once and find a new random target
				int search_size = (view_range < md->status.rhw.range) ? view_range : md->status.rhw.range;
				unit_attack(&md->bl, tbl->id, 0);
				if ((tbl = battle_getenemy(&md->bl, DEFAULT_ENEMY_TYPE(md), search_size))) {
					md->target_id = tbl->id;
					md->min_chase = md->db->range3;
				}
			}
		}
		return true;
	}

	//Monsters in berserk state, unable to use normal attacks, will always attempt a skill
	if(md->ud.walktimer == INVALID_TIMER && (md->state.skillstate == MSS_BERSERK || md->state.skillstate == MSS_ANGRY)) 
	{
		if (DIFF_TICK(md->ud.canmove_tick, tick) <= MIN_MOBTHINKTIME && DIFF_TICK(md->ud.canact_tick, tick) < -MIN_MOBTHINKTIME*IDLE_SKILL_INTERVAL) 
		{ //Only use skill if able to walk on next tick and not used a skill the last second
			mobskill_use(md, tick, -1);
		}
	}

	//Target still in attack range, no need to chase the target
	if(battle_check_range(&md->bl, tbl, md->status.rhw.range))
		return true;

	//Only update target cell / drop target after having moved at least "mob_chase_refresh" cells
	if(md->ud.walktimer != INVALID_TIMER && (!can_move || md->ud.walkpath.path_pos <= battle_config.mob_chase_refresh))
		return true;

	//Out of range...
	if (!(mode&MD_CANMOVE) || (!can_move && DIFF_TICK(tick, md->ud.canmove_tick) > 0))
	{	//Can't chase. Immobile and trapped mobs should unlock target and use an idle skill.
		if (md->ud.attacktimer == INVALID_TIMER)
		{ //Only unlock target if no more attack delay left
			//This handles triggering idle/walk skill.
			mob_unlocktarget(md,tick);
		}
		return true;
	}

	if (md->ud.walktimer != INVALID_TIMER && md->ud.target == tbl->id &&
		(
			!(battle_config.mob_ai&0x1) ||
			check_distance_blxy(tbl, md->ud.to_x, md->ud.to_y, md->status.rhw.range)
	)) //Current target tile is still within attack range.
		return true;

	//Follow up if possible.
	//Hint: Chase skills are handled in the walktobl routine
	if(!mob_can_reach(md, tbl, md->min_chase, MSS_RUSH) ||
		!unit_walktobl(&md->bl, tbl, md->status.rhw.range, 2))
		mob_unlocktarget(md,tick);

	return true;
}

static int mob_ai_sub_hard_timer(struct block_list *bl,va_list ap)
{
	struct mob_data *md = (struct mob_data*)bl;
	unsigned int tick = va_arg(ap, unsigned int);
	if (mob_ai_sub_hard(md, tick))
	{	//Hard AI triggered.
		if(!md->state.spotted)
			md->state.spotted = 1;
		md->last_pcneartime = tick;
	}
	return 0;
}

/*==========================================
 * Serious processing for mob in PC field of view (foreachclient)
 *------------------------------------------*/
static int mob_ai_sub_foreachclient(struct map_session_data *sd,va_list ap)
{
	unsigned int tick;
	tick=va_arg(ap,unsigned int);
	map_foreachinrange(mob_ai_sub_hard_timer,&sd->bl, AREA_SIZE+ACTIVE_AI_RANGE, BL_MOB,tick);

	return 0;
}

/*==========================================
 * Negligent mode MOB AI (PC is not in near)
 *------------------------------------------*/
static int mob_ai_sub_lazy(struct mob_data *md, va_list args)
{
	unsigned int tick;

	nullpo_ret(md);

	if(md->bl.prev == NULL)
		return 0;

	tick = va_arg(args,unsigned int);

	if (battle_config.mob_ai&0x20 && map[md->bl.m].users>0)
		return (int)mob_ai_sub_hard(md, tick);

	if (md->bl.prev==NULL || md->status.hp == 0)
		return 1;

	if(battle_config.mob_active_time &&
		md->last_pcneartime &&
 		!(md->status.mode&MD_BOSS) &&
		DIFF_TICK(tick,md->last_thinktime) > MIN_MOBTHINKTIME)
	{
		if (DIFF_TICK(tick,md->last_pcneartime) < battle_config.mob_active_time)
			return (int)mob_ai_sub_hard(md, tick);
		md->last_pcneartime = 0;
	}

	if(battle_config.boss_active_time &&
		md->last_pcneartime &&
		(md->status.mode&MD_BOSS) &&
		DIFF_TICK(tick,md->last_thinktime) > MIN_MOBTHINKTIME)
	{
		if (DIFF_TICK(tick,md->last_pcneartime) < battle_config.boss_active_time)
			return (int)mob_ai_sub_hard(md, tick);
		md->last_pcneartime = 0;
	}

	if(DIFF_TICK(tick,md->last_thinktime)< 10*MIN_MOBTHINKTIME)
		return 0;

	md->last_thinktime=tick;

	if (md->master_id) {
		mob_ai_sub_hard_slavemob (md,tick);
		return 0;
	}

	if( DIFF_TICK(md->next_walktime,tick) < 0 && (status_get_mode(&md->bl)&MD_CANMOVE) && unit_can_move(&md->bl) )
	{
		if( rnd()%1000 < MOB_LAZYMOVEPERC(md) )
			mob_randomwalk(md, tick);
	}
	else if( md->ud.walktimer == INVALID_TIMER )
	{
		//Because it is not unset when the mob finishes walking.
		md->state.skillstate = MSS_IDLE;
		if( rnd()%1000 < MOB_LAZYSKILLPERC(md) ) //Chance to do a mob's idle skill.
			mobskill_use(md, tick, -1);
	}

	return 0;
}

/*==========================================
 * Negligent processing for mob outside PC field of view   (interval timer function)
 *------------------------------------------*/
static int mob_ai_lazy(int tid, unsigned int tick, int id, intptr_t data)
{
	map_foreachmob(mob_ai_sub_lazy,tick);
	return 0;
}

/*==========================================
 * Serious processing for mob in PC field of view   (interval timer function)
 *------------------------------------------*/
static int mob_ai_hard(int tid, unsigned int tick, int id, intptr_t data)
{

	if (battle_config.mob_ai&0x20)
		map_foreachmob(mob_ai_sub_lazy,tick);
	else
		map_foreachpc(mob_ai_sub_foreachclient,tick);

	return 0;
}

/*==========================================
 * Initializes the delay drop structure for mob-dropped items.
 *------------------------------------------*/
static struct item_drop* mob_setdropitem(unsigned short nameid, int qty, unsigned short mob_id)
{
	struct item_drop *drop = ers_alloc(item_drop_ers, struct item_drop);
	memset(&drop->item_data, 0, sizeof(struct item));
	drop->item_data.nameid = nameid;
	drop->item_data.amount = qty;
	drop->item_data.identify = itemdb_isidentified(nameid);
	drop->mob_id = mob_id;
	drop->next = NULL;
	return drop;
}

/*==========================================
 * Initializes the delay drop structure for mob-looted items.
 *------------------------------------------*/
static struct item_drop* mob_setlootitem(struct s_mob_lootitem *item, unsigned short mob_id)
{
	struct item_drop *drop = ers_alloc(item_drop_ers, struct item_drop);
	memcpy(&drop->item_data, item, sizeof(struct item));

	/**
	 * Conditions for lotted item, so it can be announced when player pick it up
	 * 1. Not-dropped other than monster. (This will be done later on pc_takeitem/party_share_loot)
	 * 2. The mob_id is become the last lootter, instead of the real monster who drop it.
	 **/
	drop->mob_id = item->mob_id;

	drop->next = NULL;
	return drop;
}

/*==========================================
 * item drop with delay (timer function)
 *------------------------------------------*/
static int mob_delay_item_drop(int tid, unsigned int tick, int id, intptr_t data)
{
	struct item_drop_list *list;
	struct item_drop *ditem;

	list = (struct item_drop_list *)data;
	ditem = list->item;

	while (ditem) {
		struct item_drop *ditem_prev;
		map_addflooritem(&ditem->item_data,ditem->item_data.amount,
			list->m,list->x,list->y,
			list->first_charid,list->second_charid,list->third_charid,4,ditem->mob_id);
		ditem_prev = ditem;
		ditem = ditem->next;
		ers_free(item_drop_ers, ditem_prev);
	}

	ers_free(item_drop_list_ers, list);
	return 0;
}

/*==========================================
 * Sets the item_drop into the item_drop_list.
 * Also performs logging and autoloot if enabled.
 * rate is the drop-rate of the item, required for autoloot.
 * flag : Killed only by homunculus?
 *------------------------------------------*/
static void mob_item_drop(struct mob_data *md, struct item_drop_list *dlist, struct item_drop *ditem, int loot, int drop_rate, unsigned short flag)
{
	TBL_PC* sd;

	//Logs items, dropped by mobs [Lupus]
	log_pick_mob(md, loot?LOG_TYPE_LOOT:LOG_TYPE_PICKDROP_MONSTER, -ditem->item_data.amount, &ditem->item_data);

	sd = map_charid2sd(dlist->first_charid);
	if( sd == NULL ) sd = map_charid2sd(dlist->second_charid);
	if( sd == NULL ) sd = map_charid2sd(dlist->third_charid);

	if( sd
		&& (drop_rate <= sd->state.autoloot || pc_isautolooting(sd, ditem->item_data.nameid))
		&& (battle_config.idle_no_autoloot == 0 || DIFF_TICK(last_tick, sd->idletime) < battle_config.idle_no_autoloot)
		&& (battle_config.homunculus_autoloot?1:!flag)
#ifdef AUTOLOOT_DISTANCE
		&& sd->bl.m == md->bl.m
		&& check_distance_blxy(&sd->bl, dlist->x, dlist->y, AUTOLOOT_DISTANCE)
#endif
	) {	//Autoloot.
		struct party_data *p = party_search(sd->status.party_id);

		if (party_share_loot(party_search(sd->status.party_id),
			sd, &ditem->item_data, sd->status.char_id) == 0
		) {
			ers_free(item_drop_ers, ditem);
			return;
		}

		if ((itemdb_search(ditem->item_data.nameid))->flag.broadcast &&
			(!p || !(p->party.item&2)) // Somehow, if party's pickup distribution is 'Even Share', no announcemet
			)
			intif_broadcast_obtain_special_item(sd, ditem->item_data.nameid, md->mob_id, ITEMOBTAIN_TYPE_MONSTER_ITEM);
	}
	ditem->next = dlist->item;
	dlist->item = ditem;
}

int mob_timer_delete(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list* bl = map_id2bl(id);
	struct mob_data* md = BL_CAST(BL_MOB, bl);

	if( md )
	{
		if( md->deletetimer != tid )
		{
			ShowError("mob_timer_delete: Timer mismatch: %d != %d\n", tid, md->deletetimer);
			return 0;
		}
		//for Alchemist CANNIBALIZE [Lupus]
		md->deletetimer = INVALID_TIMER;
		unit_free(bl, CLR_TELEPORT);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int mob_deleteslave_sub(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	int id;

	nullpo_ret(bl);
	nullpo_ret(md = (struct mob_data *)bl);

	id=va_arg(ap,int);
	if(md->master_id > 0 && md->master_id == id )
		status_kill(bl);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int mob_deleteslave(struct mob_data *md)
{
	nullpo_ret(md);

	map_foreachinmap(mob_deleteslave_sub, md->bl.m, BL_MOB,md->bl.id);
	return 0;
}
// Mob respawning through KAIZEL or NPC_REBIRTH [Skotlex]
int mob_respawn(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list *bl = map_id2bl(id);

	if(!bl) return 0;
	status_revive(bl, (uint8)data, 0);
	return 1;
}

void mob_log_damage(struct mob_data *md, struct block_list *src, int damage)
{
	uint32 char_id = 0;
	int flag = MDLF_NORMAL;

	if( damage < 0 )
		return; //Do nothing for absorbed damage.
	if( !damage && !(src->type&DEFAULT_ENEMY_TYPE(md)) )
		return; //Do not log non-damaging effects from non-enemies.
	if( src->id == md->bl.id )
		return; //Do not log self-damage.

	switch( src->type )
	{
		case BL_PC:
		{
			struct map_session_data *sd = (TBL_PC*)src;
			char_id = sd->status.char_id;
			if( damage )
				md->attacked_id = src->id;
			break;
		}
		case BL_HOM:
		{
			struct homun_data *hd = (TBL_HOM*)src;
			flag = MDLF_HOMUN;
			if( hd->master )
				char_id = hd->master->status.char_id;
			if( damage )
				md->attacked_id = src->id;
			break;
		}
		case BL_MER:
		{
			struct mercenary_data *mer = (TBL_MER*)src;
			if( mer->master )
				char_id = mer->master->status.char_id;
			if( damage )
				md->attacked_id = src->id;
			break;
		}
		case BL_PET:
		{
			struct pet_data *pd = (TBL_PET*)src;
			flag = MDLF_PET;
			if( pd->master )
			{
				char_id = pd->master->status.char_id;
				if( damage ) //Let mobs retaliate against the pet's master [Skotlex]
					md->attacked_id = pd->master->bl.id;
			}
			break;
		}
		case BL_MOB:
		{
			struct mob_data* md2 = (TBL_MOB*)src;
			if( md2->special_state.ai && md2->master_id )
			{
				struct map_session_data* msd = map_id2sd(md2->master_id);
				if( msd )
					char_id = msd->status.char_id;
			}
			if( !damage )
				break;
			//Let players decide whether to retaliate versus the master or the mob. [Skotlex]
			if( md2->master_id && battle_config.retaliate_to_master )
				md->attacked_id = md2->master_id;
			else
				md->attacked_id = src->id;
			break;
		}
		case BL_ELEM:
		{
			struct elemental_data *ele = (TBL_ELEM*)src;
			if( ele->master )
				char_id = ele->master->status.char_id;
			if( damage )
				md->attacked_id = src->id;
			break;
		}
		default: //For all unhandled types.
			md->attacked_id = src->id;
	}

	if( char_id )
	{ //Log damage...
		int i,minpos;
		unsigned int mindmg;
		for(i=0,minpos=DAMAGELOG_SIZE-1,mindmg=UINT_MAX;i<DAMAGELOG_SIZE;i++){
			if(md->dmglog[i].id==char_id &&
				md->dmglog[i].flag==flag)
				break;
			if(md->dmglog[i].id==0) {	//Store data in first empty slot.
				md->dmglog[i].id  = char_id;
				md->dmglog[i].flag= flag;

				if(md->db->mexp)
					pc_damage_log_add(map_charid2sd(char_id),md->bl.id);
				break;
			}
			if(md->dmglog[i].dmg<mindmg && i)
			{	//Never overwrite first hit slot (he gets double exp bonus)
				minpos=i;
				mindmg=md->dmglog[i].dmg;
			}
		}
		if(i<DAMAGELOG_SIZE)
			md->dmglog[i].dmg+=damage;
		else {
			md->dmglog[minpos].id  = char_id;
			md->dmglog[minpos].flag= flag;
			md->dmglog[minpos].dmg = damage;

			if(md->db->mexp)
				pc_damage_log_add(map_charid2sd(char_id),md->bl.id);
		}
	}
	return;
}
//Call when a mob has received damage.
void mob_damage(struct mob_data *md, struct block_list *src, int damage)
{
	if (damage > 0) { //Store total damage...
		if (UINT_MAX - (unsigned int)damage > md->tdmg)
			md->tdmg+=damage;
		else if (md->tdmg == UINT_MAX)
			damage = 0; //Stop recording damage once the cap has been reached.
		else { //Cap damage log...
			damage = (int)(UINT_MAX - md->tdmg);
			md->tdmg = UINT_MAX;
		}
		if (md->state.aggressive) { //No longer aggressive, change to retaliate AI.
			md->state.aggressive = 0;
			if(md->state.skillstate== MSS_ANGRY)
				md->state.skillstate = MSS_BERSERK;
			if(md->state.skillstate== MSS_FOLLOW)
				md->state.skillstate = MSS_RUSH;
		}
		//Log damage
		if (src)
			mob_log_damage(md, src, damage);
		md->dmgtick = gettick();
	}

	if (battle_config.show_mob_info&3)
		clif_charnameack (0, &md->bl);

	if (!src)
		return;

#if PACKETVER >= 20120404
	if( !(md->status.mode&MD_BOSS) ){
		int i;
		for(i = 0; i < DAMAGELOG_SIZE; i++){ // must show hp bar to all char who already hit the mob.
			struct map_session_data *sd = map_charid2sd(md->dmglog[i].id);
			if( sd && check_distance_bl(&md->bl, &sd->bl, AREA_SIZE) ) // check if in range
				clif_monster_hp_bar(md, sd->fd);
		}
	}
#endif

	if( md->special_state.ai == AI_SPHERE ) {//LOne WOlf explained that ANYONE can trigger the marine countdown skill. [Skotlex]
		md->state.alchemist = 1;
		mobskill_use(md, gettick(), MSC_ALCHEMIST);
	}
}

/*==========================================
 * Signals death of mob.
 * type&1 -> no drops, type&2 -> no exp
 *------------------------------------------*/
int mob_dead(struct mob_data *md, struct block_list *src, int type)
{
	struct status_data *status;
	struct map_session_data *sd = NULL, *tmpsd[DAMAGELOG_SIZE];
	struct map_session_data *mvp_sd = NULL, *second_sd = NULL, *third_sd = NULL;

	struct {
		struct party_data *p;
		int id,zeny;
		unsigned int base_exp,job_exp;
	} pt[DAMAGELOG_SIZE];
	int i, temp, count, m = md->bl.m;
	int dmgbltypes = 0;  // bitfield of all bl types, that caused damage to the mob and are elligible for exp distribution
	unsigned int mvp_damage, tick = gettick();
	bool rebirth, homkillonly;

	status = &md->status;

	if( src && src->type == BL_PC ) {
		sd = (struct map_session_data *)src;
		mvp_sd = sd;
	}

	if( md->guardian_data && md->guardian_data->number >= 0 && md->guardian_data->number < MAX_GUARDIANS )
		guild_castledatasave(md->guardian_data->castle->castle_id, 10+md->guardian_data->number,0);

	if( src ) { // Use Dead skill only if not killed by Script or Command
		md->status.hp = 1;
		md->state.skillstate = MSS_DEAD;
		mobskill_use(md,tick,-1);
		md->status.hp = 0;
	}

	map_freeblock_lock();

	memset(pt,0,sizeof(pt));

	if(src && src->type == BL_MOB)
		mob_unlocktarget((struct mob_data *)src,tick);

	// filter out entries not eligible for exp distribution
	memset(tmpsd,0,sizeof(tmpsd));
	for(i = 0, count = 0, mvp_damage = 0; i < DAMAGELOG_SIZE && md->dmglog[i].id; i++) {
		struct map_session_data* tsd = map_charid2sd(md->dmglog[i].id);

		if(tsd == NULL)
			continue; // skip empty entries
		if(tsd->bl.m != m)
			continue; // skip players not on this map
		count++; //Only logged into same map chars are counted for the total.
		if (pc_isdead(tsd))
			continue; // skip dead players
		if(md->dmglog[i].flag == MDLF_HOMUN && !hom_is_active(tsd->hd))
			continue; // skip homunc's share if inactive
		if( md->dmglog[i].flag == MDLF_PET && (!tsd->status.pet_id || !tsd->pd) )
			continue; // skip pet's share if inactive

		if(md->dmglog[i].dmg > mvp_damage) {
			third_sd = second_sd;
			second_sd = mvp_sd;
			mvp_sd = tsd;
			mvp_damage = md->dmglog[i].dmg;
		}

		tmpsd[i] = tsd; // record as valid damage-log entry

		switch( md->dmglog[i].flag ) {
			case MDLF_NORMAL: dmgbltypes|= BL_PC;  break;
			case MDLF_HOMUN:  dmgbltypes|= BL_HOM; break;
			case MDLF_PET:    dmgbltypes|= BL_PET; break;
		}
	}

	// determines, if the monster was killed by homunculus' damage only
	homkillonly = (bool)( ( dmgbltypes&BL_HOM ) && !( dmgbltypes&~BL_HOM ) );

	if(!battle_config.exp_calc_type && count > 1) {	//Apply first-attacker 200% exp share bonus
		//TODO: Determine if this should go before calculating the MVP player instead of after.
		if (UINT_MAX - md->dmglog[0].dmg > md->tdmg) {
			md->tdmg += md->dmglog[0].dmg;
			md->dmglog[0].dmg<<=1;
		} else {
			md->dmglog[0].dmg+= UINT_MAX - md->tdmg;
			md->tdmg = UINT_MAX;
		}
	}

	if(!(type&2) && //No exp
		(!map[m].flag.pvp || battle_config.pvp_exp) && //Pvp no exp rule [MouseJstr]
		(!md->master_id || !md->special_state.ai) && //Only player-summoned mobs do not give exp. [Skotlex]
		(!map[m].flag.nobaseexp || !map[m].flag.nojobexp) //Gives Exp
	) { //Experience calculation.
		int bonus = 100; //Bonus on top of your share (common to all attackers).
		int pnum = 0;
		if (md->sc.data[SC_RICHMANKIM])
			bonus += md->sc.data[SC_RICHMANKIM]->val2;
		if(sd) {
			temp = status_get_class(&md->bl);
			if(sd->sc.data[SC_MIRACLE]) i = 2; //All mobs are Star Targets
			else
			ARR_FIND(0, MAX_PC_FEELHATE, i, temp == sd->hate_mob[i] &&
				(battle_config.allow_skill_without_day || sg_info[i].day_func()));
			if(i<MAX_PC_FEELHATE && (temp=pc_checkskill(sd,sg_info[i].bless_id)))
				bonus += (i==2?20:10)*temp;
		}
		if(battle_config.mobs_level_up && md->level > md->db->lv) // [Valaris]
			bonus += (md->level-md->db->lv)*battle_config.mobs_level_up_exp_rate;

		for(i = 0; i < DAMAGELOG_SIZE && md->dmglog[i].id; i++) {
			int flag=1,zeny=0;
			unsigned int base_exp, job_exp;
			double per; //Your share of the mob's exp

			if (!tmpsd[i]) continue;

			if (!battle_config.exp_calc_type && md->tdmg)
				//jAthena's exp formula based on total damage.
				per = (double)md->dmglog[i].dmg/(double)md->tdmg;
			else {
				//eAthena's exp formula based on max hp.
				per = (double)md->dmglog[i].dmg/(double)status->max_hp;
				if (per > 2) per = 2; // prevents unlimited exp gain
			}

			if (count>1 && battle_config.exp_bonus_attacker) {
				//Exp bonus per additional attacker.
				if (count > battle_config.exp_bonus_max_attacker)
					count = battle_config.exp_bonus_max_attacker;
				per += per*((count-1)*battle_config.exp_bonus_attacker)/100.;
			}

			// change experience for different sized monsters [Valaris]
			if (battle_config.mob_size_influence) {
				switch( md->special_state.size ) {
					case SZ_MEDIUM:
						per /= 2.;
						break;
					case SZ_BIG:
						per *= 2.;
						break;
				}
			}

			if( md->dmglog[i].flag == MDLF_PET )
				per *= battle_config.pet_attack_exp_rate/100.;

			if(battle_config.zeny_from_mobs && md->level) {
				 // zeny calculation moblv + random moblv [Valaris]
				zeny=(int) ((md->level+rnd()%md->level)*per*bonus/100.);
				if(md->db->mexp > 0)
					zeny*=rnd()%250;
			}

			if (map[m].flag.nobaseexp || !md->db->base_exp)
				base_exp = 0;
			else {
				double exp = apply_rate2(md->db->base_exp, per, 1);
				exp = apply_rate(exp, bonus);
				exp = apply_rate(exp, map[m].adjust.bexp);
				base_exp = (unsigned int)cap_value(exp, 1, UINT_MAX);
			}

			if (map[m].flag.nojobexp || !md->db->job_exp || md->dmglog[i].flag == MDLF_HOMUN) //Homun earned job-exp is always lost.
				job_exp = 0;
			else {
				double exp = apply_rate2(md->db->job_exp, per, 1);
				exp = apply_rate(exp, bonus);
				exp = apply_rate(exp, map[m].adjust.jexp);
				job_exp = (unsigned int)cap_value(exp, 1, UINT_MAX);
			}

			if ( ( temp = tmpsd[i]->status.party_id)>0 ) {
				int j;
				for( j = 0; j < pnum && pt[j].id != temp; j++ ); //Locate party.

				if( j == pnum ) { //Possibly add party.
					pt[pnum].p = party_search(temp);
					if(pt[pnum].p && pt[pnum].p->party.exp) {
						pt[pnum].id = temp;
						pt[pnum].base_exp = base_exp;
						pt[pnum].job_exp = job_exp;
						pt[pnum].zeny = zeny; // zeny share [Valaris]
						pnum++;
						flag = 0;
					}
				} else {	//Add to total
					if (pt[j].base_exp > UINT_MAX - base_exp)
						pt[j].base_exp = UINT_MAX;
					else
						pt[j].base_exp += base_exp;

					if (pt[j].job_exp > UINT_MAX - job_exp)
						pt[j].job_exp = UINT_MAX;
					else
						pt[j].job_exp += job_exp;

					pt[j].zeny += zeny;  // zeny share [Valaris]
					flag = 0;
				}
			}
			if(base_exp && md->dmglog[i].flag == MDLF_HOMUN) //tmpsd[i] is null if it has no homunc.
				hom_gainexp(tmpsd[i]->hd, base_exp);
			if(flag) {
				if(base_exp || job_exp) {
					if( md->dmglog[i].flag != MDLF_PET || battle_config.pet_attack_exp_to_master ) {
#ifdef RENEWAL_EXP
						int rate = pc_level_penalty_mod(tmpsd[i], md->level, md->status.class_, 1);
						if (rate != 100) {
							base_exp = (unsigned int)cap_value(apply_rate(base_exp, rate), 1, UINT_MAX);
							job_exp = (unsigned int)cap_value(apply_rate(job_exp, rate), 1, UINT_MAX);
						}
#endif
						pc_gainexp(tmpsd[i], &md->bl, base_exp, job_exp, false);
					}
				}
				if(zeny) // zeny from mobs [Valaris]
					pc_getzeny(tmpsd[i], zeny, LOG_TYPE_PICKDROP_MONSTER, NULL);
			}

			if( md->db->mexp )
				pc_damage_log_clear(tmpsd[i],md->bl.id);
		}

		for( i = 0; i < pnum; i++ ) //Party share.
			party_exp_share(pt[i].p, &md->bl, pt[i].base_exp,pt[i].job_exp,pt[i].zeny);

	} //End EXP giving.

	if( !(type&1) && !map[m].flag.nomobloot && !md->state.rebirth && (
		!md->special_state.ai || //Non special mob
		battle_config.alchemist_summon_reward == 2 || //All summoned give drops
		(md->special_state.ai==AI_SPHERE && battle_config.alchemist_summon_reward == 1) //Marine Sphere Drops items.
		) )
	{ // Item Drop
		struct item_drop_list *dlist = ers_alloc(item_drop_list_ers, struct item_drop_list);
		struct item_drop *ditem;
		struct item_data* it = NULL;
		int drop_rate;
#ifdef RENEWAL_DROP
		int drop_modifier = mvp_sd    ? pc_level_penalty_mod(mvp_sd, md->level, md->status.class_, 2)   :
							second_sd ? pc_level_penalty_mod(second_sd, md->level, md->status.class_, 2):
							third_sd  ? pc_level_penalty_mod(third_sd, md->level, md->status.class_, 2) :
							100;/* no player was attached, we dont use any modifier (100 = rates are not touched) */
#endif
		dlist->m = md->bl.m;
		dlist->x = md->bl.x;
		dlist->y = md->bl.y;
		dlist->first_charid = (mvp_sd ? mvp_sd->status.char_id : 0);
		dlist->second_charid = (second_sd ? second_sd->status.char_id : 0);
		dlist->third_charid = (third_sd ? third_sd->status.char_id : 0);
		dlist->item = NULL;

		for (i = 0; i < MAX_MOB_DROP; i++) {
			if (md->db->dropitem[i].nameid <= 0)
				continue;
			if ( !(it = itemdb_exists(md->db->dropitem[i].nameid)) )
				continue;
			drop_rate = md->db->dropitem[i].p;
			if (drop_rate <= 0) {
				if (battle_config.drop_rate0item)
					continue;
				drop_rate = 1;
			}

			// change drops depending on monsters size [Valaris]
			if (battle_config.mob_size_influence) {
				if (md->special_state.size == SZ_MEDIUM && drop_rate >= 2)
					drop_rate /= 2;
				else if( md->special_state.size == SZ_BIG)
					drop_rate *= 2;
			}

			if (src) {
				//Drops affected by luk as a fixed increase [Valaris]
				if (battle_config.drops_by_luk)
					drop_rate += status_get_luk(src)*battle_config.drops_by_luk/100;
				//Drops affected by luk as a % increase [Skotlex]
				if (battle_config.drops_by_luk2)
					drop_rate += (int)(0.5+drop_rate*status_get_luk(src)*battle_config.drops_by_luk2/10000.);
			}
			if (sd && battle_config.pk_mode &&
				(int)(md->level - sd->status.base_level) >= 20)
				drop_rate = (int)(drop_rate*1.25); // pk_mode increase drops if 20 level difference [Valaris]

			// Increase drop rate if user has SC_ITEMBOOST
			if (sd && sd->sc.data[SC_ITEMBOOST]) // now rig the drop rate to never be over 90% unless it is originally >90%.
				drop_rate = max(drop_rate,cap_value((int)(0.5+drop_rate*(sd->sc.data[SC_ITEMBOOST]->val1)/100.),0,9000));
			// Increase item drop rate for VIP.
			if (battle_config.vip_drop_increase && (sd && pc_isvip(sd))) {
				drop_rate += (int)(0.5 + (drop_rate * battle_config.vip_drop_increase) / 10000.);
				drop_rate = min(drop_rate,10000); //cap it to 100%
			}
#ifdef RENEWAL_DROP
			if( drop_modifier != 100 ) {
				drop_rate = apply_rate(drop_rate, drop_modifier);
				if( drop_rate < 1 )
					drop_rate = 1;
			}
#endif
			// attempt to drop the item
			if (rnd() % 10000 >= drop_rate)
				continue;

			if( mvp_sd && it->type == IT_PETEGG ) {
				pet_create_egg(mvp_sd, md->db->dropitem[i].nameid);
				continue;
			}

			ditem = mob_setdropitem(md->db->dropitem[i].nameid, 1, md->mob_id);

			//A Rare Drop Global Announce by Lupus
			if( mvp_sd && drop_rate <= battle_config.rare_drop_announce ) {
				char message[128];
				sprintf (message, msg_txt(NULL,541), mvp_sd->status.name, md->name, it->jname, (float)drop_rate/100);
				//MSG: "'%s' won %s's %s (chance: %0.02f%%)"
				intif_broadcast(message,strlen(message)+1,BC_DEFAULT);
			}
			// Announce first, or else ditem will be freed. [Lance]
			// By popular demand, use base drop rate for autoloot code. [Skotlex]
			mob_item_drop(md, dlist, ditem, 0, md->db->dropitem[i].p, homkillonly);
		}

		// Ore Discovery [Celest]
		if (sd == mvp_sd && pc_checkskill(sd,BS_FINDINGORE)>0 && battle_config.finding_ore_rate/10 >= rnd()%10000) {
			ditem = mob_setdropitem(itemdb_searchrandomid(IG_FINDINGORE,1), 1, md->mob_id);
			mob_item_drop(md, dlist, ditem, 0, battle_config.finding_ore_rate/10, homkillonly);
		}

		if(sd) {
			// process script-granted extra drop bonuses
			uint16 dropid = 0;

			for (i = 0; i < ARRAYLENGTH(sd->add_drop); i++) {
				if (!&sd->add_drop[i] || (!sd->add_drop[i].nameid && !sd->add_drop[i].group))
					continue;
				if ((sd->add_drop[i].race < RC_NONE_ && sd->add_drop[i].race == -md->mob_id) || //Race < RC_NONE_, use mob_id
					(sd->add_drop[i].race == RC_ALL || sd->add_drop[i].race == status->race) || //Matched race
					(sd->add_drop[i].class_ == CLASS_ALL || sd->add_drop[i].class_ == status->class_)) //Matched class
				{
					//Check if the bonus item drop rate should be multiplied with mob level/10 [Lupus]
					if (sd->add_drop[i].rate < 0) {
						//It's negative, then it should be multiplied. with mob_level/10
						//rate = base_rate * (mob_level/10) + 1
						drop_rate = (-sd->add_drop[i].rate) * md->level / 10 + 1;
						drop_rate = cap_value(drop_rate, max(battle_config.item_drop_adddrop_min,1), min(battle_config.item_drop_adddrop_max,10000));
					}
					else
						//it's positive, then it goes as it is
						drop_rate = sd->add_drop[i].rate;

					if (rnd()%10000 >= drop_rate)
						continue;
					dropid = (sd->add_drop[i].nameid > 0) ? sd->add_drop[i].nameid : itemdb_searchrandomid(sd->add_drop[i].group,1);

					mob_item_drop(md, dlist, mob_setdropitem(dropid,1,md->mob_id), 0, drop_rate, homkillonly);
				}
			}

			// process script-granted zeny bonus (get_zeny_num) [Skotlex]
			if( sd->bonus.get_zeny_num && rnd()%100 < sd->bonus.get_zeny_rate ) {
				i = sd->bonus.get_zeny_num > 0 ? sd->bonus.get_zeny_num : -md->level * sd->bonus.get_zeny_num;
				if (!i) i = 1;
				pc_getzeny(sd, 1+rnd()%i, LOG_TYPE_PICKDROP_MONSTER, NULL);
			}
		}

		// process items looted by the mob
		if (md->lootitems) {
			for (i = 0; i < md->lootitem_count; i++)
				mob_item_drop(md, dlist, mob_setlootitem(&md->lootitems[i], md->mob_id), 1, 10000, homkillonly);
		}
		if (dlist->item) //There are drop items.
			add_timer(tick + (!battle_config.delay_battle_damage?500:0), mob_delay_item_drop, 0, (intptr_t)dlist);
		else //No drops
			ers_free(item_drop_list_ers, dlist);
	} else if (md->lootitems && md->lootitem_count) {	//Loot MUST drop!
		struct item_drop_list *dlist = ers_alloc(item_drop_list_ers, struct item_drop_list);
		dlist->m = md->bl.m;
		dlist->x = md->bl.x;
		dlist->y = md->bl.y;
		dlist->first_charid = (mvp_sd ? mvp_sd->status.char_id : 0);
		dlist->second_charid = (second_sd ? second_sd->status.char_id : 0);
		dlist->third_charid = (third_sd ? third_sd->status.char_id : 0);
		dlist->item = NULL;
		for (i = 0; i < md->lootitem_count; i++)
			mob_item_drop(md, dlist, mob_setlootitem(&md->lootitems[i], md->mob_id), 1, 10000, homkillonly);
		add_timer(tick + (!battle_config.delay_battle_damage?500:0), mob_delay_item_drop, 0, (intptr_t)dlist);
	}

	if(mvp_sd && md->db->mexp > 0 && !md->special_state.ai) {
		unsigned int log_mvp[2] = {0};
		unsigned int mexp;
		struct item item;
		double exp;

		//mapflag: noexp check [Lorky]
		if (map[m].flag.nobaseexp || type&2)
			exp =1;
		else {
			exp = md->db->mexp;
			if (count > 1)
				exp += exp*(battle_config.exp_bonus_attacker*(count-1))/100.; //[Gengar]
		}

		mexp = (unsigned int)cap_value(exp, 1, UINT_MAX);

		clif_mvp_effect(mvp_sd);
		clif_mvp_exp(mvp_sd,mexp);
		pc_gainexp(mvp_sd, &md->bl, mexp,0, false);
		log_mvp[1] = mexp;

		if( !(map[m].flag.nomvploot || type&1) ) {
			//Order might be random depending on item_drop_mvp_mode config setting
			int mdrop_id[MAX_MVP_DROP];
			int mdrop_p[MAX_MVP_DROP];

			memset(mdrop_id,0,MAX_MVP_DROP*sizeof(int));
			memset(mdrop_p,0,MAX_MVP_DROP*sizeof(int));

			if(battle_config.item_drop_mvp_mode == 1) {
				//Random order
				for(i = 0; i < MAX_MVP_DROP; i++) {
					while( 1 ) {
						uint8 va = rnd()%MAX_MVP_DROP;
						if (mdrop_id[va] == 0) {
							if (md->db->mvpitem[i].nameid > 0) {
								mdrop_id[va] = md->db->mvpitem[i].nameid;
								mdrop_p[va] = md->db->mvpitem[i].p;
							}
							else
								mdrop_id[va] = -1;
							break;
						}
					}
				}
			} else {
				//Normal order
				for(i = 0; i < MAX_MVP_DROP; i++) {
					if (md->db->mvpitem[i].nameid > 0) {
						mdrop_id[i] = md->db->mvpitem[i].nameid;
						mdrop_p[i] = md->db->mvpitem[i].p;
					}
					else
						mdrop_id[i] = -1;
				}
			}

			for(i = 0; i < MAX_MVP_DROP; i++) {
				struct item_data *i_data;

				if(mdrop_id[i] <= 0 || !itemdb_exists(mdrop_id[i]))
					continue;

				temp = mdrop_p[i];
				if (temp != 10000) {
					if(temp <= 0 && !battle_config.drop_rate0item)
						temp = 1;
					if(rnd()%10000 >= temp) //if ==0, then it doesn't drop
						continue;
				}

				memset(&item,0,sizeof(item));
				item.nameid=mdrop_id[i];
				item.identify= itemdb_isidentified(item.nameid);
				clif_mvp_item(mvp_sd,item.nameid);
				log_mvp[0] = item.nameid;

				i_data = itemdb_exists(item.nameid);

				//A Rare MVP Drop Global Announce by Lupus
				if(temp<=battle_config.rare_drop_announce) {
					char message[128];
					sprintf (message, msg_txt(NULL,541), mvp_sd->status.name, md->name, i_data->jname, temp/100.);
					//MSG: "'%s' won %s's %s (chance: %0.02f%%)"
					intif_broadcast(message,strlen(message)+1,BC_DEFAULT);
				}

				if((temp = pc_additem(mvp_sd,&item,1,LOG_TYPE_PICKDROP_PLAYER)) != 0) {
					clif_additem(mvp_sd,0,0,temp);
					map_addflooritem(&item,1,mvp_sd->bl.m,mvp_sd->bl.x,mvp_sd->bl.y,mvp_sd->status.char_id,(second_sd?second_sd->status.char_id:0),(third_sd?third_sd->status.char_id:0),1,0);
				}

				if (i_data->flag.broadcast)
					intif_broadcast_obtain_special_item(mvp_sd, item.nameid, md->mob_id, ITEMOBTAIN_TYPE_MONSTER_ITEM);

				//Logs items, MVP prizes [Lupus]
				log_pick_mob(md, LOG_TYPE_MVP, -1, &item);
				//If item_drop_mvp_mode is not 2, then only one item should be granted
				if(battle_config.item_drop_mvp_mode != 2) {
					break;
				}
			}
		}

		log_mvpdrop(mvp_sd, md->mob_id, log_mvp);
	}

	if (type&2 && !sd && md->mob_id == MOBID_EMPERIUM)
		//Emperium destroyed by script. Discard mvp character. [Skotlex]
		mvp_sd = NULL;

	rebirth =  ( md->sc.data[SC_KAIZEL] || (md->sc.data[SC_REBIRTH] && !md->state.rebirth) );
	if( !rebirth ) { // Only trigger event on final kill
		if( src ) {
			switch( src->type ) { //allowed type
				case BL_PET:
				case BL_HOM:
				case BL_MER:
				case BL_ELEM:
				case BL_MOB:
				    sd = BL_CAST(BL_PC,battle_get_master(src));
			}
		}

		if( sd ) {
			if( sd->mission_mobid == md->mob_id ||
				( battle_config.taekwon_mission_mobname == 1 && mob_is_goblin(md, sd->mission_mobid) ) ||
				( battle_config.taekwon_mission_mobname == 2 && mob_is_samename(md, sd->mission_mobid) ) )
			{ //TK_MISSION [Skotlex]
				if( ++sd->mission_count >= 100 && (temp = mob_get_random_id(MOBG_Branch_Of_Dead_Tree, 0xE, sd->status.base_level)) )
				{
					pc_addfame(sd, battle_config.fame_taekwon_mission);
					sd->mission_mobid = temp;
					pc_setglobalreg(sd, add_str("TK_MISSION_ID"), temp);
					sd->mission_count = 0;
					clif_mission_info(sd, temp, 0);
				}
				pc_setglobalreg(sd, add_str("TK_MISSION_COUNT"), sd->mission_count);
			}

			if( sd->status.party_id )
				map_foreachinrange(quest_update_objective_sub,&md->bl,AREA_SIZE,BL_PC,sd->status.party_id,md->mob_id);
			else if( sd->avail_quests )
				quest_update_objective(sd, md->mob_id);

			if( sd->md && src && src->type == BL_MER && mob_db(md->mob_id)->lv > sd->status.base_level/2 )
				mercenary_kills(sd->md);
		}

		if( md->npc_event[0] && !md->state.npc_killmonster ) {
			if( sd && battle_config.mob_npc_event_type ) {
				pc_setparam(sd, SP_KILLERRID, sd->bl.id);
				npc_event(sd,md->npc_event,0);
			} else if( mvp_sd ) {
				pc_setparam(mvp_sd, SP_KILLERRID, sd?sd->bl.id:0);
				npc_event(mvp_sd,md->npc_event,0);
			} else
				npc_event_do(md->npc_event);
		} else if( mvp_sd && !md->state.npc_killmonster ) {
			pc_setparam(mvp_sd, SP_KILLEDRID, md->mob_id);
			npc_script_event(mvp_sd, NPCE_KILLNPC); // PCKillNPC [Lance]
		}
	}

	if(md->deletetimer != INVALID_TIMER) {
		delete_timer(md->deletetimer,mob_timer_delete);
		md->deletetimer = INVALID_TIMER;
	}
	/**
	 * Only loops if necessary (e.g. a poring would never need to loop)
	 **/
	if( md->can_summon )
		mob_deleteslave(md);

	map_freeblock_unlock();

	if( !rebirth ) {

		if( pcdb_checkid(md->vd->class_) ) {//Player mobs are not removed automatically by the client.
			/* first we set them dead, then we delay the outsight effect */
			clif_clearunit_area(&md->bl,CLR_DEAD);
			clif_clearunit_delayed(&md->bl, CLR_OUTSIGHT,tick+3000);
		} else
			/**
			 * We give the client some time to breath and this allows it to display anything it'd like with the dead corpose
			 * For example, this delay allows it to display soul drain effect
			 **/
			clif_clearunit_delayed(&md->bl, CLR_DEAD, tick+250);

	}

	if(!md->spawn) //Tell status_damage to remove it from memory.
		return 5; // Note: Actually, it's 4. Oh well...

	// MvP tomb [GreenBox]
	if (battle_config.mvp_tomb_enabled && md->spawn->state.boss && map[md->bl.m].flag.notomb != 1)
		mvptomb_create(md, mvp_sd ? mvp_sd->status.name : NULL, time(NULL));

	if( !rebirth )
		mob_setdelayspawn(md); //Set respawning.
	return 3; //Remove from map.
}

/**
 * Resurect a mob with x hp (reset value and respawn on map)
 * @param md : mob pointer
 * @param hp : hp to resurect him with, @FIXME unused atm
 */
void mob_revive(struct mob_data *md, unsigned int hp)
{
	unsigned int tick = gettick();
	md->state.skillstate = MSS_IDLE;
	md->last_thinktime = tick;
	md->next_walktime = tick+rnd()%1000+MIN_RANDOMWALKTIME;
	md->last_linktime = tick;
	md->last_pcneartime = 0;
	memset(md->dmglog, 0, sizeof(md->dmglog));	// Reset the damage done on the rebirthed monster, otherwise will grant full exp + damage done. [Valaris]
	md->tdmg = 0;
	if (!md->bl.prev){
		if(map_addblock(&md->bl))
			return;
	}
	clif_spawn(&md->bl);
	skill_unit_move(&md->bl,tick,1);
	mobskill_use(md, tick, MSC_SPAWN);
	if (battle_config.show_mob_info&3)
		clif_charnameack (0, &md->bl);
}

int mob_guardian_guildchange(struct mob_data *md)
{
	struct guild *g;
	nullpo_ret(md);

	if (!md->guardian_data)
		return 0;

	if (md->guardian_data->castle->guild_id == 0)
	{	//Castle with no owner? Delete the guardians.
		if (md->mob_id == MOBID_EMPERIUM)
		{	//But don't delete the emperium, just clear it's guild-data
			md->guardian_data->guild_id = 0;
			md->guardian_data->emblem_id = 0;
			md->guardian_data->guild_name[0] = '\0';
		} else {
			if (md->guardian_data->number >= 0 && md->guardian_data->number < MAX_GUARDIANS && md->guardian_data->castle->guardian[md->guardian_data->number].visible)
				guild_castledatasave(md->guardian_data->castle->castle_id, 10+md->guardian_data->number, 0);
			unit_free(&md->bl,CLR_OUTSIGHT); //Remove guardian.
		}
		return 0;
	}

	g = guild_search(md->guardian_data->castle->guild_id);
	if (g == NULL)
	{	//Properly remove guardian info from Castle data.
		ShowError("mob_guardian_guildchange: New Guild (id %d) does not exists!\n", md->guardian_data->guild_id);
		if (md->guardian_data->number >= 0 && md->guardian_data->number < MAX_GUARDIANS)
			guild_castledatasave(md->guardian_data->castle->castle_id, 10+md->guardian_data->number, 0);
		unit_free(&md->bl,CLR_OUTSIGHT);
		return 0;
	}

	md->guardian_data->guild_id = g->guild_id;
	md->guardian_data->emblem_id = g->emblem_id;
	md->guardian_data->guardup_lv = guild_checkskill(g,GD_GUARDUP);
	memcpy(md->guardian_data->guild_name, g->name, NAME_LENGTH);

	return 1;
}

/*==========================================
 * Pick a random class for the mob
 *------------------------------------------*/
int mob_random_class (int *value, size_t count)
{
	nullpo_ret(value);

	// no count specified, look into the array manually, but take only max 5 elements
	if (count < 1) {
		count = 0;
		while(count < 5 && mobdb_checkid(value[count])) count++;
		if(count < 1)	// nothing found
			return 0;
	} else {
		// check if at least the first value is valid
		if(mobdb_checkid(value[0]) == 0)
			return 0;
	}
	//Pick a random value, hoping it exists. [Skotlex]
	return mobdb_checkid(value[rnd()%count]);
}

/*==========================================
 * Change mob base class
 *------------------------------------------*/
int mob_class_change (struct mob_data *md, int mob_id)
{
	unsigned int tick = gettick();
	int i, c, hp_rate;

	nullpo_ret(md);

	if( md->bl.prev == NULL )
		return 0;

	if (!mob_id || !mobdb_checkid(mob_id))
		return 0;

	//Disable class changing for some targets...
	if (md->guardian_data)
		return 0; //Guardians/Emperium

	if( mob_is_treasure(md) )
		return 0; //Treasure Boxes

	if( md->special_state.ai > AI_ATTACK )
		return 0; //Marine Spheres and Floras.

	if( mob_is_clone(md->mob_id) )
		return 0; //Clones

	if( md->mob_id == mob_id )
		return 0; //Nothing to change.

	hp_rate = get_percentage(md->status.hp, md->status.max_hp);
	md->mob_id = mob_id;
	md->db = mob_db(mob_id);
	if (battle_config.override_mob_names==1)
		memcpy(md->name,md->db->name,NAME_LENGTH);
	else
		memcpy(md->name,md->db->jname,NAME_LENGTH);

	status_change_end(&md->bl,SC_KEEPING,INVALID_TIMER); // End before calling status_calc_mob().
	mob_stop_attack(md);
	mob_stop_walking(md, 0);
	unit_skillcastcancel(&md->bl, 0);
	status_set_viewdata(&md->bl, mob_id);
	clif_mob_class_change(md,md->vd->class_);
	status_calc_mob(md,SCO_FIRST);
	md->ud.state.speed_changed = 1; //Speed change update.

	if (battle_config.monster_class_change_recover) {
		memset(md->dmglog, 0, sizeof(md->dmglog));
		md->tdmg = 0;
	} else {
		md->status.hp = md->status.max_hp*hp_rate/100;
		if(md->status.hp < 1) md->status.hp = 1;
	}

	for(i=0,c=tick-MOB_MAX_DELAY;i<MAX_MOBSKILL;i++)
		md->skilldelay[i] = c;

	if (md->lootitems == NULL && md->db->status.mode&MD_LOOTER)
		md->lootitems = (struct s_mob_lootitem *)aCalloc(LOOTITEM_SIZE,sizeof(struct s_mob_lootitem));

	//Targets should be cleared no morph
	md->target_id = md->attacked_id = 0;

	//Need to update name display.
	clif_charnameack(0, &md->bl);
	return 0;
}

/*==========================================
 * mob heal, update display hp info of mob for players
 *------------------------------------------*/
void mob_heal(struct mob_data *md,unsigned int heal)
{
	if (battle_config.show_mob_info&3)
		clif_charnameack (0, &md->bl);
#if PACKETVER >= 20120404
	if( !(md->status.mode&MD_BOSS) ){
		int i;
		for(i = 0; i < DAMAGELOG_SIZE; i++)// must show hp bar to all char who already hit the mob.
			if( md->dmglog[i].id ) {
				struct map_session_data *sd = map_charid2sd(md->dmglog[i].id);
				if( sd && check_distance_bl(&md->bl, &sd->bl, AREA_SIZE) ) // check if in range
					clif_monster_hp_bar(md, sd->fd);
			}
	}
#endif
}

/*==========================================
 * Added by RoVeRT
 *------------------------------------------*/
int mob_warpslave_sub(struct block_list *bl,va_list ap)
{
	struct mob_data *md=(struct mob_data *)bl;
	struct block_list *master;
	short x,y,range=0;
	master = va_arg(ap, struct block_list*);
	range = va_arg(ap, int);

	if(md->master_id!=master->id)
		return 0;

	map_search_freecell(master, 0, &x, &y, range, range, 0);
	unit_warp(&md->bl, master->m, x, y,CLR_TELEPORT);
	return 1;
}

/*==========================================
 * Added by RoVeRT
 * Warps slaves. Range is the area around the master that they can
 * appear in randomly.
 *------------------------------------------*/
int mob_warpslave(struct block_list *bl, int range)
{
	if (range < 1)
		range = 1; //Min range needed to avoid crashes and stuff. [Skotlex]

	return map_foreachinmap(mob_warpslave_sub, bl->m, BL_MOB, bl, range);
}

/*==========================================
 *  Counts slave sub, curently checking if mob master is the given ID.
 *------------------------------------------*/
int mob_countslave_sub(struct block_list *bl,va_list ap)
{
	int id;
	struct mob_data *md;
	id=va_arg(ap,int);

	md = (struct mob_data *)bl;
	if( md->master_id==id )
		return 1;
	return 0;
}

/*==========================================
 * Counts the number of slaves a mob has on the map.
 *------------------------------------------*/
int mob_countslave(struct block_list *bl)
{
	return map_foreachinmap(mob_countslave_sub, bl->m, BL_MOB,bl->id);
}

/*==========================================
 * Summons amount slaves contained in the value[5] array using round-robin. [adapted by Skotlex]
 *------------------------------------------*/
int mob_summonslave(struct mob_data *md2,int *value,int amount,uint16 skill_id)
{
	struct mob_data *md;
	struct spawn_data data;
	int count = 0,k=0,hp_rate=0;

	nullpo_ret(md2);
	nullpo_ret(value);

	memset(&data, 0, sizeof(struct spawn_data));
	data.m = md2->bl.m;
	data.x = md2->bl.x;
	data.y = md2->bl.y;
	data.num = 1;
	data.state.size = md2->special_state.size;
	data.state.ai = md2->special_state.ai;

	if(mobdb_checkid(value[0]) == 0)
		return 0;
	/**
	 * Flags this monster is able to summon; saves a worth amount of memory upon deletion
	 **/
	md2->can_summon = 1;

	while(count < 5 && mobdb_checkid(value[count])) count++;
	if(count < 1) return 0;
	if (amount > 0 && amount < count) { //Do not start on 0, pick some random sub subset [Skotlex]
		k = rnd()%count;
		amount+=k; //Increase final value by same amount to preserve total number to summon.
	}

	if (!battle_config.monster_class_change_recover &&
		(skill_id == NPC_TRANSFORMATION || skill_id == NPC_METAMORPHOSIS))
		hp_rate = get_percentage(md2->status.hp, md2->status.max_hp);

	for(;k<amount;k++) {
		short x,y;
		data.id = value[k%count]; //Summon slaves in round-robin fashion. [Skotlex]
		if (mobdb_checkid(data.id) == 0)
			continue;

		if (map_search_freecell(&md2->bl, 0, &x, &y, MOB_SLAVEDISTANCE, MOB_SLAVEDISTANCE, 0)) {
			data.x = x;
			data.y = y;
		} else {
			data.x = md2->bl.x;
			data.y = md2->bl.y;
		}

		//These two need to be loaded from the db for each slave.
		if(battle_config.override_mob_names==1)
			strcpy(data.name,"--en--");
		else
			strcpy(data.name,"--ja--");

		if (!mob_parse_dataset(&data))
			continue;

		md= mob_spawn_dataset(&data);
		if(skill_id == NPC_SUMMONSLAVE){
			md->master_id=md2->bl.id;
			md->special_state.ai = md2->special_state.ai;
		}
		mob_spawn(md);

		if (hp_rate) //Scale HP
			md->status.hp = md->status.max_hp*hp_rate/100;

		//Inherit the aggressive mode of the master.
		if (battle_config.slaves_inherit_mode && md->master_id)
		{
			switch (battle_config.slaves_inherit_mode) {
			case 1: //Always aggressive
				if (!(md->status.mode&MD_AGGRESSIVE))
					sc_start4(NULL,&md->bl, SC_MODECHANGE, 100,1,0, MD_AGGRESSIVE, 0, 0);
				break;
			case 2: //Always passive
				if (md->status.mode&MD_AGGRESSIVE)
					sc_start4(NULL,&md->bl, SC_MODECHANGE, 100,1,0, 0, MD_AGGRESSIVE, 0);
				break;
			default: //Copy master.
				if (md2->status.mode&MD_AGGRESSIVE)
					sc_start4(NULL,&md->bl, SC_MODECHANGE, 100,1,0, MD_AGGRESSIVE, 0, 0);
				else
					sc_start4(NULL,&md->bl, SC_MODECHANGE, 100,1,0, 0, MD_AGGRESSIVE, 0);
				break;
			}
		}

		if (md2->state.copy_master_mode)
			md->status.mode = md2->status.mode;

		clif_skill_nodamage(&md->bl,&md->bl,skill_id,amount,1);
	}

	return 0;
}

/*==========================================
 * MOBskill lookup (get skillindex through skill_id)
 * Returns -1 if not found.
 *------------------------------------------*/
int mob_skill_id2skill_idx(int mob_id,uint16 skill_id)
{
	int i, max = mob_db(mob_id)->maxskill;
	struct mob_skill *ms=mob_db(mob_id)->skill;

	if(ms==NULL)
		return -1;

	ARR_FIND( 0, max, i, ms[i].skill_id == skill_id );
	return ( i < max ) ? i : -1;
}

/*==========================================
 * Friendly Mob whose HP is decreasing by a nearby MOB is looked for.
 *------------------------------------------*/
int mob_getfriendhprate_sub(struct block_list *bl,va_list ap)
{
	int min_rate, max_rate,rate;
	struct block_list **fr;
	struct mob_data *md;

	md = va_arg(ap,struct mob_data *);
	min_rate=va_arg(ap,int);
	max_rate=va_arg(ap,int);
	fr=va_arg(ap,struct block_list **);

	if( md->bl.id == bl->id && !(battle_config.mob_ai&0x10))
		return 0;

	if ((*fr) != NULL) //A friend was already found.
		return 0;

	if (battle_check_target(&md->bl,bl,BCT_ENEMY)>0)
		return 0;

	rate = get_percentage(status_get_hp(bl), status_get_max_hp(bl));

	if (rate >= min_rate && rate <= max_rate)
		(*fr) = bl;
	return 1;
}
static struct block_list *mob_getfriendhprate(struct mob_data *md,int min_rate,int max_rate)
{
	struct block_list *fr=NULL;
	int type = BL_MOB;

	nullpo_retr(NULL, md);

	if (md->special_state.ai) //Summoned creatures. [Skotlex]
		type = BL_PC;

	map_foreachinrange(mob_getfriendhprate_sub, &md->bl, 8, type,md,min_rate,max_rate,&fr);
	return fr;
}
/*==========================================
 * Check hp rate of its master
 *------------------------------------------*/
struct block_list *mob_getmasterhpltmaxrate(struct mob_data *md,int rate)
{
	if( md && md->master_id > 0 )
	{
		struct block_list *bl = map_id2bl(md->master_id);
		if( bl && get_percentage(status_get_hp(bl), status_get_max_hp(bl)) < rate )
			return bl;
	}

	return NULL;
}
/*==========================================
 * What a status state suits by nearby MOB is looked for.
 *------------------------------------------*/
int mob_getfriendstatus_sub(struct block_list *bl,va_list ap)
{
	int cond1,cond2;
	struct mob_data **fr, *md, *mmd;
	int flag=0;

	nullpo_ret(bl);
	nullpo_ret(md=(struct mob_data *)bl);
	nullpo_ret(mmd=va_arg(ap,struct mob_data *));

	if( mmd->bl.id == bl->id && !(battle_config.mob_ai&0x10) )
		return 0;

	if (battle_check_target(&mmd->bl,bl,BCT_ENEMY)>0)
		return 0;
	cond1=va_arg(ap,int);
	cond2=va_arg(ap,int);
	fr=va_arg(ap,struct mob_data **);
	if( cond2==-1 ){
		int j;
		for(j=SC_COMMON_MIN;j<=SC_COMMON_MAX && !flag;j++){
			if ((flag=(md->sc.data[j] != NULL))) //Once an effect was found, break out. [Skotlex]
				break;
		}
	}else
		flag=( md->sc.data[cond2] != NULL );
	if( flag^( cond1==MSC_FRIENDSTATUSOFF ) )
		(*fr)=md;

	return 0;
}

struct mob_data *mob_getfriendstatus(struct mob_data *md,int cond1,int cond2)
{
	struct mob_data* fr = NULL;
	nullpo_ret(md);

	map_foreachinrange(mob_getfriendstatus_sub, &md->bl, 8,BL_MOB, md,cond1,cond2,&fr);
	return fr;
}

/*==========================================
 * Skill use judging
 *------------------------------------------*/
int mobskill_use(struct mob_data *md, unsigned int tick, int event)
{
	struct mob_skill *ms;
	struct block_list *fbl = NULL; //Friend bl, which can either be a BL_PC or BL_MOB depending on the situation. [Skotlex]
	struct block_list *bl;
	struct mob_data *fmd = NULL;
	int i,j,n;
	short skill_target;

	nullpo_ret(md);
	nullpo_ret(ms = md->db->skill);

	if (!battle_config.mob_skill_rate || md->ud.skilltimer != INVALID_TIMER || !md->db->maxskill || (status_get_mode(&md->bl)&MD_NOCAST_SKILL))
		return 0;

	if (event == -1 && DIFF_TICK(md->ud.canact_tick, tick) > 0)
		return 0; //Skill act delay only affects non-event skills.

	//Pick a starting position and loop from that.
	i = battle_config.mob_ai&0x100?rnd()%md->db->maxskill:0;
	for (n = 0; n < md->db->maxskill; i++, n++) {
		int c2, flag = 0;

		if (i == md->db->maxskill)
			i = 0;

		if (DIFF_TICK(tick, md->skilldelay[i]) < ms[i].delay)
			continue;

		c2 = ms[i].cond2;

		if (ms[i].state != md->state.skillstate) {
			if (md->state.skillstate != MSS_DEAD && (ms[i].state == MSS_ANY ||
				(ms[i].state == MSS_ANYTARGET && md->target_id && md->state.skillstate != MSS_LOOT)
			)) //ANYTARGET works with any state as long as there's a target. [Skotlex]
				;
			else
				continue;
		}
		if (rnd() % 10000 > ms[i].permillage) //Lupus (max value = 10000)
			continue;

		if (ms[i].cond1 == event)
			flag = 1; //Trigger skill.
		else if (ms[i].cond1 == MSC_SKILLUSED)
			flag = ((event & 0xffff) == MSC_SKILLUSED && ((event >> 16) == c2 || c2 == 0));
		else if(event == -1){
			//Avoid entering on defined events to avoid "hyper-active skill use" due to the overflow of calls to this function in battle.
			switch (ms[i].cond1)
			{
				case MSC_ALWAYS:
					flag = 1; break;
				case MSC_MYHPLTMAXRATE:		// HP< maxhp%
					flag = get_percentage(md->status.hp, md->status.max_hp);
					flag = (flag <= c2);
					break;
				case MSC_MYHPINRATE:
					flag = get_percentage(md->status.hp, md->status.max_hp);
					flag = (flag >= c2 && flag <= ms[i].val[0]);
					break;
				case MSC_MYSTATUSON:		// status[num] on
				case MSC_MYSTATUSOFF:		// status[num] off
					if (!md->sc.count) {
						flag = 0;
					} else if (ms[i].cond2 == -1) {
						for (j = SC_COMMON_MIN; j <= SC_COMMON_MAX; j++)
							if ((flag = (md->sc.data[j]!=NULL)) != 0)
								break;
					} else {
						flag = (md->sc.data[ms[i].cond2]!=NULL);
					}
					flag ^= (ms[i].cond1 == MSC_MYSTATUSOFF); break;
				case MSC_FRIENDHPLTMAXRATE:	// friend HP < maxhp%
					flag = ((fbl = mob_getfriendhprate(md, 0, ms[i].cond2)) != NULL); break;
				case MSC_FRIENDHPINRATE	:
					flag = ((fbl = mob_getfriendhprate(md, ms[i].cond2, ms[i].val[0])) != NULL); break;
				case MSC_FRIENDSTATUSON:	// friend status[num] on
				case MSC_FRIENDSTATUSOFF:	// friend status[num] off
					flag = ((fmd = mob_getfriendstatus(md, ms[i].cond1, ms[i].cond2)) != NULL); break;
				case MSC_SLAVELT:		// slave < num
					flag = (mob_countslave(&md->bl) < c2 ); break;
				case MSC_ATTACKPCGT:	// attack pc > num
					flag = (unit_counttargeted(&md->bl) > c2); break;
				case MSC_SLAVELE:		// slave <= num
					flag = (mob_countslave(&md->bl) <= c2 ); break;
				case MSC_ATTACKPCGE:	// attack pc >= num
					flag = (unit_counttargeted(&md->bl) >= c2); break;
				case MSC_AFTERSKILL:
					flag = (md->ud.skill_id == c2); break;
				case MSC_RUDEATTACKED:
					flag = (md->state.attacked_count >= RUDE_ATTACKED_COUNT);
					if (flag) md->state.attacked_count = 0;	//Rude attacked count should be reset after the skill condition is met. Thanks to Komurka [Skotlex]
					break;
				case MSC_MASTERHPLTMAXRATE:
					flag = ((fbl = mob_getmasterhpltmaxrate(md, ms[i].cond2)) != NULL); break;
				case MSC_MASTERATTACKED:
					flag = (md->master_id > 0 && (fbl=map_id2bl(md->master_id)) && unit_counttargeted(fbl) > 0); break;
				case MSC_ALCHEMIST:
					flag = (md->state.alchemist);
					break;
			}
		}

		if (!flag)
			continue; //Skill requisite failed to be fulfilled.

		//Execute skill
		skill_target = (md->db->status.mode&MD_RANDOMTARGET) ? MST_RANDOM : ms[i].target;
		if (skill_get_casttype(ms[i].skill_id) == CAST_GROUND)
		{	//Ground skill.
			short x, y;
			switch (skill_target) {
				case MST_RANDOM: //Pick a random enemy within skill range.
					bl = battle_getenemy(&md->bl, DEFAULT_ENEMY_TYPE(md),
						skill_get_range2(&md->bl, ms[i].skill_id, ms[i].skill_lv));
					break;
				case MST_TARGET:
				case MST_AROUND5:
				case MST_AROUND6:
				case MST_AROUND7:
				case MST_AROUND8:
					bl = map_id2bl(md->target_id);
					break;
				case MST_MASTER:
					bl = &md->bl;
					if (md->master_id)
						bl = map_id2bl(md->master_id);
					if (bl) //Otherwise, fall through.
						break;
				case MST_FRIEND:
					bl = fbl?fbl:(fmd?&fmd->bl:&md->bl);
					break;
				default:
					bl = &md->bl;
					break;
			}
			if (!bl) continue;

			x = bl->x;
		  	y = bl->y;
			// Look for an area to cast the spell around...
			if (skill_target >= MST_AROUND5) {
				j = skill_target >= MST_AROUND1?
					(skill_target-MST_AROUND1) +1:
					(skill_target-MST_AROUND5) +1;
				map_search_freecell(&md->bl, md->bl.m, &x, &y, j, j, 3);
			}
			md->skill_idx = i;
			map_freeblock_lock();
			if( !battle_check_range(&md->bl,bl,skill_get_range2(&md->bl, ms[i].skill_id,ms[i].skill_lv)) ||
				!unit_skilluse_pos2(&md->bl, x, y,ms[i].skill_id, ms[i].skill_lv,ms[i].casttime, ms[i].cancel) )
			{
				map_freeblock_unlock();
				continue;
			}
		} else {
			//Targetted skill
			switch (skill_target) {
				case MST_RANDOM: //Pick a random enemy within skill range.
					bl = battle_getenemy(&md->bl, DEFAULT_ENEMY_TYPE(md),
						skill_get_range2(&md->bl, ms[i].skill_id, ms[i].skill_lv));
					break;
				case MST_TARGET:
					bl = map_id2bl(md->target_id);
					break;
				case MST_MASTER:
					bl = &md->bl;
					if (md->master_id)
						bl = map_id2bl(md->master_id);
					if (bl) //Otherwise, fall through.
						break;
				case MST_FRIEND:
					if (fbl) {
						bl = fbl;
						break;
					} else if (fmd) {
						bl = &fmd->bl;
						break;
					} // else fall through
				default:
					bl = &md->bl;
					break;
			}
			if (!bl) continue;

			md->skill_idx = i;
			map_freeblock_lock();
			if( !battle_check_range(&md->bl,bl,skill_get_range2(&md->bl, ms[i].skill_id,ms[i].skill_lv)) ||
				!unit_skilluse_id2(&md->bl, bl->id,ms[i].skill_id, ms[i].skill_lv,ms[i].casttime, ms[i].cancel) )
			{
				map_freeblock_unlock();
				continue;
			}
		}
		//Skill used. Post-setups...
		if ( ms[ i ].msg_id ){ //Display color message [SnakeDrak]
			struct mob_chat *mc = mob_chat(ms[i].msg_id);
			char temp[CHAT_SIZE_MAX];
 			char name[NAME_LENGTH];
 			snprintf(name, sizeof name,"%s", md->name);
 			strtok(name, "#"); // discard extra name identifier if present [Daegaladh]
 			snprintf(temp, sizeof temp,"%s : %s", name, mc->msg);
			clif_messagecolor(&md->bl, mc->color, temp);
		}
		if(!(battle_config.mob_ai&0x200)) { //pass on delay to same skill.
			for (j = 0; j < md->db->maxskill; j++)
				if (md->db->skill[j].skill_id == ms[i].skill_id)
					md->skilldelay[j]=tick;
		} else
			md->skilldelay[i]=tick;
		map_freeblock_unlock();
		return 1;
	}
	//No skill was used.
	md->skill_idx = -1;
	return 0;
}
/*==========================================
 * Skill use event processing
 *------------------------------------------*/
int mobskill_event(struct mob_data *md, struct block_list *src, unsigned int tick, int flag)
{
	int target_id, res = 0;

	if(md->bl.prev == NULL || md->status.hp == 0)
		return 0;

	target_id = md->target_id;
	if (!target_id || battle_config.mob_changetarget_byskill)
		md->target_id = src->id;

	if (flag == -1)
		res = mobskill_use(md, tick, MSC_CASTTARGETED);
	else if ((flag&0xffff) == MSC_SKILLUSED)
		res = mobskill_use(md, tick, flag);
	else if (flag&BF_SHORT)
		res = mobskill_use(md, tick, MSC_CLOSEDATTACKED);
	else if (flag&BF_LONG && !(flag&BF_MAGIC)) //Long-attacked should not include magic.
		res = mobskill_use(md, tick, MSC_LONGRANGEATTACKED);

	if (!res)
	//Restore previous target only if skill condition failed to trigger. [Skotlex]
		md->target_id = target_id;
	//Otherwise check if the target is an enemy, and unlock if needed.
	else if (battle_check_target(&md->bl, src, BCT_ENEMY) <= 0)
		md->target_id = target_id;

	return res;
}

// Player cloned mobs. [Valaris]
int mob_is_clone(int mob_id)
{
	if(mob_id < MOB_CLONE_START || mob_id > MOB_CLONE_END)
		return 0;
	if (mob_db(mob_id) == mob_dummy)
		return 0;
	return mob_id;
}

//Flag values:
//&1: Set special ai (fight mobs, not players)
//If mode is not passed, a default aggressive mode is used.
//If master_id is passed, clone is attached to him.
//Returns: ID of newly crafted copy.
int mob_clone_spawn(struct map_session_data *sd, int16 m, int16 x, int16 y, const char *event, int master_id, int mode, int flag, unsigned int duration)
{
	int mob_id;
	int i,j,inf, fd;
	struct mob_data *md;
	struct mob_skill *ms;
	struct mob_db* db;
	struct status_data *status;

	nullpo_ret(sd);

	if(pc_isdead(sd) && master_id && flag&1)
		return 0;

	ARR_FIND( MOB_CLONE_START, MOB_CLONE_END, mob_id, mob_db_data[mob_id] == NULL );
	if(mob_id >= MOB_CLONE_END)
		return 0;

	db = mob_db_data[mob_id]=(struct mob_db*)aCalloc(1, sizeof(struct mob_db));
	status = &db->status;
	strcpy(db->sprite,sd->status.name);
	strcpy(db->name,sd->status.name);
	strcpy(db->jname,sd->status.name);
	db->lv=status_get_lv(&sd->bl);
	memcpy(status, &sd->base_status, sizeof(struct status_data));
	status->rhw.atk2= status->dex + status->rhw.atk + status->rhw.atk2; //Max ATK
	status->rhw.atk = status->dex; //Min ATK
	if (status->lhw.atk) {
		status->lhw.atk2= status->dex + status->lhw.atk + status->lhw.atk2; //Max ATK
		status->lhw.atk = status->dex; //Min ATK
	}
	if (mode) //User provided mode.
		status->mode = mode;
	else if (flag&1) //Friendly Character, remove looting.
		status->mode &= ~MD_LOOTER;
	status->hp = status->max_hp;
	status->sp = status->max_sp;
	memcpy(&db->vd, &sd->vd, sizeof(struct view_data));
	db->base_exp=1;
	db->job_exp=1;
	db->range2=AREA_SIZE; //Let them have the same view-range as players.
	db->range3=AREA_SIZE; //Min chase of a screen.
	db->option=sd->sc.option;

	//Skill copy [Skotlex]
	ms = &db->skill[0];

	/**
	 * We temporarily disable sd's fd so it doesn't receive the messages from skill_check_condition_castbegin
	 **/
	fd = sd->fd;
	sd->fd = 0;

	//Go Backwards to give better priority to advanced skills.
	for (i=0,j = MAX_SKILL_TREE-1;j>=0 && i< MAX_MOBSKILL ;j--) {
		uint16 skill_id = skill_tree[pc_class2idx(sd->status.class_)][j].id;
		uint16 sk_idx = 0;
		if (!skill_id || !(sk_idx = skill_get_index(skill_id)) || sd->status.skill[sk_idx].lv < 1 ||
			(skill_get_inf2(skill_id)&(INF2_WEDDING_SKILL|INF2_GUILD_SKILL)) ||
			skill_get_nocast(skill_id)&16
		)
			continue;
		//Normal aggressive mob, disable skills that cannot help them fight
		//against players (those with flags UF_NOMOB and UF_NOPC are specific
		//to always aid players!) [Skotlex]
		if (!(flag&1) &&
			skill_get_unit_id(skill_id, 0) &&
			skill_get_unit_flag(skill_id)&(UF_NOMOB|UF_NOPC))
			continue;
		/**
		 * The clone should be able to cast the skill (e.g. have the required weapon) bugreport:5299)
		 **/
		if( !skill_check_condition_castbegin(sd,skill_id,sd->status.skill[sk_idx].lv) )
			continue;

		memset (&ms[i], 0, sizeof(struct mob_skill));
		ms[i].skill_id = skill_id;
		ms[i].skill_lv = sd->status.skill[sk_idx].lv;
		ms[i].state = MSS_ANY;
		ms[i].permillage = 500*battle_config.mob_skill_rate/100; //Default chance of all skills: 5%
		ms[i].emotion = -1;
		ms[i].cancel = 0;
		ms[i].casttime = skill_castfix(&sd->bl,skill_id, ms[i].skill_lv);
		ms[i].delay = 5000+skill_delayfix(&sd->bl,skill_id, ms[i].skill_lv);

		inf = skill_get_inf(skill_id);
		if (inf&INF_ATTACK_SKILL) {
			ms[i].target = MST_TARGET;
			ms[i].cond1 = MSC_ALWAYS;
			if (skill_get_range(skill_id, ms[i].skill_lv)  > 3)
				ms[i].state = MSS_ANYTARGET;
			else
				ms[i].state = MSS_BERSERK;
		} else if(inf&INF_GROUND_SKILL) {
			if (skill_get_inf2(skill_id)&INF2_TRAP) { //Traps!
				ms[i].state = MSS_IDLE;
				ms[i].target = MST_AROUND2;
				ms[i].delay = 60000;
			} else if (skill_get_unit_target(skill_id) == BCT_ENEMY) { //Target Enemy
				ms[i].state = MSS_ANYTARGET;
				ms[i].target = MST_TARGET;
				ms[i].cond1 = MSC_ALWAYS;
			} else { //Target allies
				ms[i].target = MST_FRIEND;
				ms[i].cond1 = MSC_FRIENDHPLTMAXRATE;
				ms[i].cond2 = 95;
			}
		} else if (inf&INF_SELF_SKILL) {
			if (skill_get_inf2(skill_id)&INF2_NO_TARGET_SELF) { //auto-select target skill.
				ms[i].target = MST_TARGET;
				ms[i].cond1 = MSC_ALWAYS;
				if (skill_get_range(skill_id, ms[i].skill_lv)  > 3) {
					ms[i].state = MSS_ANYTARGET;
				} else {
					ms[i].state = MSS_BERSERK;
				}
			} else { //Self skill
				ms[i].target = MST_SELF;
				ms[i].cond1 = MSC_MYHPLTMAXRATE;
				ms[i].cond2 = 90;
				ms[i].permillage = 2000;
				//Delay: Remove the stock 5 secs and add half of the support time.
				ms[i].delay += -5000 +(skill_get_time(skill_id, ms[i].skill_lv) + skill_get_time2(skill_id, ms[i].skill_lv))/2;
				if (ms[i].delay < 5000)
					ms[i].delay = 5000; //With a minimum of 5 secs.
			}
		} else if (inf&INF_SUPPORT_SKILL) {
			ms[i].target = MST_FRIEND;
			ms[i].cond1 = MSC_FRIENDHPLTMAXRATE;
			ms[i].cond2 = 90;
			if (skill_id == AL_HEAL)
				ms[i].permillage = 5000; //Higher skill rate usage for heal.
			else if (skill_id == ALL_RESURRECTION)
				ms[i].cond2 = 1;
			//Delay: Remove the stock 5 secs and add half of the support time.
			ms[i].delay += -5000 +(skill_get_time(skill_id, ms[i].skill_lv) + skill_get_time2(skill_id, ms[i].skill_lv))/2;
			if (ms[i].delay < 2000)
				ms[i].delay = 2000; //With a minimum of 2 secs.

			if (i+1 < MAX_MOBSKILL) { //duplicate this so it also triggers on self.
				memcpy(&ms[i+1], &ms[i], sizeof(struct mob_skill));
				db->maxskill = ++i;
				ms[i].target = MST_SELF;
				ms[i].cond1 = MSC_MYHPLTMAXRATE;
			}
		} else {
			switch (skill_id) { //Certain Special skills that are passive, and thus, never triggered.
				case MO_TRIPLEATTACK:
				case TF_DOUBLE:
				case GS_CHAINACTION:
					ms[i].state = MSS_BERSERK;
					ms[i].target = MST_TARGET;
					ms[i].cond1 = MSC_ALWAYS;
					ms[i].permillage = skill_id==MO_TRIPLEATTACK?(3000-ms[i].skill_lv*100):(ms[i].skill_lv*500);
					ms[i].delay -= 5000; //Remove the added delay as these could trigger on "all hits".
					break;
				default: //Untreated Skill
					continue;
			}
		}
		if (battle_config.mob_skill_rate!= 100)
			ms[i].permillage = ms[i].permillage*battle_config.mob_skill_rate/100;
		if (battle_config.mob_skill_delay != 100)
			ms[i].delay = ms[i].delay*battle_config.mob_skill_delay/100;

		db->maxskill = ++i;
	}

	/**
	 * We grant the session it's fd value back.
	 **/
	sd->fd = fd;

	//Finally, spawn it.
	md = mob_once_spawn_sub(&sd->bl, m, x, y, "--en--", mob_id, event, SZ_SMALL, AI_NONE);
	if (!md) return 0; //Failed?

	md->special_state.clone = 1;

	if (master_id || flag || duration) { //Further manipulate crafted char.
		if (flag&1) //Friendly Character
			md->special_state.ai = AI_ATTACK;
		if (master_id) //Attach to Master
			md->master_id = master_id;
		if (duration) //Auto Delete after a while.
		{
			if( md->deletetimer != INVALID_TIMER )
				delete_timer(md->deletetimer, mob_timer_delete);
			md->deletetimer = add_timer (gettick() + duration, mob_timer_delete, md->bl.id, 0);
		}
	}

	mob_spawn(md);

	return md->bl.id;
}

int mob_clone_delete(struct mob_data *md)
{
	const int mob_id = md->mob_id;
	if (mob_id >= MOB_CLONE_START && mob_id < MOB_CLONE_END
		&& mob_db_data[mob_id]!=NULL) {
		aFree(mob_db_data[mob_id]);
		mob_db_data[mob_id]=NULL;
		//Clear references to the db
		md->db = mob_dummy;
		md->vd = NULL;
		return 1;
	}
	return 0;
}

//
// Initialization
//
/*==========================================
 * Since un-setting [ mob ] up was used, it is an initial provisional value setup.
 *------------------------------------------*/
static int mob_makedummymobdb(int mob_id)
{
	if (mob_dummy != NULL)
	{
		if (mob_db(mob_id) == mob_dummy)
			return 1; //Using the mob_dummy data already. [Skotlex]
		if (mob_id > 0 && mob_id <= MAX_MOB_DB)
		{	//Remove the mob data so that it uses the dummy data instead.
			aFree(mob_db_data[mob_id]);
			mob_db_data[mob_id] = NULL;
		}
		return 0;
	}
	//Initialize dummy data.
	mob_dummy = (struct mob_db*)aCalloc(1, sizeof(struct mob_db)); //Initializing the dummy mob.
	sprintf(mob_dummy->sprite,"DUMMY");
	sprintf(mob_dummy->name,"Dummy");
	sprintf(mob_dummy->jname,"Dummy");
	mob_dummy->lv=1;
	mob_dummy->status.max_hp=1000;
	mob_dummy->status.max_sp=1;
	mob_dummy->status.rhw.range=1;
	mob_dummy->status.rhw.atk=7;
	mob_dummy->status.rhw.atk2=10;
	mob_dummy->status.str=1;
	mob_dummy->status.agi=1;
	mob_dummy->status.vit=1;
	mob_dummy->status.int_=1;
	mob_dummy->status.dex=6;
	mob_dummy->status.luk=2;
	mob_dummy->status.speed=300;
	mob_dummy->status.adelay=1000;
	mob_dummy->status.amotion=500;
	mob_dummy->status.dmotion=500;
	mob_dummy->base_exp=2;
	mob_dummy->job_exp=1;
	mob_dummy->range2=10;
	mob_dummy->range3=10;

	return 0;
}

//Adjusts the drop rate of item according to the criteria given. [Skotlex]
static unsigned int mob_drop_adjust(int baserate, int rate_adjust, unsigned short rate_min, unsigned short rate_max)
{
	double rate = baserate;

	if (battle_config.logarithmic_drops && rate_adjust > 0 && rate_adjust != 100 && baserate > 0) //Logarithmic drops equation by Ishizu-Chan
		//Equation: Droprate(x,y) = x * (5 - log(x)) ^ (ln(y) / ln(5))
		//x is the normal Droprate, y is the Modificator.
		rate = rate * pow((5.0 - log10(rate)), (log(rate_adjust/100.) / log(5.0))) + 0.5;
	else
		//Classical linear rate adjustment.
		rate = rate * rate_adjust/100;

	return (unsigned int)cap_value(rate,rate_min,rate_max);
}

/**
 * Check if global item drop rate is overriden for given item
 * in db/mob_item_ratio.txt
 * @param nameid ID of the item
 * @param mob_id ID of the monster
 * @param rate_adjust pointer to store ratio if found
 */
static void item_dropratio_adjust(unsigned short nameid, int mob_id, int *rate_adjust)
{
	struct s_mob_item_drop_ratio *item_ratio = (struct s_mob_item_drop_ratio *)idb_get(mob_item_drop_ratio, nameid);
	if( item_ratio) {
		if( item_ratio->mob_id[0] ) { // only for listed mobs
			int i;
			ARR_FIND(0, MAX_ITEMRATIO_MOBS, i, item_ratio->mob_id[i] == mob_id);
			if( i < MAX_ITEMRATIO_MOBS ) // found
				*rate_adjust = item_ratio->drop_ratio;
		}
		else // for all mobs
			*rate_adjust = item_ratio->drop_ratio;
	}
}

/*==========================================
 * processes one mobdb entry
 *------------------------------------------*/
static bool mob_parse_dbrow(char** str)
{
	struct mob_db *db, entry;
	struct status_data *status;
	int mob_id, i;
	double exp, maxhp;
	struct mob_data data;

	mob_id = atoi(str[0]);

	if (mob_id <= 1000 || mob_id > MAX_MOB_DB) {
		ShowError("mob_parse_dbrow: Invalid monster ID %d, must be in range %d-%d.\n", mob_id, 1000, MAX_MOB_DB);
		return false;
	}
	if (pcdb_checkid(mob_id)) {
		ShowError("mob_parse_dbrow: Invalid monster ID %d, reserved for player classes.\n", mob_id);
		return false;
	}

	if (mob_id >= MOB_CLONE_START && mob_id < MOB_CLONE_END) {
		ShowError("mob_parse_dbrow: Invalid monster ID %d. Range %d-%d is reserved for player clones. Please increase MAX_MOB_DB (%d).\n", mob_id, MOB_CLONE_START, MOB_CLONE_END-1, MAX_MOB_DB);
		return false;
	}

	memset(&entry, 0, sizeof(entry));

	db = &entry;
	status = &db->status;

	db->vd.class_ = mob_id;
	safestrncpy(db->sprite, str[1], sizeof(db->sprite));
	safestrncpy(db->jname, str[2], sizeof(db->jname));
	safestrncpy(db->name, str[3], sizeof(db->name));
	db->lv = atoi(str[4]);
	db->lv = cap_value(db->lv, 1, USHRT_MAX);
	status->max_hp = atoi(str[5]);
	status->max_sp = atoi(str[6]);

	exp = (double)atoi(str[7]) * (double)battle_config.base_exp_rate / 100.;
	db->base_exp = (unsigned int)cap_value(exp, 0, UINT_MAX);

	exp = (double)atoi(str[8]) * (double)battle_config.job_exp_rate / 100.;
	db->job_exp = (unsigned int)cap_value(exp, 0, UINT_MAX);

	status->rhw.range = atoi(str[9]);
	status->rhw.atk = atoi(str[10]);
	status->rhw.atk2 = atoi(str[11]);
	status->def = atoi(str[12]);
	status->mdef = atoi(str[13]);
	status->str = atoi(str[14]);
	status->agi = atoi(str[15]);
	status->vit = atoi(str[16]);
	status->int_ = atoi(str[17]);
	status->dex = atoi(str[18]);
	status->luk = atoi(str[19]);
	//All status should be min 1 to prevent divisions by zero from some skills. [Skotlex]
	if (status->str < 1) status->str = 1;
	if (status->agi < 1) status->agi = 1;
	if (status->vit < 1) status->vit = 1;
	if (status->int_< 1) status->int_= 1;
	if (status->dex < 1) status->dex = 1;
	if (status->luk < 1) status->luk = 1;

	db->range2 = atoi(str[20]);
	db->range3 = atoi(str[21]);
	if (battle_config.view_range_rate != 100) {
		db->range2 = db->range2 * battle_config.view_range_rate / 100;
		if (db->range2 < 1)
			db->range2 = 1;
	}
	if (battle_config.chase_range_rate != 100) {
		db->range3 = db->range3 * battle_config.chase_range_rate / 100;
		if (db->range3 < db->range2)
			db->range3 = db->range2;
	}
	//Tests showed that chase range is effectively 2 cells larger than expected [Playtester]
	db->range3 += 2;

	status->size = atoi(str[22]);
	status->race = atoi(str[23]);

	i = atoi(str[24]); //Element
	status->def_ele = i%20;
	status->ele_lv = (unsigned char)floor(i/20.);
	if (!CHK_ELEMENT(status->def_ele)) {
		ShowError("mob_parse_dbrow: Invalid element type %d for monster ID %d (max=%d).\n", status->def_ele, mob_id, ELE_ALL-1);
		return false;
	}
	if (!CHK_ELEMENT_LEVEL(status->ele_lv)) {
		ShowError("mob_parse_dbrow: Invalid element level %d for monster ID %d, must be in range 1-%d.\n", status->ele_lv, mob_id, MAX_ELE_LEVEL);
		return false;
	}

	status->mode = (int)strtol(str[25], NULL, 0);
	if (!battle_config.monster_active_enable)
		status->mode &= ~MD_AGGRESSIVE;

	if( status->mode&MD_BOSS )
		status->class_ = CLASS_BOSS;
	else if( mob_is_guardian(mob_id) )
		status->class_ = CLASS_GUARDIAN;
	else
		status->class_ = CLASS_NORMAL;

	status->speed = atoi(str[26]);
	status->aspd_rate = 1000;
	i = atoi(str[27]);
	status->adelay = cap_value(i, battle_config.monster_max_aspd*2, 4000);
	i = atoi(str[28]);
	status->amotion = cap_value(i, battle_config.monster_max_aspd, 2000);
	//If the attack animation is longer than the delay, the client crops the attack animation!
	//On aegis there is no real visible effect of having a recharge-time less than amotion anyway.
	if (status->adelay < status->amotion)
		status->adelay = status->amotion;
	status->dmotion = atoi(str[29]);
	if(battle_config.monster_damage_delay_rate != 100)
		status->dmotion = status->dmotion * battle_config.monster_damage_delay_rate / 100;

	// Fill in remaining status data by using a dummy monster.
	data.bl.type = BL_MOB;
	data.level = db->lv;
	memcpy(&data.status, status, sizeof(struct status_data));
	status_calc_misc(&data.bl, status, db->lv);

	// MVP EXP Bonus: MEXP
	// Some new MVP's MEXP multipled by high exp-rate cause overflow. [LuzZza]
	exp = (double)atoi(str[30]) * (double)battle_config.mvp_exp_rate / 100.;
	db->mexp = (unsigned int)cap_value(exp, 0, UINT_MAX);

	//Now that we know if it is an mvp or not, apply battle_config modifiers [Skotlex]
	maxhp = (double)status->max_hp;
	if (db->mexp > 0) { //Mvp
		if (battle_config.mvp_hp_rate != 100)
			maxhp = maxhp * (double)battle_config.mvp_hp_rate / 100.;
	} else //Normal mob
		if (battle_config.monster_hp_rate != 100)
			maxhp = maxhp * (double)battle_config.monster_hp_rate / 100.;

	status->max_hp = (unsigned int)cap_value(maxhp, 1, UINT_MAX);
	if(status->max_sp < 1) status->max_sp = 1;

	//Since mobs always respawn with full life...
	status->hp = status->max_hp;
	status->sp = status->max_sp;

	// MVP Drops: MVP1id,MVP1per,MVP2id,MVP2per,MVP3id,MVP3per
	for(i = 0; i < MAX_MVP_DROP; i++) {
		db->mvpitem[i].nameid = atoi(str[31+i*2]);

		if( db->mvpitem[i].nameid ){
			if( itemdb_search(db->mvpitem[i].nameid) ){
				db->mvpitem[i].p = atoi(str[32+i*2]);
				continue;
			}else{
				ShowWarning( "Monster \"%s\"(id: %d) is dropping an unknown item \"%s\"(MVP-Drop %d)\n", db->name, mob_id, str[31+i*2], ( i / 2 ) + 1 );
			}
		}

		// Delete the item
		db->mvpitem[i].nameid = 0;
		db->mvpitem[i].p = 0;
	}

	for(i = 0; i < MAX_MOB_DROP; i++) {
		int k = 31 + MAX_MVP_DROP*2 + i*2;

		db->dropitem[i].nameid = atoi(str[k]);

		if( db->dropitem[i].nameid ){
			if( itemdb_search( db->dropitem[i].nameid ) ){
				db->dropitem[i].p = atoi(str[k+1]);
				continue;
			}else{
				ShowWarning( "Monster \"%s\"(id: %d) is dropping an unknown item \"%s\"(Drop %d)\n", db->name, mob_id, str[k], ( i / 2 ) + 1 );
			}
		}

		// Delete the item
		db->dropitem[i].nameid = 0;
		db->dropitem[i].p = 0;
	}

	// Finally insert monster's data into the database.
	if (mob_db_data[mob_id] == NULL)
		mob_db_data[mob_id] = (struct mob_db*)aCalloc(1, sizeof(struct mob_db));
	else
		//Copy over spawn data
		memcpy(&db->spawn, mob_db_data[mob_id]->spawn, sizeof(db->spawn));

	memcpy(mob_db_data[mob_id], db, sizeof(struct mob_db));
	return true;
}

/*==========================================
 * mob_db.txt reading
 *------------------------------------------*/
static bool mob_readdb_sub(char* fields[], int columns, int current)
{
	return mob_parse_dbrow(fields);
}

/*==========================================
 * mob_db table reading
 *------------------------------------------*/
static int mob_read_sqldb(void)
{
	const char* mob_db_name[] = {
#ifndef RENEWAL
		mob_db_db,
#else
		mob_db_re_db,
#endif
		mob_db2_db };
	int fi;

	for( fi = 0; fi < ARRAYLENGTH(mob_db_name); ++fi ) {
		uint32 lines = 0, count = 0;

		// retrieve all rows from the mob database
		if( SQL_ERROR == Sql_Query(mmysql_handle, "SELECT * FROM `%s`", mob_db_name[fi]) ) {
			Sql_ShowDebug(mmysql_handle);
			continue;
		}

		// process rows one by one
		while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ) {
			// wrap the result into a TXT-compatible format
			char line[1024];
			char* str[31+2*MAX_MVP_DROP+2*MAX_MOB_DROP];
			char* p;
			int i;

			lines++;
			for(i = 0, p = line; i < 31+2*MAX_MVP_DROP+2*MAX_MOB_DROP; i++)
			{
				char* data;
				size_t len;
				Sql_GetData(mmysql_handle, i, &data, &len);

				strcpy(p, data);
				str[i] = p;
				p+= len + 1;
			}

			if (!mob_parse_dbrow(str))
				continue;

			count++;
		}

		// free the query result
		Sql_FreeResult(mmysql_handle);

		ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, mob_db_name[fi]);
	}
	return 0;
}

/*==========================================
 * MOB display graphic change data reading
 *------------------------------------------*/
static bool mob_readdb_mobavail(char* str[], int columns, int current)
{
	int mob_id, sprite_id;

	mob_id = atoi(str[0]);

	if(mob_db(mob_id) == mob_dummy)	// invalid class (probably undefined in db)
	{
		ShowWarning("mob_readdb_mobavail: Unknown mob id %d.\n", mob_id);
		return false;
	}

	sprite_id = atoi(str[1]);

	memset(&mob_db_data[mob_id]->vd, 0, sizeof(struct view_data));
	mob_db_data[mob_id]->vd.class_ = sprite_id;

	//Player sprites
	if(pcdb_checkid(sprite_id) && columns==12) {
		mob_db_data[mob_id]->vd.sex=atoi(str[2]);
		mob_db_data[mob_id]->vd.hair_style=atoi(str[3]);
		mob_db_data[mob_id]->vd.hair_color=atoi(str[4]);
		mob_db_data[mob_id]->vd.weapon=atoi(str[5]);
		mob_db_data[mob_id]->vd.shield=atoi(str[6]);
		mob_db_data[mob_id]->vd.head_top=atoi(str[7]);
		mob_db_data[mob_id]->vd.head_mid=atoi(str[8]);
		mob_db_data[mob_id]->vd.head_bottom=atoi(str[9]);
		mob_db_data[mob_id]->option=atoi(str[10])&~(OPTION_HIDE|OPTION_CLOAK|OPTION_INVISIBLE);
		mob_db_data[mob_id]->vd.cloth_color=atoi(str[11]); // Monster player dye option - Valaris
	}
	else if(columns==3)
		mob_db_data[mob_id]->vd.head_bottom=atoi(str[2]); // mob equipment [Valaris]
	else if( columns != 2 )
		return false;

	return true;
}

/*==========================================
 * Reading of random monster data
 * MobGroup,MobID,DummyName,Rate
 *------------------------------------------*/
static int mob_read_randommonster_sub(const char* filename, bool silent) {
	FILE *fp;
	char line[1024];
	unsigned int entries = 0;

	if ((fp = fopen(filename,"r")) == NULL) {
		if (!silent)
			ShowError("mob_read_randommonster: can't read %s\n",filename);
		return -1;
	}

	while(fgets(line, sizeof(line), fp)) {
		struct s_randomsummon_group *msummon = NULL;
		int mob_id, group = 0;
		unsigned short i = 0;
		char *str[4], *p;
		bool set_default = false;

		if (line[0] == '/' && line[1] == '/')
			continue;
		if (strstr(line,"import")) {
			char w1[16], w2[64];

			if (sscanf(line,"%15[^:]: %63[^\r\n]",w1,w2) == 2 &&
				strcmpi(w1,"import") == 0)
			{
				mob_read_randommonster_sub(w2, 0);
				continue;
			}
		}
		p = line;
		while(ISSPACE(*p))
			++p;
		memset(str,0,sizeof(str));
		for (i = 0; i < 5 && p; i++) {
			str[i] = p;
			p = strchr(p,',');
			if (p)
				*p++=0;
		}

		if (str[0] == NULL || str[2] == NULL)
			continue;

		if (ISDIGIT(str[0][0]) && ISDIGIT(str[0][1]))
			group = atoi(str[0]);
		else if (!script_get_constant(str[0], &group)) {
			ShowError("mob_read_randommonster_sub: Invalid random monster group '%s' at line '%s'.\n", str[0], line);
			continue;
		}

		mob_id = atoi(str[1]);
		if (mob_id != 0 && mob_db(mob_id) == mob_dummy) {
			ShowError("mob_read_randommonster_sub: Invalid random monster group '%s' at line '%s'.\n", str[0], line);
			continue;
		}
		else if (mob_id == 0){
			mob_id = atoi(str[3]);
			if (mob_db(mob_id) == mob_dummy) {
				ShowError("mob_read_randommonster_sub: Invalid random monster group '%s' at line '%s'.\n", str[0], line);
				continue;
			}
			set_default = true;
		}

		if (!(msummon = (struct s_randomsummon_group *)idb_get(mob_summon_db, group))) {
			CREATE(msummon, struct s_randomsummon_group, 1);
			CREATE(msummon->list, struct s_randomsummon_entry, (msummon->count = 1));
			msummon->list[0].mob_id = mob_id;
			msummon->list[0].rate = atoi(str[3]);
			msummon->random_id = group;
			idb_put(mob_summon_db, group, msummon);
		}
		else {
			ARR_FIND(0, msummon->count, i, set_default || (i > 0 && msummon->list[i].mob_id == mob_id));
			if (i >= msummon->count)
				RECREATE(msummon->list, struct s_randomsummon_entry, ++msummon->count);
			msummon->list[i].mob_id = mob_id;
			msummon->list[i].rate = atoi(str[3]);
		}

		entries++;
	}

	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", entries, filename);
	return 0;
}

static void mob_read_randommonster(const char* basedir, bool silent) {
	char filepath[256];
	sprintf(filepath, "%s/%s", basedir, "mob_random_db.txt");
	mob_read_randommonster_sub(filepath, silent);
	return;
}

//processes one mob_chat_db entry [SnakeDrak]
//db struct: Line_ID,Color_Code,Dialog
static bool mob_parse_row_chatdb(char* fields[], int columns, int current)
{
	char* msg;
	struct mob_chat *ms;
	int msg_id;
	size_t len;

	msg_id = atoi(fields[0]);

	if (msg_id <= 0 || msg_id > MAX_MOB_CHAT){
		ShowError("mob_parse_row_chatdb: Invalid chat ID: %d at %s, line %d\n", msg_id, columns, current);
		return false;
	}

	if (mob_chat_db[msg_id] == NULL)
		mob_chat_db[msg_id] = (struct mob_chat*)aCalloc(1, sizeof (struct mob_chat));

	ms = mob_chat_db[msg_id];
	//MSG ID
	ms->msg_id=msg_id;
	//Color
	ms->color=strtoul(fields[1],NULL,0);
	//Message
	msg = fields[2];
	len = strlen(msg);

	while( len && ( msg[len-1]=='\r' || msg[len-1]=='\n' ) )
	{// find EOL to strip
		len--;
	}

	if(len>(CHAT_SIZE_MAX-1)){
		ShowError("mob_chat: readdb: Message too long! Line %d, id: %d\n", current, msg_id);
		return false;
	}
	else if( !len ){
		ShowWarning("mob_parse_row_chatdb: Empty message for id %d.\n", msg_id);
		return false;
	}

	msg[len] = 0;  // strip previously found EOL
	safestrncpy(ms->msg, fields[2], CHAT_SIZE_MAX);

	return true;
}

/*==========================================
 * processes one mob_skill_db entry
 *------------------------------------------*/
static bool mob_parse_row_mobskilldb(char** str, int columns, int current)
{
	static const struct {
		char str[32];
		enum MobSkillState id;
	} state[] = {
		{	"any",		MSS_ANY		}, //All states except Dead
		{	"idle",		MSS_IDLE	},
		{	"walk",		MSS_WALK	},
		{	"loot",		MSS_LOOT	},
		{	"dead",		MSS_DEAD	},
		{	"attack",	MSS_BERSERK	}, //Retaliating attack
		{	"angry",	MSS_ANGRY	}, //Preemptive attack (aggressive mobs)
		{	"chase",	MSS_RUSH	}, //Chase escaping target
		{	"follow",	MSS_FOLLOW	}, //Preemptive chase (aggressive mobs)
		{	"anytarget",MSS_ANYTARGET	}, //Berserk+Angry+Rush+Follow
	};
	static const struct {
		char str[32];
		int id;
	} cond1[] = {
		// enum e_mob_skill_condition
		{ "always",            MSC_ALWAYS            },
		{ "myhpltmaxrate",     MSC_MYHPLTMAXRATE     },
		{ "myhpinrate",        MSC_MYHPINRATE        },
		{ "friendhpltmaxrate", MSC_FRIENDHPLTMAXRATE },
		{ "friendhpinrate",    MSC_FRIENDHPINRATE    },
		{ "mystatuson",        MSC_MYSTATUSON        },
		{ "mystatusoff",       MSC_MYSTATUSOFF       },
		{ "friendstatuson",    MSC_FRIENDSTATUSON    },
		{ "friendstatusoff",   MSC_FRIENDSTATUSOFF   },
		{ "attackpcgt",        MSC_ATTACKPCGT        },
		{ "attackpcge",        MSC_ATTACKPCGE        },
		{ "slavelt",           MSC_SLAVELT           },
		{ "slavele",           MSC_SLAVELE           },
		{ "closedattacked",    MSC_CLOSEDATTACKED    },
		{ "longrangeattacked", MSC_LONGRANGEATTACKED },
		{ "skillused",         MSC_SKILLUSED         },
		{ "afterskill",        MSC_AFTERSKILL        },
		{ "casttargeted",      MSC_CASTTARGETED      },
		{ "rudeattacked",      MSC_RUDEATTACKED      },
		{ "masterhpltmaxrate", MSC_MASTERHPLTMAXRATE },
		{ "masterattacked",    MSC_MASTERATTACKED    },
		{ "alchemist",         MSC_ALCHEMIST         },
		{ "onspawn",           MSC_SPAWN             },
	}, cond2[] ={
		{	"anybad",		-1				},
		{	"stone",		SC_STONE		},
		{	"freeze",		SC_FREEZE		},
		{	"stun",			SC_STUN			},
		{	"sleep",		SC_SLEEP		},
		{	"poison",		SC_POISON		},
		{	"curse",		SC_CURSE		},
		{	"silence",		SC_SILENCE		},
		{	"confusion",	SC_CONFUSION	},
		{	"blind",		SC_BLIND		},
		{	"hiding",		SC_HIDING		},
		{	"sight",		SC_SIGHT		},
	}, target[] = {
		// enum e_mob_skill_target
		{	"target",	MST_TARGET	},
		{	"randomtarget",	MST_RANDOM	},
		{	"self",		MST_SELF	},
		{	"friend",	MST_FRIEND	},
		{	"master",	MST_MASTER	},
		{	"around5",	MST_AROUND5	},
		{	"around6",	MST_AROUND6	},
		{	"around7",	MST_AROUND7	},
		{	"around8",	MST_AROUND8	},
		{	"around1",	MST_AROUND1	},
		{	"around2",	MST_AROUND2	},
		{	"around3",	MST_AROUND3	},
		{	"around4",	MST_AROUND4	},
		{	"around",	MST_AROUND	},
	};
	static int last_mob_id = 0;  // ensures that only one error message per mob id is printed

	struct s_mob_skill *skill = NULL;
	struct mob_skill *ms = NULL;
	int mob_id;
	int i = 0, j, tmp;

	mob_id = atoi(str[0]);

	if (mob_id > 0 && mob_db(mob_id) == mob_dummy)
	{
		if (mob_id != last_mob_id) {
			ShowError("mob_parse_row_mobskilldb: Non existant Mob id %d\n", mob_id);
			last_mob_id = mob_id;
		}
		return false;
	}
	else if (mob_id == 0)
		return false;

	// Looking for existing entry
	if (!(skill = (struct s_mob_skill *)idb_get(mob_skill_db, mob_id)))
		CREATE(skill, struct s_mob_skill, 1);

	if( strcmp(str[1],"clear") == 0 && skill->mob_id != 0 ) {
		idb_remove(mob_skill_db, skill->mob_id);
		aFree(skill);
		ShowInfo("Cleared skill for mob id '%d'\n", mob_id);
		return true;
	}

	ARR_FIND( 0, MAX_MOBSKILL, i, skill->skill[i].skill_id == 0 );
	if( i == MAX_MOBSKILL )
	{
		if (mob_id != last_mob_id) {
			ShowError("mob_parse_row_mobskilldb: Too many skills for monster %d[%s]\n", mob_id, mob_db_data[mob_id]->sprite);
			last_mob_id = mob_id;
		}
		return false;
	}

	ms = &skill->skill[i];

	//State
	ARR_FIND( 0, ARRAYLENGTH(state), j, strcmp(str[2],state[j].str) == 0 );
	if( j < ARRAYLENGTH(state) )
		ms->state = state[j].id;
	else {
		ShowWarning("mob_parse_row_mobskilldb: Unrecognized state %s\n", str[2]);
		ms->state = MSS_ANY;
	}

	//Skill ID
	j = atoi(str[3]);
	if (j <= 0 || j > MAX_SKILL_ID || !skill_get_index(j)) //fixed Lupus
	{
		if (mob_id < 0)
			ShowError("mob_parse_row_mobskilldb: Invalid Skill ID (%d) for all mobs\n", j);
		else
			ShowError("mob_parse_row_mobskilldb: Invalid Skill ID (%d) for mob %d (%s)\n", j, mob_id, mob_db_data[mob_id]->sprite);
		return false;
	}
	ms->skill_id = j;

	//Skill lvl
	j = atoi(str[4]) <= 0 ? 1 : atoi(str[4]);
	ms->skill_lv = j > battle_config.mob_max_skilllvl ? battle_config.mob_max_skilllvl : j; //we strip max skill level

	//Apply battle_config modifiers to rate (permillage) and delay [Skotlex]
	tmp = atoi(str[5]);
	if (battle_config.mob_skill_rate != 100)
		tmp = tmp*battle_config.mob_skill_rate/100;
	if (tmp > 10000)
		ms->permillage = 10000;
	else if (!tmp && battle_config.mob_skill_rate)
		ms->permillage = 1;
	else
		ms->permillage = tmp;
	ms->casttime = atoi(str[6]);
	ms->delay = atoi(str[7]);
	if (battle_config.mob_skill_delay != 100)
		ms->delay = ms->delay*battle_config.mob_skill_delay/100;
	if (ms->delay < 0 || ms->delay > MOB_MAX_DELAY) //time overflow?
		ms->delay = MOB_MAX_DELAY;
	ms->cancel = atoi(str[8]);
	if( strcmp(str[8],"yes")==0 )
		ms->cancel=1;

	//Target
	ARR_FIND( 0, ARRAYLENGTH(target), j, strcmp(str[9],target[j].str) == 0 );
	if( j < ARRAYLENGTH(target) )
		ms->target = target[j].id;
	else {
		ShowWarning("mob_parse_row_mobskilldb: Unrecognized target %s for %d\n", str[9], mob_id);
		ms->target = MST_TARGET;
	}

	//Check that the target condition is right for the skill type. [Skotlex]
	if (skill_get_casttype(ms->skill_id) == CAST_GROUND)
	{	//Ground skill.
		if (ms->target > MST_AROUND)
		{
			ShowWarning("mob_parse_row_mobskilldb: Wrong mob skill target for ground skill %d (%s) for %s.\n",
				ms->skill_id, skill_get_name(ms->skill_id),
				mob_id < 0 ? "all mobs" : mob_db_data[mob_id]->sprite);
			ms->target = MST_TARGET;
		}
	} else if (ms->target > MST_MASTER) {
		ShowWarning("mob_parse_row_mobskilldb: Wrong mob skill target 'around' for non-ground skill %d (%s) for %s.\n",
			ms->skill_id, skill_get_name(ms->skill_id),
			mob_id < 0 ? "all mobs" : mob_db_data[mob_id]->sprite);
		ms->target = MST_TARGET;
	}

	//Cond1
	ARR_FIND( 0, ARRAYLENGTH(cond1), j, strcmp(str[10],cond1[j].str) == 0 );
	if( j < ARRAYLENGTH(cond1) )
		ms->cond1 = cond1[j].id;
	else {
		ShowWarning("mob_parse_row_mobskilldb: Unrecognized condition 1 %s for %d\n", str[10], mob_id);
		ms->cond1 = -1;
	}

	//Cond2
	// numeric value
	ms->cond2 = atoi(str[11]);
	// or special constant
	ARR_FIND( 0, ARRAYLENGTH(cond2), j, strcmp(str[11],cond2[j].str) == 0 );
	if( j < ARRAYLENGTH(cond2) )
		ms->cond2 = cond2[j].id;

	ms->val[0] = (int)strtol(str[12],NULL,0);
	ms->val[1] = (int)strtol(str[13],NULL,0);
	ms->val[2] = (int)strtol(str[14],NULL,0);
	ms->val[3] = (int)strtol(str[15],NULL,0);
	ms->val[4] = (int)strtol(str[16],NULL,0);

	if(ms->skill_id == NPC_EMOTION && mob_id > 0 &&
		ms->val[1] == mob_db(mob_id)->status.mode)
	{
		ms->val[1] = 0;
		ms->val[4] = 1; //request to return mode to normal.
	}
	if(ms->skill_id == NPC_EMOTION_ON && mob_id > 0 && ms->val[1])
	{	//Adds a mode to the mob.
		//Remove aggressive mode when the new mob type is passive.
		if (!(ms->val[1]&MD_AGGRESSIVE))
			ms->val[3] |= MD_AGGRESSIVE;
		ms->val[2] |= ms->val[1]; //Add the new mode.
		ms->val[1] = 0; //Do not "set" it.
	}

	if(*str[17])
		ms->emotion = atoi(str[17]);
	else
		ms->emotion = -1;

	if(str[18] != NULL && mob_chat_db[atoi(str[18])]!=NULL)
		ms->msg_id = atoi(str[18]);
	else
		ms->msg_id = 0;

	skill->count++;
	if (!skill->mob_id) { // Insert new entry
		skill->mob_id = mob_id;
		idb_put(mob_skill_db, mob_id, skill);
	}
	return true;
}

/*==========================================
 * mob_skill_db.txt reading
 *------------------------------------------*/
static void mob_readskilldb(const char* basedir, bool silent) {
	if( battle_config.mob_skill_rate == 0 ) {
		ShowStatus("Mob skill use disabled. Not reading mob skills.\n");
		return;
	}
	sv_readdb(basedir, "mob_skill_db.txt", ',', 19, 19, -1, &mob_parse_row_mobskilldb, silent);
}

/**
 * mob_skill_db table reading [CalciumKid]
 * not overly sure if this is all correct
 * seems to work though...
 */
static int mob_read_sqlskilldb(void)
{
	const char* mob_skill_db_name[] = {
#ifndef RENEWAL
		mob_skill_db_db,
#else
		mob_skill_db_re_db,
#endif
		mob_skill_db2_db };
	int fi;

	if( battle_config.mob_skill_rate == 0 ) {
		ShowStatus("Mob skill use disabled. Not reading mob skills.\n");
		return 0;
	}

	for( fi = 0; fi < ARRAYLENGTH(mob_skill_db_name); ++fi ) {
		uint32 lines = 0, count = 0;

		// retrieve all rows from the mob skill database
		if( SQL_ERROR == Sql_Query(mmysql_handle, "SELECT * FROM `%s`", mob_skill_db_name[fi]) ) {
			Sql_ShowDebug(mmysql_handle);
			continue;
		}

		// process rows one by one
		while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ) {
			// wrap the result into a TXT-compatible format
			char* str[19];
			char* dummy = "";
			int i;
			++lines;
			for( i = 0; i < 19; ++i )
			{
				Sql_GetData(mmysql_handle, i, &str[i], NULL);
				if( str[i] == NULL ) str[i] = dummy; // get rid of NULL columns
			}

			if (!mob_parse_row_mobskilldb(str, 19, count))
				continue;

			count++;
		}

		// free the query result
		Sql_FreeResult(mmysql_handle);

		ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, mob_skill_db_name[fi]);
	}
	return 0;
}

/*==========================================
 * mob_race2_db.txt reading
 *------------------------------------------*/
static bool mob_readdb_race2(char* fields[], int columns, int current)
{
	int race, i;

	race = atoi(fields[0]);

	if (!CHK_RACE2(race))
	{
		ShowWarning("mob_readdb_race2: Unknown race2 %d.\n", race);
		return false;
	}

	for(i = 1; i<columns; i++)
	{
		int mobid = atoi(fields[i]);
		if (mob_db(mobid) == mob_dummy)
		{
			ShowWarning("mob_readdb_race2: Unknown mob id %d for race2 %d.\n", mobid, race);
			continue;
		}
		mob_db_data[mobid]->race2 = race;
	}
	return true;
}

/**
 * Read mob_item_ratio.txt
 */
static bool mob_readdb_itemratio(char* str[], int columns, int current)
{
	unsigned short nameid;
	int ratio, i;
	struct s_mob_item_drop_ratio *item_ratio;
	nameid = atoi(str[0]);

	if (itemdb_exists(nameid) == NULL) {
		ShowWarning("mob_readdb_itemratio: Invalid item id %hu.\n", nameid);
		return false;
	}

	ratio = atoi(str[1]);

	if (!(item_ratio = (struct s_mob_item_drop_ratio *)idb_get(mob_item_drop_ratio,nameid)))
		CREATE(item_ratio, struct s_mob_item_drop_ratio, 1);

	item_ratio->drop_ratio = ratio;
	memset(item_ratio->mob_id, 0, sizeof(item_ratio->mob_id));
	for (i = 0; i < columns-2; i++) {
		uint16 mob_id = atoi(str[i+2]);
		if (mob_db(mob_id) == mob_dummy)
			ShowError("mob_readdb_itemratio: Invalid monster with ID %hu (Item:%hu Col:%d).\n", mob_id, nameid, columns);
		else
			item_ratio->mob_id[i] = atoi(str[i+2]);
	}

	if (!item_ratio->nameid) {
		item_ratio->nameid = nameid;
		idb_put(mob_item_drop_ratio, nameid, item_ratio);
	}

	return true;
}

/**
 * Free drop ratio data
 **/
static int mob_item_drop_ratio_free(DBKey key, DBData *data, va_list ap) {
	struct s_mob_item_drop_ratio *item_ratio = (struct s_mob_item_drop_ratio *)db_data2ptr(data);
	aFree(item_ratio);
	return 0;
}

/**
 * Adjust drop ratio for each monster
 **/
static void mob_drop_ratio_adjust(void){
	unsigned short i;

	for( i = 0; i <= MAX_MOB_DB; i++ ){
		struct mob_db *mob;
		struct item_data *id;
		unsigned short nameid;
		int j, rate, rate_adjust = 0, mob_id;

		mob = mob_db(i);

		// Skip dummy mobs.
		if( mob == mob_dummy ){
			continue;
		}

		mob_id = i;

		for( j = 0; j < MAX_MVP_DROP; j++ ){
			nameid = mob->mvpitem[j].nameid;
			rate = mob->mvpitem[j].p;

			if( nameid == 0 || rate == 0 ){
				continue;
			}

			rate_adjust = battle_config.item_rate_mvp;

			// Adjust the rate if there is an entry in mob_item_ratio
			item_dropratio_adjust( nameid, mob_id, &rate_adjust );

			// Adjust rate with given algorithms
			rate = mob_drop_adjust( rate, rate_adjust, battle_config.item_drop_mvp_min, battle_config.item_drop_mvp_max );

			// calculate and store Max available drop chance of the MVP item
			if( rate ){
				id = itemdb_search( nameid );

				// Item is not known anymore(should never happen)
				if( !id ){
					ShowWarning( "Monster \"%s\"(id:%d) is dropping an unknown item(id: %d)\n", mob->name, mob_id, nameid );
					mob->mvpitem[j].nameid = 0;
					mob->mvpitem[j].p = 0;
					continue;
				}

				if( id->maxchance == -1 || ( id->maxchance < rate/10 + 1 ) ){
					// item has bigger drop chance or sold in shops
					id->maxchance = rate/10 + 1; // reduce MVP drop info to not spoil common drop rate
				}
			}

			mob->mvpitem[j].p = rate;
		}

		for( j = 0; j < MAX_MOB_DROP; j++ ){
			unsigned short ratemin, ratemax;
			bool is_treasurechest;

			nameid = mob->dropitem[j].nameid;
			rate = mob->dropitem[j].p;

			if( nameid == 0 || rate == 0 ){
				continue;
			}

			id = itemdb_search( nameid );

			// Item is not known anymore(should never happen)
			if( !id ){
				ShowWarning( "Monster \"%s\"(id:%d) is dropping an unknown item(id: %d)\n", mob->name, mob_id, nameid );
				mob->dropitem[j].nameid = 0;
				mob->dropitem[j].p = 0;
				continue;
			}

			if( battle_config.drop_rateincrease && rate < 5000 ){
				rate++;
			}

			// Treasure box drop rates [Skotlex]
			if( ( mob_id >= MOBID_TREAS01 && mob_id <= MOBID_TREAS40 ) || ( mob_id >= MOBID_TREAS41 && mob_id <= MOBID_TREAS49 ) ){
				is_treasurechest = true;

				rate_adjust = battle_config.item_rate_treasure;
				ratemin = battle_config.item_drop_treasure_min;
				ratemax = battle_config.item_drop_treasure_max;
			}else{
				is_treasurechest = false;

				 // Added suport to restrict normal drops of MVP's [Reddozen]
				switch( id->type ){
					case IT_HEALING:
						rate_adjust = (mob->status.mode&MD_BOSS) ? battle_config.item_rate_heal_boss : battle_config.item_rate_heal;
						ratemin = battle_config.item_drop_heal_min;
						ratemax = battle_config.item_drop_heal_max;
						break;
					case IT_USABLE:
					case IT_CASH:
						rate_adjust = (mob->status.mode&MD_BOSS) ? battle_config.item_rate_use_boss : battle_config.item_rate_use;
						ratemin = battle_config.item_drop_use_min;
						ratemax = battle_config.item_drop_use_max;
						break;
					case IT_WEAPON:
					case IT_ARMOR:
					case IT_PETARMOR:
						rate_adjust = (mob->status.mode&MD_BOSS) ? battle_config.item_rate_equip_boss : battle_config.item_rate_equip;
						ratemin = battle_config.item_drop_equip_min;
						ratemax = battle_config.item_drop_equip_max;
						break;
					case IT_CARD:
						rate_adjust = (mob->status.mode&MD_BOSS) ? battle_config.item_rate_card_boss : battle_config.item_rate_card;
						ratemin = battle_config.item_drop_card_min;
						ratemax = battle_config.item_drop_card_max;
						break;
					default:
						rate_adjust = (mob->status.mode&MD_BOSS) ? battle_config.item_rate_common_boss : battle_config.item_rate_common;
						ratemin = battle_config.item_drop_common_min;
						ratemax = battle_config.item_drop_common_max;
						break;
				}
			}

			item_dropratio_adjust( nameid, mob_id, &rate_adjust );
			rate = mob_drop_adjust( rate, rate_adjust, ratemin, ratemax );

			// calculate and store Max available drop chance of the item
			// but skip treasure chests.
			if( rate && !is_treasurechest ){
				unsigned short k;

				if( id->maxchance == -1 || ( id->maxchance < rate ) ){
					id->maxchance = rate; // item has bigger drop chance or sold in shops
				}

				for( k = 0; k < MAX_SEARCH; k++ ){
					if( id->mob[k].chance <= rate ){
						break;
					}
				}

				if( k != MAX_SEARCH ){
					if( id->mob[k].id != mob_id ){
						memmove( &id->mob[k+1], &id->mob[k], (MAX_SEARCH-k-1)*sizeof(id->mob[0]) );
					}

					id->mob[k].chance = rate;
					id->mob[k].id = mob_id;
				}
			}

			mob->dropitem[j].p = rate;
		}
	}

	// Now that we are done we can delete the stored item ratios
	mob_item_drop_ratio->clear( mob_item_drop_ratio, mob_item_drop_ratio_free );
}

/**
 * Copy skill from DB to monster
 * @param mob Monster DB entry
 * @param skill Monster skill entries
 **/
static void mob_skill_db_set_single_sub(struct mob_db *mob, struct s_mob_skill *skill) {
	uint8 i;

	nullpo_retv(mob);
	nullpo_retv(skill);

	for (i = 0; mob->maxskill < MAX_MOBSKILL && i < skill->count; i++) {
		mob->skill[mob->maxskill++] = skill->skill[i];
	}

	if (i < skill->count)
		ShowWarning("Monster '%s' (%d, src:%d) reaches max skill limit %d. Ignores '%d' skills left.\n", mob->sprite, mob->vd.class_, skill->mob_id, MAX_MOBSKILL, skill->count-i);
}

/**
 * Check the skill & monster id before put the skills
 * @param skill
 **/
static void mob_skill_db_set_single(struct s_mob_skill *skill) {
	struct mob_db *mob = NULL;

	nullpo_retv(skill);

	// Specific monster
	if (skill->mob_id >= 0) {
		mob = mob_db(skill->mob_id);
		if (mob != mob_dummy)
			//memcpy(&mob->skill, skill, sizeof(skill));
			mob_skill_db_set_single_sub(mob, skill);
	}
	// Global skill
	else {
		uint16 i, id = skill->mob_id;
		id *= -1;
		for (i = 0; i < MAX_MOB_DB; i++) {
			mob = mob_db(i);
			if (mob == mob_dummy)
				continue;
			if (   (!(id&1) && mob->status.mode&MD_BOSS) // Bosses
				|| (!(id&2) && !(mob->status.mode&MD_BOSS)) // Normal monsters
				)
				continue;
			mob_skill_db_set_single_sub(mob, skill);
		}
	}
	
}

/**
 * Free monster skill data
 **/
static int mob_skill_db_free(DBKey key, DBData *data, va_list ap) {
	struct s_mob_skill *skill = (struct s_mob_skill *)db_data2ptr(data);
	if (skill)
		aFree(skill);
	return 0;
}

/**
 * Free random summon data
 **/
static int mob_summon_db_free(DBKey key, DBData *data, va_list ap) {
	struct s_randomsummon_group *msummon = (struct s_randomsummon_group *)db_data2ptr(data);
	if (msummon) {
		if (msummon->list) {
			aFree(msummon->list);
			msummon->list = NULL;
			msummon->count = 0;
		}
		aFree(msummon);
		msummon = NULL;
	}
	return 0;
}

/**
 * Set monster skills
 **/
static void mob_skill_db_set(void) {
	DBIterator *iter = db_iterator(mob_skill_db);
	struct s_mob_skill *skill = NULL;

	for (skill = (struct s_mob_skill *)dbi_first(iter);  dbi_exists(iter); skill = (struct s_mob_skill *)dbi_next(iter)) {
		mob_skill_db_set_single(skill);
	}
	dbi_destroy(iter);

	//ShowStatus("Set skills to '%d' monsters.\n", db_size(mob_skill_db));
	mob_skill_db->clear(mob_skill_db, mob_skill_db_free);
}

/**
 * read all mob-related databases
 */
static void mob_load(void)
{
	int i;
	const char* dbsubpath[] = {
		"",
		"/"DBIMPORT,
	};

	for(i=0; i<ARRAYLENGTH(dbsubpath); i++){	
		int n1 = strlen(db_path)+strlen(dbsubpath[i])+1;
		int n2 = strlen(db_path)+strlen(DBPATH)+strlen(dbsubpath[i])+1;

		char* dbsubpath1 = (char*)aMalloc(n1+1);
		char* dbsubpath2 = (char*)aMalloc(n2+1);
		
		if(i==0) {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n2,"%s/%s%s",db_path,DBPATH,dbsubpath[i]);
		}
		else {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n1,"%s%s",db_path,dbsubpath[i]);
		}
		
		if (db_use_sqldbs && i==0) //only read once for sql
		{
			mob_read_sqldb();
			mob_read_sqlskilldb();
		} else {
			sv_readdb(dbsubpath2, "mob_db.txt", ',', 31+2*MAX_MVP_DROP+2*MAX_MOB_DROP, 31+2*MAX_MVP_DROP+2*MAX_MOB_DROP, -1, &mob_readdb_sub, i);
			mob_readskilldb(dbsubpath2,i);
		}

		sv_readdb(dbsubpath1, "mob_avail.txt", ',', 2, 12, -1, &mob_readdb_mobavail, i);
		sv_readdb(dbsubpath2, "mob_race2_db.txt", ',', 2, 20, -1, &mob_readdb_race2, i);
		sv_readdb(dbsubpath1, "mob_item_ratio.txt", ',', 2, 2+MAX_ITEMRATIO_MOBS, -1, &mob_readdb_itemratio, i);
		sv_readdb(dbsubpath1, "mob_chat_db.txt", '#', 3, 3, MAX_MOB_CHAT, &mob_parse_row_chatdb, i);
		mob_read_randommonster(dbsubpath2, i);
		
		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}

	mob_drop_ratio_adjust();
	mob_skill_db_set();
}

void mob_reload(void) {
	int i;

	//Mob skills need to be cleared before re-reading them. [Skotlex]
	for (i = 0; i < MAX_MOB_DB; i++) {
		if (mob_db_data[i]) {
			memset(&mob_db_data[i]->skill,0,sizeof(mob_db_data[i]->skill));
			mob_db_data[i]->maxskill = 0;
		}
	}

	// Clear item_drop_ratio_db
	mob_item_drop_ratio->clear(mob_item_drop_ratio, mob_item_drop_ratio_free);
	mob_skill_db->clear(mob_skill_db, mob_skill_db_free);
	mob_summon_db->clear(mob_summon_db, mob_summon_db_free);
	mob_load();
}

void mob_clear_spawninfo()
{	//Clears spawn related information for a script reload.
	int i;
	for (i = 0; i < MAX_MOB_DB; i++)
		if (mob_db_data[i])
			memset(&mob_db_data[i]->spawn,0,sizeof(mob_db_data[i]->spawn));
}

/*==========================================
 * Circumference initialization of mob
 *------------------------------------------*/
void do_init_mob(void){
	memset(mob_db_data,0,sizeof(mob_db_data)); //Clear the array
	mob_db_data[0] = (struct mob_db*)aCalloc(1, sizeof (struct mob_db));	//This mob is used for random spawns
	mob_makedummymobdb(0); //The first time this is invoked, it creates the dummy mob
	item_drop_ers = ers_new(sizeof(struct item_drop),"mob.c::item_drop_ers",ERS_OPT_CLEAN);
	item_drop_list_ers = ers_new(sizeof(struct item_drop_list),"mob.c::item_drop_list_ers",ERS_OPT_NONE);
	mob_item_drop_ratio = idb_alloc(DB_OPT_BASE);
	mob_skill_db = idb_alloc(DB_OPT_BASE);
	mob_summon_db = idb_alloc(DB_OPT_BASE);
	mob_load();

	add_timer_func_list(mob_delayspawn,"mob_delayspawn");
	add_timer_func_list(mob_delay_item_drop,"mob_delay_item_drop");
	add_timer_func_list(mob_ai_hard,"mob_ai_hard");
	add_timer_func_list(mob_ai_lazy,"mob_ai_lazy");
	add_timer_func_list(mob_timer_delete,"mob_timer_delete");
	add_timer_func_list(mob_spawn_guardian_sub,"mob_spawn_guardian_sub");
	add_timer_func_list(mob_respawn,"mob_respawn");
	add_timer_interval(gettick()+MIN_MOBTHINKTIME,mob_ai_hard,0,0,MIN_MOBTHINKTIME);
	add_timer_interval(gettick()+MIN_MOBTHINKTIME*10,mob_ai_lazy,0,0,MIN_MOBTHINKTIME*10);
}

/*==========================================
 * Clean memory usage.
 *------------------------------------------*/
void do_final_mob(void){
	int i;
	if (mob_dummy)
	{
		aFree(mob_dummy);
		mob_dummy = NULL;
	}
	for (i = 0; i <= MAX_MOB_DB; i++)
	{
		if (mob_db_data[i] != NULL)
		{
			aFree(mob_db_data[i]);
			mob_db_data[i] = NULL;
		}
	}
	for (i = 0; i <= MAX_MOB_CHAT; i++)
	{
		if (mob_chat_db[i] != NULL)
		{
			aFree(mob_chat_db[i]);
			mob_chat_db[i] = NULL;
		}
	}
	mob_item_drop_ratio->destroy(mob_item_drop_ratio,mob_item_drop_ratio_free);
	mob_skill_db->destroy(mob_skill_db, mob_skill_db_free);
	mob_summon_db->destroy(mob_summon_db, mob_summon_db_free);
	ers_destroy(item_drop_ers);
	ers_destroy(item_drop_list_ers);
}
