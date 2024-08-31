// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mob.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/db.hpp>
#include <common/ers.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "achievement.hpp"
#include "battle.hpp"
#include "clif.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "map.hpp"
#include "mercenary.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pet.hpp"
#include "quest.hpp"

using namespace rathena;

#define ACTIVE_AI_RANGE 2	//Distance added on top of 'AREA_SIZE' at which mobs enter active AI mode.

const t_tick MOB_MAX_DELAY = 24 * 3600 * 1000;
#define RUDE_ATTACKED_COUNT 1	//After how many rude-attacks should the skill be used?

// On official servers, monsters will only seek targets that are closer to walk to than their
// search range. The search range is affected depending on if the monster is walking or not.
// On some maps there can be a quite long path for just walking two cells in a direction and
// the client does not support displaying walk paths that are longer than 14 cells, so this
// option reduces position lag in such situation. But doing a complex search for every possible
// target, might be CPU intensive.
// Disable this to make monsters not do any path search when looking for a target (old behavior).
#define ACTIVEPATHSEARCH

// Limits for the monster database
#define MIN_MOB_DB 1000
#define MAX_MOB_DB 3999
#define MIN_MOB_DB2 20020
#define MAX_MOB_DB2 31999

// These define the range of available IDs for clones. [Valaris]
#define MOB_CLONE_START MAX_MOB_DB
#define MOB_CLONE_END MIN_MOB_DB2

// holds Monster Spawn informations
std::unordered_map<uint16, std::vector<spawn_info>> mob_spawn_data;

MobItemRatioDatabase mob_item_drop_ratio;

/// Mob skill struct for temporary storage
struct s_mob_skill_db {
	int32 mob_id; ///< Monster ID. -1 boss types, -2 normal types, -3 all monsters
	std::vector<std::shared_ptr<s_mob_skill>> skill; ///< Skills
};

std::unordered_map<int32, std::shared_ptr<s_mob_skill_db>> mob_skill_db; /// Monster skill temporary db. s_mob_skill_db -> mobid

std::unordered_map<uint32, std::shared_ptr<s_item_drop_list>> mob_delayed_drops;
std::unordered_map<uint32, std::shared_ptr<s_item_drop_list>> mob_looted_drops;
MobSummonDatabase mob_summon_db;
MobChatDatabase mob_chat_db;
MapDropDatabase map_drop_db;

/*==========================================
 * Local prototype declaration   (only required thing)
 *------------------------------------------*/
static TIMER_FUNC(mob_spawn_guardian_sub);
int mob_skill_id2skill_idx(int mob_id,uint16 skill_id);

/*========================================== [Playtester]
* Removes all characters that spotted the monster but are no longer online
* @param md: Monster whose spotted log should be cleaned
*------------------------------------------*/
void mob_clean_spotted(struct mob_data *md) {
	int i;
	for (i = 0; i < DAMAGELOG_SIZE; i++) {
		if (md->spotted_log[i] && !map_charid2sd(md->spotted_log[i]))
			md->spotted_log[i] = 0;
	}
}

/*========================================== [Playtester]
* Adds a char_id to the spotted log of a monster
* @param md: Monster to whose spotted log char_id should be added
* @param char_id: Char_id to add to the spotted log
*------------------------------------------*/
void mob_add_spotted(struct mob_data *md, uint32 char_id) {
	int i;

	//Check if char_id is already logged
	for (i = 0; i < DAMAGELOG_SIZE; i++) {
		if (md->spotted_log[i] == char_id)
			return;
	}

	//Not logged, add char_id to first empty slot
	for (i = 0; i < DAMAGELOG_SIZE; i++) {
		if (md->spotted_log[i] == 0) {
			md->spotted_log[i] = char_id;
			return;
		}
	}
}

/*========================================== [Playtester]
* Checks if a monster was spotted
* @param md: Monster to check
* @return Returns true if the monster is spotted, otherwise 0
*------------------------------------------*/
bool mob_is_spotted(struct mob_data *md) {
	int i;

	//Check if monster is spotted
	for (i = 0; i < DAMAGELOG_SIZE; i++) {
		if (md->spotted_log[i] != 0)
			return true; //Spotted
	}

	return false; //Not spotted
}

/**
 * Tomb spawn time calculations
 * @param nd: NPC data
 */
int mvptomb_setdelayspawn(struct npc_data *nd) {
	if (nd->u.tomb.spawn_timer != INVALID_TIMER)
		delete_timer(nd->u.tomb.spawn_timer, mvptomb_delayspawn);
	nd->u.tomb.spawn_timer = add_timer(gettick() + battle_config.mvp_tomb_delay, mvptomb_delayspawn, nd->bl.id, 0);
	return 0;
}

/**
 * Tomb spawn with delay (timer function)
 * @param tid: Timer ID
 * @param tick: Time
 * @param id: Block list ID
 * @param data: Used for add_timer_func_list
 */
TIMER_FUNC(mvptomb_delayspawn){
	struct npc_data *nd = BL_CAST(BL_NPC, map_id2bl(id));

	if (nd) {
		if (nd->u.tomb.spawn_timer != tid) {
			ShowError("mvptomb_delayspawn: Timer mismatch: %d != %d\n", tid, nd->u.tomb.spawn_timer);
			return 0;
		}
		nd->u.tomb.spawn_timer = INVALID_TIMER;
		clif_spawn(&nd->bl);
	}
	return 0;
}

/**
 * Create and display a tombstone on the map
 * @param md: the mob to create a tombstone for
 * @param killer: name of player who killed the mob
 * @param time: time of mob's death
 * @author [GreenBox]
 */
void mvptomb_create(struct mob_data *md, char *killer, time_t time)
{
	struct npc_data *nd;

	if ( md->tomb_nid )
		mvptomb_destroy(md);

	CREATE(nd, struct npc_data, 1);
	new (nd) npc_data();

	nd->bl.id = md->tomb_nid = npc_get_new_npc_id();

	nd->ud.dir = md->ud.dir;
	nd->bl.m = md->bl.m;
	nd->bl.x = md->bl.x;
	nd->bl.y = md->bl.y;
	nd->bl.type = BL_NPC;

	safestrncpy(nd->name, msg_txt(nullptr,656), sizeof(nd->name));

	nd->class_ = 565;
	nd->speed = DEFAULT_NPC_WALK_SPEED;
	nd->subtype = NPCTYPE_TOMB;

	nd->u.tomb.md = md;
	nd->u.tomb.kill_time = time;
	nd->u.tomb.spawn_timer = INVALID_TIMER;

	nd->dynamicnpc.owner_char_id = 0;
	nd->dynamicnpc.last_interaction = 0;
	nd->dynamicnpc.removal_tid = INVALID_TIMER;

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

	mvptomb_setdelayspawn(nd);
}

/**
 * Destroys MVP Tomb
 * @param md: Mob data
 */
void mvptomb_destroy(struct mob_data *md) {
	struct npc_data *nd;

	if ( (nd = map_id2nd(md->tomb_nid)) ) {
		int i;
		struct map_data *mapdata = map_getmapdata(nd->bl.m);

		clif_clearunit_area( nd->bl, CLR_OUTSIGHT );
		map_delblock(&nd->bl);

		ARR_FIND( 0, mapdata->npc_num, i, mapdata->npc[i] == nd );
		if( !(i == mapdata->npc_num) ) {
			mapdata->npc_num--;
			mapdata->npc[i] = mapdata->npc[mapdata->npc_num];
			mapdata->npc[mapdata->npc_num] = nullptr;
		}
		map_deliddb(&nd->bl);
		aFree(nd);
	}

	md->tomb_nid = 0;
}

/**
 * Sub function for mob namesearch. Here is defined which are accepted.
*/
static bool mobdb_searchname_sub(uint16 mob_id, const char * const str, bool full_cmp)
{
	std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);

	if (mob == nullptr)
		return false;
	
	if( mobdb_checkid(mob_id) <= 0 )
		return false; // invalid mob_id (includes clone check)
	if(!mob->base_exp && !mob->job_exp && !mob_has_spawn(mob_id))
		return false; // Monsters with no base/job exp and no spawn point are, by this criteria, considered "slave mobs" and excluded from search results
	if( full_cmp ) {
		// str must equal the db value
		if( strcmpi(mob->name.c_str(), str) == 0 || 
			strcmpi(mob->jname.c_str(), str) == 0 || 
			strcmpi(mob->sprite.c_str(), str) == 0 )
			return true;
	} else {
		// str must be in the db value
		if( stristr(mob->name.c_str(), str) != nullptr ||
			stristr(mob->jname.c_str(), str) != nullptr ||
			stristr(mob->sprite.c_str(), str) != nullptr )
			return true;
	}
	return false;
}

/**
 * Searches for the Mobname
*/
uint16 mobdb_searchname_(const char * const str, bool full_cmp)
{
	for( auto const &mobdb_pair : mob_db ) {
		const uint16 mob_id = mobdb_pair.first;
		if( mobdb_searchname_sub(mob_id, str, full_cmp) )
			return mob_id;
	}
	return 0;
}

uint16 mobdb_searchname(const char * const str)
{
	return mobdb_searchname_(str, true);
}

std::shared_ptr<s_mob_db> mobdb_search_aegisname( const char* str ){
	for( auto &mobdb_pair : mob_db ){
		if( strcmpi( str, mobdb_pair.second->sprite.c_str() ) == 0 ){
			return mobdb_pair.second;
		}
	}

	return nullptr;
}

/*==========================================
 * Searches up to N matches. Returns number of matches [Skotlex]
 *------------------------------------------*/
uint16 mobdb_searchname_array_(const char *str, uint16 * out, uint16 size, bool full_cmp)
{
	uint16 count = 0;
	const auto &mob_list = mob_db.getCache();

	for( const auto &mob : mob_list ) {
		if (mob == nullptr)
			continue;
		if( mobdb_searchname_sub(mob->id, str, full_cmp) ) {
			if( count < size )
				out[count] = mob->id;
			count++;
		}
	}

	return count;
}

uint16 mobdb_searchname_array(const char *str, uint16 * out, uint16 size)
{
	return mobdb_searchname_array_(str, out, size, false);
}

/*==========================================
 * Id Mob is checked.
 *------------------------------------------*/
int mobdb_checkid(const int id)
{
	if (!mob_db.exists(id))
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
	std::shared_ptr<s_mob_db> db = mob_db.find(mob_id);

	if (db == nullptr)
		return nullptr;

	return &db->vd;
}

e_mob_bosstype s_mob_db::get_bosstype(){
	if( status_has_mode( &this->status, MD_MVP ) ){
		return BOSSTYPE_MVP;
	}else if( this->status.class_ == CLASS_BOSS ){
		return BOSSTYPE_MINIBOSS;
	}else{
		return BOSSTYPE_NONE;
	}
}

e_mob_bosstype mob_data::get_bosstype(){
	if( status_has_mode( &this->status, MD_MVP ) ){
		return BOSSTYPE_MVP;
	}else if( this->status.class_ == CLASS_BOSS ){
		return BOSSTYPE_MINIBOSS;
	}else{
		return BOSSTYPE_NONE;
	}
}

/**
 * Create unique view data associated to a spawned monster.
 * @param md: Mob to adjust
 */
void mob_set_dynamic_viewdata( struct mob_data* md ){
	// If it is a valid monster and it has not already been created
	if( md && !md->vd_changed ){
		// Allocate a dynamic entry
		struct view_data* vd = (struct view_data*)aMalloc( sizeof( struct view_data ) );

		// Copy the current values
		memcpy( vd, md->vd, sizeof( struct view_data ) );

		// Update the pointer to the new entry
		md->vd = vd;

		// Flag it as changed so it is freed later on
		md->vd_changed = true;
	}
}

/**
 * Free any view data associated to a spawned monster.
 * @param md: Mob to free
 */
void mob_free_dynamic_viewdata( struct mob_data* md ){
	// If it is a valid monster and it has already been allocated
	if( md && md->vd_changed ){
		// Free it
		aFree( md->vd );

		// Remove the reference
		md->vd = nullptr;

		// Unflag it as changed
		md->vd_changed = false;
	}
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
		safestrncpy(data->name, mob_db.find(data->id)->name.c_str(), sizeof(data->name));
	else if(strcmp(data->name,"--ja--")==0)
		safestrncpy(data->name, mob_db.find(data->id)->jname.c_str(), sizeof(data->name));

	return 1;
}

/*==========================================
 * Generates the basic mob data using the spawn_data provided.
 *------------------------------------------*/
struct mob_data* mob_spawn_dataset(struct spawn_data *data)
{
	struct mob_data *md = (struct mob_data*)aCalloc(1, sizeof(struct mob_data));
	new(md) mob_data();
	md->bl.id= npc_get_new_npc_id();
	md->bl.type = BL_MOB;
	md->bl.m = data->m;
	md->bl.x = data->x;
	md->bl.y = data->y;
	md->mob_id = data->id;
	md->state.boss = data->state.boss;
	md->db = mob_db.find(md->mob_id);
	if (data->level > 0)
		md->level = data->level;
	memcpy(md->name, data->name, NAME_LENGTH);
	if (data->state.ai)
		md->special_state.ai = data->state.ai;
	if (data->state.size)
		md->special_state.size = data->state.size;
	if (data->eventname[0] && strlen(data->eventname) >= 4)
		safestrncpy(md->npc_event, data->eventname, EVENT_NAME_LENGTH);
	if(status_has_mode(&md->db->status,MD_LOOTER))
		md->lootitems = (struct s_mob_lootitem *)aCalloc(LOOTITEM_SIZE,sizeof(struct s_mob_lootitem));
	md->spawn_timer = INVALID_TIMER;
	md->deletetimer = INVALID_TIMER;
	md->skill_idx = -1;
	md->centerX = data->x;
	md->centerY = data->y;
	status_set_viewdata(&md->bl, md->mob_id);
	status_change_init(&md->bl);
	unit_dataset(&md->bl);

	map_addiddb(&md->bl);
	return md;
}

/*==========================================
 * Fetches a random mob_id [Skotlex]
 * type: Where to fetch from (see enum e_random_monster)
 * flag: Type of checks to apply (see enum e_random_monster_flags)
 * lv: Mob level to check against
 *------------------------------------------*/
int mob_get_random_id(int type, enum e_random_monster_flags flag, int lv)
{
	std::shared_ptr<s_randomsummon_group> summon = mob_summon_db.find(type);

	if (type == MOBG_BLOODY_DEAD_BRANCH && flag&RMF_MOB_NOT_BOSS)
		flag = static_cast<e_random_monster_flags>(flag&~RMF_MOB_NOT_BOSS);
	
	if (!summon) {
		ShowError("mob_get_random_id: Invalid type (%d) of random monster.\n", type);
		return 0;
	}
	if (summon->list.empty()) {
		ShowError("mob_get_random_id: Random monster type %d is not defined.\n", type);
		return 0;
	}

	for( size_t i = 0, max = summon->list.size() * 3; i < max; i++ ){
		std::shared_ptr<s_randomsummon_entry> entry = util::umap_random( summon->list );
		std::shared_ptr<s_mob_db> mob = mob_db.find( entry->mob_id );

		if(mob == nullptr ||
			mob_is_clone( entry->mob_id ) ||
			(flag&RMF_DB_RATE && (entry->rate < 1000000 && entry->rate <= rnd() % 1000000)) ||
			(flag&RMF_CHECK_MOB_LV && lv < mob->lv) ||
			(flag&RMF_MOB_NOT_BOSS && status_has_mode(&mob->status,MD_STATUSIMMUNE) ) ||
			(flag&RMF_MOB_NOT_SPAWN && !mob_has_spawn( entry->mob_id )) ||
			(flag&RMF_MOB_NOT_PLANT && status_has_mode(&mob->status,MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC) )
		){
			continue;
		}

		return entry->mob_id;
	}

	if (!mob_db.exists( summon->default_mob_id )) {
		ShowError("mob_get_random_id: Default monster is not defined for type %d.\n", type);
		return 0;
	}

	return summon->default_mob_id;
}

/*==========================================
 * Kill Steal Protection [Zephyrus]
 *------------------------------------------*/
bool mob_ksprotected (struct block_list *src, struct block_list *target)
{
	struct block_list *s_bl, *t_bl;
	map_session_data
		*sd,    // Source
		*t_sd;  // Mob Target
	struct mob_data *md;
	t_tick tick = gettick();

	if( !battle_config.ksprotection )
		return false; // KS Protection Disabled

	if( !(md = BL_CAST(BL_MOB,target)) )
		return false; // Tarjet is not MOB

	if( (s_bl = battle_get_master(src)) == nullptr )
		s_bl = src;

	if( !(sd = BL_CAST(BL_PC,s_bl)) )
		return false; // Master is not PC

	t_bl = map_id2bl(md->target_id);
	if( !t_bl || (s_bl = battle_get_master(t_bl)) == nullptr )
		s_bl = t_bl;

	t_sd = BL_CAST(BL_PC,s_bl);

	do {
		struct status_change_entry *sce;
		map_session_data *pl_sd; // Owner
		struct map_data *mapdata = map_getmapdata(md->bl.m);
		char output[128];
		
		if( mapdata->getMapFlag(MF_ALLOWKS) || mapdata_flag_ks(mapdata) )
			return false; // Ignores GVG, PVP and AllowKS map flags

		if( md->get_bosstype() == BOSSTYPE_MVP || md->master_id )
			return false; // MVP, Slaves mobs ignores KS

		if( (sce = md->sc.getSCE(SC_KSPROTECTED)) == nullptr )
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

		if( (pl_sd = map_id2sd(sce->val1)) == nullptr || pl_sd->bl.m != md->bl.m )
			break;

		if( !pl_sd->state.noks )
			return false; // No KS Protected, but normal players should be protected too

		// Message to KS
		if( DIFF_TICK(sd->ks_floodprotect_tick, tick) <= 0 )
		{
			sprintf(output, "[KS Warning!! - Owner : %s]", pl_sd->status.name);
			clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);

			sd->ks_floodprotect_tick = tick + 2000;
		}

		// Message to Owner
		if( DIFF_TICK(pl_sd->ks_floodprotect_tick, tick) <= 0 )
		{
			sprintf(output, "[Watch out! %s is trying to KS you!]", sd->status.name);
			clif_messagecolor(&pl_sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);

			pl_sd->ks_floodprotect_tick = tick + 2000;
		}

		return true;
	} while(0);

	status_change_start(src, target, SC_KSPROTECTED, 10000, sd->bl.id, sd->state.noks, sd->status.party_id, sd->status.guild_id, battle_config.ksprotection, SCSTART_NOAVOID);

	return false;
}

struct mob_data *mob_once_spawn_sub(struct block_list *bl, int16 m, int16 x, int16 y, const char *mobname, int mob_id, const char *event, unsigned int size, enum mob_ai ai)
{
	struct spawn_data data;

	memset(&data, 0, sizeof(struct spawn_data)); //why ? this might screw attribute object and cause leak..
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

	struct map_data *mapdata = map_getmapdata(m);
	// if none found, pick random position on map
	if (x <= 0 || x >= mapdata->xs || y <= 0 || y >= mapdata->ys)
		map_search_freecell(nullptr, m, &x, &y, -1, -1, 1);

	data.x = x;
	data.y = y;

	if (!mob_parse_dataset(&data))
		return nullptr;

	return mob_spawn_dataset(&data);
}

/*==========================================
 * Spawn a single mob on the specified coordinates.
 *------------------------------------------*/
int mob_once_spawn(map_session_data* sd, int16 m, int16 x, int16 y, const char* mobname, int mob_id, int amount, const char* event, unsigned int size, enum mob_ai ai)
{
	struct mob_data* md = nullptr;
	int count, lv;

	if (m < 0 || amount <= 0)
		return 0; // invalid input

	lv = (sd) ? sd->status.base_level : 255;

	for (count = 0; count < amount; count++)
	{
		int c = (mob_id >= 0) ? mob_id : mob_get_random_id(-mob_id - 1, (battle_config.random_monster_checklv) ? static_cast<e_random_monster_flags>(RMF_DB_RATE|RMF_CHECK_MOB_LV) : RMF_DB_RATE, lv);
		md = mob_once_spawn_sub((sd) ? &sd->bl : nullptr, m, x, y, mobname, c, event, size, ai);

		if (!md)
			continue;

		if (mob_id == MOBID_EMPERIUM)
		{
			std::shared_ptr<guild_castle> gc = castle_db.mapindex2gc(map_getmapdata(m)->index);
			auto g = (gc) ? guild_search(gc->guild_id) : nullptr;
			if (gc)
			{
				md->guardian_data = (struct guardian_data*)aCalloc(1, sizeof(struct guardian_data));
				new(md->guardian_data) guardian_data();
				md->guardian_data->castle = gc;
				md->guardian_data->number = MAX_GUARDIANS;
				md->guardian_data->guild_id = gc->guild_id;
				if (g)
				{
					md->guardian_data->emblem_id = g->guild.emblem_id;
					memcpy(md->guardian_data->guild_name, g->guild.name, NAME_LENGTH);
				}
				else if (gc->guild_id) // Guild is not yet available, retry after the configured timespan.
					add_timer(gettick() + battle_config.mob_respawn_time,mob_spawn_guardian_sub,md->bl.id,md->guardian_data->guild_id);
			}
		}	// end addition [Valaris]

		mob_spawn(md);

		if (mob_id < 0 && battle_config.dead_branch_active)
			//Behold Aegis's masterful decisions yet again...
			//"I understand the "Aggressive" part, but the "Can Move" and "Can Attack" is just stupid" - Poki#3
			sc_start4(nullptr,&md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE|MD_CANATTACK|MD_CANMOVE|MD_ANGRY, 0, 60000);
	}

	return (md) ? md->bl.id : 0; // id of last spawned mob
}

/*==========================================
 * Spawn mobs in the specified area.
 *------------------------------------------*/
int mob_once_spawn_area(map_session_data* sd, int16 m, int16 x0, int16 y0, int16 x1, int16 y1, const char* mobname, int mob_id, int amount, const char* event, unsigned int size, enum mob_ai ai)
{
	int i, max, id = 0;
	int lx = -1, ly = -1;

	if (m < 0 || amount <= 0)
		return 0; // invalid input

	// normalize x/y coordinates
	if (x0 > x1)
		std::swap(x0, x1);
	if (y0 > y1)
		std::swap(y0, y1);

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
static TIMER_FUNC(mob_spawn_guardian_sub){
	//Needed because the guild_data may not be available at guardian spawn time.
	struct block_list* bl = map_id2bl(id);
	struct mob_data* md;
	int guardup_lv;

	if (bl == nullptr) //It is possible mob was already removed from map when the castle has no owner. [Skotlex]
		return 0;

	if (bl->type != BL_MOB)
	{
		ShowError("mob_spawn_guardian_sub: Block error!\n");
		return 0;
	}

	md = (struct mob_data*)bl;
	nullpo_ret(md->guardian_data);
	auto g = guild_search((int)data);

	if (g == nullptr)
	{	//Liberate castle, if the guild is not found this is an error! [Skotlex]
		ShowError("mob_spawn_guardian_sub: Couldn't load guild %d!\n", (int)data);
		if (md->mob_id == MOBID_EMPERIUM)
		{	//Not sure this is the best way, but otherwise we'd be invoking this for ALL guardians spawned later on.
			md->guardian_data->guild_id = 0;
			if (md->guardian_data->castle->guild_id) //Free castle up.
			{
				ShowNotice("Clearing ownership of castle %d (%s)\n", md->guardian_data->castle->castle_id, md->guardian_data->castle->castle_name);
				guild_castledatasave(md->guardian_data->castle->castle_id, CD_GUILD_ID, 0);
			}
		} else {
			if (md->guardian_data->number >= 0 && md->guardian_data->number < MAX_GUARDIANS && md->guardian_data->castle->guardian[md->guardian_data->number].visible)
				guild_castledatasave(md->guardian_data->castle->castle_id, CD_ENABLED_GUARDIAN00 + md->guardian_data->number,0);
			unit_free(&md->bl,CLR_OUTSIGHT); //Remove guardian.
		}
		return 0;
	}
	guardup_lv = guild_checkskill(g->guild, GD_GUARDUP);
	md->guardian_data->emblem_id = g->guild.emblem_id;
	memcpy(md->guardian_data->guild_name, g->guild.name, NAME_LENGTH);
	md->guardian_data->guardup_lv = guardup_lv;
	if( guardup_lv )
		status_calc_mob(md, SCO_NONE); //Give bonuses.
	return 0;
}

/*==========================================
 * Summoning Guardians [Valaris]
 *------------------------------------------*/
int mob_spawn_guardian(const char* mapname, int16 x, int16 y, const char* mobname, int mob_id, const char* event, int guardian, bool has_index)
{
	struct mob_data *md=nullptr;
	struct spawn_data data;
	std::shared_ptr<MapGuild> g = nullptr;
	int16 m;
	memset(&data, 0, sizeof(struct spawn_data)); //fixme
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
		mob_id = mob_get_random_id(-mob_id-1, RMF_DB_RATE, 0);
		if (!mob_id) return 0;
	}

	data.id = mob_id;

	if( !has_index )
	{
		guardian = -1;
	}
	else if( guardian < 0 || guardian >= MAX_GUARDIANS )
	{
		ShowError("mob_spawn_guardian: Invalid guardian index %d for guardian %d (castle map %s)\n", guardian, mob_id, mapname);
		return 0;
	}

	if((x<=0 || y<=0) && !map_search_freecell(nullptr, m, &x, &y, -1,-1, 1))
	{
		ShowWarning("mob_spawn_guardian: Couldn't locate a spawn cell for guardian class %d (index %d) at castle map %s\n",mob_id, guardian, mapname);
		return 0;
	}
	data.x = x;
	data.y = y;
	safestrncpy(data.name, mobname, sizeof(data.name));
	safestrncpy(data.eventname, event, sizeof(data.eventname));
	if (!mob_parse_dataset(&data))
		return 0;

	std::shared_ptr<guild_castle> gc = castle_db.mapname2gc(mapname);
	if (gc == nullptr)
	{
		ShowError("mob_spawn_guardian: No castle set at map %s\n", mapname);
		return 0;
	}
	if (!gc->guild_id)
		ShowWarning("mob_spawn_guardian: Spawning guardian %d on a castle with no guild (castle map %s)\n", mob_id, mapname);
	else
		g = guild_search(gc->guild_id);

	if( has_index && gc->guardian[guardian].id )
  	{	//Check if guardian already exists, refuse to spawn if so.
		struct mob_data *md2 = (TBL_MOB*)map_id2bl(gc->guardian[guardian].id);
		if (md2 && md2->bl.type == BL_MOB &&
			md2->guardian_data && md2->guardian_data->number == guardian)
		{
			ShowError("mob_spawn_guardian: Attempted to spawn guardian in position %d which already has a guardian (castle map %s)\n", guardian, mapname);
			return 0;
		}
	}

	md = mob_spawn_dataset(&data);
	md->guardian_data = (struct guardian_data*)aCalloc(1, sizeof(struct guardian_data));
	new (md->guardian_data) guardian_data();
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
		md->guardian_data->emblem_id = g->guild.emblem_id;
		memcpy (md->guardian_data->guild_name, g->guild.name, NAME_LENGTH);
		md->guardian_data->guardup_lv = guild_checkskill(g->guild,GD_GUARDUP);
	} else if (md->guardian_data->guild_id)
		add_timer(gettick() + battle_config.mob_respawn_time,mob_spawn_guardian_sub,md->bl.id,md->guardian_data->guild_id);
	mob_spawn(md);

	return md->bl.id;
}

/*==========================================
 * Summoning BattleGround [Zephyrus]
 *------------------------------------------*/
int mob_spawn_bg(const char* mapname, int16 x, int16 y, const char* mobname, int mob_id, const char* event, unsigned int bg_id)
{
	struct mob_data *md = nullptr;
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
		mob_id = mob_get_random_id(-mob_id-1, RMF_DB_RATE, 0);
		if( !mob_id ) return 0;
	}

	data.id = mob_id;
	if( (x <= 0 || y <= 0) && !map_search_freecell(nullptr, m, &x, &y, -1,-1, 1) )
	{
		ShowWarning("mob_spawn_bg: Couldn't locate a spawn cell for guardian class %d (bg_id %d) at map %s\n",mob_id, bg_id, mapname);
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
 * Returns true if a mob is currently chasing something
 * based on its skillstate.
 * Chase states: MSS_LOOT, MSS_RUSH, MSS_FOLLOW
 *------------------------------------------*/
bool mob_is_chasing(int state)
{
	switch (state) {
		case MSS_LOOT:
		case MSS_RUSH:
		case MSS_FOLLOW:
			return true;
	}
	return false;
}

/*==========================================
 * Checks if a monster can reach a target by walking
 * Range: Maximum number of cells to be walked
 *------------------------------------------*/
int mob_can_reach(struct mob_data *md,struct block_list *bl,int range)
{
	nullpo_ret(md);
	nullpo_ret(bl);
	return unit_can_reach_bl(&md->bl, bl, range, 0, nullptr, nullptr);
}

/*==========================================
 * Links nearby mobs (supportive mobs)
 *------------------------------------------*/
int mob_linksearch(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	int mob_id;
	struct block_list *target;
	t_tick tick;

	nullpo_ret(bl);
	md=(struct mob_data *)bl;
	mob_id = va_arg(ap, int);
	target = va_arg(ap, struct block_list *);
	tick=va_arg(ap, t_tick);

	if (md->mob_id == mob_id && status_has_mode(&md->status,MD_ASSIST) && DIFF_TICK(tick, md->last_linktime) >= MIN_MOBLINKTIME
		&& !md->target_id)
	{
		md->last_linktime = tick;
		if( mob_can_reach(md,target,md->db->range2) ){	// Reachability judging
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
TIMER_FUNC(mob_delayspawn){
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
	unsigned int spawntime;

	if (!md->spawn) //Doesn't has respawn data!
		return unit_free(&md->bl,CLR_DEAD);

	spawntime = md->spawn->delay1; //Base respawn time
	if (md->spawn->delay2) //random variance
		spawntime+= rnd()%md->spawn->delay2;

	//Apply the spawn delay fix [Skotlex]
	std::shared_ptr<s_mob_db> db = mob_db.find(md->spawn->id);

	if (status_has_mode(&db->status,MD_STATUSIMMUNE)) { // Status Immune
		if (battle_config.boss_spawn_delay != 100) {
			// Divide by 100 first to prevent overflows
			//(precision loss is minimal as duration is in ms already)
			spawntime = spawntime/100*battle_config.boss_spawn_delay;
		}
	} else if (status_has_mode(&db->status,MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC)) { // Plant type
		if (battle_config.plant_spawn_delay != 100) {
			spawntime = spawntime/100*battle_config.plant_spawn_delay;
		}
	} else if (battle_config.mob_spawn_delay != 100) {	//Normal mobs
		spawntime = spawntime/100*battle_config.mob_spawn_delay;
	}

	spawntime = u32max(1000, spawntime); //Monsters should never respawn faster than 1 second

	if( md->spawn_timer != INVALID_TIMER )
		delete_timer(md->spawn_timer, mob_delayspawn);
	md->spawn_timer = add_timer(gettick()+spawntime, mob_delayspawn, md->bl.id, 0);
	return 0;
}

int mob_count_sub(struct block_list *bl, va_list ap) {
    int mobid[10], i;
    ARR_FIND(0, 10, i, (mobid[i] = va_arg(ap, int)) == 0); //fetch till 0
	mob_data* md = BL_CAST(BL_MOB, bl);
	if (md && mobid[0]) { //if there one let's check it otherwise go backward
        ARR_FIND(0, 10, i, md->mob_id == mobid[i]);
        return (i < 10) ? 1 : 0;
    }
	// If not counting monsters, count all
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
	t_tick tick = gettick();

	md->last_thinktime = tick;
	if (md->bl.prev != nullptr)
		unit_remove_map(&md->bl,CLR_RESPAWN);
	else
	if (md->spawn && md->mob_id != md->spawn->id)
	{
		md->mob_id = md->spawn->id;
		status_set_viewdata(&md->bl, md->mob_id);
		md->db = mob_db.find(md->mob_id);
		memcpy(md->name,md->spawn->name,NAME_LENGTH);
	}

	if (md->spawn) { //Respawn data
		md->bl.m = md->spawn->m;
		md->bl.x = md->centerX;
		md->bl.y = md->centerY;

		// Search can be skipped for boss monster spawns if spawn location is fixed
		// We can't skip normal monsters as they should pick a random location if the cell is blocked (e.g. Icewall)
		// The center cell has a 1/(xs*ys) chance to be picked if free
		if ((!md->spawn->state.boss || (md->bl.x == 0 && md->bl.y == 0) || md->spawn->xs != 1 || md->spawn->ys != 1)
			&& (md->spawn->xs + md->spawn->ys < 1 || !rnd_chance(1, md->spawn->xs * md->spawn->ys) || !map_getcell(md->bl.m, md->bl.x, md->bl.y, CELL_CHKREACH)))
		{
			// Officially the area is split into 4 squares, 4 lines and 1 dot and for each of those there is one attempt
			// We simplify this to trying 8 times in the whole area and then at the center cell even though that's not fully accurate

			// Try to spawn monster in defined area (8 tries)
			if (!map_search_freecell(&md->bl, -1, &md->bl.x, &md->bl.y, md->spawn->xs-1, md->spawn->ys-1, battle_config.no_spawn_on_player?4:0, 8))
			{
				// If area search failed and center cell not reachable, try to spawn the monster anywhere on the map (50 tries)
				if (!map_getcell(md->bl.m, md->bl.x, md->bl.y, CELL_CHKREACH) && !map_search_freecell(&md->bl, -1, &md->bl.x, &md->bl.y, -1, -1, battle_config.no_spawn_on_player?4:0))
				{
					// Retry again later
					if (md->spawn_timer != INVALID_TIMER)
						delete_timer(md->spawn_timer, mob_delayspawn);
					md->spawn_timer = add_timer(tick + battle_config.mob_respawn_time, mob_delayspawn, md->bl.id, 0);
					return 1;
				}
			}
		}
		if( battle_config.no_spawn_on_player > 99 && map_foreachinallrange(mob_count_sub, &md->bl, AREA_SIZE, BL_PC) )
		{ // retry again later (players on sight)
			if( md->spawn_timer != INVALID_TIMER )
				delete_timer(md->spawn_timer, mob_delayspawn);
			md->spawn_timer = add_timer(tick + battle_config.mob_respawn_time,mob_delayspawn,md->bl.id,0);
			return 1;
		}
	}

	memset(&md->state, 0, sizeof(md->state));
	status_calc_mob(md, SCO_FIRST);
	md->attacked_id = 0;
	md->norm_attacked_id = 0;
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

	md->state.aggressive = status_has_mode(&md->status,MD_ANGRY)?1:0;
	md->state.skillstate = MSS_IDLE;
	md->ud.state.blockedmove = false;
	md->next_walktime = tick+rnd()%1000+MIN_RANDOMWALKTIME;
	md->last_linktime = tick;
	md->dmgtick = tick - 5000;
	md->last_pcneartime = 0;
	md->last_canmove = tick;
	md->last_skillcheck = 0;

	t_tick c = tick - MOB_MAX_DELAY;

	for (i = 0; i < MAX_MOBSKILL; i++)
		md->skilldelay[i] = c;
	for (i = 0; i < DAMAGELOG_SIZE; i++)
		md->spotted_log[i] = 0;

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
	if( map_getmapdata(md->bl.m)->users )
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
	// Special feature that makes monsters always attack the person that provoked them
	if(battle_config.mob_ai&0x800 && md->state.provoke_flag)
	{
		if (md->state.provoke_flag == target->id)
			return 1;
		else if (!(battle_config.mob_ai&0x4))
			return 0;
	}

	switch (md->state.skillstate) {
		case MSS_BERSERK:
			if (!(mode&MD_CHANGETARGETMELEE))
				return 0;
			if (!(battle_config.mob_ai&0x80) && md->norm_attacked_id != target->id)
				return 0;
			return (battle_config.mob_ai&0x4 || check_distance_bl(&md->bl, target, md->status.rhw.range+1));
		case MSS_RUSH:
			return (mode&MD_CHANGETARGETCHASE);
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
	// When an angry monster is provoked, it will switch to retaliate AI
	if (md->state.provoke_flag && md->state.aggressive)
		md->state.aggressive = 0;
	md->min_chase = cap_value(dist + md->db->range3 - md->status.rhw.range, md->db->range3, MAX_MINCHASE);
	return 0;
}

/*==========================================
 * The ?? routine of an active monster
 *------------------------------------------*/
static int mob_ai_sub_hard_activesearch(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	struct block_list **target;
	enum e_mode mode;
	int dist;

	nullpo_ret(bl);
	md=va_arg(ap,struct mob_data *);
	target= va_arg(ap,struct block_list**);
	mode= static_cast<enum e_mode>(va_arg(ap, int));

	//If can't seek yet, not an enemy, or you can't attack it, skip.
	if ((*target) == bl || !status_check_skilluse(&md->bl, bl, 0, 0))
		return 0;

	if ((mode&MD_TARGETWEAK) && status_get_lv(bl) >= md->level-5)
		return 0;

	if(battle_check_target(&md->bl,bl,BCT_ENEMY)<=0)
		return 0;

	if (bl->type == BL_PC && BL_CAST(BL_PC, bl)->state.gangsterparadise &&
		!status_has_mode(&md->status,MD_STATUSIMMUNE))
		return 0; //Gangster paradise protection.

	if (battle_config.hom_setting&HOMSET_FIRST_TARGET &&
		(*target) != nullptr && (*target)->type == BL_HOM && bl->type != BL_HOM)
		return 0; //For some reason Homun targets are never overriden.

	dist = distance_bl(&md->bl, bl);
	if(
		((*target) == nullptr || !check_distance_bl(&md->bl, *target, dist)) &&
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
		md->min_chase = cap_value(dist + md->db->range3 - md->status.rhw.range, md->db->range3, MAX_MINCHASE);
		return 1;

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
	if (mob_can_reach(md, bl, md->db->range3) && (
		(*target) == nullptr ||
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

	if(nd->u.warp.mapindex != map_getmapdata(target->m)->index)
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
static int mob_ai_sub_hard_slavemob(struct mob_data *md,t_tick tick)
{
	struct block_list *bl;

	bl=map_id2bl(md->master_id);

	if (!bl || status_isdead(bl)) {
		status_kill(&md->bl);
		return 1;
	}
	if (bl->prev == nullptr)
		return 0; //Master not on a map? Could be warping, do not process.

	if(status_has_mode(&md->status,MD_CANMOVE))
	{	//If the mob can move, follow around. [Check by Skotlex]
		int old_dist = md->master_dist;

		// Distance with between slave and master is measured.
		md->master_dist = distance_bl(&md->bl, bl);

		if (battle_config.slave_stick_with_master || md->special_state.ai == AI_ABR || md->special_state.ai == AI_BIONIC) {
			// Since the master was in near immediately before, teleport is carried out and it pursues.
			if (bl->m != md->bl.m || (old_dist < 10 && md->master_dist > 18) || md->master_dist > MAX_MINCHASE) {
				md->master_dist = 0;
				unit_warp(&md->bl, bl->m, bl->x, bl->y, CLR_TELEPORT);
				return 1;
			}
		}

		// Slave is busy with a target.
		if(md->target_id) {
			// Player's slave should come back when master's too far, even if it is doing with a target.
			if (bl->type == BL_PC && md->master_dist > 5) {
				mob_unlocktarget(md, tick);
				unit_walktobl(&md->bl, bl, MOB_SLAVEDISTANCE, 1);
				return 1;
			} else {
				return 0;
			}
		}

		// Approach master if within view range, chase back to Master's area also if standing on top of the master.
		if ((md->master_dist > MOB_SLAVEDISTANCE || md->master_dist == 0) && unit_can_move(&md->bl)) {
			int16 x = bl->x, y = bl->y;

			if (map_search_freecell(&md->bl, bl->m, &x, &y, MOB_SLAVEDISTANCE, MOB_SLAVEDISTANCE, 1)) {
				if (unit_walktoxy(&md->bl, x, y, 0) == 0) { // Slave is too far from master (outside of battle_config.max_walk_path range), stay put
					mob_stop_walking(md, USW_FIXPOS);
					return 0; // Fail here so target will be picked back up when in range
				} else { // Slave will walk back to master if in range
					mob_stop_attack(md);
					return 1;
				}
			}
		}
	} else if (bl->m != md->bl.m && map_flag_gvg2(md->bl.m)) {
		//Delete the summoned mob if it's in a gvg ground and the master is elsewhere. [Skotlex]
		status_kill(&md->bl);
		return 1;
	}

	//Avoid attempting to lock the master's target too often to avoid unnecessary overload. [Skotlex]
	if (DIFF_TICK(tick, md->last_linktime) >= MIN_MOBLINKTIME && !md->target_id)
  	{
		struct unit_data *ud = unit_bl2ud(bl);
		md->last_linktime = tick;

		if (ud) {
			struct block_list *tbl=nullptr;
			if (ud->target && ud->state.attack_continue)
				tbl = map_id2bl(ud->target);
			else if (ud->target_to && ud->state.attack_continue)
				tbl = map_id2bl(ud->target_to);
			else if (ud->skilltarget) {
				tbl = map_id2bl(ud->skilltarget);
				//Required check as skilltarget is not always an enemy. [Skotlex]
				if (tbl && battle_check_target(&md->bl, tbl, BCT_ENEMY) <= 0)
					tbl = nullptr;
			}
			if (tbl && status_check_skilluse(&md->bl, tbl, 0, 0)) {
				md->target_id=tbl->id;
				md->min_chase = cap_value(distance_bl(&md->bl, tbl) + md->db->range3 - md->status.rhw.range, md->db->range3, MAX_MINCHASE);
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
int mob_unlocktarget(struct mob_data *md, t_tick tick)
{
	nullpo_ret(md);

	// Remember if the monster was in a "chasing" state
	bool chasestate = mob_is_chasing(md->state.skillstate);

	switch (md->state.skillstate) {
	case MSS_WALK:
		if (md->ud.walktimer != INVALID_TIMER)
			break;
		//Because it is not unset when the mob finishes walking.
		md->state.skillstate = MSS_IDLE;
		[[fallthrough]];
	case MSS_IDLE:
		if( md->ud.walktimer == INVALID_TIMER && md->idle_event[0] && npc_event_do_id( md->idle_event, md->bl.id ) > 0 ){
			md->idle_event[0] = 0;
			break;
		}
		// Idle skill.
		if (DIFF_TICK(tick, md->last_skillcheck) >= MOB_SKILL_INTERVAL && mobskill_use(md, tick, -1))
			break;
		//Random walk.
		if (!md->master_id &&
			DIFF_TICK(md->next_walktime, tick) <= 0 &&
			!mob_randomwalk(md,tick))
			//Delay next random walk when this one failed.
			if (md->next_walktime < md->ud.canmove_tick)
				md->next_walktime = md->ud.canmove_tick;
			else
				md->next_walktime = tick+rnd()%1000;
		break;
	default:
		mob_stop_attack(md);
		unit_stop_walking_soon(md->bl); //Stop chasing.
		if (status_has_mode(&md->status,MD_ANGRY) && !md->state.aggressive)
			md->state.aggressive = 1; //Restore angry state when switching to idle
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
	
	if (!md->ud.state.ignore_cell_stack_limit && battle_config.official_cell_stack_limit > 0
		&& (chasestate || battle_config.mob_ai & 0x8)
		&& map_count_oncell(md->bl.m, md->bl.x, md->bl.y, BL_CHAR | BL_NPC, 1) > battle_config.official_cell_stack_limit) {
		unit_walktoxy(&md->bl, md->bl.x, md->bl.y, 8);
	}

	return 0;
}
/*==========================================
 * Random walk
 *------------------------------------------*/
int mob_randomwalk(struct mob_data *md,t_tick tick)
{
	const int d=7;
	int i,r,rdir,dx,dy,max;

	nullpo_ret(md);

	// Initialize next_walktime
	if (md->next_walktime == INVALID_TIMER) {
		md->next_walktime = tick+rnd()%1000+MIN_RANDOMWALKTIME;
		return 1;
	}

	if(DIFF_TICK(md->next_walktime,tick)>0 ||
	   status_has_mode(&md->status,MD_NORANDOMWALK) ||
	   !unit_can_move(&md->bl) ||
	   !status_has_mode(&md->status,MD_CANMOVE))
		return 0;

	r=rnd();
	rdir=rnd()%4; // Randomize direction in which we iterate to prevent monster cluttering up in one corner
	dx=r%(d*2+1)-d;
	dy=r/(d*2+1)%(d*2+1)-d;
	max=(d*2+1)*(d*2+1);
	for(i=0;i<max;i++){	// Search of a movable place
		int x = dx + md->bl.x;
		int y = dy + md->bl.y;
		if(((x != md->bl.x) || (y != md->bl.y)) && map_getcell(md->bl.m,x,y,CELL_CHKPASS) && unit_walktoxy(&md->bl,x,y,0)){
			break;
		}
		// Could not move to cell, try the 7th cell in direction randomly decided by rdir
		// We don't move step-by-step because this will make monster stick to the walls
		switch(rdir) {
		case 0:
			dx += d;
			if (dx > d) {
				dx -= d*2+1;
				dy += d;
				if (dy > d) {
					dy -= d*2+1;
				}
			}
			break;
		case 1:
			dx -= d;
			if (dx < -d) {
				dx += d*2+1;
				dy -= d;
				if (dy < -d) {
					dy += d*2+1;
				}
			}
			break;
		case 2:
			dy += d;
			if (dy > d) {
				dy -= d*2+1;
				dx += d;
				if (dx > d) {
					dx -= d*2+1;
				}
			}
			break;
		case 3:
			dy -= d;
			if (dy < -d) {
				dy += d*2+1;
				dx -= d;
				if (dx < -d) {
					dx += d*2+1;
				}
			}
			break;
		}
	}
	if(i==max){
		// None of the available cells worked, try again next interval
		if(battle_config.mob_stuck_warning) {
			md->move_fail_count++;
			if(md->move_fail_count>1000){
				ShowWarning("MOB can't move. random spawn %d, class = %d, at %s (%d,%d)\n",md->bl.id,md->mob_id,map_getmapdata(md->bl.m)->name, md->bl.x, md->bl.y);
				md->move_fail_count=0;
				mob_spawn(md);
			}
		}
		return 0;
	}
	md->state.skillstate=MSS_WALK;
	md->move_fail_count=0;
	md->next_walktime = tick+rnd()%1000+MIN_RANDOMWALKTIME + unit_get_walkpath_time(md->bl);
	return 1;
}

int mob_warpchase(struct mob_data *md, struct block_list *target)
{
	struct npc_data *warp = nullptr;
	int distance = AREA_SIZE;
	if (!(target && battle_config.mob_ai&0x40 && battle_config.mob_warp&1))
		return 0; //Can't warp chase.

	if (target->m == md->bl.m && check_distance_bl(&md->bl, target, AREA_SIZE))
		return 0; //No need to do a warp chase.

	if (md->ud.walktimer != INVALID_TIMER &&
		map_getcell(md->bl.m,md->ud.to_x,md->ud.to_y,CELL_CHKNPC))
		return 1; //Already walking to a warp.

	//Search for warps within mob's viewing range.
	map_foreachinallrange(mob_warpchase_sub, &md->bl,
		md->db->range2, BL_NPC, target, &warp, &distance);

	if (warp && unit_walktobl(&md->bl, &warp->bl, 1, 1))
		return 1;
	return 0;
}

/*==========================================
 * AI of MOB whose is near a Player
 *------------------------------------------*/
static bool mob_ai_sub_hard(struct mob_data *md, t_tick tick)
{
	struct block_list *tbl = nullptr, *abl = nullptr;
	int mode;
	int view_range, can_move;

	if(md->bl.prev == nullptr || md->status.hp == 0)
		return false;

	// Monsters force-walked by script commands should not be searching for targets.
	if (md->ud.state.force_walk)
		return false;

	if (DIFF_TICK(tick, md->last_thinktime) < MIN_MOBTHINKTIME)
		return false;

	md->last_thinktime = tick;

	if (md->ud.skilltimer != INVALID_TIMER)
		return false;

	// Abnormalities
	if(( md->sc.opt1 && md->sc.opt1 != OPT1_STONEWAIT && md->sc.opt1 != OPT1_BURNING ) || status_db.hasSCF(&md->sc, SCF_MOBLOSETARGET)) {//Should reset targets.
		md->target_id = md->attacked_id = md->norm_attacked_id = 0;
		return false;
	}

	if (md->sc.count && md->sc.getSCE(SC_BLIND))
		view_range = 3;
	else
		view_range = md->db->range2;
	mode = status_get_mode(&md->bl);

	can_move = (mode&MD_CANMOVE) && unit_can_move(&md->bl);
	if (can_move)
		md->last_canmove = tick;

	if (md->target_id)
	{	//Check validity of current target. [Skotlex]
		tbl = map_id2bl(md->target_id);
		if (!tbl || tbl->m != md->bl.m ||
			(md->ud.attacktimer == INVALID_TIMER && !status_check_skilluse(&md->bl, tbl, 0, 0)) ||
			(md->ud.walktimer != INVALID_TIMER && !(battle_config.mob_ai&0x1) && !check_distance_bl(&md->bl, tbl, md->min_chase)) ||
			(
				tbl->type == BL_PC &&
				((((TBL_PC*)tbl)->state.gangsterparadise && !(mode&MD_STATUSIMMUNE)) ||
				((TBL_PC*)tbl)->invincible_timer != INVALID_TIMER)
		)) {	//No valid target
			if (mob_warpchase(md, tbl))
				return true; //Chasing this target.
			if (tbl && md->ud.walktimer != INVALID_TIMER && (!can_move || md->ud.walkpath.path_pos <= battle_config.mob_chase_refresh))
				return true; //Walk at least "mob_chase_refresh" cells before dropping the target unless target is non-existent
			mob_unlocktarget(md, tick); //Unlock target
			tbl = nullptr;
		}
	}

	// Check for target change.
	if( md->attacked_id && mode&MD_CANATTACK )
	{
		if( md->attacked_id == md->target_id )
		{	//Rude attacked check.
			if( !battle_check_range(&md->bl, tbl, md->status.rhw.range)
			&&  ( //Can't attack back and can't reach back.
					(!can_move && DIFF_TICK(tick, md->ud.canmove_tick) > 0 && (battle_config.mob_ai&0x2 || md->sc.getSCE(SC_SPIDERWEB)
						|| md->sc.getSCE(SC_BITE) || md->sc.getSCE(SC_VACUUM_EXTREME) || md->sc.getSCE(SC_THORNSTRAP)
						|| md->sc.getSCE(SC__MANHOLE) // Not yet confirmed if boss will teleport once it can't reach target.
						|| md->walktoxy_fail_count > 0)
					)
					|| !mob_can_reach(md, tbl, md->min_chase)
				)
			&&  md->state.attacked_count++ >= RUDE_ATTACKED_COUNT
			&&  !mobskill_use(md, tick, MSC_RUDEATTACKED) // If can't rude Attack
			&&  can_move && unit_escape(&md->bl, tbl, rnd()%10 +1)) // Attempt escape
			{	//Escaped
				md->attacked_id = md->norm_attacked_id = 0;
				return true;
			}
		}
		else
		if( (abl = map_id2bl(md->attacked_id)) && (!tbl || mob_can_changetarget(md, abl, mode)) )
		{
			int dist;
			if( md->bl.m != abl->m || abl->prev == nullptr
				|| (dist = distance_bl(&md->bl, abl)) >= MAX_MINCHASE // Attacker longer than visual area
				|| battle_check_target(&md->bl, abl, BCT_ENEMY) <= 0 // Attacker is not enemy of mob
				|| (battle_config.mob_ai&0x2 && !status_check_skilluse(&md->bl, abl, 0, 0)) // Cannot normal attack back to Attacker
				|| (!battle_check_range(&md->bl, abl, md->status.rhw.range) // Not on Melee Range and ...
				&& ( // Reach check
					(!can_move && DIFF_TICK(tick, md->ud.canmove_tick) > 0 && (battle_config.mob_ai&0x2 || md->sc.getSCE(SC_SPIDERWEB)
						|| md->sc.getSCE(SC_BITE) || md->sc.getSCE(SC_VACUUM_EXTREME) || md->sc.getSCE(SC_THORNSTRAP)
						|| md->sc.getSCE(SC__MANHOLE) // Not yet confirmed if boss will teleport once it can't reach target.
						|| md->walktoxy_fail_count > 0)
					)
					|| !mob_can_reach(md, abl, dist+md->db->range3)
				   )
				) )
			{ // Rude attacked
				if (abl->id != md->bl.id //Self damage does not cause rude attack
				&& md->state.attacked_count++ >= RUDE_ATTACKED_COUNT				
				&& !mobskill_use(md, tick, MSC_RUDEATTACKED) && can_move
				&& !tbl && unit_escape(&md->bl, abl, rnd()%10 +1))
				{	//Escaped.
					//TODO: Maybe it shouldn't attempt to run if it has another, valid target?
					md->attacked_id = md->norm_attacked_id = 0;
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
				//If a monster can change the target to the attacker, it will change the target
				md->target_id = md->attacked_id; // set target
				if (md->state.attacked_count)
					md->state.attacked_count--; //Should we reset rude attack count?
				md->min_chase = cap_value(dist + md->db->range3 - md->status.rhw.range, md->db->range3, MAX_MINCHASE);
				tbl = abl; //Set the new target
			}
		}

		//Clear it since it's been checked for already.
		md->attacked_id = md->norm_attacked_id = 0;
	}

	bool slave_lost_target = false;

	// Processing of slave monster
	if (md->master_id > 0) {
		if (mob_ai_sub_hard_slavemob(md, tick) == 1)
			return true;
		slave_lost_target = true;
	}

	// Scan area for targets
	if (!tbl && can_move && mode&MD_LOOTER && md->lootitems && DIFF_TICK(tick, md->ud.canact_tick) > 0 &&
		(md->lootitem_count < LOOTITEM_SIZE || battle_config.monster_loot_type != 1))
	{	// Scan area for items to loot, avoid trying to loot if the mob is full and can't consume the items.
		map_foreachinshootrange (mob_ai_sub_hard_lootsearch, &md->bl, view_range, BL_ITEM, md, &tbl);
	}

	if ((mode&MD_AGGRESSIVE && (!tbl || slave_lost_target)) || md->state.skillstate == MSS_FOLLOW)
	{
		map_foreachinallrange (mob_ai_sub_hard_activesearch, &md->bl, view_range, DEFAULT_ENEMY_TYPE(md), md, &tbl, mode);
	}
	else
	if (mode&MD_CHANGECHASE && (md->state.skillstate == MSS_RUSH || md->state.skillstate == MSS_FOLLOW))
	{
		int search_size;
		search_size = view_range<md->status.rhw.range ? view_range:md->status.rhw.range;
		map_foreachinallrange (mob_ai_sub_hard_changechase, &md->bl, search_size, DEFAULT_ENEMY_TYPE(md), md, &tbl);
	}

	if (!tbl) { //No targets available.
		/* bg guardians follow allies when no targets nearby */
		if( md->bg_id && mode&MD_CANATTACK ) {
			if( md->ud.walktimer != INVALID_TIMER )
				return true;/* we are already moving */
			map_foreachinallrange (mob_ai_sub_hard_bg_ally, &md->bl, view_range, BL_PC, md, &tbl, mode);
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
		if (md->lootitems == nullptr)
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
			if (!unit_walktobl(&md->bl, tbl, 0, 0))
				mob_unlocktarget(md, tick); //Can't loot...
			else
				unit_set_target(&md->ud, tbl->id); //Remember current loot target
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

	// At this point we know the target is attackable, attempt to attack

	// Monsters in angry state, after having used a normal attack, will always attempt a skill
	if (md->ud.walktimer == INVALID_TIMER && md->state.skillstate == MSS_ANGRY && md->ud.skill_id == 0)
	{
		// Only use skill if able to walk on next tick and not attempted a skill the last second
		if (DIFF_TICK(md->ud.canmove_tick, tick) <= MIN_MOBTHINKTIME && DIFF_TICK(tick, md->last_skillcheck) >= MOB_SKILL_INTERVAL){
			if (mobskill_use(md, tick, -1)) {
				// After the monster used an angry skill, it will not attack for aDelay
				// Setting the delay here because not all monster skill use situations will cause an attack delay
				md->ud.attackabletime = tick + md->status.adelay;
				return true;
			}
		}
	}

	// Normal attack / berserk skill is only used when target is in range
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

	//Target still in attack range, no need to chase the target
	if(battle_check_range(&md->bl, tbl, md->status.rhw.range))
		return true;

	//Only update target cell / drop target after having moved at least "mob_chase_refresh" cells
	if(md->ud.walktimer != INVALID_TIMER && (!can_move || md->ud.walkpath.path_pos <= battle_config.mob_chase_refresh)
		&& mob_is_chasing(md->state.skillstate))
		return true;

	// Out of range
	if (!(mode&MD_CANMOVE) || (!can_move && (md->sc.cant.move || DIFF_TICK(tick, md->ud.canmove_tick) > 0)))
	{	// Can't chase. Immobile and trapped mobs will use idle skills and unlock their target after a while
		if (md->ud.attacktimer == INVALID_TIMER)
		{ // Only switch mode if no more attack delay left
			if (DIFF_TICK(tick, md->last_canmove) > battle_config.mob_unlock_time) {
				// Unlock target or use idle/walk skill
				mob_unlocktarget(md, tick);
			}
			else {
				// Use idle skill but keep target for now
				md->state.skillstate = MSS_IDLE;
				if (DIFF_TICK(tick, md->last_skillcheck) >= MOB_SKILL_INTERVAL)
					mobskill_use(md, tick, -1);
			}
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
	if (!mob_can_reach(md, tbl, md->min_chase)) {
		mob_unlocktarget(md, tick);
		return true;
	}
	// Monsters can use chase skills before starting to walk
	// So we need to change the state and check for a skill here already
	// But only use skill if able to walk on next tick and not attempted a skill the last second
	// Skills during movement are handled in the walktobl routine
	if (md->ud.walktimer == INVALID_TIMER
		&& DIFF_TICK(md->ud.canmove_tick, tick) <= MIN_MOBTHINKTIME
		&& DIFF_TICK(tick, md->last_skillcheck) >= MOB_SKILL_INTERVAL) {
		md->state.skillstate = md->state.aggressive ? MSS_FOLLOW : MSS_RUSH;
		if (mobskill_use(md, tick, -1))
			return true;
	}

	if(!unit_walktobl(&md->bl, tbl, md->status.rhw.range, 2))
		mob_unlocktarget(md, tick);

	return true;
}

static int mob_ai_sub_hard_timer(struct block_list *bl,va_list ap)
{
	struct mob_data *md = (struct mob_data*)bl;
	uint32 char_id = va_arg(ap, uint32);
	t_tick tick = va_arg(ap, t_tick);
	if (mob_ai_sub_hard(md, tick))
	{	//Hard AI triggered.
		mob_add_spotted(md, char_id);
		md->last_pcneartime = tick;
	}
	return 0;
}

/*==========================================
 * Serious processing for mob in PC field of view (foreachclient)
 *------------------------------------------*/
static int mob_ai_sub_foreachclient(map_session_data *sd,va_list ap)
{
	t_tick tick=va_arg(ap,t_tick);
	map_foreachinallrange(mob_ai_sub_hard_timer,&sd->bl, AREA_SIZE+ACTIVE_AI_RANGE, BL_MOB, sd->status.char_id, tick);

	return 0;
}

/*==========================================
 * Negligent mode MOB AI (PC is not in near)
 *------------------------------------------*/
static int mob_ai_sub_lazy(struct mob_data *md, va_list args)
{
	nullpo_ret(md);

	if(md->bl.prev == nullptr)
		return 0;

	// Monsters force-walked by script commands should not be searching for targets.
	if (md->ud.state.force_walk)
		return false;

	t_tick tick = va_arg(args,t_tick);

	if (battle_config.mob_ai&0x20 && map_getmapdata(md->bl.m)->users>0)
		return (int)mob_ai_sub_hard(md, tick);

	if (md->bl.prev==nullptr || md->status.hp == 0)
		return 1;

	if(battle_config.mob_active_time &&
		md->last_pcneartime &&
 		!status_has_mode(&md->status,MD_STATUSIMMUNE) &&
		DIFF_TICK(tick,md->last_thinktime) > MIN_MOBTHINKTIME)
	{
		if (DIFF_TICK(tick,md->last_pcneartime) < battle_config.mob_active_time)
			return (int)mob_ai_sub_hard(md, tick);
		md->last_pcneartime = 0;
	}

	if(battle_config.boss_active_time &&
		md->last_pcneartime &&
		status_has_mode(&md->status,MD_STATUSIMMUNE) &&
		DIFF_TICK(tick,md->last_thinktime) > MIN_MOBTHINKTIME)
	{
		if (DIFF_TICK(tick,md->last_pcneartime) < battle_config.boss_active_time)
			return (int)mob_ai_sub_hard(md, tick);
		md->last_pcneartime = 0;
	}

	//Clean the spotted log
	mob_clean_spotted(md);

	if(DIFF_TICK(tick,md->last_thinktime) < MOB_SKILL_INTERVAL)
		return 0;

	md->last_thinktime=tick;

	if (md->master_id) {
		if (!mob_is_spotted(md)) {
			// Get mob data of master
			mob_data* mmd = map_id2md(md->master_id);
			// If neither master nor slave have been spotted we don't have to execute the slave AI
			if (mmd && !mob_is_spotted(mmd))
				return 0;
		}
		mob_ai_sub_hard_slavemob (md,tick);
		return 0;
	}

	if (md->ud.walktimer == INVALID_TIMER) {
		// Because it is not unset when the mob finishes walking.
		md->state.skillstate = MSS_IDLE;
		if (md->idle_event[0] && npc_event_do_id( md->idle_event, md->bl.id ) > 0) {
			md->idle_event[0] = 0;
			return 0;
		}
	}

	if( DIFF_TICK(md->next_walktime,tick) < 0 && status_has_mode(&md->status,MD_CANMOVE) && unit_can_move(&md->bl) )
	{
		// Move probability for mobs away from players
		// In Aegis, this is 100% for mobs that have been activated by players and none otherwise.
		if( mob_is_spotted(md) &&
			((!status_has_mode(&md->status,MD_STATUSIMMUNE) && rnd()%100 < battle_config.mob_nopc_move_rate) ||
			(status_has_mode(&md->status,MD_STATUSIMMUNE) && rnd()%100 < battle_config.boss_nopc_move_rate)))
			mob_randomwalk(md, tick);
	}
	else if( md->ud.walktimer == INVALID_TIMER )
	{
		// Probability for mobs far from players from doing their IDLE skill.
		// In Aegis, this is 100% for mobs that have been activated by players and none otherwise.
		if( mob_is_spotted(md) &&
			((!status_has_mode(&md->status,MD_STATUSIMMUNE) && rnd()%100 < battle_config.mob_nopc_idleskill_rate) ||
			(status_has_mode(&md->status,MD_STATUSIMMUNE) && rnd()%100 < battle_config.boss_nopc_idleskill_rate)))
			mobskill_use(md, tick, -1);
	}

	return 0;
}

/*==========================================
 * Negligent processing for mob outside PC field of view   (interval timer function)
 *------------------------------------------*/
static TIMER_FUNC(mob_ai_lazy){
	map_foreachmob(mob_ai_sub_lazy,tick);
	return 0;
}

/*==========================================
 * Serious processing for mob in PC field of view   (interval timer function)
 *------------------------------------------*/
static TIMER_FUNC(mob_ai_hard){

	if (battle_config.mob_ai&0x20)
		map_foreachmob(mob_ai_sub_lazy,tick);
	else
		map_foreachpc(mob_ai_sub_foreachclient,tick);

	return 0;
}

/**
 * Set random option for item when dropped from monster
 * @param item: Item data
 * @param mobdrop: Drop data
 * @author [Cydh]
 **/
void mob_setdropitem_option( item& item, s_mob_drop& mobdrop ){
	std::shared_ptr<s_random_opt_group> group = random_option_group.find( mobdrop.randomopt_group );

	if (group != nullptr) {
		group->apply( item );
	}
}

/*==========================================
 * Initializes the delay drop structure for mob-dropped items.
 *------------------------------------------*/
static std::shared_ptr<s_item_drop> mob_setdropitem( s_mob_drop& mobdrop, int qty, unsigned short mob_id ){
	std::shared_ptr<s_item_drop> drop = std::make_shared<s_item_drop>();

	drop->item_data = { 0 };
	drop->item_data.nameid = mobdrop.nameid;
	drop->item_data.amount = qty;
	drop->item_data.identify = itemdb_isidentified( mobdrop.nameid );
	mob_setdropitem_option( drop->item_data, mobdrop );
	drop->mob_id = mob_id;

	return drop;
}

/*==========================================
 * Initializes the delay drop structure for mob-looted items.
 *------------------------------------------*/
static std::shared_ptr<s_item_drop> mob_setlootitem( s_mob_lootitem& item, unsigned short mob_id ){
	std::shared_ptr<s_item_drop> drop = std::make_shared<s_item_drop>();

	memcpy( &drop->item_data, &item, sizeof( struct item ) );

	/**
	 * Conditions for looted item, so it can be announced when player pick it up
	 * 1. Not-dropped other than monster. (This will be done later on pc_takeitem/party_share_loot)
	 * 2. The mob_id becomes the last looter, instead of the real monster who drop it.
	 **/
	drop->mob_id = item.mob_id;

	return drop;
}

/**
 * Makes all items from a drop list drop
 * @param list: list with all items that should drop
 * @param loot: whether the items in the list are new drops or previously looted items
 */
void mob_process_drop_list(std::shared_ptr<s_item_drop_list>& list, bool loot)
{
	// First regular drop always drops at center
	enum directions dir = DIR_CENTER;
	// Looted drops start north instead
	if (loot)
		dir = DIR_NORTH;

	for (std::shared_ptr<s_item_drop>& ditem : list->items) {
		map_addflooritem(&ditem->item_data, ditem->item_data.amount,
			list->m, list->x, list->y,
			list->first_charid, list->second_charid, list->third_charid, 4, ditem->mob_id, !loot, dir, BL_CHAR|BL_PET);
		// The drop location loops between three locations: SE -> W -> N -> SE
		if (dir <= DIR_NORTH)
			dir = DIR_SOUTHEAST;
		else if (dir == DIR_SOUTHEAST)
			dir = DIR_WEST;
		else
			dir = DIR_NORTH;
	}
}

/*==========================================
 * item drop with delay (timer function)
 *------------------------------------------*/
static TIMER_FUNC(mob_delay_item_drop) {
	uint32 bl_id = static_cast<uint32>(id);

	// Regular drops
	std::shared_ptr<s_item_drop_list> list = util::umap_find(mob_delayed_drops, bl_id);
	if (list != nullptr) {
		mob_process_drop_list(list, false);
		mob_delayed_drops.erase(bl_id);
	}

	// Looted drops
	list = util::umap_find(mob_looted_drops, bl_id);
	if (list != nullptr) {
		mob_process_drop_list(list, true);
		mob_looted_drops.erase(bl_id);
	}

	return 0;
}

/*==========================================
 * Sets the item_drop into the item_drop_list.
 * Also performs logging and autoloot if enabled.
 * rate is the drop-rate of the item, required for autoloot.
 * flag : Killed only by homunculus/mercenary?
 *------------------------------------------*/
static void mob_item_drop(struct mob_data *md, std::shared_ptr<s_item_drop_list>& dlist, std::shared_ptr<s_item_drop>& ditem, int loot, int drop_rate, bool flag)
{
	TBL_PC* sd;
	bool test_autoloot;
	//Logs items, dropped by mobs [Lupus]
	log_pick_mob(md, loot?LOG_TYPE_LOOT:LOG_TYPE_PICKDROP_MONSTER, -ditem->item_data.amount, &ditem->item_data);

	sd = map_charid2sd(dlist->first_charid);
	if( sd == nullptr ) sd = map_charid2sd(dlist->second_charid);
	if( sd == nullptr ) sd = map_charid2sd(dlist->third_charid);
	test_autoloot = sd 
		&& (drop_rate <= sd->state.autoloot || pc_isautolooting(sd, ditem->item_data.nameid))
		&& (flag ? ((battle_config.homunculus_autoloot ? (battle_config.hom_idle_no_share == 0 || !pc_isidle_hom(sd)) : 0) || (battle_config.mercenary_autoloot ? (battle_config.mer_idle_no_share == 0 || !pc_isidle_mer(sd)) : 0)) :
			(battle_config.idle_no_autoloot == 0 || DIFF_TICK(last_tick, sd->idletime) < battle_config.idle_no_autoloot));
#ifdef AUTOLOOT_DISTANCE
		test_autoloot = test_autoloot && sd->bl.m == md->bl.m
		&& check_distance_blxy(&sd->bl, dlist->x, dlist->y, AUTOLOOT_DISTANCE);
#endif
	if( test_autoloot ) {	//Autoloot.
		struct party_data *p = party_search(sd->status.party_id);

		if ((itemdb_search(ditem->item_data.nameid))->flag.broadcast &&
			(!p || !(p->party.item & 2)) // Somehow, if party's pickup distribution is 'Even Share', no announcemet
			)
			intif_broadcast_obtain_special_item(sd, ditem->item_data.nameid, md->mob_id, ITEMOBTAIN_TYPE_MONSTER_ITEM);

		if (party_share_loot(party_search(sd->status.party_id),
			sd, &ditem->item_data, sd->status.char_id) == 0
		) {
			return;
		}
	}

	dlist->items.push_back( ditem );
}

TIMER_FUNC(mob_timer_delete){
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
TIMER_FUNC(mob_respawn){
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

	switch( src->type )
	{
		case BL_PC:
		{
			map_session_data *sd = (TBL_PC*)src;
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
			s_mercenary_data *mer = (TBL_MER*)src;
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
				map_session_data* msd = map_id2sd(md2->master_id);
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
			s_elemental_data *ele = (TBL_ELEM*)src;
			if( ele->master )
				char_id = ele->master->status.char_id;
			if( damage )
				md->attacked_id = src->id;
			break;
		}
		default: //For all unhandled types.
			md->attacked_id = src->id;
	}

	//Self damage increases tap bonus
	if (!char_id && src->id == md->bl.id && damage > 0) {
		char_id = src->id;
		flag = MDLF_SELF;
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

				if( md->get_bosstype() == BOSSTYPE_MVP )
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

			if( md->get_bosstype() == BOSSTYPE_MVP )
				pc_damage_log_add(map_charid2sd(char_id),md->bl.id);
		}
	}
	return;
}
//Call when a mob has received damage.
void mob_damage(struct mob_data *md, struct block_list *src, int damage)
{
	if( src != nullptr && md->special_state.ai == AI_SPHERE && !md->dmglog[0].id ) {//LOne WOlf explained that ANYONE can trigger the marine countdown skill. [Skotlex]
		md->state.can_escape = 1;
	}

	if (src && damage > 0) { //Store total damage...
		if (UINT_MAX - (unsigned int)damage > md->tdmg)
			md->tdmg += damage;
		else if (md->tdmg == UINT_MAX)
			damage = 0; //Stop recording damage once the cap has been reached.
		else { //Cap damage log...
			damage = (int)(UINT_MAX - md->tdmg);
			md->tdmg = UINT_MAX;
		}
		if ((src != &md->bl) && md->state.aggressive) //No longer aggressive, change to retaliate AI.
			md->state.aggressive = 0;
		//Log damage
		mob_log_damage(md, src, damage);
		md->dmgtick = gettick();
	}

	if (battle_config.show_mob_info&3)
		clif_name_area(&md->bl);

#if PACKETVER >= 20120404
	if (battle_config.monster_hp_bars_info && !map_getmapflag(md->bl.m, MF_HIDEMOBHPBAR)) {
		int i;
		if (md->special_state.ai == AI_ABR || md->special_state.ai == AI_BIONIC) {
			clif_summon_hp_bar(*md);
		}
		for(i = 0; i < DAMAGELOG_SIZE; i++){ // must show hp bar to all char who already hit the mob.
			map_session_data *sd = map_charid2sd(md->dmglog[i].id);
			if( sd && check_distance_bl(&md->bl, &sd->bl, AREA_SIZE) ) // check if in range
				clif_monster_hp_bar(md, sd->fd);
		}
	}
#endif
}

/**
 * Get modified drop rate
 * @param src: Source object
 * @param mob: Monster data
 * @param base_rate: Base drop rate
 * @param drop_modifier: RENEWAL_DROP level modifier
 * @return Modified drop rate
 */
int mob_getdroprate(struct block_list *src, std::shared_ptr<s_mob_db> mob, int base_rate, int drop_modifier, mob_data* md)
{
	int drop_rate = base_rate;

	if (md && battle_config.mob_size_influence) {  // Change drops depending on monsters size [Valaris]
		unsigned int mob_size = md->special_state.size;
		if (mob_size == SZ_MEDIUM && drop_rate >= 2)
			drop_rate /= 2; // SZ_MEDIUM actually is small size modification... this is not a bug!
		else if (mob_size == SZ_BIG)
			drop_rate *= 2;
	}

	if (src) {
		if (battle_config.drops_by_luk) // Drops affected by luk as a fixed increase [Valaris]
			drop_rate += status_get_luk(src) * battle_config.drops_by_luk / 100;
		if (battle_config.drops_by_luk2) // Drops affected by luk as a % increase [Skotlex]
			drop_rate += (int)(0.5 + drop_rate * status_get_luk(src) * battle_config.drops_by_luk2 / 10000.);

		if (src->type == BL_PC) { // Player specific drop rate adjustments
			map_session_data *sd = (map_session_data*)src;
			int drop_rate_bonus = 100;

			// In PK mode players get an additional drop chance bonus of 25% if there is a 20 level difference
			if( battle_config.pk_mode && (int)(mob->lv - sd->status.base_level) >= 20 ){
				drop_rate_bonus += 25;
			}

			// Add class and race specific bonuses
			drop_rate_bonus += sd->indexed_bonus.dropaddclass[mob->status.class_] + sd->indexed_bonus.dropaddclass[CLASS_ALL];
			drop_rate_bonus += sd->indexed_bonus.dropaddrace[mob->status.race] + sd->indexed_bonus.dropaddrace[RC_ALL];

			if (sd->sc.getSCE(SC_ITEMBOOST))
				drop_rate_bonus += sd->sc.getSCE(SC_ITEMBOOST)->val1;
			if (sd->sc.getSCE(SC_PERIOD_RECEIVEITEM_2ND))
				drop_rate_bonus += sd->sc.getSCE(SC_PERIOD_RECEIVEITEM_2ND)->val1;

			int cap;

			if (pc_isvip(sd)) { // Increase item drop rate for VIP.
				// Unsure how the VIP and other bonuses should stack, this is additive.
				drop_rate_bonus += battle_config.vip_drop_increase;
				cap = battle_config.drop_rate_cap_vip;
			} else
				cap = battle_config.drop_rate_cap;

			drop_rate = (int)( 0.5 + drop_rate * drop_rate_bonus / 100. );

			// Now limit the drop rate to never be exceed the cap (default: 90%), unless it is originally above it already.
			if( drop_rate > cap && base_rate < cap ){
				drop_rate = cap;
			}
		}
	}

#ifdef RENEWAL_DROP
	drop_rate = apply_rate( drop_rate, drop_modifier );
#endif

	// Cap it to 100%
	drop_rate = min( drop_rate, 10000 );

	// If the monster's drop rate can become 0
	if( battle_config.drop_rate0item ){
		drop_rate = max( drop_rate, 0 );
	}else{
		// If not - cap to 0.01% drop rate - as on official servers
		drop_rate = max( drop_rate, 1 );
	}

	return drop_rate;
}

/*==========================================
 * Signals death of mob.
 * type&1 -> no drops, type&2 -> no exp
 *------------------------------------------*/
int mob_dead(struct mob_data *md, struct block_list *src, int type)
{
	struct status_data *status;
	map_session_data *sd = nullptr, *tmpsd[DAMAGELOG_SIZE];
	map_session_data *mvp_sd = nullptr, *second_sd = nullptr, *third_sd = nullptr;

	struct {
		struct party_data *p;
		int id,zeny;
		t_exp base_exp;
		t_exp job_exp;
	} pt[DAMAGELOG_SIZE];
	int i, temp, count, m = md->bl.m;
	int dmgbltypes = 0;  // bitfield of all bl types, that caused damage to the mob and are elligible for exp distribution
	unsigned int mvp_damage;
	t_tick tick = gettick();
	bool rebirth, homkillonly, merckillonly;

	status = &md->status;

	if( src && src->type == BL_PC ) {
		sd = (map_session_data *)src;
		mvp_sd = sd;
	}

	if( md->guardian_data && md->guardian_data->number >= 0 && md->guardian_data->number < MAX_GUARDIANS )
		guild_castledatasave(md->guardian_data->castle->castle_id, CD_ENABLED_GUARDIAN00 + md->guardian_data->number,0);

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
		map_session_data* tsd = nullptr;
		if (md->dmglog[i].flag == MDLF_SELF) {
			//Self damage counts as exp tap
			count++;
			continue;
		}
		tsd = map_charid2sd(md->dmglog[i].id);
		if (tsd == nullptr)
			continue; // skip empty entries
		if (tsd->bl.m != m)
			continue; // skip players not on this map
		count++; //Only logged into same map chars are counted for the total.
		if (pc_isdead(tsd))
			continue; // skip dead players
		if (md->dmglog[i].flag == MDLF_HOMUN && !hom_is_active(tsd->hd))
			continue; // skip homunc's share if inactive
		if (md->dmglog[i].flag == MDLF_PET && (!tsd->status.pet_id || !tsd->pd))
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
	// determines if the monster was killed by mercenary damage only
	merckillonly = (bool)((dmgbltypes & BL_MER) && !(dmgbltypes & ~BL_MER));

	if(battle_config.exp_calc_type == 2 && count > 1) {	//Apply first-attacker 200% exp share bonus
		if (UINT_MAX - md->dmglog[0].dmg > md->tdmg) {
			md->tdmg += md->dmglog[0].dmg;
			md->dmglog[0].dmg *= 2;
		} else {
			md->dmglog[0].dmg+= UINT_MAX - md->tdmg;
			md->tdmg = UINT_MAX;
		}
	}

	if(!(type&2) && //No exp
		(!map_getmapflag(m, MF_PVP) || battle_config.pvp_exp) && //Pvp no exp rule [MouseJstr]
		(!md->master_id || !md->special_state.ai) && //Only player-summoned mobs do not give exp. [Skotlex]
		(!map_getmapflag(m, MF_NOBASEEXP) || !map_getmapflag(m, MF_NOJOBEXP)) //Gives Exp
	) { //Experience calculation.
		int bonus = 100; //Bonus on top of your share (common to all attackers).
		int pnum = 0;
#ifndef RENEWAL
		if (md->sc.getSCE(SC_RICHMANKIM))
			bonus += md->sc.getSCE(SC_RICHMANKIM)->val2;
#else
		if (sd && sd->sc.getSCE(SC_RICHMANKIM))
			bonus += sd->sc.getSCE(SC_RICHMANKIM)->val2;
#endif
		if(sd) {
			temp = status_get_class(&md->bl);
			if(sd->sc.getSCE(SC_MIRACLE)) i = 2; //All mobs are Star Targets
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
			t_exp base_exp, job_exp;
			double per; //Your share of the mob's exp

			if (!tmpsd[i]) continue;

			if (battle_config.exp_calc_type == 1 || md->tdmg == 0) {
				// eAthena's exp formula based on max hp
				per = (double)md->dmglog[i].dmg / (double)status->max_hp;
			}
			else {
				// Aegis's exp formula based on total damage
				per = (double)md->dmglog[i].dmg / (double)md->tdmg;
			}
			// To prevent exploits
			if (per > 1) per = 1;

			//Exclude rebirth tap from this calculation
			count -= md->state.rebirth;
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
				if( md->get_bosstype() == BOSSTYPE_MVP )
					zeny*=rnd()%250;
			}

			if (map_getmapflag(m, MF_NOBASEEXP) || !md->db->base_exp)
				base_exp = 0;
			else {
				double exp = apply_rate2(md->db->base_exp, per, 1);
				exp = apply_rate(exp, bonus);
				exp = apply_rate(exp, map_getmapflag(m, MF_BEXP));
				base_exp = (t_exp)cap_value(exp, 1, MAX_EXP);
			}

			if (map_getmapflag(m, MF_NOJOBEXP) || !md->db->job_exp
#ifndef RENEWAL
				|| md->dmglog[i].flag == MDLF_HOMUN // Homun earned job-exp is always lost.
#endif
			)
				job_exp = 0;
			else {
				double exp = apply_rate2(md->db->job_exp, per, 1);
				exp = apply_rate(exp, bonus);
				exp = apply_rate(exp, map_getmapflag(m, MF_JEXP));
				job_exp = (t_exp)cap_value(exp, 1, MAX_EXP);
			}

			if ((base_exp > 0 || job_exp > 0) && md->dmglog[i].flag == MDLF_HOMUN && homkillonly && battle_config.hom_idle_no_share && pc_isidle_hom(tmpsd[i]))
				base_exp = job_exp = 0;

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
					pt[j].base_exp = util::safe_addition_cap( pt[j].base_exp, base_exp, MAX_EXP );
					pt[j].job_exp = util::safe_addition_cap( pt[j].job_exp, job_exp, MAX_EXP );
					pt[j].zeny += zeny;  // zeny share [Valaris]
					flag = 0;
				}
			}
#ifdef RENEWAL
			if (base_exp && tmpsd[i] && tmpsd[i]->hd)
				hom_gainexp(tmpsd[i]->hd, base_exp * battle_config.homunculus_exp_gain / 100); // Homunculus only receive 10% of EXP
#else
			if (base_exp && md->dmglog[i].flag == MDLF_HOMUN) //tmpsd[i] is null if it has no homunc.
				hom_gainexp(tmpsd[i]->hd, base_exp);
#endif
			if(flag) {
				if(base_exp || job_exp) {
					if( md->dmglog[i].flag != MDLF_PET || battle_config.pet_attack_exp_to_master ) {
#ifdef RENEWAL_EXP
						int rate = pc_level_penalty_mod( tmpsd[i], PENALTY_EXP, nullptr, md );
						if (rate != 100) {
							if (base_exp)
								base_exp = (t_exp)cap_value(apply_rate(base_exp, rate), 1, MAX_EXP);
							if (job_exp)
								job_exp = (t_exp)cap_value(apply_rate(job_exp, rate), 1, MAX_EXP);
						}
#endif
						pc_gainexp(tmpsd[i], &md->bl, base_exp, job_exp, 0);
					}
				}
				if(zeny) // zeny from mobs [Valaris]
					pc_getzeny(tmpsd[i], zeny, LOG_TYPE_PICKDROP_MONSTER);
			}

			if( md->get_bosstype() == BOSSTYPE_MVP )
				pc_damage_log_clear(tmpsd[i],md->bl.id);
		}

		for( i = 0; i < pnum; i++ ) //Party share.
			party_exp_share(pt[i].p, &md->bl, pt[i].base_exp,pt[i].job_exp,pt[i].zeny);

	} //End EXP giving.

	// Looted items have an independent drop position and also don't show special effects when dropped
	// So we need put them into a separate list
	std::shared_ptr<s_item_drop_list> lootlist = std::make_shared<s_item_drop_list>();
	lootlist->m = md->bl.m;
	lootlist->x = md->bl.x;
	lootlist->y = md->bl.y;
	lootlist->first_charid = (mvp_sd ? mvp_sd->status.char_id : 0);
	lootlist->second_charid = (second_sd ? second_sd->status.char_id : 0);
	lootlist->third_charid = (third_sd ? third_sd->status.char_id : 0);

	// Process items looted by the mob
	if (md->lootitems) {
		for (i = 0; i < md->lootitem_count; i++) {
			std::shared_ptr<s_item_drop> ditem = mob_setlootitem(md->lootitems[i], md->mob_id);
			mob_item_drop(md, lootlist, ditem, 1, 10000, homkillonly || merckillonly);
		}
	}

	if( !(type&1) && !map_getmapflag(m, MF_NOMOBLOOT) && !md->state.rebirth && (
		!md->special_state.ai || //Non special mob
		battle_config.alchemist_summon_reward == 2 || //All summoned give drops
		(md->special_state.ai==AI_SPHERE && battle_config.alchemist_summon_reward == 1) //Marine Sphere Drops items.
		) )
	{ // Item Drop
		int drop_rate, drop_modifier = 100;

#ifdef RENEWAL_DROP
		drop_modifier = pc_level_penalty_mod( mvp_sd != nullptr ? mvp_sd : second_sd != nullptr ? second_sd : third_sd, PENALTY_DROP, nullptr, md );
#endif

		std::shared_ptr<s_item_drop_list> dlist = std::make_shared<s_item_drop_list>();
		dlist->m = md->bl.m;
		dlist->x = md->bl.x;
		dlist->y = md->bl.y;
		dlist->first_charid = (mvp_sd ? mvp_sd->status.char_id : 0);
		dlist->second_charid = (second_sd ? second_sd->status.char_id : 0);
		dlist->third_charid = (third_sd ? third_sd->status.char_id : 0);

		// Ore Discovery [Celest]
		if (sd == mvp_sd && pc_checkskill(sd,BS_FINDINGORE)>0 && battle_config.finding_ore_rate/10 >= rnd()%10000) {
			s_mob_drop mobdrop = {};

			mobdrop.nameid = itemdb_group.get_random_item_id(IG_FINDINGORE,1);

			std::shared_ptr<s_item_drop> ditem = mob_setdropitem( mobdrop, 1, md->mob_id );

			mob_item_drop(md, dlist, ditem, 0, battle_config.finding_ore_rate/10, homkillonly || merckillonly);
		}

		if(sd) {
			// process script-granted extra drop bonuses
			t_itemid dropid = 0;

			for (const auto &it : sd->add_drop) {
				if (!&it || (!it.nameid && !it.group))
					continue;
				if ((it.race < RC_NONE_ && it.race == -md->mob_id) || //Race < RC_NONE_, use mob_id
					(it.race == RC_ALL || it.race == status->race) || //Matched race
					(it.class_ == CLASS_ALL || it.class_ == status->class_)) //Matched class
				{
					//Check if the bonus item drop rate should be multiplied with mob level/10 [Lupus]
					if (it.rate < 0) {
						//It's negative, then it should be multiplied. with mob_level/10
						//rate = base_rate * (mob_level/10) + 1
						drop_rate = (-it.rate) * md->level / 10 + 1;
						drop_rate = cap_value(drop_rate, max(battle_config.item_drop_adddrop_min,1), min(battle_config.item_drop_adddrop_max,10000));
					}
					else
						//it's positive, then it goes as it is
						drop_rate = it.rate;

					if (rnd()%10000 >= drop_rate)
						continue;
					dropid = (it.nameid > 0) ? it.nameid : itemdb_group.get_random_item_id(it.group,1);

					s_mob_drop mobdrop = {};

					mobdrop.nameid = dropid;

					std::shared_ptr<s_item_drop> ditem = mob_setdropitem(mobdrop, 1, md->mob_id);

					mob_item_drop( md, dlist, ditem, 0, drop_rate, homkillonly || merckillonly );
				}
			}

			// process script-granted zeny bonus (get_zeny_num) [Skotlex]
			if( sd->bonus.get_zeny_num && rnd()%100 < sd->bonus.get_zeny_rate ) {
				i = sd->bonus.get_zeny_num > 0 ? sd->bonus.get_zeny_num : -md->level * sd->bonus.get_zeny_num;
				if (!i) i = 1;
				pc_getzeny(sd, 1+rnd()%i, LOG_TYPE_PICKDROP_MONSTER);
			}
		}

		// Regular mob drops drop after script-granted drops
		for (i = 0; i < MAX_MOB_DROP_TOTAL; i++) {
			if (md->db->dropitem[i].nameid == 0)
				continue;

			std::shared_ptr<item_data> it = item_db.find(md->db->dropitem[i].nameid);

			if (it == nullptr)
				continue;

			drop_rate = mob_getdroprate(src, md->db, md->db->dropitem[i].rate, drop_modifier, md);

			// attempt to drop the item
			if (rnd() % 10000 >= drop_rate)
				continue;

			if (mvp_sd && it->type == IT_PETEGG) {
				pet_create_egg(mvp_sd, md->db->dropitem[i].nameid);
				continue;
			}

			std::shared_ptr<s_item_drop> ditem = mob_setdropitem(md->db->dropitem[i], 1, md->mob_id);

			//A Rare Drop Global Announce by Lupus
			if (mvp_sd && md->db->dropitem[i].rate <= battle_config.rare_drop_announce) {
				char message[128];
				sprintf(message, msg_txt(nullptr, 541), mvp_sd->status.name, md->name, it->ename.c_str(), (float)drop_rate / 100);
				//MSG: "'%s' won %s's %s (chance: %0.02f%%)"
				intif_broadcast(message, strlen(message) + 1, BC_DEFAULT);
			}
			// Announce first, or else ditem will be freed. [Lance]
			// By popular demand, use base drop rate for autoloot code. [Skotlex]
			mob_item_drop(md, dlist, ditem, 0, battle_config.autoloot_adjust ? drop_rate : md->db->dropitem[i].rate, homkillonly || merckillonly);
		}

		// Process map specific drops
		std::shared_ptr<s_map_drops> mapdrops;

		// If it is an instance map, we check for map specific drops of the original map
		if( map[md->bl.m].instance_id > 0 ){
			mapdrops = map_drop_db.find( map[md->bl.m].instance_src_map );
		}else{
			mapdrops = map_drop_db.find( md->bl.m );
		}

		if( mapdrops != nullptr ){
			// Process map wide drops
			for( const auto& it : mapdrops->globals ){
				if( rnd_chance( it.second->rate, 100000u ) ){
					// 'Cheat' for autoloot command: rate is changed from n/100000 to n/10000
					int32 map_drops_rate = max(1, (it.second->rate / 10));
					std::shared_ptr<s_item_drop> ditem = mob_setdropitem(*it.second, 1, md->mob_id);
					mob_item_drop( md, dlist, ditem, 0, map_drops_rate, homkillonly || merckillonly );
				}
			}

			// Process map drops for this specific mob
			const auto& specific = mapdrops->specific.find( md->mob_id );

			if( specific != mapdrops->specific.end() ){
				for( const auto& it : specific->second ){
					if( rnd_chance( it.second->rate, 100000u ) ){
						// 'Cheat' for autoloot command: rate is changed from n/100000 to n/10000
						int32 map_drops_rate = max(1, (it.second->rate / 10));
						std::shared_ptr<s_item_drop> ditem = mob_setdropitem(*it.second, 1, md->mob_id);
						mob_item_drop( md, dlist, ditem, 0, map_drops_rate, homkillonly || merckillonly );
					}
				}
			}
		}

		// There are drop items.
		if (!dlist->items.empty() || !lootlist->items.empty()) {
			mob_delayed_drops[md->bl.id] = dlist;
			mob_looted_drops[md->bl.id] = lootlist;
			add_timer(tick + (!battle_config.delay_battle_damage ? 500 : 0), mob_delay_item_drop, md->bl.id, 0);
		}
	}
	// Loot MUST drop!
	else if (!lootlist->items.empty()) {
		mob_looted_drops[md->bl.id] = lootlist;
		add_timer(tick + (!battle_config.delay_battle_damage ? 500 : 0), mob_delay_item_drop, md->bl.id, 0);
	}

	if( mvp_sd && md->get_bosstype() == BOSSTYPE_MVP ){
		t_itemid log_mvp_nameid = 0;
		t_exp log_mvp_exp = 0;

		clif_mvp_effect( mvp_sd );

		//mapflag: noexp check [Lorky]
		if( md->db->mexp > 0 && !( map_getmapflag( m, MF_NOBASEEXP ) || type&2 ) ){
			log_mvp_exp = md->db->mexp;

#if defined(RENEWAL_EXP)
			int penalty = pc_level_penalty_mod( mvp_sd, PENALTY_MVP_EXP, nullptr, md );

			log_mvp_exp = cap_value( apply_rate( log_mvp_exp, penalty ), 0, MAX_EXP );
#endif

			if( battle_config.exp_bonus_attacker > 0 && count > 1 ){
				if( count > battle_config.exp_bonus_max_attacker ){
					count = battle_config.exp_bonus_max_attacker;
				}

				log_mvp_exp += log_mvp_exp * ( battle_config.exp_bonus_attacker * ( count - 1 ) ) / 100;
			}

			log_mvp_exp = cap_value( log_mvp_exp, 1, MAX_EXP );

			clif_mvp_exp( mvp_sd, log_mvp_exp );
			pc_gainexp( mvp_sd, &md->bl, log_mvp_exp, 0, 0 );
		}

		if( !(map_getmapflag(m, MF_NOMVPLOOT) || type&1) ) {
			//Order might be random depending on item_drop_mvp_mode config setting
			struct s_mob_drop mdrop[MAX_MVP_DROP_TOTAL];

			memset(&mdrop,0,sizeof(mdrop));

			if(battle_config.item_drop_mvp_mode == 1) {
				//Random order
				for(i = 0; i < MAX_MVP_DROP_TOTAL; i++) {
					while( 1 ) {
						uint8 va = rnd()%MAX_MVP_DROP_TOTAL;
						if (mdrop[va].nameid == 0) {
							if (md->db->mvpitem[i].nameid > 0)
								memcpy(&mdrop[va],&md->db->mvpitem[i],sizeof(mdrop[va]));
							break;
						}
					}
				}
			} else {
				//Normal order
				for(i = 0; i < MAX_MVP_DROP_TOTAL; i++) {
					if (md->db->mvpitem[i].nameid > 0)
						memcpy(&mdrop[i],&md->db->mvpitem[i],sizeof(mdrop[i]));
				}
			}

#if defined(RENEWAL_DROP)
			int penalty = pc_level_penalty_mod( mvp_sd, PENALTY_MVP_DROP, nullptr, md );
#endif

			for(i = 0; i < MAX_MVP_DROP_TOTAL; i++) {
				if(mdrop[i].nameid == 0)
					continue;

				std::shared_ptr<item_data> i_data = item_db.find(mdrop[i].nameid);

				if (i_data == nullptr)
					continue;

				temp = mdrop[i].rate;

#if defined(RENEWAL_DROP)
				temp = cap_value( apply_rate( temp, penalty ), 0, 10000 );
#endif

				if (temp != 10000) {
					if(temp <= 0 && !battle_config.drop_rate0item)
						temp = 1;
					if(rnd()%10000 >= temp) //if ==0, then it doesn't drop
						continue;
				}

				struct item item = {};
				item.nameid=mdrop[i].nameid;
				item.identify= itemdb_isidentified(item.nameid);
				clif_mvp_item(mvp_sd,item.nameid);
				log_mvp_nameid = item.nameid;

				//A Rare MVP Drop Global Announce by Lupus
				if(temp<=battle_config.rare_drop_announce) {
					char message[128];
					sprintf (message, msg_txt(nullptr,541), mvp_sd->status.name, md->name, i_data->ename.c_str(), temp/100.);
					//MSG: "'%s' won %s's %s (chance: %0.02f%%)"
					intif_broadcast(message,strlen(message)+1,BC_DEFAULT);
				}

				mob_setdropitem_option( item, mdrop[i] );

				if((temp = pc_additem(mvp_sd,&item,1,LOG_TYPE_PICKDROP_PLAYER)) != 0) {
					clif_additem(mvp_sd,0,0,temp);
					map_addflooritem(&item,1,mvp_sd->bl.m,mvp_sd->bl.x,mvp_sd->bl.y,mvp_sd->status.char_id,(second_sd?second_sd->status.char_id:0),(third_sd?third_sd->status.char_id:0),1,0,true,DIR_CENTER);
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

		log_mvpdrop(mvp_sd, md->mob_id, log_mvp_nameid, log_mvp_exp);
	}

	if (type&2 && !sd && md->mob_id == MOBID_EMPERIUM)
		//Emperium destroyed by script. Discard mvp character. [Skotlex]
		mvp_sd = nullptr;

	rebirth =  ( md->sc.getSCE(SC_KAIZEL) || md->sc.getSCE(SC_ULTIMATE_S) || (md->sc.getSCE(SC_REBIRTH) && !md->state.rebirth) );
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

		if (sd) {
			std::shared_ptr<s_mob_db> mission_mdb = mob_db.find(sd->mission_mobid), mob = mob_db.find(md->mob_id);

			if ((sd->mission_mobid == md->mob_id) || (mission_mdb != nullptr &&
				((battle_config.taekwon_mission_mobname == 1 && util::vector_exists(status_get_race2(&md->bl), RC2_GOBLIN) && util::vector_exists(mission_mdb->race2, RC2_GOBLIN)) ||
				(battle_config.taekwon_mission_mobname == 2 && mob->jname.compare(mission_mdb->jname) == 0))))
			{ //TK_MISSION [Skotlex]
				if (++(sd->mission_count) >= 100 && (temp = mob_get_random_id(MOBG_BRANCH_OF_DEAD_TREE, static_cast<e_random_monster_flags>(RMF_CHECK_MOB_LV|RMF_MOB_NOT_BOSS|RMF_MOB_NOT_SPAWN), sd->status.base_level)))
				{
					pc_addfame(*sd, battle_config.fame_taekwon_mission);
					sd->mission_mobid = temp;
					pc_setglobalreg(sd, add_str(TKMISSIONID_VAR), temp);
					sd->mission_count = 0;
					clif_mission_info(sd, temp, 0);
				}
				pc_setglobalreg(sd, add_str(TKMISSIONCOUNT_VAR), sd->mission_count);
			}

			if (sd->status.party_id)
				map_foreachinallrange(quest_update_objective_sub, &md->bl, AREA_SIZE, BL_PC, sd->status.party_id, md);
			else if (sd->avail_quests)
				quest_update_objective(sd, md);

			if (achievement_db.mobexists(md->mob_id)) {
				if (battle_config.achievement_mob_share > 0 && sd->status.party_id > 0)
					map_foreachinallrange(achievement_update_objective_sub, &md->bl, AREA_SIZE, BL_PC, sd->status.party_id, md->mob_id);
				else
					achievement_update_objective(sd, AG_BATTLE, 1, md->mob_id);
			}

			// The master or Mercenary can increase the kill count, if the monster level is greater or equal than half the baselevel of the master
			if (sd->md && src && (src->type == BL_PC || src->type == BL_MER) && mob->lv >= sd->status.base_level / 2)
				mercenary_kills(sd->md);
		}

		if( md->npc_event[0] && !md->state.npc_killmonster ) {
			if( sd && battle_config.mob_npc_event_type ) {
				pc_setparam(sd, SP_KILLEDGID, md->bl.id);
				pc_setparam(sd, SP_KILLEDRID, md->mob_id);
				pc_setparam(sd, SP_KILLERRID, sd->bl.id);
				npc_event(sd,md->npc_event,0);
			} else if( mvp_sd ) {
				pc_setparam(mvp_sd, SP_KILLEDGID, md->bl.id);
				pc_setparam(mvp_sd, SP_KILLEDRID, md->mob_id);
				pc_setparam(mvp_sd, SP_KILLERRID, sd?sd->bl.id:0);
				npc_event(mvp_sd,md->npc_event,0);
			} else
				npc_event_do(md->npc_event);
		} else if( mvp_sd && !md->state.npc_killmonster ) {
			pc_setparam(mvp_sd, SP_KILLEDGID, md->bl.id);
			pc_setparam(mvp_sd, SP_KILLEDRID, md->mob_id);
			npc_script_event( *mvp_sd, NPCE_KILLNPC );
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
			clif_clearunit_area( md->bl, CLR_DEAD );
			clif_clearunit_delayed(&md->bl, CLR_OUTSIGHT,tick+3000);
		} else
			/**
			 * We give the client some time to breath and this allows it to display anything it'd like with the dead corpose
			 * For example, this delay allows it to display soul drain effect
			 **/
			clif_clearunit_delayed(&md->bl, CLR_DEAD, tick+250);

	}

	if(!md->spawn){ //Tell status_damage to remove it from memory.
		struct unit_data *ud = unit_bl2ud(&md->bl);

		// If the unit is currently in a walk script, it will be removed there
		return ud->state.walk_script ? 3 : 5; // Note: Actually, it's 4. Oh well...
	}

	// MvP tomb [GreenBox]
	if (battle_config.mvp_tomb_enabled && md->spawn->state.boss && map_getmapflag(md->bl.m, MF_NOTOMB) != 1)
		mvptomb_create(md, mvp_sd ? mvp_sd->status.name : nullptr, time(nullptr));

	if( !rebirth )
		mob_setdelayspawn(md); //Set respawning.
	return 3; //Remove from map.
}

/**
 * Resurrects a mob (reset values and respawn on map)
 * @param md : mob pointer
 * @param hp : hp to resurrect it with (only used for exp calculation)
 */
void mob_revive(struct mob_data *md, unsigned int hp)
{
	t_tick tick = gettick();
	md->state.skillstate = MSS_IDLE;
	md->last_thinktime = tick;
	md->next_walktime = tick+rnd()%1000+MIN_RANDOMWALKTIME;
	md->last_linktime = tick;
	md->last_pcneartime = 0;
	//We reset the damage log and then set the already lost damage as self damage so players don't get exp for it [Playtester]
	memset(md->dmglog, 0, sizeof(md->dmglog));
	mob_log_damage(md, &md->bl, md->status.max_hp - hp);
	md->tdmg = 0;
	if (!md->bl.prev){
		if(map_addblock(&md->bl))
			return;
	}
	clif_spawn(&md->bl);
	skill_unit_move(&md->bl,tick,1);
	mobskill_use(md, tick, MSC_SPAWN);
	if (battle_config.show_mob_info&3)
		clif_name_area(&md->bl);
}

int mob_guardian_guildchange(struct mob_data *md)
{
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
				guild_castledatasave(md->guardian_data->castle->castle_id, CD_ENABLED_GUARDIAN00 + md->guardian_data->number, 0);
			unit_free(&md->bl,CLR_OUTSIGHT); //Remove guardian.
		}
		return 0;
	}

	auto g = guild_search(md->guardian_data->castle->guild_id);
	if (g == nullptr)
	{	//Properly remove guardian info from Castle data.
		ShowError("mob_guardian_guildchange: New Guild (id %d) does not exists!\n", md->guardian_data->guild_id);
		if (md->guardian_data->number >= 0 && md->guardian_data->number < MAX_GUARDIANS)
			guild_castledatasave(md->guardian_data->castle->castle_id, CD_ENABLED_GUARDIAN00 + md->guardian_data->number, 0);
		unit_free(&md->bl,CLR_OUTSIGHT);
		return 0;
	}

	md->guardian_data->guild_id = g->guild.guild_id;
	md->guardian_data->emblem_id = g->guild.emblem_id;
	md->guardian_data->guardup_lv = guild_checkskill(g->guild, GD_GUARDUP);
	memcpy(md->guardian_data->guild_name, g->guild.name, NAME_LENGTH);

	return 1;
}

/*==========================================
 * Pick a random class for the mob
 *------------------------------------------*/
int mob_random_class(int *value, size_t count)
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

/**
* Returns the SpawnInfos of the mob_db entry (mob_spawn_data[mobid])
* if mobid is not in mob_spawn_data returns empty spawn_info vector
* @param mob_id - Looking for spawns of this Monster ID
*/
const std::vector<spawn_info> mob_get_spawns(uint16 mob_id)
{
	auto mob_spawn_it = mob_spawn_data.find(mob_id);
	if ( mob_spawn_it != mob_spawn_data.end() )
		return mob_spawn_it->second;
	return std::vector<spawn_info>();
}

/**
 * Checks if a monster is spawned. Returns true if yes, false otherwise.
 * @param mob_id - Monster ID which is checked
*/
bool mob_has_spawn(uint16 mob_id)
{
	// It's enough to check if the monster is in mob_spawn_data, because
	// none or empty spawns are ignored. Thus the monster is spawned.
	return mob_spawn_data.find(mob_id) != mob_spawn_data.end();
}

/**
 * Adds a spawn info to the specific mob. (To mob_spawn_data)
 * @param mob_id - Monster ID spawned
 * @param new_spawn - spawn_info holding the map and quantity of the spawn
*/
void mob_add_spawn(uint16 mob_id, const struct spawn_info& new_spawn)
{
	unsigned short m = new_spawn.mapindex;

	if( new_spawn.qty <= 0 )
		return; //ignore empty spawns

	std::vector<spawn_info>& spawns = mob_spawn_data[mob_id];
	// Search if the map is already in spawns
	auto itSameMap = std::find_if(spawns.begin(), spawns.end(), 
		[&m] (const spawn_info &s) { return (s.mapindex == m); });
	
	if( itSameMap != spawns.end() )
		itSameMap->qty += new_spawn.qty; // add quantity, if map is found
	else
		spawns.push_back(new_spawn); // else, add the whole spawn info
	
	// sort spawns by spawn quantity
	std::sort(spawns.begin(), spawns.end(),
		[](const spawn_info & a, const spawn_info & b) -> bool
		{ return a.qty > b.qty; });
/** Note
	Spawns are sorted after every addition. This makes reloadscript slower, but
	some spawns may be added directly by loadscript or something similar.
*/
}

/*==========================================
 * Change mob base class
 *------------------------------------------*/
int mob_class_change (struct mob_data *md, int mob_id)
{
	t_tick tick = gettick();
	int i, hp_rate;

	nullpo_ret(md);

	if( md->bl.prev == nullptr )
		return 0;

	if (!mob_id || !mobdb_checkid(mob_id))
		return 0;

	//Disable class changing for some targets...
	if (md->guardian_data)
		return 0; //Guardians/Emperium

	if (util::vector_exists(status_get_race2(&md->bl), RC2_TREASURE))
		return 0; //Treasure Boxes

	if( md->special_state.ai > AI_ATTACK )
		return 0; //Marine Spheres and Floras.

	if( mob_is_clone(md->mob_id) )
		return 0; //Clones

	if( md->mob_id == mob_id )
		return 0; //Nothing to change.

	hp_rate = get_percentage(md->status.hp, md->status.max_hp);
	md->mob_id = mob_id;
	md->db = mob_db.find(mob_id);
	if (battle_config.override_mob_names==1)
		memcpy(md->name,md->db->name.c_str(),NAME_LENGTH);
	else
		memcpy(md->name,md->db->jname.c_str(),NAME_LENGTH);

	status_change_end(&md->bl,SC_KEEPING); // End before calling status_calc_mob().
	status_change_end(&md->bl,SC_BARRIER);
	mob_stop_attack(md);
	mob_stop_walking(md, 0);
	unit_skillcastcancel(&md->bl, 0);
	status_set_viewdata(&md->bl, mob_id);
	clif_mob_class_change(md,md->vd->class_);
	status_calc_mob(md,SCO_FIRST);

	if (battle_config.monster_class_change_recover) {
		memset(md->dmglog, 0, sizeof(md->dmglog));
		md->tdmg = 0;
	} else {
		md->status.hp = md->status.max_hp*hp_rate/100;
		if(md->status.hp < 1) md->status.hp = 1;
	}

	t_tick c = tick - MOB_MAX_DELAY;

	for(i=0;i<MAX_MOBSKILL;i++)
		md->skilldelay[i] = c;

	if (md->lootitems == nullptr && status_has_mode(&md->db->status,MD_LOOTER))
		md->lootitems = (struct s_mob_lootitem *)aCalloc(LOOTITEM_SIZE,sizeof(struct s_mob_lootitem));

	//Targets should be cleared no morph
	md->target_id = md->attacked_id = md->norm_attacked_id = 0;

	//Need to update name display.
	clif_name_area(&md->bl);
	return 0;
}

/*==========================================
 * mob heal, update display hp info of mob for players
 *------------------------------------------*/
void mob_heal(struct mob_data *md,unsigned int heal)
{
	if (battle_config.show_mob_info&3)
		clif_name_area(&md->bl);
#if PACKETVER >= 20120404
	if (battle_config.monster_hp_bars_info && !map_getmapflag(md->bl.m, MF_HIDEMOBHPBAR)) {
		int i;
		if (md->special_state.ai == AI_ABR || md->special_state.ai == AI_BIONIC) {
			clif_summon_hp_bar(*md);
		}
		for(i = 0; i < DAMAGELOG_SIZE; i++)// must show hp bar to all char who already hit the mob.
			if( md->dmglog[i].id ) {
				map_session_data *sd = map_charid2sd(md->dmglog[i].id);
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

/**
 * Remove slaves if master logs off.
 * @param bl: Mob data
 * @param ap: List of arguments
 * @return 1 on removal, otherwise 0
 */
int mob_removeslaves_sub(block_list *bl, va_list ap) {
	int id = va_arg(ap, int);
	mob_data *md = (mob_data *)bl;

	if (md != nullptr && md->master_id == id) {
		unit_free(bl, CLR_OUTSIGHT);
		return 1;
	}

	return 0;
}

/**
 * Remove slaves on a map.
 * @param bl: Player data
 * @return 1 on removal, otherwise 0
 */
int mob_removeslaves(block_list *bl) {
	return map_foreachinmap(mob_removeslaves_sub, bl->m, BL_MOB, bl->id);
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

		if (skill_id != NPC_DEATHSUMMON && map_search_freecell(&md2->bl, 0, &x, &y, MOB_SLAVEDISTANCE, MOB_SLAVEDISTANCE, 0)) {
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

		if (skill_id == NPC_SUMMONSLAVE) // Only appies to NPC_SUMMONSLAVE
			status_calc_slave_mode(md, md2); // Inherit the aggressive mode of the master.

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
	std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);

	if (mob == nullptr)
		return -1;

	std::vector<std::shared_ptr<s_mob_skill>> &skills = mob->skill;

	if (skills.empty())
		return -1;

	for (int i = 0; i < skills.size(); i++) {
		if (skills[i]->skill_id == skill_id)
			return i;
	}

	return -1;
}

/*==========================================
 * Friendly Mob whose HP is decreasing by a nearby MOB is looked for.
 *------------------------------------------*/
int mob_getfriendhprate_sub(struct block_list *bl,va_list ap)
{
	int64 min_rate, max_rate,rate;
	struct block_list **fr;
	struct mob_data *md;

	md = va_arg(ap,struct mob_data *);
	min_rate=va_arg(ap,int64);
	max_rate=va_arg(ap,int64);
	fr=va_arg(ap,struct block_list **);

	if( md->bl.id == bl->id && !(battle_config.mob_ai&0x10))
		return 0;

	if ((*fr) != nullptr) //A friend was already found.
		return 0;

	if (battle_check_target(&md->bl,bl,BCT_ENEMY)>0)
		return 0;

	rate = get_percentage(status_get_hp(bl), status_get_max_hp(bl));

	if (rate >= min_rate && rate <= max_rate)
		(*fr) = bl;
	return 1;
}
static struct block_list *mob_getfriendhprate(struct mob_data *md,int64 min_rate,int64 max_rate)
{
	struct block_list *fr=nullptr;
	int type = BL_MOB;

	nullpo_retr(nullptr, md);

	if (md->special_state.ai) //Summoned creatures. [Skotlex]
		type = BL_PC;

	map_foreachinallrange(mob_getfriendhprate_sub, &md->bl, 8, type,md,min_rate,max_rate,&fr);
	return fr;
}
/*==========================================
 * Check hp rate of its master
 *------------------------------------------*/
struct block_list *mob_getmasterhpltmaxrate(struct mob_data *md,int64 rate)
{
	if( md && md->master_id > 0 )
	{
		struct block_list *bl = map_id2bl(md->master_id);
		if( bl && get_percentage(status_get_hp(bl), status_get_max_hp(bl)) < rate )
			return bl;
	}

	return nullptr;
}

bool mob_getstatus_sub( struct mob_data& md, e_mob_skill_condition condition, sc_type type ){
	bool found = false;

	if( type == SC_NONE ){
		for( int i = SC_COMMON_MIN; i <= SC_COMMON_MAX; i++ ){
			if( md.sc.getSCE( i ) != nullptr ){
				// Once an effect was found, break out. [Skotlex]
				found = true;
				break;
			}
		}
	}else{
		found = md.sc.getSCE( type ) != nullptr;
	}

	switch( condition ){
		case MSC_MYSTATUSON:
		case MSC_FRIENDSTATUSON:
			return found;
		case MSC_MYSTATUSOFF:
		case MSC_FRIENDSTATUSOFF:
			return !found;
		default:
			return false;
	}
}

/*==========================================
 * What a status state suits by nearby MOB is looked for.
 *------------------------------------------*/
int mob_getfriendstatus_sub( struct block_list *bl, va_list ap ){
	struct mob_data *md, *mmd;

	nullpo_ret(bl);
	nullpo_ret(md=(struct mob_data *)bl);
	nullpo_ret(mmd=va_arg(ap,struct mob_data *));

	if( mmd->bl.id == bl->id && !(battle_config.mob_ai&0x10) )
		return 0;

	if (battle_check_target(&mmd->bl,bl,BCT_ENEMY)>0)
		return 0;

	int64 cond1 = va_arg( ap, int64 );
	int64 cond2 = va_arg( ap, int64 );
	struct mob_data** fr = va_arg( ap, struct mob_data** );

	if( mob_getstatus_sub( *md, static_cast<e_mob_skill_condition>( cond1 ), static_cast<sc_type>( cond2 ) ) ){
		*fr = md;
	}

	return 0;
}

struct mob_data *mob_getfriendstatus(struct mob_data *md,int64 cond1,int64 cond2)
{
	struct mob_data* fr = nullptr;
	nullpo_ret(md);

	map_foreachinallrange(mob_getfriendstatus_sub, &md->bl, 8,BL_MOB, md,cond1,cond2,&fr);
	return fr;
}

// Display message from mob_chat_db.yml
bool mob_chat_display_message(mob_data &md, uint16 msg_id) {
	std::shared_ptr<s_mob_chat> mc = mob_chat_db.find(msg_id);

	if (mc != nullptr) {
		std::string name = md.name, output;
		std::size_t unique = name.find("#");

		if (unique != std::string::npos)
			name = name.substr(0, unique); // discard extra name identifier if present [Daegaladh]
		output = name + " : " + mc->msg;

		clif_messagecolor(&md.bl, mc->color, output.c_str(), true, AREA_CHAT_WOC);
		return true;
	}
	return false;
}

/*==========================================
 * Skill use judging
 *------------------------------------------*/
int mobskill_use(struct mob_data *md, t_tick tick, int event, int64 damage)
{
	struct block_list *fbl = nullptr; //Friend bl, which can either be a BL_PC or BL_MOB depending on the situation. [Skotlex]
	struct block_list *bl;
	struct mob_data *fmd = nullptr;
	int i,j,n;
	short skill_target;

	nullpo_ret(md);

	std::vector<std::shared_ptr<s_mob_skill>> &ms = md->db->skill;

	if (!battle_config.mob_skill_rate || md->ud.skilltimer != INVALID_TIMER || ms.empty() || status_has_mode(&md->status,MD_NOCAST))
		return 0;

	// Monsters check their non-attack-state skills once per second, but we ignore this for events for now
	if (event == -1)
		md->last_skillcheck = tick;

	//Pick a starting position and loop from that.
	i = battle_config.mob_ai&0x100?rnd()%ms.size():0;
	for (n = 0; n < ms.size(); i++, n++) {
		int64 c2;
		int flag = 0;

		if (i == ms.size())
			i = 0;

		if (DIFF_TICK(tick, md->skilldelay[i]) < ms[i]->delay)
			continue;

		c2 = ms[i]->cond2;

		if (ms[i]->state != md->state.skillstate) {
			if (md->state.skillstate != MSS_DEAD && (ms[i]->state == MSS_ANY ||
				(ms[i]->state == MSS_ANYTARGET && md->target_id && md->state.skillstate != MSS_LOOT)
			)) //ANYTARGET works with any state as long as there's a target. [Skotlex]
				;
			else
				continue;
		}
		if (rnd() % 10000 > ms[i]->permillage) //Lupus (max value = 10000)
			continue;

		if (ms[i]->cond1 == event)
			flag = 1; //Trigger skill.
		else if (ms[i]->cond1 == MSC_SKILLUSED)
			flag = ((event & 0xffff) == MSC_SKILLUSED && ((event >> 16) == c2 || c2 == 0));
		else if (ms[i]->cond1 == MSC_GROUNDATTACKED && damage > 0)
			flag = ((event & 0xffff) == MSC_SKILLUSED && skill_get_inf((event >> 16))&INF_GROUND_SKILL);
		else if (ms[i]->cond1 == MSC_DAMAGEDGT && damage > 0 && !((event & 0xffff) == MSC_SKILLUSED)) //Avoid double check if skill has been used [datawulf]
			flag = (damage > c2);
		else if(event == -1){
			//Avoid entering on defined events to avoid "hyper-active skill use" due to the overflow of calls to this function in battle.
			switch (ms[i]->cond1)
			{
				case MSC_ALWAYS:
					flag = 1; break;
				case MSC_MYHPLTMAXRATE:		// HP< maxhp%
					flag = get_percentage(md->status.hp, md->status.max_hp);
					flag = (flag <= c2);
					break;
				case MSC_MYHPINRATE:
					flag = get_percentage(md->status.hp, md->status.max_hp);
					flag = (flag >= c2 && flag <= ms[i]->val[0]);
					break;
				case MSC_MYSTATUSON:		// status[num] on
				case MSC_MYSTATUSOFF:		// status[num] off
					if( !md->sc.count ){
						flag = 0;
					}else if( mob_getstatus_sub( *md, static_cast<e_mob_skill_condition>( ms[i]->cond1 ), static_cast<sc_type>( ms[i]->cond2 ) ) ){
						flag = 1;
					}else{
						flag = 0;
					}
					break;
				case MSC_FRIENDHPLTMAXRATE:	// friend HP < maxhp%
					flag = ((fbl = mob_getfriendhprate(md, 0, ms[i]->cond2)) != nullptr); break;
				case MSC_FRIENDHPINRATE	:
					flag = ((fbl = mob_getfriendhprate(md, ms[i]->cond2, ms[i]->val[0])) != nullptr); break;
				case MSC_FRIENDSTATUSON:	// friend status[num] on
				case MSC_FRIENDSTATUSOFF:	// friend status[num] off
					flag = ((fmd = mob_getfriendstatus(md, ms[i]->cond1, ms[i]->cond2)) != nullptr); break;
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
					flag = ((fbl = mob_getmasterhpltmaxrate(md, ms[i]->cond2)) != nullptr); break;
				case MSC_MASTERATTACKED:
					flag = (md->master_id > 0 && (fbl=map_id2bl(md->master_id)) && unit_counttargeted(fbl) > 0); break;
				case MSC_ALCHEMIST:
					flag = (md->state.can_escape); break;
				case MSC_MOBNEARBYGT:
					flag = (map_foreachinallrange(mob_count_sub, &md->bl, AREA_SIZE, BL_MOB) > c2 ); break;
			}
		}

		if (!flag)
			continue; //Skill requisite failed to be fulfilled.

		//Execute skill
		skill_target = status_has_mode(&md->db->status,MD_RANDOMTARGET) ? MST_RANDOM : ms[i]->target;
		if (skill_get_casttype(ms[i]->skill_id) == CAST_GROUND)
		{	//Ground skill.
			short x, y;
			switch (skill_target) {
				case MST_RANDOM: //Pick a random enemy within skill range.
					bl = battle_getenemy(&md->bl, DEFAULT_ENEMY_TYPE(md),
						skill_get_range2(&md->bl, ms[i]->skill_id, ms[i]->skill_lv, true));
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
					if (bl)
						break;
					[[fallthrough]];
				case MST_FRIEND:
					bl = fbl?fbl:(fmd?&fmd->bl:&md->bl);
					break;
				default:
					bl = &md->bl;
					break;
			}
			if (!bl) {
				if (battle_config.mob_ai & 0x1000)
					continue;
				else
					break;
			}

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
			if (!battle_check_range(&md->bl, bl, skill_get_range2(&md->bl, ms[i]->skill_id, ms[i]->skill_lv, true)) ||
				!unit_skilluse_pos2(&md->bl, x, y, ms[i]->skill_id, ms[i]->skill_lv, ms[i]->casttime, ms[i]->cancel))
			{
				map_freeblock_unlock();
				if (battle_config.mob_ai & 0x1000)
					continue;
				else
					break;
			}
		} else {
			//Targetted skill
			switch (skill_target) {
				case MST_RANDOM: //Pick a random enemy within skill range.
					bl = battle_getenemy(&md->bl, DEFAULT_ENEMY_TYPE(md),
						skill_get_range2(&md->bl, ms[i]->skill_id, ms[i]->skill_lv, true));
					break;
				case MST_TARGET:
					bl = map_id2bl(md->target_id);
					break;
				case MST_MASTER:
					bl = &md->bl;
					if (md->master_id)
						bl = map_id2bl(md->master_id);
					if (bl)
						break;
					[[fallthrough]];
				case MST_FRIEND:
					if (fbl) {
						bl = fbl;
						break;
					} else if (fmd) {
						bl = &fmd->bl;
						break;
					}
					[[fallthrough]];
				default:
					bl = &md->bl;
					break;
			}
			if (!bl) {
				if (battle_config.mob_ai & 0x1000)
					continue;
				else
					break;
			}

			md->skill_idx = i;
			map_freeblock_lock();
			if (!battle_check_range(&md->bl, bl, skill_get_range2(&md->bl, ms[i]->skill_id, ms[i]->skill_lv, true)) ||
				!unit_skilluse_id2(&md->bl, bl->id, ms[i]->skill_id, ms[i]->skill_lv, ms[i]->casttime, ms[i]->cancel))
			{
				map_freeblock_unlock();
				if (battle_config.mob_ai & 0x1000)
					continue;
				else
					break;
			}
		}
		//Skill used. Post-setups...
		if ( ms[i]->msg_id ){ //Display color message [SnakeDrak]
			mob_chat_display_message(*md, ms[i]->msg_id);
		}
		if(!(battle_config.mob_ai&0x200)) { //pass on delay to same skill.
			for (j = 0; j < ms.size(); j++)
				if (ms[j]->skill_id == ms[i]->skill_id)
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
int mobskill_event(struct mob_data *md, struct block_list *src, t_tick tick, int flag, int64 damage)
{
	int target_id, res = 0;

	if(md->bl.prev == nullptr || md->status.hp == 0)
		return 0;

	target_id = md->target_id;
	if (!target_id || battle_config.mob_changetarget_byskill)
		md->target_id = src->id;

	if (flag == -1)
		res = mobskill_use(md, tick, MSC_CASTTARGETED);
	else if ((flag&0xffff) == MSC_SKILLUSED)
		res = mobskill_use(md, tick, flag, damage);
	else if (flag&BF_SHORT)
		res = mobskill_use(md, tick, MSC_CLOSEDATTACKED, damage);
	else if (flag&BF_LONG && !(flag&BF_MAGIC)) //Long-attacked should not include magic.
		res = mobskill_use(md, tick, MSC_LONGRANGEATTACKED, damage);
	else if (damage > 0) //Trigger for any damage dealt from other attack types without affecting other triggers [datawulf]
		res = mobskill_use(md, tick, -2, damage);

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
	if (!mob_db.exists(mob_id))
		return 0;
	return mob_id;
}

/**
 * Previously, using skill_nocast with flag 16
 * @param skill_id
 * @return True:If disabled, False:If enabled
 * @!TODO : Move this hardcodes!
 **/
static bool mob_clone_disabled_skills(uint16 skill_id) {
	switch (skill_id) {
		case PR_TURNUNDEAD:
		case PR_MAGNUS:
			return true;
	}
	return false;
}

//Flag values:
//&1: Set special ai (fight mobs, not players)
//If mode is not passed, a default aggressive mode is used.
//If master_id is passed, clone is attached to him.
//Returns: ID of newly crafted copy.
int mob_clone_spawn(map_session_data *sd, int16 m, int16 x, int16 y, const char *event, int master_id, enum e_mode mode, int flag, unsigned int duration)
{
	int mob_id;
	int inf, fd;
	struct mob_data *md;
	struct status_data *status;

	nullpo_ret(sd);

	if(pc_isdead(sd) && master_id && flag&1)
		return 0;

	ARR_FIND( MOB_CLONE_START, MOB_CLONE_END, mob_id, !mob_db.exists(mob_id) );
	if(mob_id >= MOB_CLONE_END)
		return 0;

	std::shared_ptr<s_mob_db> db = std::make_shared<s_mob_db>();

	mob_db.put( mob_id, db );

	status = &db->status;
	db->sprite = sd->status.name;
	db->name = sd->status.name;
	db->jname = sd->status.name;
	db->lv=status_get_lv(&sd->bl);
	memcpy(status, &sd->base_status, sizeof(struct status_data));
	status->rhw.atk2= status->dex + status->rhw.atk + status->rhw.atk2; //Max ATK
	status->rhw.atk = status->dex; //Min ATK
	if (status->lhw.atk) {
		status->lhw.atk2= status->dex + status->lhw.atk + status->lhw.atk2; //Max ATK
		status->lhw.atk = status->dex; //Min ATK
	}
	if (mode > MD_NONE) //User provided mode.
		status->mode = mode;
	else if (flag&1) //Friendly Character, remove looting.
		status->mode = static_cast<enum e_mode>(status->mode&(~MD_LOOTER));
	status->hp = status->max_hp;
	status->sp = status->max_sp;
	memcpy(&db->vd, &sd->vd, sizeof(struct view_data));
	db->base_exp=1;
	db->job_exp=1;
	db->range2=AREA_SIZE; //Let them have the same view-range as players.
	db->range3=AREA_SIZE; //Min chase of a screen.
	db->option=sd->sc.option;

	/**
	 * We temporarily disable sd's fd so it doesn't receive the messages from skill_check_condition_castbegin
	 **/
	fd = sd->fd;
	sd->fd = 0;

	//Go Backwards to give better priority to advanced skills.
	std::shared_ptr<s_skill_tree> tree = skill_tree_db.find(sd->status.class_);

	if( tree != nullptr && !tree->skills.empty() ){
		std::vector<uint16> skill_list;

		for (const auto &it : tree->skills)
			skill_list.push_back(it.first);
		std::sort(skill_list.rbegin(), skill_list.rend());

		for (const auto &it : skill_list) {
			if (db->skill.size() >= MAX_MOBSKILL)
				break;
			uint16 skill_id = it;
			uint16 sk_idx = 0;

			if (!skill_id || !(sk_idx = skill_get_index(skill_id)) || sd->status.skill[sk_idx].lv < 1 ||
				skill_get_inf2_(skill_id, { INF2_ISWEDDING, INF2_ISGUILD }) ||
				mob_clone_disabled_skills(skill_id)
			)
				continue;
			//Normal aggressive mob, disable skills that cannot help them fight
			//against players (those with flags UF_NOMOB and UF_NOPC are specific
			//to always aid players!) [Skotlex]
			if (!(flag&1) &&
				skill_get_unit_id(skill_id) &&
				skill_get_unit_flag_(skill_id, { UF_NOMOB, UF_NOPC }))
				continue;
			/**
			 * The clone should be able to cast the skill (e.g. have the required weapon) bugreport:5299)
			 **/
			if( !skill_check_condition_castbegin(*sd,skill_id,sd->status.skill[sk_idx].lv) )
				continue;

			std::shared_ptr<s_mob_skill> ms = std::make_shared<s_mob_skill>();

			ms->skill_id = skill_id;
			ms->skill_lv = sd->status.skill[sk_idx].lv;
			ms->state = MSS_ANY;
			ms->permillage = 500*battle_config.mob_skill_rate/100; //Default chance of all skills: 5%
			ms->emotion = -1;
			ms->cancel = 0;
			ms->casttime = skill_castfix(&sd->bl,skill_id, ms->skill_lv);
			ms->delay = 5000+skill_delayfix(&sd->bl,skill_id, ms->skill_lv);
			ms->msg_id = 0;

			inf = skill_get_inf(skill_id);
			if (inf&INF_ATTACK_SKILL) {
				ms->target = MST_TARGET;
				ms->cond1 = MSC_ALWAYS;
				if (skill_get_range(skill_id, ms->skill_lv)  > 3)
					ms->state = MSS_ANYTARGET;
				else
					ms->state = MSS_BERSERK;
			} else if(inf&INF_GROUND_SKILL) {
				if (skill_get_inf2(skill_id, INF2_ISTRAP)) { //Traps!
					ms->state = MSS_IDLE;
					ms->target = MST_AROUND2;
					ms->delay = 60000;
				} else if (skill_get_unit_target(skill_id) == BCT_ENEMY) { //Target Enemy
					ms->state = MSS_ANYTARGET;
					ms->target = MST_TARGET;
					ms->cond1 = MSC_ALWAYS;
				} else { //Target allies
					ms->target = MST_FRIEND;
					ms->cond1 = MSC_FRIENDHPLTMAXRATE;
					ms->cond2 = 95;
				}
			} else if (inf&INF_SELF_SKILL) {
				if (skill_get_inf2(skill_id, INF2_NOTARGETSELF)) { //auto-select target skill.
					ms->target = MST_TARGET;
					ms->cond1 = MSC_ALWAYS;
					if (skill_get_range(skill_id, ms->skill_lv)  > 3) {
						ms->state = MSS_ANYTARGET;
					} else {
						ms->state = MSS_BERSERK;
					}
				} else { //Self skill
					ms->target = MST_SELF;
					ms->cond1 = MSC_MYHPLTMAXRATE;
					ms->cond2 = 90;
					ms->permillage = 2000;
					//Delay: Remove the stock 5 secs and add half of the support time.
					ms->delay += -5000 +(skill_get_time(skill_id, ms->skill_lv) + skill_get_time2(skill_id, ms->skill_lv))/2;
					if (ms->delay < 5000)
						ms->delay = 5000; //With a minimum of 5 secs.
				}
			} else if (inf&INF_SUPPORT_SKILL) {
				ms->target = MST_FRIEND;
				ms->cond1 = MSC_FRIENDHPLTMAXRATE;
				ms->cond2 = 90;
				if (skill_id == AL_HEAL)
					ms->permillage = 5000; //Higher skill rate usage for heal.
				else if (skill_id == ALL_RESURRECTION)
					ms->cond2 = 1;
				//Delay: Remove the stock 5 secs and add half of the support time.
				ms->delay += -5000 +(skill_get_time(skill_id, ms->skill_lv) + skill_get_time2(skill_id, ms->skill_lv))/2;
				if (ms->delay < 2000)
					ms->delay = 2000; //With a minimum of 2 secs.

				if (db->skill.size() < MAX_MOBSKILL) { //duplicate this so it also triggers on self.
					ms->target = MST_SELF;
					ms->cond1 = MSC_MYHPLTMAXRATE;
					db->skill.push_back(ms);
				}
			} else {
				switch (skill_id) { //Certain Special skills that are passive, and thus, never triggered.
					case MO_TRIPLEATTACK:
					case TF_DOUBLE:
					case GS_CHAINACTION:
						ms->state = MSS_BERSERK;
						ms->target = MST_TARGET;
						ms->cond1 = MSC_ALWAYS;
						ms->permillage = skill_id==MO_TRIPLEATTACK?(3000-ms->skill_lv*100):(ms->skill_lv*500);
						ms->delay -= 5000; //Remove the added delay as these could trigger on "all hits".
						break;
					default: //Untreated Skill
						continue;
				}
			}
			if (battle_config.mob_skill_rate!= 100)
				ms->permillage = ms->permillage*battle_config.mob_skill_rate/100;
			if (battle_config.mob_skill_delay != 100)
				ms->delay = ms->delay*battle_config.mob_skill_delay/100;

			db->skill.push_back(ms);
		}
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

int mob_clone_delete(struct mob_data *md){
	uint32 mob_id = md->mob_id;

	if (mob_is_clone(mob_id)) {
		mob_db.erase(mob_id);
		//Clear references to the db
		md->db = nullptr;
		md->vd = nullptr;
		return 1;
	}
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
 * in db/mob_item_ratio.yml
 * @param nameid ID of the item
 * @param mob_id ID of the monster
 * @param rate_adjust pointer to store ratio if found
 */
static void item_dropratio_adjust(t_itemid nameid, int mob_id, int *rate_adjust)
{
	std::shared_ptr<s_mob_item_drop_ratio> item_ratio = mob_item_drop_ratio.find(nameid);
	if( item_ratio) {
		// If it is empty it is applied to all monsters, if not it is only applied if the monster is in the vector
		if( item_ratio->mob_ids.empty() || util::vector_exists( item_ratio->mob_ids, static_cast<uint16>( mob_id ) ) )
			*rate_adjust = item_ratio->drop_ratio;
	}
}

const std::string MobDatabase::getDefaultLocation() {
	return std::string(db_path) + "/mob_db.yml";
}

bool MobDatabase::parseDropNode(std::string nodeName, const ryml::NodeRef& node, uint8 max, s_mob_drop *drops) {
	const auto& dropNode = node[c4::to_csubstr(nodeName)];
	uint16 i;

	// Find first empty spot
	for( i = 0; i < max; i++ ){
		if( drops[i].nameid == 0 ){
			break;
		}
	}

	for (const auto& dropit : dropNode) {
		uint16 index;

		if (this->nodeExists(dropit, "Index")) {
			if (!this->asUInt16(dropit, "Index", index))
				return false;

			if (index >= max) {
				this->invalidWarning(dropit["Index"], "Invalid monster %s index %hu. Must be between 0~%hu, skipping.\n", nodeName.c_str(), index, max - 1);
				continue;
			}
		} else {
			index = i++;

			if (index >= max) {
				this->invalidWarning(dropit, "Maximum of %d monster %s met, skipping.\n", max, nodeName.c_str());
				continue;
			}
		}

		std::string item_name;

		if (!this->asString(dropit, "Item", item_name))
			return false;

		std::shared_ptr<item_data> item = item_db.search_aegisname( item_name.c_str() );

		if (item == nullptr) {
			this->invalidWarning(dropit["Item"], "Monster %s item %s does not exist, skipping.\n", nodeName.c_str(), item_name.c_str());
			continue;
		}

		uint16 rate;

		if (!this->asUInt16Rate(dropit, "Rate", rate))
			return false;

		bool steal = false;

		if (this->nodeExists(dropit, "StealProtected")) {
			if (!this->asBool(dropit, "StealProtected", steal))
				return false;
		}

		uint16 group = 0;

		if (this->nodeExists(dropit, "RandomOptionGroup")) {
			std::string group_name;

			if (!this->asString(dropit, "RandomOptionGroup", group_name))
				return false;

			if (!random_option_group.option_get_id(group_name.c_str(), group))
				this->invalidWarning(dropit["RandomOptionGroup"], "Unknown random option group %s for monster %s, defaulting to no group.\n", group_name.c_str(), nodeName.c_str());
		}

		drops[index].nameid = item->nameid;
		drops[index].rate = rate;
		drops[index].steal_protected = steal;
		drops[index].randomopt_group = group;
	}

	return true;
}

/**
 * Mob DB constructor
 */
s_mob_db::s_mob_db()
{
	status.max_hp = 1;
	status.max_sp = 1;
	status.str = 1;
	status.agi = 1;
	status.vit = 1;
	status.int_ = 1;
	status.dex = 1;
	status.luk = 1;
	status.ele_lv = 1;
	status.speed = DEFAULT_WALK_SPEED;
	status.adelay = cap_value(0, battle_config.monster_max_aspd * 2, 4000);
	status.amotion = cap_value(0, battle_config.monster_max_aspd, 2000);
	status.clientamotion = cap_value(status.amotion, 1, USHRT_MAX);
	status.mode = static_cast<e_mode>(MONSTER_TYPE_06);

	vd.class_ = id;
}

/**
 * Reads and parses an entry from the mob_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 MobDatabase::parseBodyNode(const ryml::NodeRef& node) {
	uint32 mob_id;

	if (!this->asUInt32(node, "Id", mob_id))
		return 0;

	if (!((mob_id > MIN_MOB_DB && mob_id < MAX_MOB_DB) || (mob_id > MIN_MOB_DB2 && mob_id < MAX_MOB_DB2))) {
		this->invalidWarning(node["Id"], "Invalid monster ID %d, must be in range %d-%d or %d-%d.\n", mob_id, MIN_MOB_DB, MAX_MOB_DB, MIN_MOB_DB2, MAX_MOB_DB2);
		return false;
	}

	std::shared_ptr<s_mob_db> mob = this->find(mob_id);
	bool exists = mob != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "AegisName", "Name" }))
			return 0;

		mob = std::make_shared<s_mob_db>();
		mob->id = mob_id;
		mob->vd.class_ = static_cast<uint16>(mob->id);
	}

	if (this->nodeExists(node, "AegisName")) {
		std::string name;

		if (!this->asString(node, "AegisName", name))
			return 0;

		if (name.size() > NAME_LENGTH) {
			this->invalidWarning(node["AegisName"], "AegisName \"%s\" exceeds maximum of %d characters, capping...\n", name.c_str(), NAME_LENGTH - 1);
		}

		name.resize(NAME_LENGTH);
		mob->sprite = name;
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		if (name.size() > NAME_LENGTH) {
			this->invalidWarning(node["Name"], "Name \"%s\" exceeds maximum of %d characters, capping...\n", name.c_str(), NAME_LENGTH - 1);
		}

		name.resize(NAME_LENGTH);
		mob->name = name;
	}

	if (this->nodeExists(node, "JapaneseName")) {
		std::string name;

		if (!this->asString(node, "JapaneseName", name))
			return 0;

		if (name.size() > NAME_LENGTH) {
			this->invalidWarning(node["JapaneseName"], "JapaneseName \"%s\" exceeds maximum of %d characters, capping...\n", name.c_str(), NAME_LENGTH - 1);
		}

		name.resize(NAME_LENGTH);
		mob->jname = name;
	} else if (!exists) {
		mob->jname = mob->name;
	}

	if (this->nodeExists(node, "Level")) {
		uint16 level;

		if (!this->asUInt16(node, "Level", level))
			return 0;

		mob->lv = level;
	}

	if (this->nodeExists(node, "Hp")) {
		uint32 hp;

		if (!this->asUInt32(node, "Hp", hp))
			return 0;

		mob->status.max_hp = hp;
	}
	
	if (this->nodeExists(node, "Sp")) {
		uint32 sp;

		if (!this->asUInt32(node, "Sp", sp))
			return 0;

		mob->status.max_sp = sp;
	}
	
	if (this->nodeExists(node, "BaseExp")) {
		t_exp exp;

		if (!this->asUInt64(node, "BaseExp", exp))
			return 0;

		mob->base_exp = static_cast<t_exp>(cap_value((double)exp * (double)battle_config.base_exp_rate / 100., 0, MAX_EXP));
	}
	
	if (this->nodeExists(node, "JobExp")) {
		t_exp exp;

		if (!this->asUInt64(node, "JobExp", exp))
			return 0;

		mob->job_exp = static_cast<t_exp>(cap_value((double)exp * (double)battle_config.job_exp_rate / 100., 0, MAX_EXP));
	}
	
	if (this->nodeExists(node, "MvpExp")) {
		t_exp exp;

		if (!this->asUInt64(node, "MvpExp", exp))
			return 0;

		mob->mexp = static_cast<t_exp>(cap_value((double)exp * (double)battle_config.mvp_exp_rate / 100., 0, MAX_EXP));
	}

	if (this->nodeExists(node, "Attack")) {
		uint16 atk;

		if (!this->asUInt16(node, "Attack", atk))
			return 0;

		mob->status.rhw.atk = atk;
	}
	
	if (this->nodeExists(node, "Attack2")) {
		uint16 atk;

		if (!this->asUInt16(node, "Attack2", atk))
			return 0;

#ifdef RENEWAL
		mob->status.rhw.matk = atk;
#else
		mob->status.rhw.atk2 = atk;
#endif
	}

	if (this->nodeExists(node, "Defense")) {
		uint16 def;

		if (!this->asUInt16(node, "Defense", def))
			return 0;

		if (def < DEFTYPE_MIN || def > DEFTYPE_MAX) {
			this->invalidWarning(node["Defense"], "Invalid monster defense %d, capping...\n", def);
			def = cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX);
		}

		mob->status.def = static_cast<defType>(def);
	}

	if (this->nodeExists(node, "MagicDefense")) {
		uint16 def;

		if (!this->asUInt16(node, "MagicDefense", def))
			return 0;

		if (def < DEFTYPE_MIN || def > DEFTYPE_MAX) {
			this->invalidWarning(node["MagicDefense"], "Invalid monster magic defense %d, capping...\n", def);
			def = cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX);
		}

		mob->status.mdef = static_cast<defType>(def);
	}

	if (this->nodeExists(node, "Resistance")) {
		uint16 res;

		if (!this->asUInt16(node, "Resistance", res))
			return 0;

		mob->status.res = res;
	}

	if (this->nodeExists(node, "MagicResistance")) {
		uint16 mres;

		if (!this->asUInt16(node, "MagicResistance", mres))
			return 0;

		mob->status.mres = mres;
	}

	if (this->nodeExists(node, "Str")) {
		uint16 stat;

		if (!this->asUInt16(node, "Str", stat))
			return 0;

		mob->status.str = max(1, stat);
	}

	if (this->nodeExists(node, "Agi")) {
		uint16 stat;

		if (!this->asUInt16(node, "Agi", stat))
			return 0;

		mob->status.agi = max(1, stat);
	}

	if (this->nodeExists(node, "Vit")) {
		uint16 stat;

		if (!this->asUInt16(node, "Vit", stat))
			return 0;

		mob->status.vit = max(1, stat);
	}

	if (this->nodeExists(node, "Int")) {
		uint16 stat;

		if (!this->asUInt16(node, "Int", stat))
			return 0;

		mob->status.int_ = max(1, stat);
	}

	if (this->nodeExists(node, "Dex")) {
		uint16 stat;

		if (!this->asUInt16(node, "Dex", stat))
			return 0;

		mob->status.dex = max(1, stat);
	}

	if (this->nodeExists(node, "Luk")) {
		uint16 stat;

		if (!this->asUInt16(node, "Luk", stat))
			return 0;

		mob->status.luk = max(1, stat);
	}

	if (this->nodeExists(node, "AttackRange")) {
		uint16 range;

		if (!this->asUInt16(node, "AttackRange", range))
			return 0;

		mob->status.rhw.range = range;
	}
	
	if (this->nodeExists(node, "SkillRange")) {
		uint16 range;

		if (!this->asUInt16(node, "SkillRange", range))
			return 0;

		mob->range2 = range;
	}
	
	if (this->nodeExists(node, "ChaseRange")) {
		uint16 range;

		if (!this->asUInt16(node, "ChaseRange", range))
			return 0;

		mob->range3 = range;
	}
	
	if (this->nodeExists(node, "Size")) {
		std::string size;

		if (!this->asString(node, "Size", size))
			return 0;

		std::string size_constant = "Size_" + size;
		int64 constant;

		if (!script_get_constant(size_constant.c_str(), &constant)) {
			this->invalidWarning(node["Size"], "Unknown monster size %s, defaulting to Small.\n", size.c_str());
			constant = SZ_SMALL;
		}

		if (constant < SZ_SMALL || constant > SZ_BIG) {
			this->invalidWarning(node["Size"], "Invalid monster size %s, defaulting to Small.\n", size.c_str());
			constant = SZ_SMALL;
		}

		mob->status.size = static_cast<e_size>(constant);
	}
	
	if (this->nodeExists(node, "Race")) {
		std::string race;

		if (!this->asString(node, "Race", race))
			return 0;

		std::string race_constant = "RC_" + race;
		int64 constant;

		if (!script_get_constant(race_constant.c_str(), &constant)) {
			this->invalidWarning(node["Race"], "Unknown monster race %s, defaulting to Formless.\n", race.c_str());
			constant = RC_FORMLESS;
		}

		if (!CHK_RACE(constant)) {
			this->invalidWarning(node["Race"], "Invalid monster race %s, defaulting to Formless.\n", race.c_str());
			constant = RC_FORMLESS;
		}

		mob->status.race = static_cast<e_race>(constant);
	}

	if (this->nodeExists(node, "RaceGroups")) {
		const auto& raceNode = node["RaceGroups"];

		for (const auto &raceit : raceNode) {
			std::string raceName;
			c4::from_chars(raceit.key(), &raceName);
			std::string raceName_constant = "RC2_" + raceName;
			int64 constant;

			if (!script_get_constant(raceName_constant.c_str(), &constant)) {
				this->invalidWarning(raceNode[raceit.key()], "Unknown monster race group %s, skipping.\n", raceName.c_str());
				continue;
			}

			if (!CHK_RACE2(constant)) {
				this->invalidWarning(raceNode[raceit.key()], "Invalid monster race group %s, skipping.\n", raceName.c_str());
				continue;
			}

			bool active;

			if (!this->asBool(raceNode, raceName, active))
				return 0;

			if (active)
				mob->race2.push_back(static_cast<e_race2>(constant));
			else
				util::vector_erase_if_exists(mob->race2, static_cast<e_race2>(constant));
		}
	}

	if (this->nodeExists(node, "Element")) {
		std::string ele;

		if (!this->asString(node, "Element", ele))
			return 0;

		std::string ele_constant = "ELE_" + ele;
		int64 constant;

		if (!script_get_constant(ele_constant.c_str(), &constant)) {
			this->invalidWarning(node["Element"], "Unknown monster element %s, defaulting to Neutral.\n", ele.c_str());
			constant = ELE_NEUTRAL;
		}

		if (!CHK_ELEMENT(constant)) {
			this->invalidWarning(node["Element"], "Invalid monster element %s, defaulting to Neutral.\n", ele.c_str());
			constant = ELE_NEUTRAL;
		}

		mob->status.def_ele = static_cast<e_element>(constant);
	}

	if (this->nodeExists(node, "ElementLevel")) {
		uint16 level;

		if (!this->asUInt16(node, "ElementLevel", level))
			return 0;

		if (!CHK_ELEMENT_LEVEL(level)) {
			this->invalidWarning(node["ElementLevel"], "Invalid monster element level %hu, defaulting to 1.\n", level);
			level = 1;
		}

		mob->status.ele_lv = static_cast<uint8>(level);
	}

	if (this->nodeExists(node, "WalkSpeed")) {
		uint16 speed;

		if (!this->asUInt16(node, "WalkSpeed", speed))
			return 0;

		if (speed < MIN_WALK_SPEED || speed > MAX_WALK_SPEED) {
			this->invalidWarning(node["WalkSpeed"], "Invalid monster walk speed %hu, capping...\n", speed);
			speed = cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);
		}

		mob->status.speed = speed;
	}

	if (this->nodeExists(node, "AttackDelay")) {
		uint16 speed;

		if (!this->asUInt16(node, "AttackDelay", speed))
			return 0;

		mob->status.adelay = cap_value(speed, battle_config.monster_max_aspd * 2, 4000);
	}
	
	if (this->nodeExists(node, "AttackMotion")) {
		uint16 speed;

		if (!this->asUInt16(node, "AttackMotion", speed))
			return 0;

		mob->status.amotion = cap_value(speed, battle_config.monster_max_aspd, 2000);
	}

	if (this->nodeExists(node, "ClientAttackMotion")) {
		uint16 speed;

		if (!this->asUInt16(node, "ClientAttackMotion", speed))
			return 0;

		mob->status.clientamotion = cap_value(speed, 1, USHRT_MAX);
	} else {
		if (!exists)
			mob->status.clientamotion = cap_value(mob->status.amotion, 1, USHRT_MAX);
	}

	if (this->nodeExists(node, "DamageMotion")) {
		uint16 speed;

		if (!this->asUInt16(node, "DamageMotion", speed))
			return 0;

		if (battle_config.monster_damage_delay_rate != 100)
			speed = speed * battle_config.monster_damage_delay_rate / 100;

		mob->status.dmotion = speed;
	}
	
	if (this->nodeExists(node, "DamageTaken")) {
		uint16 damage;

		if (!this->asUInt16Rate(node, "DamageTaken", damage, 100))
			return 0;

		mob->damagetaken = damage;
	}

	if (this->nodeExists(node, "Ai")) {
		std::string ai;

		if (!this->asString(node, "Ai", ai))
			return 0;

		std::string ai_constant = "MONSTER_TYPE_" + ai;
		int64 constant;

		if (!script_get_constant(ai_constant.c_str(), &constant)) {
			this->invalidWarning(node["Ai"], "Unknown monster AI %s, defaulting to 06.\n", ai.c_str());
			constant = MONSTER_TYPE_06;
		}

		if (constant < MD_NONE || constant > MD_MASK) {
			this->invalidWarning(node["Ai"], "Invalid monster AI %s, defaulting to 06.\n", ai.c_str());
			constant = MONSTER_TYPE_06;
		}

		mob->status.mode = static_cast<e_mode>(constant);
	}

	if (this->nodeExists(node, "Class")) {
		std::string class_;

		if (!this->asString(node, "Class", class_))
			return 0;

		std::string class_constant = "CLASS_" + class_;
		int64 constant;

		if (!script_get_constant(class_constant.c_str(), &constant)) {
			this->invalidWarning(node["Class"], "Unknown monster class %s, defaulting to Normal.\n", class_.c_str());
			constant = CLASS_NORMAL;
		}

		if (constant < CLASS_NORMAL || constant > CLASS_EVENT) {
			this->invalidWarning(node["Class"], "Invalid monster class %s, defaulting to Normal.\n", class_.c_str());
			constant = CLASS_NORMAL;
		}

		mob->status.class_ = static_cast<uint8>(constant);
	}

	if (this->nodeExists(node, "Modes")) {
		const auto& modeNode = node["Modes"];

		for (const auto& modeit : modeNode) {
			std::string modeName;
			c4::from_chars(modeit.key(), &modeName);
			std::string modeName_constant = "MD_" + modeName;
			int64 constant;

			if (!script_get_constant(modeName_constant.c_str(), &constant)) {
				this->invalidWarning(modeNode[modeit.key()], "Unknown monster mode %s, skipping.\n", modeName.c_str());
				continue;
			}

			if (constant < MD_NONE || constant > MD_SKILLIMMUNE) {
				this->invalidWarning(modeNode[modeit.key()], "Invalid monster mode %s, skipping.\n", modeName.c_str());
				continue;
			}

			bool active;

			if (!this->asBool(modeNode, modeName, active))
				return 0;

			if (active)
				mob->status.mode = static_cast<e_mode>(mob->status.mode | constant);
			else
				mob->status.mode = static_cast<e_mode>(mob->status.mode & ~constant);
		}
	}

	if (this->nodeExists(node, "MvpDrops")) {
		if (!this->parseDropNode("MvpDrops", node, MAX_MVP_DROP, mob->mvpitem))
			return 0;
	}

	if (this->nodeExists(node, "Drops")) {
		if (!this->parseDropNode("Drops", node, MAX_MOB_DROP, mob->dropitem))
			return 0;
	}

	if (!exists)
		this->put(mob_id, mob);

	return true;
}

void MobDatabase::loadingFinished() {
	for (auto &mobdata : *this) {
		std::shared_ptr<s_mob_db> mob = mobdata.second;

		switch (mob->status.class_) {
			case CLASS_BOSS:
				mob->status.mode = static_cast<e_mode>(mob->status.mode | (MD_DETECTOR | MD_STATUSIMMUNE | MD_KNOCKBACKIMMUNE));
				break;
			case CLASS_GUARDIAN:
				mob->status.mode = static_cast<e_mode>(mob->status.mode | MD_STATUSIMMUNE);
				break;
			case CLASS_BATTLEFIELD:
				mob->status.mode = static_cast<e_mode>(mob->status.mode | (MD_STATUSIMMUNE | MD_SKILLIMMUNE));
				break;
			case CLASS_EVENT:
				mob->status.mode = static_cast<e_mode>(mob->status.mode | MD_FIXEDITEMDROP);
				break;
		}

		if (battle_config.view_range_rate != 100)
			mob->range2 = max(1, mob->range2 * battle_config.view_range_rate / 100);

		if (battle_config.chase_range_rate != 100)
			mob->range3 = max(mob->range2, mob->range3 * battle_config.chase_range_rate / 100);

		// If the attack animation is longer than the delay, the client crops the attack animation!
		// On aegis there is no real visible effect of having a recharge-time less than amotion anyway.
		mob->status.adelay = max(mob->status.adelay, mob->status.amotion);
		mob->status.aspd_rate = 1000;

		if (!battle_config.monster_active_enable)
			mob->status.mode = static_cast<enum e_mode>(mob->status.mode & ~MD_AGGRESSIVE);

		// Fill in remaining status data by using a dummy monster.
		mob_data data;

		data.bl.type = BL_MOB;
		data.level = mob->lv;
		memcpy(&data.status, &mob->status, sizeof(status_data));
		status_calc_misc(&data.bl, &mob->status, mob->lv);

		// Now that we know if it is a MVP or not, apply battle_config modifiers [Skotlex]
		double maxhp = (double)mob->status.max_hp;

		if (mob->get_bosstype() == BOSSTYPE_MVP) { // MVP
			if (battle_config.mvp_hp_rate != 100)
				maxhp = maxhp * (double)battle_config.mvp_hp_rate / 100.;
		} else { // Normal mob
			if (battle_config.monster_hp_rate != 100)
				maxhp = maxhp * (double)battle_config.monster_hp_rate / 100.;
		}

		mob->status.max_hp = cap_value(static_cast<uint32>(maxhp), 1, UINT32_MAX);
		mob->status.max_sp = cap_value(mob->status.max_sp, 1, UINT32_MAX);
		mob->status.hp = mob->status.max_hp;
		mob->status.sp = mob->status.max_sp;
	}

	TypesafeCachedYamlDatabase::loadingFinished();
}

MobDatabase mob_db;

/**
 * Convert SQL data to YAML Node
 * @param str: Array of parsed SQL data
 * @return True on success or false otherwise
 */
static bool mob_read_sqldb_sub(std::vector<std::string> str) {
	ryml::Tree tree;
	ryml::NodeRef node = tree.rootref();
	node |= ryml::MAP;
	int32 index = -1;

	node["Id"] << str[++index];
	if (!str[++index].empty())
		node["AegisName"] << str[index];
	if (!str[++index].empty())
		node["Name"] << str[index];
	if (!str[++index].empty())
		node["JapaneseName"] << str[index];
	if (!str[++index].empty() && std::stoi(str[index]) > 1)
		node["Level"] << str[index];
	if (!str[++index].empty() && std::stoul(str[index]) > 1)
		node["Hp"] << str[index];
	if (!str[++index].empty() && std::stoul(str[index]) > 1)
		node["Sp"] << str[index];
	if (!str[++index].empty())
		node["BaseExp"] << str[index];
	if (!str[++index].empty())
		node["JobExp"] << str[index];
	if (!str[++index].empty())
		node["MvpExp"] << str[index];
	if (!str[++index].empty())
		node["Attack"] << str[index];
	if (!str[++index].empty())
		node["Attack2"] << str[index];
	if (!str[++index].empty())
		node["Defense"] << str[index];
	if (!str[++index].empty())
		node["MagicDefense"] << str[index];
	if (!str[++index].empty() && std::stoi(str[index]) > 1)
		node["Str"] << str[index];
	if (!str[++index].empty() && std::stoi(str[index]) > 1)
		node["Agi"] << str[index];
	if (!str[++index].empty() && std::stoi(str[index]) > 1)
		node["Vit"] << str[index];
	if (!str[++index].empty() && std::stoi(str[index]) > 1)
		node["Int"] << str[index];
	if (!str[++index].empty() && std::stoi(str[index]) > 1)
		node["Dex"] << str[index];
	if (!str[++index].empty() && std::stoi(str[index]) > 1)
		node["Luk"] << str[index];
	if (!str[++index].empty())
		node["AttackRange"] << str[index];
	if (!str[++index].empty())
		node["SkillRange"] << str[index];
	if (!str[++index].empty())
		node["ChaseRange"] << str[index];
	if (!str[++index].empty() && strcmp(str[index].c_str(), "Small") != 0)
		node["Size"] << str[index];
	if (!str[++index].empty() && strcmp(str[index].c_str(), "Formless") != 0)
		node["Race"] << str[index];

	ryml::NodeRef raceGroupsNode = node["RaceGroups"];
	raceGroupsNode |= ryml::MAP;

	for (uint16 i = 1; i < RC2_MAX; i++) {
		if (!str[i + index].empty()) {
			raceGroupsNode[c4::to_csubstr(script_get_constant_str("RC2_", i) + 4)] << (std::stoi(str[i + index]) ? "true" : "false");
		}
	}

	index += RC2_MAX - 1;

	if (!str[++index].empty() && strcmp(str[index].c_str(), "Neutral") != 0)
		node["Element"] << str[index];
	if (!str[++index].empty() && std::stoi(str[index]) > 1)
		node["ElementLevel"] << str[index];
	if (!str[++index].empty())
		node["WalkSpeed"] << str[index];
	if (!str[++index].empty())
		node["AttackDelay"] << str[index];
	if (!str[++index].empty())
		node["AttackMotion"] << str[index];
	if (!str[++index].empty())
		node["DamageMotion"] << str[index];
	if (!str[++index].empty())
		node["DamageTaken"] << str[index];
	if (!str[++index].empty() && strcmp(str[index].c_str(), "06") != 0)
		node["Ai"] << str[index];
	if (!str[++index].empty() && strcmp(str[index].c_str(), "Normal") != 0)
		node["Class"] << str[index];

	ryml::NodeRef modes = node["Modes"];
	modes |= ryml::MAP;

	if (!str[++index].empty())
		modes["CanMove"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["Looter"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["Aggressive"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["Assist"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["CastSensorIdle"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["NoRandomWalk"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["NoCast"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["CanAttack"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["CastSensorChase"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["ChangeChase"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["Angry"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["ChangeTargetMelee"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["ChangeTargetChase"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["TargetWeak"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["RandomTarget"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["IgnoreMelee"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["IgnoreMagic"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["IgnoreRanged"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["Mvp"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["IgnoreMisc"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["KnockBackImmune"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["TeleportBlock"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["FixedItemDrop"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["Detector"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["StatusImmune"] << (std::stoi(str[index]) ? "true" : "false");
	if (!str[++index].empty())
		modes["SkillImmune"] << (std::stoi(str[index]) ? "true" : "false");

	ryml::NodeRef mvpDropsNode = node["MvpDrops"];
	mvpDropsNode |= ryml::SEQ;

	for (uint8 i = 0; i < MAX_MVP_DROP; i++) {
		if (!str[++index].empty()) {
			ryml::NodeRef entry = mvpDropsNode[i];
			entry |= ryml::MAP;

			entry["Item"] << str[index];
			if (!str[++index].empty())
				entry["Rate"] << str[index];
			if (!str[++index].empty() && strcmp(str[index].c_str(), "None") != 0)
				entry["RandomOptionGroup"] << str[index];
			if (!str[++index].empty() && std::stoi(str[index]) >= 0)
				entry["Index"] << str[index];
		} else
			index += 3;
	}

	ryml::NodeRef dropsNode = node["Drops"];
	dropsNode |= ryml::SEQ;

	for (uint8 i = 0; i < MAX_MOB_DROP; i++) {
		if (!str[++index].empty()) {
			ryml::NodeRef entry = dropsNode[i];
			entry |= ryml::MAP;

			entry["Item"] << str[index];
			if (!str[++index].empty())
				entry["Rate"] << str[index];
			if (!str[++index].empty())
				entry["StealProtected"] << (std::stoi(str[index]) ? "true" : "false");
			if (!str[++index].empty() && strcmp(str[index].c_str(), "None") != 0)
				entry["RandomOptionGroup"] << str[index];
			if (!str[++index].empty() && std::stoi(str[index]) >= 0)
				entry["Index"] << str[index];
		} else
			index += 4;
	}

#ifdef RENEWAL
	if (!str[++index].empty())
		node["Resistance"] << std::stoi(str[index]);
	if (!str[++index].empty())
		node["MagicResistance"] << std::stoi(str[index]);
#endif

	if( !modes.has_children() ){
		node.remove_child( modes );
	}

	if( !mvpDropsNode.has_children() ){
		node.remove_child( mvpDropsNode );
	}

	if( !dropsNode.has_children() ){
		node.remove_child( dropsNode );
	}

	return mob_db.parseBodyNode(node) > 0;
}

/**
 * Read SQL mob_db table
 */
static int mob_read_sqldb(void)
{
	const char* mob_db_name[] = {
		mob_table,
		mob2_table
	};

	for( uint8 fi = 0; fi < ARRAYLENGTH(mob_db_name); ++fi ) {
		// retrieve all rows from the mob database
		if( SQL_ERROR == Sql_Query(mmysql_handle, "SELECT `id`,`name_aegis`,`name_english`,`name_japanese`,`level`,`hp`,`sp`,`base_exp`,`job_exp`,`mvp_exp`,`attack`,`attack2`,`defense`,`magic_defense`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`attack_range`,`skill_range`,`chase_range`,`size`,`race`,"
			"`racegroup_goblin`,`racegroup_kobold`,`racegroup_orc`,`racegroup_golem`,`racegroup_guardian`,`racegroup_ninja`,`racegroup_gvg`,`racegroup_battlefield`,`racegroup_treasure`,`racegroup_biolab`,`racegroup_manuk`,`racegroup_splendide`,`racegroup_scaraba`,`racegroup_ogh_atk_def`,`racegroup_ogh_hidden`,`racegroup_bio5_swordman_thief`,`racegroup_bio5_acolyte_merchant`,`racegroup_bio5_mage_archer`,`racegroup_bio5_mvp`,`racegroup_clocktower`,`racegroup_thanatos`,`racegroup_faceworm`,`racegroup_hearthunter`,`racegroup_rockridge`,`racegroup_werner_lab`,`racegroup_temple_demon`,`racegroup_illusion_vampire`,`racegroup_malangdo`,`racegroup_ep172alpha`,`racegroup_ep172beta`,`racegroup_ep172bath`,`racegroup_illusion_turtle`,`racegroup_rachel_sanctuary`,`racegroup_illusion_luanda`,`racegroup_illusion_frozen`,`racegroup_illusion_moonlight`,"
			"`element`,`element_level`,`walk_speed`,`attack_delay`,`attack_motion`,`damage_motion`,`damage_taken`,`ai`,`class`,"
			"`mode_canmove`,`mode_looter`,`mode_aggressive`,`mode_assist`,`mode_castsensoridle`,`mode_norandomwalk`,`mode_nocast`,`mode_canattack`,`mode_castsensorchase`,`mode_changechase`,`mode_angry`,`mode_changetargetmelee`,`mode_changetargetchase`,`mode_targetweak`,`mode_randomtarget`,`mode_ignoremelee`,`mode_ignoremagic`,`mode_ignoreranged`,`mode_mvp`,`mode_ignoremisc`,`mode_knockbackimmune`,`mode_teleportblock`,`mode_fixeditemdrop`,`mode_detector`,`mode_statusimmune`,`mode_skillimmune`,"
			"`mvpdrop1_item`,`mvpdrop1_rate`,`mvpdrop1_option`,`mvpdrop1_index`,`mvpdrop2_item`,`mvpdrop2_rate`,`mvpdrop2_option`,`mvpdrop2_index`,`mvpdrop3_item`,`mvpdrop3_rate`,`mvpdrop3_option`,`mvpdrop3_index`,"
			"`drop1_item`,`drop1_rate`,`drop1_nosteal`,`drop1_option`,`drop1_index`,`drop2_item`,`drop2_rate`,`drop2_nosteal`,`drop2_option`,`drop2_index`,`drop3_item`,`drop3_rate`,`drop3_nosteal`,`drop3_option`,`drop3_index`,`drop4_item`,`drop4_rate`,`drop4_nosteal`,`drop4_option`,`drop4_index`,`drop5_item`,`drop5_rate`,`drop5_nosteal`,`drop5_option`,`drop5_index`,`drop6_item`,`drop6_rate`,`drop6_nosteal`,`drop6_option`,`drop6_index`,`drop7_item`,`drop7_rate`,`drop7_nosteal`,`drop7_option`,`drop7_index`,`drop8_item`,`drop8_rate`,`drop8_nosteal`,`drop8_option`,`drop8_index`,`drop9_item`,`drop9_rate`,`drop9_nosteal`,`drop9_option`,`drop9_index`,`drop10_item`,`drop10_rate`,`drop10_nosteal`,`drop10_option`,`drop10_index`"
#ifdef RENEWAL
			",`resistance`,`magic_resistance`"
#endif
			" FROM `%s`", mob_db_name[fi]) ) {
			Sql_ShowDebug(mmysql_handle);
			continue;
		}

		uint32 total_columns = Sql_NumColumns(mmysql_handle);
		uint64 total_rows = Sql_NumRows(mmysql_handle), rows = 0, count = 0;

		ShowStatus("Loading '" CL_WHITE "%" PRIdPTR CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'\n", total_rows, mob_db_name[fi]);

		// process rows one by one
		while( SQL_SUCCESS == Sql_NextRow(mmysql_handle) ) {
#ifdef DETAILED_LOADING_OUTPUT
			ShowStatus("Loading [%" PRIu64 "/%" PRIu64 "] entries in '" CL_WHITE "%s" CL_RESET "'" CL_CLL "\r", ++rows, total_rows, mob_db_name[fi]);
#endif
			std::vector<std::string> data = {};

			for (uint32 i = 0; i < total_columns; i++) {
				char *str;

				Sql_GetData(mmysql_handle, i, &str, nullptr);
				if (str == nullptr)
					data.push_back("");
				else
					data.push_back(str);
			}

			if (!mob_read_sqldb_sub(data))
				continue;

			count++;
		}

		// free the query result
		Sql_FreeResult(mmysql_handle);

		ShowStatus("Done reading '" CL_WHITE "%" PRIu64 CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, mob_db_name[fi]);
	}

	mob_db.loadingFinished();

	return 0;
}

const std::string MobAvailDatabase::getDefaultLocation() {
	return std::string(db_path) + "/" + DBIMPORT + "/mob_avail.yml";
}

/**
 * Reads and parses an entry from the mob_avail.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 MobAvailDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string mob_name;

	if (!this->asString(node, "Mob", mob_name))
		return 0;

	std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname(mob_name.c_str());

	if (mob == nullptr) {
		this->invalidWarning(node["Mob"], "Unknown mob %s.\n", mob_name.c_str());
		return 0;
	}

	if (this->nodeExists(node, "Sprite")) {
		std::string sprite;

		if (!this->asString(node, "Sprite", sprite))
			return 0;

		int64 constant;

		if (script_get_constant(sprite.c_str(), &constant)) {
			if (npcdb_checkid(constant) == 0 && pcdb_checkid(constant) == 0) {
				this->invalidWarning(node["Sprite"], "Unknown sprite constant %s.\n", sprite.c_str());
				return 0;
			}
		} else {
			std::shared_ptr<s_mob_db> sprite_mob = mobdb_search_aegisname(sprite.c_str());

			if (sprite_mob == nullptr) {
				this->invalidWarning(node["Sprite"], "Unknown mob sprite constant %s.\n", sprite.c_str());
				return 0;
			}

			constant = sprite_mob->id;
		}

		mob->vd.class_ = (unsigned short)constant;
	} else {
		this->invalidWarning(node["Sprite"], "Sprite is missing.\n");
		return 0;
	}

	if (this->nodeExists(node, "Sex")) {
		if (pcdb_checkid(mob->vd.class_) == 0) {
			this->invalidWarning(node["Sex"], "Sex is only applicable to Job sprites.\n");
			return 0;
		}

		std::string sex;

		if (!this->asString(node, "Sex", sex))
			return 0;

		std::string sex_constant = "SEX_" + sex;

		int64 constant;

		if (!script_get_constant(sex_constant.c_str(), &constant)) {
			this->invalidWarning(node["Sex"], "Unknown sex constant %s.\n", sex.c_str());
			return 0;
		}

		if (constant < SEX_FEMALE || constant > SEX_MALE) {
			this->invalidWarning(node["Sex"], "Sex %s is not valid.\n", sex.c_str());
			return 0;
		}

		mob->vd.sex = (char)constant;
	}

	if (this->nodeExists(node, "HairStyle")) {
		if (pcdb_checkid(mob->vd.class_) == 0) {
			this->invalidWarning(node["HairStyle"], "HairStyle is only applicable to Job sprites.\n");
			return 0;
		}

		uint16 hair_style;

		if (!this->asUInt16(node, "HairStyle", hair_style))
			return 0;

		if (hair_style < MIN_HAIR_STYLE || hair_style > MAX_HAIR_STYLE) {
			this->invalidWarning(node["HairStyle"], "HairStyle %d is out of range %d~%d. Setting to MIN_HAIR_STYLE.\n", hair_style, MIN_HAIR_STYLE, MAX_HAIR_STYLE);
			hair_style = MIN_HAIR_STYLE;
		}

		mob->vd.hair_style = hair_style;
	}

	if (this->nodeExists(node, "HairColor")) {
		if (pcdb_checkid(mob->vd.class_) == 0) {
			this->invalidWarning(node["HairColor"], "HairColor is only applicable to Job sprites.\n");
			return 0;
		}

		uint16 hair_color;

		if (!this->asUInt16(node, "HairColor", hair_color))
			return 0;

		if (hair_color < MIN_HAIR_COLOR || hair_color > MAX_HAIR_COLOR) {
			this->invalidWarning(node["HairColor"], "HairColor %d is out of range %d~%d. Setting to MIN_HAIR_COLOR.\n", hair_color, MIN_HAIR_COLOR, MAX_HAIR_COLOR);
			hair_color = MIN_HAIR_COLOR;
		}

		mob->vd.hair_color = hair_color;
	}

	if (this->nodeExists(node, "ClothColor")) {
		if (pcdb_checkid(mob->vd.class_) == 0) {
			this->invalidWarning(node["ClothColor"], "ClothColor is only applicable to Job sprites.\n");
			return 0;
		}

		uint32 cloth_color;

		if (!this->asUInt32(node, "ClothColor", cloth_color))
			return 0;

		if (cloth_color < MIN_CLOTH_COLOR || cloth_color > MAX_CLOTH_COLOR) {
			this->invalidWarning(node["ClothColor"], "ClothColor %d is out of range %d~%d. Setting to MIN_CLOTH_CLOR.\n", cloth_color, MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
			cloth_color = MIN_CLOTH_COLOR;
		}

		mob->vd.cloth_color = cloth_color;
	}

	if (this->nodeExists(node, "Weapon")) {
		if (pcdb_checkid(mob->vd.class_) == 0) {
			this->invalidWarning(node["Weapon"], "Weapon is only applicable to Job sprites.\n");
			return 0;
		}

		std::string weapon;

		if (!this->asString(node, "Weapon", weapon))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname( weapon.c_str() );

		if (item == nullptr) {
			this->invalidWarning(node["Weapon"], "Weapon %s is not a valid item.\n", weapon.c_str());
			return 0;
		}

		mob->vd.weapon = item->nameid;
	}

	if (this->nodeExists(node, "Shield")) {
		if (pcdb_checkid(mob->vd.class_) == 0) {
			this->invalidWarning(node["Shield"], "Shield is only applicable to Job sprites.\n");
			return 0;
		}

		std::string shield;

		if (!this->asString(node, "Shield", shield))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname( shield.c_str() );

		if (item == nullptr) {
			this->invalidWarning(node["Shield"], "Shield %s is not a valid item.\n", shield.c_str());
			return 0;
		}

		mob->vd.shield = item->nameid;
	}

	if (this->nodeExists(node, "HeadTop")) {
		if (pcdb_checkid(mob->vd.class_) == 0) {
			this->invalidWarning(node["HeadTop"], "HeadTop is only applicable to Job sprites.\n");
			return 0;
		}

		std::string head;

		if (!this->asString(node, "HeadTop", head))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname( head.c_str() );

		if (item == nullptr) {
			this->invalidWarning(node["HeadTop"], "HeadTop %s is not a valid item.\n", head.c_str());
			return 0;
		}

		mob->vd.head_top = item->look;
	}

	if (this->nodeExists(node, "HeadMid")) {
		if (pcdb_checkid(mob->vd.class_) == 0) {
			this->invalidWarning(node["HeadMid"], "HeadMid is only applicable to Job sprites.\n");
			return 0;
		}

		std::string head;

		if (!this->asString(node, "HeadMid", head))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname( head.c_str() );

		if (item == nullptr) {
			this->invalidWarning(node["HeadMid"], "HeadMid %s is not a valid item.\n", head.c_str());
			return 0;
		}

		mob->vd.head_mid = item->look;
	}

	if (this->nodeExists(node, "HeadLow")) {
		if (pcdb_checkid(mob->vd.class_) == 0) {
			this->invalidWarning(node["HeadLow"], "HeadLow is only applicable to Job sprites.\n");
			return 0;
		}

		std::string head;

		if (!this->asString(node, "HeadLow", head))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname( head.c_str() );

		if (item == nullptr) {
			this->invalidWarning(node["HeadLow"], "HeadLow %s is not a valid item.\n", head.c_str());
			return 0;
		}

		mob->vd.head_bottom = item->look;
	}

	if (this->nodeExists(node, "Robe")) {
		if (pcdb_checkid(mob->vd.class_) == 0) {
			this->invalidWarning(node["Robe"], "Robe is only applicable to Job sprites.\n");
			return 0;
		}

		std::string robe;

		if (!this->asString(node, "Robe", robe))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname(robe.c_str());

		if (item == nullptr) {
			this->invalidWarning(node["Robe"], "Robe %s is not a valid item.\n", robe.c_str());
			return 0;
		}

		mob->vd.robe = item->look;
	}

	if (this->nodeExists(node, "PetEquip")) {
		std::shared_ptr<s_pet_db> pet_db_ptr = pet_db.find(mob->id);

		if (pet_db_ptr == nullptr) {
			this->invalidWarning(node["PetEquip"], "PetEquip is only applicable to defined pets.\n");
			return 0;
		}

		std::string equipment;

		if (!this->asString(node, "PetEquip", equipment))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname( equipment.c_str() );

		if (item == nullptr) {
			this->invalidWarning(node["PetEquip"], "PetEquip %s is not a valid item.\n", equipment.c_str());
			return 0;
		}

		mob->vd.head_bottom = item->nameid;
	}

	if (this->nodeExists(node, "Options")) {
		const auto& optionNode = node["Options"];

		for (const auto& it : optionNode) {
			std::string option;
			c4::from_chars(it.key(), &option);
			std::string option_constant = "OPTION_" + option;
			int64 constant;

			if (!script_get_constant(option_constant.c_str(), &constant)) {
				this->invalidWarning(optionNode, "Unknown option constant %s, skipping.\n", option.c_str());
				continue;
			}

			bool active;

			if (!this->asBool(optionNode, option, active))
				continue;

#ifdef NEW_CARTS
			if (constant & OPTION_CART) {
				this->invalidWarning(optionNode, "OPTION_CART was replace by SC_PUSH_CART, skipping.\n");
				continue;
			}
#endif

			if (active)
				mob->option |= constant;
			else
				mob->option &= ~constant;
		}

		mob->option &= ~(OPTION_HIDE | OPTION_CLOAK | OPTION_INVISIBLE | OPTION_CHASEWALK); // Remove hiding types
	}

	return 1;
}

MobAvailDatabase mob_avail_db;


const std::string MobSummonDatabase::getDefaultLocation() {
	return std::string(db_path) + "/mob_summon.yml";
}

/**
 * Reads and parses an entry from the mob_summon.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 MobSummonDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string group_name;

	if (!this->asString(node, "Group", group_name))
		return 0;

	std::string group_name_constant = "MOBG_" + group_name;
	int64 constant;

	if (!script_get_constant(group_name_constant.c_str(), &constant) || constant < MOBG_BRANCH_OF_DEAD_TREE || constant >= MOBG_MAX) {
		this->invalidWarning(node["Group"], "Invalid monster group %s.\n", group_name.c_str());
		return 0;
	}

	uint16 id = static_cast<uint16>(constant);

	std::shared_ptr<s_randomsummon_group> summon = this->find(id);
	bool exists = summon != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Default" }))
			return 0;

		summon = std::make_shared<s_randomsummon_group>();
		summon->random_id = id;
	}

	if (this->nodeExists(node, "Default")) {
		std::string mob_name;

		if (!this->asString(node, "Default", mob_name))
			return 0;

		std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname(mob_name.c_str());

		if (mob == nullptr) {
			this->invalidWarning(node["Default"], "Unknown mob %s.\n", mob_name.c_str());
			return 0;
		}

		summon->default_mob_id = mob->id;
	}

	if (this->nodeExists(node, "Summon")) {
		const auto& MobNode = node["Summon"];

		for (const auto& mobit : MobNode) {
			if (!this->nodesExist(mobit, { "Mob", "Rate" })) {
				continue;
			}

			std::string mob_name;

			if (!this->asString(mobit, "Mob", mob_name))
				continue;

			std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname(mob_name.c_str());

			if (mob == nullptr) {
				this->invalidWarning(mobit["Mob"], "Unknown mob %s.\n", mob_name.c_str());
				continue;
			}

			uint32 rate;

			if (!this->asUInt32(mobit, "Rate", rate))
				continue;

			uint16 mob_id = mob->id;

			if (rate == 0) {
				if (summon->list.erase(mob_id) == 0)
					this->invalidWarning(mobit["Rate"], "Failed to remove %s, the monster doesn't exist in group %s.\n", mob_name.c_str(), group_name.c_str());
				continue;
			}

			std::shared_ptr<s_randomsummon_entry> entry = util::umap_find(summon->list, mob_id);

			if (entry == nullptr) {
				entry = std::make_shared<s_randomsummon_entry>();
				entry->mob_id = mob_id;
				summon->list[mob_id] = entry;
			}
			entry->rate = rate;
		}
	}

	if (!exists)
		this->put(id, summon);

	return 1;
}

const std::string MobChatDatabase::getDefaultLocation() {
	return std::string(db_path) + "/mob_chat_db.yml";
}

/**
 * Reads and parses an entry from the mob_chat_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 MobChatDatabase::parseBodyNode(const ryml::NodeRef& node) {
	uint16 id;

	if (!this->asUInt16(node, "Id", id))
		return 0;

	std::shared_ptr<s_mob_chat> chat = this->find(id);
	bool exists = chat != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Dialog" })) {
			return 0;
		}

		chat = std::make_shared<s_mob_chat>();
		chat->msg_id = id;
	}
	
	if (this->nodeExists(node, "Color")) {
		std::string hex;

		if (!this->asString(node, "Color", hex))
			return 0;

		chat->color = strtoul(hex.c_str(), nullptr, 0);
	} else {
		if (!exists)
			chat->color = strtoul("0xFF0000", nullptr, 0);
	}

	if (this->nodeExists(node, "Dialog")) {
		std::string msg;

		if (!this->asString(node, "Dialog", msg))
			return 0;

		if (msg.length() > (CHAT_SIZE_MAX-1)) {
			this->invalidWarning(node["Dialog"], "Message too long!\n");
			return 0;
		}
		chat->msg = msg;
	}

	if (!exists)
		this->put(id, chat);

	return 1;
}

/*==========================================
 * processes one mob_skill_db entry
 *------------------------------------------*/
static bool mob_parse_row_mobskilldb( char** str, size_t columns, size_t current ){
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
		{ "mobnearbygt",       MSC_MOBNEARBYGT       },
		{ "groundattacked",    MSC_GROUNDATTACKED    },
		{ "damagedgt",         MSC_DAMAGEDGT         },
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
	int mob_id;
	int j, tmp;

	mob_id = atoi(str[0]);

	std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);

	if (mob_id > 0 && mob == nullptr)
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
	std::shared_ptr<s_mob_skill_db> skill = util::umap_find(mob_skill_db, mob_id);

	if (skill == nullptr)
		skill = std::make_shared<s_mob_skill_db>();

	if( strcmp(str[1],"clear") == 0 && skill->mob_id != 0 ) {
		mob_skill_db.erase(skill->mob_id);
		ShowInfo("Cleared skill for mob id '%d'\n", mob_id);
		return true;
	}

	if (skill->skill.size() >= MAX_MOBSKILL) {
		if (mob_id != last_mob_id) {
			ShowError("mob_parse_row_mobskilldb: Too many skills for monster %d[%s]\n", mob_id, mob->sprite.c_str());
			last_mob_id = mob_id;
		}
		return false;
	}

	std::shared_ptr<s_mob_skill> ms = std::make_shared<s_mob_skill>();

	//State
	ARR_FIND( 0, ARRAYLENGTH(state), j, strcmp(str[2],state[j].str) == 0 );
	if( j < ARRAYLENGTH(state) )
		ms->state = state[j].id;
	else {
		ShowError("mob_parse_row_mobskilldb: Unrecognized state '%s' in line %d\n", str[2], current);
		return false;
	}

	//Skill ID
	j = atoi(str[3]);
	if (j <= 0 || j > MAX_SKILL_ID || !skill_get_index(j)) //fixed Lupus
	{
		if (mob_id < 0)
			ShowError("mob_parse_row_mobskilldb: Invalid Skill ID (%d) for all mobs\n", j);
		else
			ShowError("mob_parse_row_mobskilldb: Invalid Skill ID (%d) for mob %d (%s)\n", j, mob_id, mob->sprite.c_str());
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
				mob_id < 0 ? "all mobs" : mob->sprite.c_str());
			ms->target = MST_TARGET;
		}
	} else if (ms->target > MST_MASTER) {
		ShowWarning("mob_parse_row_mobskilldb: Wrong mob skill target 'around' for non-ground skill %d (%s) for %s.\n",
			ms->skill_id, skill_get_name(ms->skill_id),
			mob_id < 0 ? "all mobs" : mob->sprite.c_str());
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

	ms->val[0] = (int)strtol(str[12],nullptr,0);
	ms->val[1] = (int)strtol(str[13],nullptr,0);
	ms->val[2] = (int)strtol(str[14],nullptr,0);
	ms->val[3] = (int)strtol(str[15],nullptr,0);
	ms->val[4] = (int)strtol(str[16],nullptr,0);

	if(ms->skill_id == NPC_EMOTION && mob_id > 0 &&
		ms->val[1] == mob->status.mode)
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

	if (*str[18]) {
		uint16 id = static_cast<uint16>(strtol(str[18], nullptr, 10));
		if (mob_chat_db.find(id) != nullptr)
			ms->msg_id = id;
		else {
			ms->msg_id = 0;
			ShowWarning("mob_parse_row_mobskilldb: Unknown chat ID %s for monster %d.\n", str[18], mob_id);
		}
	}
	else
		ms->msg_id = 0;

	skill->skill.push_back(ms);

	if (!skill->mob_id) { // Insert new entry
		skill->mob_id = mob_id;
		mob_skill_db.insert({ mob_id, skill });
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
		mob_skill_table,
		mob_skill2_table };
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
			char dummy[255] = "";
			int i;
			++lines;
			for( i = 0; i < 19; ++i )
			{
				Sql_GetData(mmysql_handle, i, &str[i], nullptr);
				if( str[i] == nullptr ) 
					str[i] = dummy; // get rid of nullptr columns
			}

			if (!mob_parse_row_mobskilldb(str, 19, count))
				continue;

			count++;
		}

		// free the query result
		Sql_FreeResult(mmysql_handle);

		ShowStatus("Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, mob_skill_db_name[fi]);
	}
	return 0;
}

const std::string MobItemRatioDatabase::getDefaultLocation() {
	return std::string(db_path) + "/mob_item_ratio.yml";
}

/**
 * Reads and parses an entry from the mob_item_ratio.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 MobItemRatioDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string item_name;

	if (!this->asString(node, "Item", item_name))
		return 0;

	std::shared_ptr<item_data> item = item_db.search_aegisname(item_name.c_str());

	if (item == nullptr) {
		this->invalidWarning(node["Item"], "Item %s does not exist, skipping.\n", item_name.c_str());
		return 0;
	}

	t_itemid nameid = item->nameid;

	std::shared_ptr<s_mob_item_drop_ratio> data = this->find(nameid);
	bool exists = data != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Ratio" })) {
			return 0;
		}

		data = std::make_shared<s_mob_item_drop_ratio>();
		data->nameid = nameid;
	}
	
	if (this->nodeExists(node, "Ratio")) {
		uint32 ratio;

		if (!this->asUInt32(node, "Ratio", ratio))
			return 0;

		data->drop_ratio = ratio;
	}

	if (this->nodeExists(node, "List")) {
		const auto& MobNode = node["List"];

		for (const auto& mobit : MobNode) {
			std::string mob_name;
			c4::from_chars(mobit.key(), &mob_name);

			std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname(mob_name.c_str());

			if (mob == nullptr) {
				this->invalidWarning(node["List"], "Unknown mob %s, skipping.\n", mob_name.c_str());
				continue;
			}
			uint16 mob_id = mob->id;

			bool active;

			if (!this->asBool(node["List"], mob_name, active))
				return 0;

			if (!active) {
				util::vector_erase_if_exists(data->mob_ids, mob_id);
				continue;
			}

			data->mob_ids.push_back(mob_id);
		}
	}

	if (!exists)
		this->put(nameid, data);

	return 1;
}

/**
 * Adjust drop ratio for each monster
 **/
static void mob_drop_ratio_adjust(void){
	for( auto &pair : mob_db ){
		std::shared_ptr<s_mob_db> mob = pair.second;
		struct item_data *id;
		t_itemid nameid;
		int j, rate, rate_adjust = 0, mob_id = pair.first;

		if( mob_is_clone( mob_id ) ){
			continue;
		}

		for( j = 0; j < MAX_MVP_DROP_TOTAL; j++ ){
			nameid = mob->mvpitem[j].nameid;
			rate = mob->mvpitem[j].rate;

			if( nameid == 0 || rate == 0 ){
				continue;
			}

			rate_adjust = battle_config.item_rate_mvp;

			// Adjust the rate if there is an entry in mob_item_ratio
			item_dropratio_adjust( nameid, mob_id, &rate_adjust );

			// remove the item if the rate of item_dropratio_adjust is 0
			if (rate_adjust == 0) {
				mob->mvpitem[j].nameid = 0;
				mob->mvpitem[j].rate = 0;
				continue;
			}

			// Adjust rate with given algorithms
			rate = mob_drop_adjust( rate, rate_adjust, battle_config.item_drop_mvp_min, battle_config.item_drop_mvp_max );

			// calculate and store Max available drop chance of the MVP item
			if( rate ){
				id = itemdb_search( nameid );

				// Item is not known anymore(should never happen)
				if( !id ){
					ShowWarning( "Monster \"%s\"(id:%u) is dropping an unknown item(id: %u)\n", mob->name.c_str(), mob_id, nameid );
					mob->mvpitem[j].nameid = 0;
					mob->mvpitem[j].rate = 0;
					continue;
				}

				if( id->maxchance == -1 || ( id->maxchance < rate/10 + 1 ) ){
					// item has bigger drop chance or sold in shops
					id->maxchance = rate/10 + 1; // reduce MVP drop info to not spoil common drop rate
				}
			}

			mob->mvpitem[j].rate = rate;
		}

		for( j = 0; j < MAX_MOB_DROP_TOTAL; j++ ){
			unsigned short ratemin, ratemax;
			bool is_treasurechest;

			nameid = mob->dropitem[j].nameid;
			rate = mob->dropitem[j].rate;

			if( nameid == 0 || rate == 0 ){
				continue;
			}

			id = itemdb_search( nameid );

			// Item is not known anymore(should never happen)
			if( !id ){
				ShowWarning( "Monster \"%s\"(id:%hu) is dropping an unknown item(id: %u)\n", mob->name.c_str(), mob_id, nameid );
				mob->dropitem[j].nameid = 0;
				mob->dropitem[j].rate = 0;
				continue;
			}

			if( battle_config.drop_rateincrease && rate < 5000 ){
				rate++;
			}

			// Treasure box drop rates [Skotlex]
			if (util::vector_exists(mob->race2, RC2_TREASURE)) {
				is_treasurechest = true;

				rate_adjust = battle_config.item_rate_treasure;
				ratemin = battle_config.item_drop_treasure_min;
				ratemax = battle_config.item_drop_treasure_max;
			} else {
				bool is_mvp = mob->get_bosstype() == BOSSTYPE_MVP;
				bool is_boss = mob->get_bosstype() == BOSSTYPE_MINIBOSS;

				is_treasurechest = false;

				 // Added suport to restrict normal drops of MVP's [Reddozen]
				switch( id->type ){
					case IT_HEALING:
						rate_adjust = is_mvp ? battle_config.item_rate_heal_mvp : (is_boss ? battle_config.item_rate_heal_boss : battle_config.item_rate_heal);
						ratemin = battle_config.item_drop_heal_min;
						ratemax = battle_config.item_drop_heal_max;
						break;
					case IT_USABLE:
					case IT_CASH:
						rate_adjust = is_mvp ? battle_config.item_rate_use_mvp : (is_boss ? battle_config.item_rate_use_boss : battle_config.item_rate_use);
						ratemin = battle_config.item_drop_use_min;
						ratemax = battle_config.item_drop_use_max;
						break;
					case IT_WEAPON:
					case IT_ARMOR:
					case IT_PETARMOR:
						rate_adjust = is_mvp ? battle_config.item_rate_equip_mvp : (is_boss ? battle_config.item_rate_equip_boss : battle_config.item_rate_equip);
						ratemin = battle_config.item_drop_equip_min;
						ratemax = battle_config.item_drop_equip_max;
						break;
					case IT_CARD:
						rate_adjust = is_mvp ? battle_config.item_rate_card_mvp : (is_boss ? battle_config.item_rate_card_boss : battle_config.item_rate_card);
						ratemin = battle_config.item_drop_card_min;
						ratemax = battle_config.item_drop_card_max;
						break;
					default:
						rate_adjust = is_mvp ? battle_config.item_rate_common_mvp : (is_boss ? battle_config.item_rate_common_boss : battle_config.item_rate_common);
						ratemin = battle_config.item_drop_common_min;
						ratemax = battle_config.item_drop_common_max;
						break;
				}
			}

			item_dropratio_adjust( nameid, mob_id, &rate_adjust );

			// remove the item if the rate of item_dropratio_adjust is 0
			if (rate_adjust == 0) {
				mob->dropitem[j].nameid = 0;
				mob->dropitem[j].rate = 0;
				continue;
			}

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

			mob->dropitem[j].rate = rate;
		}
	}

	// Now that we are done we can delete the stored item ratios
	mob_item_drop_ratio.clear();
}

const std::string MapDropDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/map_drops.yml";
}

uint64 MapDropDatabase::parseBodyNode( const ryml::NodeRef& node ){
	std::string mapname;

	if( !this->asString( node, "Map", mapname ) ){
		return 0;
	}

	uint16 mapindex = mapindex_name2idx( mapname.c_str(), nullptr );

	if( mapindex == 0 ){
		this->invalidWarning( node["Map"], "Unknown map \"%s\".\n", mapname.c_str() );
		return 0;
	}

	int16 mapid = map_mapindex2mapid( mapindex );

	if( mapid < 0 ){
		// Silently ignore. Map might be on a different map-server
		return 0;
	}

	std::shared_ptr<s_map_drops> mapdrops = this->find( mapid );
	bool exists = mapdrops != nullptr;

	if( !exists ){
		mapdrops = std::make_shared<s_map_drops>();
		mapdrops->mapid = mapid;
	}

	if( this->nodeExists( node, "GlobalDrops" ) ){
		const ryml::NodeRef& globalNode = node["GlobalDrops"];

		for( const auto& it : globalNode ){
			if( !this->parseDrop( it, mapdrops->globals ) ){
				return 0;
			}
		}
	}

	if( this->nodeExists( node, "SpecificDrops" ) ){
		const ryml::NodeRef& specificNode = node["SpecificDrops"];

		for( const auto& monsterNode : specificNode ){
			if( !this->nodesExist( monsterNode, { "Monster", "Drops" } ) ){
				return 0;
			}

			std::string mobname;

			if( !this->asString( monsterNode, "Monster", mobname ) ){
				return 0;
			}

			std::shared_ptr<s_mob_db> mob = mobdb_search_aegisname( mobname.c_str() );

			if( mob == nullptr ){
				this->invalidWarning( monsterNode["Monster"], "Unknown monster \"%s\".\n", mobname.c_str() );
				return 0;
			}

			std::unordered_map<uint16, std::shared_ptr<s_mob_drop>>& specificDrops = mapdrops->specific[mob->id];

			for( const auto& it : monsterNode["Drops"] ){
				if( !this->parseDrop( it, specificDrops ) ){
					return 0;
				}
			}
		}
	}

	if( !exists ){
		this->put( mapid, mapdrops );
	}

	return 1;
}

bool MapDropDatabase::parseDrop( const ryml::NodeRef& node, std::unordered_map<uint16, std::shared_ptr<s_mob_drop>>& drops ){
	uint16 index;

	if( !this->asUInt16( node, "Index", index ) ){
		return false;
	}

	std::shared_ptr<s_mob_drop> drop = util::umap_find( drops, index );
	bool exists = drop != nullptr;

	if( !exists ){
		if( !this->nodesExist( node, { "Item", "Rate" } ) ){
			return false;
		}

		drop = std::make_shared<s_mob_drop>();
		drop->steal_protected = true;
	}

	if( this->nodeExists( node, "Item" ) ){
		std::string itemname;

		if( !this->asString( node, "Item", itemname ) ){
			return 0;
		}

		std::shared_ptr<item_data> item = item_db.search_aegisname( itemname.c_str() );

		if( item == nullptr ){
			this->invalidWarning( node["Item"], "Item %s does not exist.\n", itemname.c_str() );
			return false;
		}

		drop->nameid = item->nameid;
	}

	if( this->nodeExists( node, "Rate" ) ){
		uint32 rate;

		if( !this->asUInt32Rate( node, "Rate", rate, 100000 ) ){
			return false;
		}

		if( rate == 0 ){
			if( exists ){
				drops.erase( index );
				return true;
			}else{
				this->invalidWarning( node["Rate"], "Rate %" PRIu32 " is below minimum of 1.\n", rate );
				return false;
			}
		}else if( rate > 100000 ){
			this->invalidWarning( node["Rate"], "Rate %" PRIu32 " exceeds maximum of 100000.\n", rate );
			return false;
		}

		drop->rate = rate;
	}

	if( this->nodeExists( node, "RandomOptionGroup" ) ){
		std::string name;

		if( !this->asString( node, "RandomOptionGroup", name ) ){
			return false;
		}

		if( !random_option_group.option_get_id( name, drop->randomopt_group ) ){
			this->invalidWarning( node["RandomOptionGroup"], "Unknown random option group \"%s\".\n", name.c_str() );
			return false;
		}
	}else{
		if( !exists ){
			drop->randomopt_group = 0;
		}
	}

	if( !exists ){
		drops[index] = drop;
	}

	return true;
}

/**
 * Copy skill from DB to monster
 * @param mob Monster DB entry
 * @param skill Monster skill entries
 **/
static void mob_skill_db_set_single_sub(std::shared_ptr<s_mob_db> mob, struct s_mob_skill_db *skill) {
	nullpo_retv(skill);

	if (mob == nullptr)
		return;

	size_t i = 0;

	for (i = 0; mob->skill.size() < MAX_MOBSKILL && i < skill->skill.size(); i++) {
		mob->skill.push_back(skill->skill[i]);
	}

	if (i < skill->skill.size())
		ShowWarning("Monster '%s' (%d, src:%d) reaches max skill limit %d. Ignores '%zu' skills left.\n", mob->sprite.c_str(), mob->id, skill->mob_id, MAX_MOBSKILL, skill->skill.size() - i);
}

/**
 * Check the skill & monster id before put the skills
 * @param skill
 **/
static void mob_skill_db_set_single(struct s_mob_skill_db *skill) {
	nullpo_retv(skill);

	// Specific monster
	if (skill->mob_id >= 0) {
		std::shared_ptr<s_mob_db> mob = mob_db.find(skill->mob_id);

		if (mob != nullptr)
			mob_skill_db_set_single_sub(mob, skill);
	}
	// Global skill
	else {
		uint16 id = skill->mob_id;
		id *= -1;
		for( auto &pair : mob_db ){
			if ( mob_is_clone(pair.first) ){
				continue;
			}
			if (   (!(id&1) && status_has_mode(&pair.second->status,MD_STATUSIMMUNE)) // Bosses
				|| (!(id&2) && !status_has_mode(&pair.second->status,MD_STATUSIMMUNE)) // Normal monsters
				)
				continue;
			mob_skill_db_set_single_sub(pair.second, skill);
		}
	}
	
}

/**
 * Set monster skills
 **/
static void mob_skill_db_set(void) {
	for (auto skill : mob_skill_db) {
		mob_skill_db_set_single(skill.second.get());
	}

	//ShowStatus("Set skills to '%d' monsters.\n", db_size(mob_skill_db));
}

/**
 * read all mob-related databases
 */
static void mob_load(void)
{
	const char* dbsubpath[] = {
		"",
		"/" DBIMPORT,
	};

	// First we parse all the possible monsters to add additional data in the second loop
	if( db_use_sqldbs )
		mob_read_sqldb();
	else
		mob_db.load();

	mob_chat_db.load();	// load before mob_skill_db

	for(int i = 0; i < ARRAYLENGTH(dbsubpath); i++){	
		size_t n1 = strlen( db_path ) + strlen( dbsubpath[i] ) + 1;
		size_t n2 = strlen( db_path ) + strlen( DBPATH ) + strlen( dbsubpath[i] ) + 1;

		char* dbsubpath1 = (char*)aMalloc(n1+1);
		char* dbsubpath2 = (char*)aMalloc(n2+1);
		bool silent = i > 0;

		if(i==0) {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n2,"%s/%s%s",db_path,DBPATH,dbsubpath[i]);
		} else {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n1,"%s%s",db_path,dbsubpath[i]);
		}

		if( db_use_sqldbs && i == 0 )
			mob_read_sqlskilldb();
		else
			mob_readskilldb(dbsubpath2, silent);


		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}

	mob_item_drop_ratio.load();
	mob_avail_db.load();
	mob_summon_db.load();
	map_drop_db.load();

	mob_drop_ratio_adjust();
	mob_skill_db_set();
}

/**
 * Initialize monster data
 */
void mob_db_load(bool is_reload){
	mob_load();
}

/**
 * Re-link monster drop data with item data
 * Fixes the need of a @reloadmobdb after a @reloaditemdb
 * @author Epoque
 */
void mob_reload_itemmob_data(void) {
	for( auto const &pair : mob_db ){
		int d, k;

		if( mob_is_clone( pair.first ) ){
			continue;
		}

		for(d = 0; d < MAX_MOB_DROP_TOTAL; d++) {
			struct item_data *id;
			if( !pair.second->dropitem[d].nameid )
				continue;
			id = itemdb_search(pair.second->dropitem[d].nameid);

			for (k = 0; k < MAX_SEARCH; k++) {
				if (id->mob[k].chance <= pair.second->dropitem[d].rate)
					break;
			}

			if (k == MAX_SEARCH)
				continue;

			if (id->mob[k].id != pair.first)
				memmove(&id->mob[k+1], &id->mob[k], (MAX_SEARCH-k-1)*sizeof(id->mob[0]));
			id->mob[k].chance = pair.second->dropitem[d].rate;
			id->mob[k].id = pair.first;
		}
	}
}

/**
 * Apply the proper view data on monsters during mob_db reload.
 * @param md: Mob to adjust
 * @param args: va_list of arguments
 * @return 0
 */
static int mob_reload_sub( struct mob_data *md, va_list args ){
	// Slaves have to be killed
	if( md->master_id != 0 ){
		unit_remove_map( &md->bl, CLR_OUTSIGHT );
		return 0;
	}

	// Relink the mob to the new database entry
	md->db = mob_db.find(md->mob_id);

	if( md->db == nullptr ){
		ShowWarning( "mob_reload_sub: Monster %s (ID: %hu) does not exist anymore.\n", md->name, md->mob_id );
		if( md->bl.prev != nullptr ){
			ShowDebug( "mob_reload_sub: The monster was removed from map %s (%hu/%hu).\n", map_mapid2mapname( md->bl.m ), md->bl.x, md->bl.y );
		}

		unit_remove_map( &md->bl, CLR_OUTSIGHT );

		return 0;
	}

	// Recalculate the monster status based on the new data
	status_calc_mob(md, SCO_NONE);

	// If the view data was not overwritten manually
	if( !md->vd_changed ){
		// Get the new view data from the mob database
		md->vd = mob_get_viewdata(md->mob_id);

		// If they are spawned right now
		if( md->bl.prev != nullptr ){
			// Respawn all mobs on client side so that they are displayed correctly(if their view id changed)
			clif_clearunit_area( md->bl, CLR_OUTSIGHT );
			clif_spawn(&md->bl);
		}
	}

	return 0;
}

/**
 * Apply the proper view data on NPCs during mob_db reload.
 * @param md: NPC to adjust
 * @param args: va_list of arguments
 * @return 0
 */
static int mob_reload_sub_npc( struct npc_data *nd, va_list args ){
	// If the view data points to a mob
	if( mobdb_checkid(nd->class_) ){
		struct view_data *vd = mob_get_viewdata(nd->class_);

		if (vd) // Get the new view data from the mob database
			memcpy(&nd->vd, vd, sizeof(struct view_data));
		if (nd->bl.prev) // If they are spawned right now
			unit_refresh(&nd->bl); // Respawn all NPCs on client side so that they are displayed correctly(if their view id changed)
	}

	return 0;
}

/**
 * Reload monster data
 */
void mob_reload(void) {
	do_final_mob(true);
	mob_db_load(true);
	map_foreachmob(mob_reload_sub);
	map_foreachnpc(mob_reload_sub_npc);
}

/**
 * Clear spawn data for all monsters
 */
void mob_clear_spawninfo()
{	//Clears spawn related information for a script reload.
	mob_spawn_data.clear();
}

/*==========================================
 * Circumference initialization of mob
 *------------------------------------------*/
void do_init_mob(void){
	mob_db_load(false);

	add_timer_func_list(mob_delayspawn,"mob_delayspawn");
	add_timer_func_list(mob_delay_item_drop,"mob_delay_item_drop");
	add_timer_func_list(mob_ai_hard,"mob_ai_hard");
	add_timer_func_list(mob_ai_lazy,"mob_ai_lazy");
	add_timer_func_list(mob_timer_delete,"mob_timer_delete");
	add_timer_func_list(mob_spawn_guardian_sub,"mob_spawn_guardian_sub");
	add_timer_func_list(mob_respawn,"mob_respawn");
	add_timer_func_list(mvptomb_delayspawn,"mvptomb_delayspawn");
	add_timer_interval(gettick()+MIN_MOBTHINKTIME,mob_ai_hard,0,0,MIN_MOBTHINKTIME);
	add_timer_interval(gettick()+MIN_MOBTHINKTIME*10,mob_ai_lazy,0,0,MIN_MOBTHINKTIME*10);
}

/*==========================================
 * Clean memory usage.
 *------------------------------------------*/
void do_final_mob(bool is_reload){
	mob_db.clear();
	mob_chat_db.clear();
	mob_skill_db.clear();
	mob_item_drop_ratio.clear();
	mob_summon_db.clear();
	map_drop_db.clear();
	if( !is_reload ) {
		mob_delayed_drops.clear();
	}
}
