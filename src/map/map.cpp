// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "map.hpp"

#include <cstdlib>
#include <cmath>

#include <config/core.hpp>

#include <common/cbasetypes.hpp>
#include <common/cli.hpp>
#include <common/core.hpp>
#include <common/ers.hpp>
#include <common/grfio.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp> // WFIFO*()
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "achievement.hpp"
#include "atcommand.hpp"
#include "battle.hpp"
#include "battleground.hpp"
#include "cashshop.hpp"
#include "channel.hpp"
#include "chat.hpp"
#include "chrif.hpp"
#include "clan.hpp"
#include "clif.hpp"
#include "duel.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "log.hpp"
#include "mapreg.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "navi.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pet.hpp"
#include "quest.hpp"
#include "storage.hpp"
#include "trade.hpp"

using namespace rathena;
using namespace rathena::server_map;

std::string default_codepage = "";

int32 map_server_port = 3306;
std::string map_server_ip = "127.0.0.1";
std::string map_server_id = "ragnarok";
std::string map_server_pw = "";
std::string map_server_db = "ragnarok";
Sql* mmysql_handle;
Sql* qsmysql_handle; /// For query_sql

int32 db_use_sqldbs = 0;
char barter_table[32] = "barter";
char buyingstores_table[32] = "buyingstores";
char buyingstore_items_table[32] = "buyingstore_items";
#ifdef RENEWAL
char item_table[32] = "item_db_re";
char item2_table[32] = "item_db2_re";
char mob_table[32] = "mob_db_re";
char mob2_table[32] = "mob_db2_re";
char mob_skill_table[32] = "mob_skill_db_re";
char mob_skill2_table[32] = "mob_skill_db2_re";
#else
char item_table[32] = "item_db";
char item2_table[32] = "item_db2";
char mob_table[32] = "mob_db";
char mob2_table[32] = "mob_db2";
char mob_skill_table[32] = "mob_skill_db";
char mob_skill2_table[32] = "mob_skill_db2";
#endif
char sales_table[32] = "sales";
char vendings_table[32] = "vendings";
char vending_items_table[32] = "vending_items";
char market_table[32] = "market";
char partybookings_table[32] = "party_bookings";
char roulette_table[32] = "db_roulette";
char guild_storage_log_table[32] = "guild_storage_log";

// log database
std::string log_db_ip = "127.0.0.1";
uint16 log_db_port = 3306;
std::string log_db_id = "ragnarok";
std::string log_db_pw = "";
std::string log_db_db = "log";
Sql* logmysql_handle;

// inter config
struct inter_conf inter_config {};

// DBMap declaration
static DBMap* id_db=nullptr; /// int32 id -> struct block_list*
static DBMap* pc_db=nullptr; /// int32 id -> map_session_data*
static DBMap* mobid_db=nullptr; /// int32 id -> struct mob_data*
static DBMap* bossid_db=nullptr; /// int32 id -> struct mob_data* (MVP db)
static DBMap* map_db=nullptr; /// uint32 mapindex -> struct map_data*
static DBMap* nick_db=nullptr; /// uint32 char_id -> struct charid2nick* (requested names of offline characters)
static DBMap* charid_db=nullptr; /// uint32 char_id -> map_session_data*
static DBMap* regen_db=nullptr; /// int32 id -> struct block_list* (status_natural_heal processing)
static DBMap* map_msg_db=nullptr;

static int32 map_users=0;

#define BLOCK_SIZE 8
#define block_free_max 1048576
struct block_list *block_free[block_free_max];
static int32 block_free_count = 0, block_free_lock = 0;

#define BL_LIST_MAX 1048576
static struct block_list *bl_list[BL_LIST_MAX];
static int32 bl_list_count = 0;

#ifndef MAP_MAX_MSG
	#define MAP_MAX_MSG 1550
#endif

struct map_data map[MAX_MAP_PER_SERVER];
int32 map_num = 0;

int32 map_port=0;

int32 autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
int32 minsave_interval = 100;
int16 save_settings = CHARSAVE_ALL;
bool agit_flag = false;
bool agit2_flag = false;
bool agit3_flag = false;
int32 night_flag = 0; // 0=day, 1=night [Yor]

struct charid_request {
	struct charid_request* next;
	int32 charid;// who want to be notified of the nick
};
struct charid2nick {
	char nick[NAME_LENGTH];
	struct charid_request* requests;// requests of notification on this nick
};

// This is the main header found at the very beginning of the map cache
struct map_cache_main_header {
	uint32 file_size;
	uint16 map_count;
};

// This is the header appended before every compressed map cells info in the map cache
struct map_cache_map_info {
	char name[MAP_NAME_LENGTH];
	int16 xs;
	int16 ys;
	int32 len;
};

char motd_txt[256] = "conf/motd.txt";
char charhelp_txt[256] = "conf/charhelp.txt";
char channel_conf[256] = "conf/channels.conf";

const char *MSG_CONF_NAME_RUS;
const char *MSG_CONF_NAME_SPN;
const char *MSG_CONF_NAME_GRM;
const char *MSG_CONF_NAME_CHN;
const char *MSG_CONF_NAME_MAL;
const char *MSG_CONF_NAME_IDN;
const char *MSG_CONF_NAME_FRN;
const char *MSG_CONF_NAME_POR;
const char *MSG_CONF_NAME_THA;

char wisp_server_name[NAME_LENGTH] = "Server"; // can be modified in char-server configuration file

int32 console = 0;
int32 enable_spy = 0; //To enable/disable @spy commands, which consume too much cpu time when sending packets. [Skotlex]
int32 enable_grf = 0;	//To enable/disable reading maps from GRF files, bypassing mapcache [blackhole89]

#ifdef MAP_GENERATOR
struct s_generator_options {
	bool navi;
	bool itemmoveinfo;
	bool reputation;
} gen_options;
#endif

/**
 * Get the map data
 * @param mapid: Map ID to lookup
 * @return map_data on success or nullptr on failure
 */
struct map_data *map_getmapdata(int16 mapid)
{
	if (mapid < 0 || mapid >= MAX_MAP_PER_SERVER)
		return nullptr;

	return &map[mapid];
}

/*==========================================
 * server player count (of all mapservers)
 *------------------------------------------*/
void map_setusers(int32 users)
{
	map_users = users;
}

int32 map_getusers(void)
{
	return map_users;
}

/*==========================================
 * server player count (this mapserver only)
 *------------------------------------------*/
int32 map_usercount(void)
{
	return pc_db->size(pc_db);
}

void map_destroyblock( block_list* bl ){
	if( bl == nullptr ){
		return;
	}

	switch( bl->type ){
		case BL_NUL:
			// Dummy type, has no destructor
			break;

		case BL_PC:
			// Do not call the destructor here, it will be done in chrif_auth_delete
			//reinterpret_cast<map_session_data*>( bl )->~map_session_data();
			break;

		case BL_MOB:
			reinterpret_cast<mob_data*>( bl )->~mob_data();
			break;

		case BL_PET:
			reinterpret_cast<pet_data*>( bl )->~pet_data();
			break;

		case BL_HOM:
			reinterpret_cast<homun_data*>( bl )->~homun_data();
			break;

		case BL_MER:
			reinterpret_cast<s_mercenary_data*>( bl )->~s_mercenary_data();
			break;

		case BL_ITEM:
			reinterpret_cast<flooritem_data*>( bl )->~flooritem_data();
			break;

		case BL_SKILL:
			reinterpret_cast<skill_unit*>( bl )->~skill_unit();
			break;

		case BL_NPC:
			reinterpret_cast<npc_data*>( bl )->~npc_data();
			break;

		case BL_CHAT:
			reinterpret_cast<chat_data*>( bl )->~chat_data();
			break;

		case BL_ELEM:
			reinterpret_cast<s_elemental_data*>( bl )->~s_elemental_data();
			break;

		default:
			ShowError( "map_destroyblock: unknown type %d\n", bl->type );
			break;
	}

	aFree( bl );
}

/*==========================================
 * Attempt to free a map blocklist
 *------------------------------------------*/
int32 map_freeblock (struct block_list *bl)
{
	nullpo_retr(block_free_lock, bl);
	if (block_free_lock == 0 || block_free_count >= block_free_max)
	{
		map_destroyblock( bl );
		bl = nullptr;
		if (block_free_count >= block_free_max)
			ShowWarning("map_freeblock: too many free block! %d %d\n", block_free_count, block_free_lock);
	} else
		block_free[block_free_count++] = bl;

	return block_free_lock;
}
/*==========================================
 * Lock blocklist, (prevent map_freeblock usage)
 *------------------------------------------*/
int32 map_freeblock_lock (void)
{
	return ++block_free_lock;
}

/*==========================================
 * Remove the lock on map_bl
 *------------------------------------------*/
int32 map_freeblock_unlock (void)
{
	if ((--block_free_lock) == 0) {
		int32 i;
		for (i = 0; i < block_free_count; i++)
		{
			map_destroyblock( block_free[i] );
			block_free[i] = nullptr;
		}
		block_free_count = 0;
	} else if (block_free_lock < 0) {
		ShowError("map_freeblock_unlock: lock count < 0 !\n");
		block_free_lock = 0;
	}

	return block_free_lock;
}

// Timer function to check if there some remaining lock and remove them if so.
// Called each 1s
TIMER_FUNC(map_freeblock_timer){
	if (block_free_lock > 0) {
		ShowError("map_freeblock_timer: block_free_lock(%d) is invalid.\n", block_free_lock);
		block_free_lock = 1;
		map_freeblock_unlock();
	}

	return 0;
}

//
// blocklist
//
/*==========================================
 * Handling of map_bl[]
 * The address of bl_heal is set in bl->prev
 *------------------------------------------*/
static struct block_list bl_head;

#ifdef CELL_NOSTACK
/*==========================================
 * These pair of functions update the counter of how many objects
 * lie on a tile.
 *------------------------------------------*/
static void map_addblcell(struct block_list *bl)
{
	struct map_data *mapdata = map_getmapdata(bl->m);

	if( bl->m<0 || bl->x<0 || bl->x>=mapdata->xs || bl->y<0 || bl->y>=mapdata->ys || !(bl->type&BL_CHAR) )
		return;
	mapdata->cell[bl->x+bl->y*mapdata->xs].cell_bl++;
	return;
}

static void map_delblcell(struct block_list *bl)
{
	struct map_data *mapdata = map_getmapdata(bl->m);

	if( bl->m <0 || bl->x<0 || bl->x>=mapdata->xs || bl->y<0 || bl->y>=mapdata->ys || !(bl->type&BL_CHAR) )
		return;
	mapdata->cell[bl->x+bl->y*mapdata->xs].cell_bl--;
}
#endif

/*==========================================
 * Adds a block to the map.
 * Returns 0 on success, 1 on failure (illegal coordinates).
 *------------------------------------------*/
int32 map_addblock(struct block_list* bl)
{
	int16 m, x, y;
	int32 pos;

	nullpo_ret(bl);

	if (bl->prev != nullptr) {
		ShowError("map_addblock: bl->prev != nullptr\n");
		return 1;
	}

	m = bl->m;
	x = bl->x;
	y = bl->y;

	if( m < 0 )
	{
		ShowError("map_addblock: invalid map id (%d), only %d are loaded.\n", m, map_num);
		return 1;
	}

	struct map_data *mapdata = map_getmapdata(m);

	if (mapdata == nullptr || mapdata->cell == nullptr) // Player warped to a freed map. Stop them!
		return 1;

	if( x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys )
	{
		ShowError("map_addblock: out-of-bounds coordinates (\"%s\",%d,%d), map is %dx%d\n", mapdata->name, x, y, mapdata->xs, mapdata->ys);
		return 1;
	}

	pos = x/BLOCK_SIZE+(y/BLOCK_SIZE)*mapdata->bxs;

	if (bl->type == BL_MOB) {
		bl->next = mapdata->block_mob[pos];
		bl->prev = &bl_head;
		if (bl->next) bl->next->prev = bl;
		mapdata->block_mob[pos] = bl;
	} else {
		bl->next = mapdata->block[pos];
		bl->prev = &bl_head;
		if (bl->next) bl->next->prev = bl;
		mapdata->block[pos] = bl;
	}

#ifdef CELL_NOSTACK
	map_addblcell(bl);
#endif

	return 0;
}

/*==========================================
 * Removes a block from the map.
 *------------------------------------------*/
int32 map_delblock(struct block_list* bl)
{
	int32 pos;
	nullpo_ret(bl);

	// blocklist (2ways chainlist)
	if (bl->prev == nullptr) {
		if (bl->next != nullptr) {
			// can't delete block (already at the beginning of the chain)
			ShowError("map_delblock error : bl->next!=nullptr\n");
		}
		return 0;
	}

#ifdef CELL_NOSTACK
	map_delblcell(bl);
#endif

	struct map_data *mapdata = map_getmapdata(bl->m);

	nullpo_ret(mapdata);

	pos = bl->x/BLOCK_SIZE+(bl->y/BLOCK_SIZE)*mapdata->bxs;

	if (bl->next)
		bl->next->prev = bl->prev;
	if (bl->prev == &bl_head) {
	//Since the head of the list, update the block_list map of []
		if (bl->type == BL_MOB) {
			nullpo_ret(mapdata->block_mob);
			mapdata->block_mob[pos] = bl->next;
		} else {
			nullpo_ret(mapdata->block);
			mapdata->block[pos] = bl->next;
		}
	} else {
		bl->prev->next = bl->next;
	}
	bl->next = nullptr;
	bl->prev = nullptr;

	return 0;
}

/**
 * Moves a block a x/y target position. [Skotlex]
 * Pass flag as 1 to prevent doing skill_unit_move checks
 * (which are executed by default on BL_CHAR types)
 * @param bl : block(object) to move
 * @param x1 : new x position
 * @param y1 : new y position
 * @param tick : when this was scheduled
 * @return 0:success, 1:fail
 */
int32 map_moveblock(struct block_list *bl, int32 x1, int32 y1, t_tick tick)
{
	nullpo_ret(bl);

	int32 x0 = bl->x, y0 = bl->y;
	status_change *sc = nullptr;
	int32 moveblock = ( x0/BLOCK_SIZE != x1/BLOCK_SIZE || y0/BLOCK_SIZE != y1/BLOCK_SIZE);

	if (!bl->prev) {
		//Block not in map, just update coordinates, but do naught else.
		bl->x = x1;
		bl->y = y1;
		return 0;
	}

	//TODO: Perhaps some outs of bounds checking should be placed here?
	if (bl->type&BL_CHAR) {
		sc = status_get_sc(bl);

		skill_unit_move(bl,tick,2);
		if ( sc != nullptr && !sc->empty() ) //at least one to cancel
		{
			status_change_end(bl, SC_CLOSECONFINE);
			status_change_end(bl, SC_CLOSECONFINE2);
			status_change_end(bl, SC_TINDER_BREAKER);
			status_change_end(bl, SC_TINDER_BREAKER2);
	//		status_change_end(bl, SC_BLADESTOP); //Won't stop when you are knocked away, go figure...
			status_change_end(bl, SC_TATAMIGAESHI);
			status_change_end(bl, SC_MAGICROD);
			status_change_end(bl, SC_SU_STOOP);
			if (sc->getSCE(SC_PROPERTYWALK) &&
				sc->getSCE(SC_PROPERTYWALK)->val3 >= skill_get_maxcount(sc->getSCE(SC_PROPERTYWALK)->val1,sc->getSCE(SC_PROPERTYWALK)->val2) )
				status_change_end(bl,SC_PROPERTYWALK);
		}
	} else
	if (bl->type == BL_NPC)
		npc_unsetcells((TBL_NPC*)bl);

	if (moveblock) map_delblock(bl);
#ifdef CELL_NOSTACK
	else map_delblcell(bl);
#endif
	bl->x = x1;
	bl->y = y1;
	if (moveblock) {
		if(map_addblock(bl))
			return 1;
	}
#ifdef CELL_NOSTACK
	else map_addblcell(bl);
#endif

	if (bl->type&BL_CHAR) {

		skill_unit_move(bl,tick,3);

		if( bl->type == BL_PC && ((TBL_PC*)bl)->shadowform_id ) {//Shadow Form Target Moving
			struct block_list *d_bl;
			if( (d_bl = map_id2bl(((TBL_PC*)bl)->shadowform_id)) == nullptr || !check_distance_bl(bl,d_bl,10) ) {
				if( d_bl )
					status_change_end(d_bl,SC__SHADOWFORM);
				((TBL_PC*)bl)->shadowform_id = 0;
			}
		}

		if (sc != nullptr && !sc->empty()) {
			if (sc->getSCE(SC_DANCING))
				skill_unit_move_unit_group(skill_id2group(sc->getSCE(SC_DANCING)->val2), bl->m, x1-x0, y1-y0);
			else {
				if (sc->getSCE(SC_CLOAKING) && sc->getSCE(SC_CLOAKING)->val1 < 3 && !skill_check_cloaking(bl, nullptr))
					status_change_end(bl, SC_CLOAKING);
				if (sc->getSCE(SC_WARM))
					skill_unit_move_unit_group(skill_id2group(sc->getSCE(SC_WARM)->val4), bl->m, x1-x0, y1-y0);
				if (sc->getSCE(SC_BANDING))
					skill_unit_move_unit_group(skill_id2group(sc->getSCE(SC_BANDING)->val4), bl->m, x1-x0, y1-y0);

				if (sc->getSCE(SC_NEUTRALBARRIER_MASTER))
					skill_unit_move_unit_group(skill_id2group(sc->getSCE(SC_NEUTRALBARRIER_MASTER)->val2), bl->m, x1-x0, y1-y0);
				else if (sc->getSCE(SC_STEALTHFIELD_MASTER))
					skill_unit_move_unit_group(skill_id2group(sc->getSCE(SC_STEALTHFIELD_MASTER)->val2), bl->m, x1-x0, y1-y0);

				if( sc->getSCE(SC__SHADOWFORM) ) {//Shadow Form Caster Moving
					struct block_list *d_bl;
					if( (d_bl = map_id2bl(sc->getSCE(SC__SHADOWFORM)->val2)) == nullptr || !check_distance_bl(bl,d_bl,10) )
						status_change_end(bl,SC__SHADOWFORM);
				}

				if (sc->getSCE(SC_PROPERTYWALK)
					&& sc->getSCE(SC_PROPERTYWALK)->val3 < skill_get_maxcount(sc->getSCE(SC_PROPERTYWALK)->val1,sc->getSCE(SC_PROPERTYWALK)->val2)
					&& map_find_skill_unit_oncell(bl,bl->x,bl->y,SO_ELECTRICWALK,nullptr,0) == nullptr
					&& map_find_skill_unit_oncell(bl,bl->x,bl->y,NPC_ELECTRICWALK,nullptr,0) == nullptr
					&& map_find_skill_unit_oncell(bl,bl->x,bl->y,SO_FIREWALK,nullptr,0) == nullptr
					&& map_find_skill_unit_oncell(bl,bl->x,bl->y,NPC_FIREWALK,nullptr,0) == nullptr
					&& skill_unitsetting(bl,sc->getSCE(SC_PROPERTYWALK)->val1,sc->getSCE(SC_PROPERTYWALK)->val2,x0, y0,0)) {
						sc->getSCE(SC_PROPERTYWALK)->val3++;
				}


			}
			/* Guild Aura Moving */
			if( bl->type == BL_PC && ((TBL_PC*)bl)->state.gmaster_flag ) {
				if (sc->getSCE(SC_LEADERSHIP))
					skill_unit_move_unit_group(skill_id2group(sc->getSCE(SC_LEADERSHIP)->val4), bl->m, x1-x0, y1-y0);
				if (sc->getSCE(SC_GLORYWOUNDS))
					skill_unit_move_unit_group(skill_id2group(sc->getSCE(SC_GLORYWOUNDS)->val4), bl->m, x1-x0, y1-y0);
				if (sc->getSCE(SC_SOULCOLD))
					skill_unit_move_unit_group(skill_id2group(sc->getSCE(SC_SOULCOLD)->val4), bl->m, x1-x0, y1-y0);
				if (sc->getSCE(SC_HAWKEYES))
					skill_unit_move_unit_group(skill_id2group(sc->getSCE(SC_HAWKEYES)->val4), bl->m, x1-x0, y1-y0);
			}
		}
	} else
	if (bl->type == BL_NPC)
		npc_setcells((TBL_NPC*)bl);

	return 0;
}

/*==========================================
 * Counts specified number of objects on given cell.
 * flag:
 *		0x1 - only count standing units
 *------------------------------------------*/
int32 map_count_oncell(int16 m, int16 x, int16 y, int32 type, int32 flag)
{
	int32 bx,by;
	struct block_list *bl;
	int32 count = 0;
	struct map_data *mapdata = map_getmapdata(m);

	if (x < 0 || y < 0 || (x >= mapdata->xs) || (y >= mapdata->ys))
		return 0;

	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	if (type&~BL_MOB)
		for( bl = mapdata->block[bx+by*mapdata->bxs] ; bl != nullptr ; bl = bl->next )
			if(bl->x == x && bl->y == y && bl->type&type) {
				if (bl->type == BL_NPC) {	// Don't count hidden or invisible npc. Cloaked npc are counted
					npc_data *nd = BL_CAST(BL_NPC, bl);
					if (nd->m < 0 || nd->sc.option&OPTION_HIDE || nd->dynamicnpc.owner_char_id != 0)
						continue;
				}
				if(flag&1) {
					struct unit_data *ud = unit_bl2ud(bl);
					if(!ud || ud->walktimer == INVALID_TIMER)
						count++;
				} else {
					count++;
				}
			}

	if (type&BL_MOB)
		for( bl = mapdata->block_mob[bx+by*mapdata->bxs] ; bl != nullptr ; bl = bl->next )
			if(bl->x == x && bl->y == y) {
				if(flag&1) {
					struct unit_data *ud = unit_bl2ud(bl);
					if(!ud || ud->walktimer == INVALID_TIMER)
						count++;
				} else {
					count++;
				}
			}

	return count;
}

/*
 * Looks for a skill unit on a given cell
 * flag&1: runs battle_check_target check based on unit->group->target_flag
 */
struct skill_unit* map_find_skill_unit_oncell(struct block_list* target,int16 x,int16 y,uint16 skill_id,struct skill_unit* out_unit, int32 flag) {
	int16 bx,by;
	struct block_list *bl;
	struct skill_unit *unit;
	struct map_data *mapdata = map_getmapdata(target->m);

	if (x < 0 || y < 0 || (x >= mapdata->xs) || (y >= mapdata->ys))
		return nullptr;

	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	for( bl = mapdata->block[bx+by*mapdata->bxs] ; bl != nullptr ; bl = bl->next )
	{
		if (bl->x != x || bl->y != y || bl->type != BL_SKILL)
			continue;

		unit = (struct skill_unit *) bl;
		if( unit == out_unit || !unit->alive || !unit->group || unit->group->skill_id != skill_id )
			continue;
		if( !(flag&1) || battle_check_target(unit,target,unit->group->target_flag) > 0 )
			return unit;
	}
	return nullptr;
}

/*==========================================
 * Adapted from foreachinarea for an easier invocation. [Skotlex]
 *------------------------------------------*/
int32 map_foreachinrangeV(int32 (*func)(struct block_list*,va_list),struct block_list* center, int16 range, int32 type, va_list ap, bool wall_check)
{
	int32 bx, by, m;
	int32 returnCount = 0;	//total sum of returned values of func() [Skotlex]
	struct block_list *bl;
	int32 blockcount = bl_list_count, i;
	int32 x0, x1, y0, y1;
	va_list ap_copy;

	m = center->m;
	if( m < 0 )
		return 0;

	struct map_data *mapdata = map_getmapdata(m);

	if( mapdata == nullptr || mapdata->block == nullptr ){
		return 0;
	}

	x0 = i16max(center->x - range, 0);
	y0 = i16max(center->y - range, 0);
	x1 = i16min(center->x + range, mapdata->xs - 1);
	y1 = i16min(center->y + range, mapdata->ys - 1);

	if ( type&~BL_MOB ) {
		for( by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++ ) {
			for( bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++ ) {
				for(bl = mapdata->block[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next ) {
					if( bl->type&type
						&& bl->x >= x0 && bl->x <= x1 && bl->y >= y0 && bl->y <= y1
#ifdef CIRCULAR_AREA
						&& check_distance_bl(center, bl, range)
#endif
						&& ( !wall_check || path_search_long(nullptr, center->m, center->x, center->y, bl->x, bl->y, CELL_CHKWALL) )
					  	&& bl_list_count < BL_LIST_MAX )
						bl_list[ bl_list_count++ ] = bl;
				}
			}
		}
	}

	if ( type&BL_MOB ) {
		for( by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++ ) {
			for( bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++ ) {
				for(bl = mapdata->block_mob[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next ) {
					if( bl->x >= x0 && bl->x <= x1 && bl->y >= y0 && bl->y <= y1
#ifdef CIRCULAR_AREA
						&& check_distance_bl(center, bl, range)
#endif
						&& ( !wall_check || path_search_long(nullptr, center->m, center->x, center->y, bl->x, bl->y, CELL_CHKWALL) )
					  	&& bl_list_count < BL_LIST_MAX )
						bl_list[ bl_list_count++ ] = bl;
				}
			}
		}
	}

	if( bl_list_count >= BL_LIST_MAX )
		ShowWarning("map_foreachinrange: block count too many!\n");

	map_freeblock_lock();

	for( i = blockcount; i < bl_list_count; i++ ) {
		if( bl_list[ i ]->prev ) { //func() may delete this bl_list[] slot, checking for prev ensures it wasn't queued for deletion.
			va_copy(ap_copy, ap);
			returnCount += func(bl_list[i], ap_copy);
			va_end(ap_copy);
		}
	}

	map_freeblock_unlock();

	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]
}

int32 map_foreachinrange(int32 (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int32 type, ...)
{
	int32 returnCount = 0;
	va_list ap;
 	va_start(ap,type);
	returnCount = map_foreachinrangeV(func,center,range,type,ap,battle_config.skill_wall_check>0);
 	va_end(ap);
	return returnCount;
}

int32 map_foreachinallrange(int32 (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int32 type, ...)
{
	int32 returnCount = 0;
	va_list ap;
 	va_start(ap,type);
	returnCount = map_foreachinrangeV(func,center,range,type,ap,false);
 	va_end(ap);
	return returnCount;
}

/*==========================================
 * Same as foreachinrange, but there must be a shoot-able range between center and target to be counted in. [Skotlex]
 *------------------------------------------*/
int32 map_foreachinshootrange(int32 (*func)(struct block_list*,va_list),struct block_list* center, int16 range, int32 type,...)
{
	int32 returnCount = 0;
	va_list ap;
 	va_start(ap,type);
	returnCount = map_foreachinrangeV(func,center,range,type,ap,true);
 	va_end(ap);
	return returnCount;
}


/*========================================== [Playtester]
 * range = map m (x0,y0)-(x1,y1)
 * Apply *func with ... arguments for the range.
 * @param m: ID of map
 * @param x0: West end of area
 * @param y0: South end of area
 * @param x1: East end of area
 * @param y1: North end of area
 * @param type: Type of bl to search for
*------------------------------------------*/
int32 map_foreachinareaV(int32 (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int32 type, va_list ap, bool wall_check)
{
	int32 bx, by, cx, cy;
	int32 returnCount = 0;	//total sum of returned values of func()
	struct block_list *bl;
	int32 blockcount = bl_list_count, i;
	va_list ap_copy;

	if (m < 0)
		return 0;

	if (x1 < x0)
		std::swap(x0, x1);
	if (y1 < y0)
		std::swap(y0, y1);

	struct map_data *mapdata = map_getmapdata(m);

	if( mapdata == nullptr || mapdata->block == nullptr ){
		return 0;
	}

	x0 = i16max(x0, 0);
	y0 = i16max(y0, 0);
	x1 = i16min(x1, mapdata->xs - 1);
	y1 = i16min(y1, mapdata->ys - 1);

	if( wall_check ) {
		cx = x0 + (x1 - x0) / 2;
		cy = y0 + (y1 - y0) / 2;
	}

	if( type&~BL_MOB ) {
		for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
			for (bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++) {
				for(bl = mapdata->block[bx + by * mapdata->bxs]; bl != nullptr; bl = bl->next) {
					if ( bl->type&type
						&& bl->x >= x0 && bl->x <= x1 && bl->y >= y0 && bl->y <= y1
						&& ( !wall_check || path_search_long(nullptr, m, cx, cy, bl->x, bl->y, CELL_CHKWALL) )
						&& bl_list_count < BL_LIST_MAX )
						bl_list[bl_list_count++] = bl;
				}
			}
		}
	}

	if( type&BL_MOB ) {
		for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
			for (bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++) {
				for(bl = mapdata->block_mob[bx + by * mapdata->bxs]; bl != nullptr; bl = bl->next) {
					if ( bl->x >= x0 && bl->x <= x1 && bl->y >= y0 && bl->y <= y1
						&& ( !wall_check || path_search_long(nullptr, m, cx, cy, bl->x, bl->y, CELL_CHKWALL) )
						&& bl_list_count < BL_LIST_MAX )
						bl_list[bl_list_count++] = bl;
				}
			}
		}
	}

	if (bl_list_count >= BL_LIST_MAX)
		ShowWarning("map_foreachinarea: block count too many!\n");

	map_freeblock_lock();

	for (i = blockcount; i < bl_list_count; i++) {
		if (bl_list[i]->prev) { //func() may delete this bl_list[] slot, checking for prev ensures it wasn't queued for deletion.
			va_copy(ap_copy, ap);
			returnCount += func(bl_list[i], ap_copy);
			va_end(ap_copy);
		}
	}

	map_freeblock_unlock();

	bl_list_count = blockcount;
	return returnCount;
}

int32 map_foreachinallarea(int32 (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int32 type, ...)
{
	int32 returnCount = 0;
	va_list ap;
 	va_start(ap,type);
	returnCount = map_foreachinareaV(func,m,x0,y0,x1,y1,type,ap,false);
 	va_end(ap);
	return returnCount;
}

int32 map_foreachinshootarea(int32 (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int32 type, ...)
{
	int32 returnCount = 0;
	va_list ap;
 	va_start(ap,type);
	returnCount = map_foreachinareaV(func,m,x0,y0,x1,y1,type,ap,true);
 	va_end(ap);
	return returnCount;
}
int32 map_foreachinarea(int32 (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int32 type, ...)
{
	int32 returnCount = 0;
	va_list ap;
 	va_start(ap,type);
	returnCount = map_foreachinareaV(func,m,x0,y0,x1,y1,type,ap,battle_config.skill_wall_check>0);
 	va_end(ap);
	return returnCount;
}

/*==========================================
 * Adapted from forcountinarea for an easier invocation. [pakpil]
 *------------------------------------------*/
int32 map_forcountinrange(int32 (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int32 count, int32 type, ...)
{
	int32 bx, by, m;
	int32 returnCount = 0;	//total sum of returned values of func() [Skotlex]
	struct block_list *bl;
	int32 blockcount = bl_list_count, i;
	int32 x0, x1, y0, y1;
	struct map_data *mapdata;
	va_list ap;

	m = center->m;
	mapdata = map_getmapdata(m);

	if( mapdata == nullptr || mapdata->block == nullptr ){
		return 0;
	}

	x0 = i16max(center->x - range, 0);
	y0 = i16max(center->y - range, 0);
	x1 = i16min(center->x + range, mapdata->xs - 1);
	y1 = i16min(center->y + range, mapdata->ys - 1);

	if ( type&~BL_MOB )
		for ( by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++ ) {
			for( bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++ ) {
				for( bl = mapdata->block[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next ) {
					if( bl->type&type
						&& bl->x >= x0 && bl->x <= x1 && bl->y >= y0 && bl->y <= y1
#ifdef CIRCULAR_AREA
						&& check_distance_bl(center, bl, range)
#endif
					  	&& bl_list_count < BL_LIST_MAX )
						bl_list[ bl_list_count++ ] = bl;
				}
			}
		}
	if( type&BL_MOB )
		for( by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++ ) {
			for( bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++ ){
				for( bl = mapdata->block_mob[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next ) {
					if( bl->x >= x0 && bl->x <= x1 && bl->y >= y0 && bl->y <= y1
#ifdef CIRCULAR_AREA
						&& check_distance_bl(center, bl, range)
#endif
						&& bl_list_count < BL_LIST_MAX )
						bl_list[ bl_list_count++ ] = bl;
				}
			}
		}

	if( bl_list_count >= BL_LIST_MAX )
		ShowWarning("map_forcountinrange: block count too many!\n");

	map_freeblock_lock();

	for( i = blockcount; i < bl_list_count; i++ )
		if( bl_list[ i ]->prev ) { //func() may delete this bl_list[] slot, checking for prev ensures it wasn't queued for deletion.
			va_start(ap, type);
			returnCount += func(bl_list[ i ], ap);
			va_end(ap);
			if( count && returnCount >= count )
				break;
		}

	map_freeblock_unlock();

	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]
}
int32 map_forcountinarea(int32 (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int32 count, int32 type, ...)
{
	int32 bx, by;
	int32 returnCount = 0;	//total sum of returned values of func() [Skotlex]
	struct block_list *bl;
	int32 blockcount = bl_list_count, i;
	va_list ap;

	if ( m < 0 )
		return 0;

	if ( x1 < x0 )
		std::swap(x0, x1);
	if ( y1 < y0 )
		std::swap(y0, y1);

	struct map_data *mapdata = map_getmapdata(m);

	if( mapdata == nullptr || mapdata->block == nullptr ){
		return 0;
	}

	x0 = i16max(x0, 0);
	y0 = i16max(y0, 0);
	x1 = i16min(x1, mapdata->xs - 1);
	y1 = i16min(y1, mapdata->ys - 1);

	if ( type&~BL_MOB )
		for( by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++ )
			for( bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++ )
				for( bl = mapdata->block[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next )
					if( bl->type&type && bl->x >= x0 && bl->x <= x1 && bl->y >= y0 && bl->y <= y1 && bl_list_count < BL_LIST_MAX )
						bl_list[ bl_list_count++ ] = bl;

	if( type&BL_MOB )
		for( by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++ )
			for( bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++ )
				for( bl = mapdata->block_mob[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next )
					if( bl->x >= x0 && bl->x <= x1 && bl->y >= y0 && bl->y <= y1 && bl_list_count < BL_LIST_MAX )
						bl_list[ bl_list_count++ ] = bl;

	if( bl_list_count >= BL_LIST_MAX )
		ShowWarning("map_forcountinarea: block count too many!\n");

	map_freeblock_lock();

	for( i = blockcount; i < bl_list_count; i++ )
		if(bl_list[ i ]->prev) { //func() may delete this bl_list[] slot, checking for prev ensures it wasn't queued for deletion.
			va_start(ap, type);
			returnCount += func(bl_list[ i ], ap);
			va_end(ap);
			if( count && returnCount >= count )
				break;
		}

	map_freeblock_unlock();

	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]
}

/*==========================================
 * Move bl and do func* with va_list while moving.
 * Movement is set by dx dy which are distance in x and y
 *------------------------------------------*/
int32 map_foreachinmovearea(int32 (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int32 type, ...)
{
	int32 bx, by, m;
	int32 returnCount = 0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl;
	int32 blockcount = bl_list_count, i;
	int16 x0, x1, y0, y1;
	va_list ap;

	if ( !range ) return 0;
	if ( !dx && !dy ) return 0; //No movement.

	m = center->m;

	struct map_data *mapdata = map_getmapdata(m);

	if( mapdata == nullptr || mapdata->block == nullptr ){
		return 0;
	}

	x0 = center->x - range;
	x1 = center->x + range;
	y0 = center->y - range;
	y1 = center->y + range;

	if ( x1 < x0 )
		std::swap(x0, x1);
	if ( y1 < y0 )
		std::swap(y0, y1);

	if( dx == 0 || dy == 0 ) {
		//Movement along one axis only.
		if( dx == 0 ){
			if( dy < 0 ) //Moving south
				y0 = y1 + dy + 1;
			else //North
				y1 = y0 + dy - 1;
		} else { //dy == 0
			if( dx < 0 ) //West
				x0 = x1 + dx + 1;
			else //East
				x1 = x0 + dx - 1;
		}

		x0 = i16max(x0, 0);
		y0 = i16max(y0, 0);
		x1 = i16min(x1, mapdata->xs - 1);
		y1 = i16min(y1, mapdata->ys - 1);

		for( by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++ ) {
			for( bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++ ) {
				if ( type&~BL_MOB ) {
					for( bl = mapdata->block[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next ) {
						if( bl->type&type &&
							bl->x >= x0 && bl->x <= x1 &&
							bl->y >= y0 && bl->y <= y1 &&
							bl_list_count < BL_LIST_MAX )
							bl_list[ bl_list_count++ ] = bl;
					}
				}
				if ( type&BL_MOB ) {
					for( bl = mapdata->block_mob[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next ) {
						if( bl->x >= x0 && bl->x <= x1 &&
							bl->y >= y0 && bl->y <= y1 &&
							bl_list_count < BL_LIST_MAX )
							bl_list[ bl_list_count++ ] = bl;
					}
				}
			}
		}
	} else { // Diagonal movement
		x0 = i16max(x0, 0);
		y0 = i16max(y0, 0);
		x1 = i16min(x1, mapdata->xs - 1);
		y1 = i16min(y1, mapdata->ys - 1);

		for( by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++ ) {
			for( bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++ ) {
				if ( type & ~BL_MOB ) {
					for( bl = mapdata->block[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next ) {
						if( bl->type&type &&
							bl->x >= x0 && bl->x <= x1 &&
							bl->y >= y0 && bl->y <= y1 &&
							bl_list_count < BL_LIST_MAX )
						if( ( dx > 0 && bl->x < x0 + dx) ||
							( dx < 0 && bl->x > x1 + dx) ||
							( dy > 0 && bl->y < y0 + dy) ||
							( dy < 0 && bl->y > y1 + dy) )
							bl_list[ bl_list_count++ ] = bl;
					}
				}
				if ( type&BL_MOB ) {
					for( bl = mapdata->block_mob[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next ) {
						if( bl->x >= x0 && bl->x <= x1 &&
							bl->y >= y0 && bl->y <= y1 &&
							bl_list_count < BL_LIST_MAX)
						if( ( dx > 0 && bl->x < x0 + dx) ||
							( dx < 0 && bl->x > x1 + dx) ||
							( dy > 0 && bl->y < y0 + dy) ||
							( dy < 0 && bl->y > y1 + dy) )
							bl_list[ bl_list_count++ ] = bl;
					}
				}
			}
		}

	}

	if( bl_list_count >= BL_LIST_MAX )
		ShowWarning("map_foreachinmovearea: block count too many!\n");

	map_freeblock_lock();	// Prohibit the release from memory

	for( i = blockcount; i < bl_list_count; i++ )
		if( bl_list[ i ]->prev ) { //func() may delete this bl_list[] slot, checking for prev ensures it wasn't queued for deletion.
			va_start(ap, type);
			returnCount += func(bl_list[ i ], ap);
			va_end(ap);
		}

	map_freeblock_unlock();	// Allow Free

	bl_list_count = blockcount;
	return returnCount;
}

// -- moonsoul	(added map_foreachincell which is a rework of map_foreachinarea but
//			 which only checks the exact single x/y passed to it rather than an
//			 area radius - may be more useful in some instances)
//
int32 map_foreachincell(int32 (*func)(struct block_list*,va_list), int16 m, int16 x, int16 y, int32 type, ...)
{
	int32 bx, by;
	int32 returnCount = 0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl;
	int32 blockcount = bl_list_count, i;
	struct map_data *mapdata = map_getmapdata(m);
	va_list ap;

	if( mapdata == nullptr || mapdata->block == nullptr ){
		return 0;
	}

	if ( x < 0 || y < 0 || x >= mapdata->xs || y >= mapdata->ys ) return 0;

	by = y / BLOCK_SIZE;
	bx = x / BLOCK_SIZE;

	if( type&~BL_MOB )
		for( bl = mapdata->block[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next )
			if( bl->type&type && bl->x == x && bl->y == y && bl_list_count < BL_LIST_MAX )
				bl_list[ bl_list_count++ ] = bl;
	if( type&BL_MOB )
		for( bl = mapdata->block_mob[ bx + by * mapdata->bxs]; bl != nullptr; bl = bl->next )
			if( bl->x == x && bl->y == y && bl_list_count < BL_LIST_MAX)
				bl_list[ bl_list_count++ ] = bl;

	if( bl_list_count >= BL_LIST_MAX )
		ShowWarning("map_foreachincell: block count too many!\n");

	map_freeblock_lock();

	for( i = blockcount; i < bl_list_count; i++ )
		if( bl_list[ i ]->prev ) { //func() may delete this bl_list[] slot, checking for prev ensures it wasn't queued for deletion.
			va_start(ap, type);
			returnCount += func(bl_list[ i ], ap);
			va_end(ap);
		}

	map_freeblock_unlock();

	bl_list_count = blockcount;
	return returnCount;
}

/*============================================================
* For checking a path between two points (x0, y0) and (x1, y1)
*------------------------------------------------------------*/
int32 map_foreachinpath(int32 (*func)(struct block_list*,va_list),int16 m,int16 x0,int16 y0,int16 x1,int16 y1,int16 range,int32 length, int32 type,...)
{
	int32 returnCount = 0;  //total sum of returned values of func() [Skotlex]
//////////////////////////////////////////////////////////////
//
// sharp shooting 3 [Skotlex]
//
//////////////////////////////////////////////////////////////
// problem:
// Same as Sharp Shooting 1. Hits all targets within range of
// the line.
// (t1,t2 t3 and t4 get hit)
//
//     target 1
//      x t4
//     t2
// t3 x
//   x
//  S
//////////////////////////////////////////////////////////////
// Methodology:
// My trigonometrics and math are a little rusty... so the approach I am writing
// here is basically do a double for to check for all targets in the square that
// contains the initial and final positions (area range increased to match the
// radius given), then for each object to test, calculate the distance to the
// path and include it if the range fits and the target is in the line (0<k<1,
// as they call it).
// The implementation I took as reference is found at
// http://astronomy.swin.edu.au/~pbourke/geometry/pointline/
// (they have a link to a C implementation, too)
// This approach is a lot like #2 commented on this function, which I have no
// idea why it was commented. I won't use doubles/floats, but pure int32 math for
// speed purposes. The range considered is always the same no matter how
// close/far the target is because that's how SharpShooting works currently in
// kRO.

	//Generic map_foreach* variables.
	int32 i, blockcount = bl_list_count;
	struct block_list *bl;
	int32 bx, by;
	//method specific variables
	int32 magnitude2, len_limit; //The square of the magnitude
	int32 k, xi, yi, xu, yu;
	int32 mx0 = x0, mx1 = x1, my0 = y0, my1 = y1;
	va_list ap;

	//Avoid needless calculations by not getting the sqrt right away.
	#define MAGNITUDE2(x0, y0, x1, y1) ( ( ( x1 ) - ( x0 ) ) * ( ( x1 ) - ( x0 ) ) + ( ( y1 ) - ( y0 ) ) * ( ( y1 ) - ( y0 ) ) )

	if ( m < 0 )
		return 0;

	len_limit = magnitude2 = MAGNITUDE2(x0, y0, x1, y1);
	if ( magnitude2 < 1 ) //Same begin and ending point, can't trace path.
		return 0;

	if ( length ) { //Adjust final position to fit in the given area.
		//TODO: Find an alternate method which does not requires a square root calculation.
		k = (int32)sqrt((float)magnitude2);
		mx1 = x0 + (x1 - x0) * length / k;
		my1 = y0 + (y1 - y0) * length / k;
		len_limit = MAGNITUDE2(x0, y0, mx1, my1);
	}
	//Expand target area to cover range.
	if ( mx0 > mx1 ) {
		mx0 += range;
		mx1 -= range;
	} else {
		mx0 -= range;
		mx1 += range;
	}
	if (my0 > my1) {
		my0 += range;
		my1 -= range;
	} else {
		my0 -= range;
		my1 += range;
	}

	//The two fors assume mx0 < mx1 && my0 < my1
	if ( mx0 > mx1 )
		std::swap(mx0, mx1);
	if ( my0 > my1 )
		std::swap(my0, my1);

	struct map_data *mapdata = map_getmapdata(m);

	if( mapdata == nullptr || mapdata->block == nullptr ){
		return 0;
	}

	mx0 = max(mx0, 0);
	my0 = max(my0, 0);
	mx1 = min(mx1, mapdata->xs - 1);
	my1 = min(my1, mapdata->ys - 1);

	range *= range << 8; //Values are shifted later on for higher precision using int32 math.

	if ( type&~BL_MOB )
		for ( by = my0 / BLOCK_SIZE; by <= my1 / BLOCK_SIZE; by++ ) {
			for( bx = mx0 / BLOCK_SIZE; bx <= mx1 / BLOCK_SIZE; bx++ ) {
				for( bl = mapdata->block[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next ) {
					if( bl->prev && bl->type&type && bl_list_count < BL_LIST_MAX ) {
						xi = bl->x;
						yi = bl->y;

						k = ( xi - x0 ) * ( x1 - x0 ) + ( yi - y0 ) * ( y1 - y0 );

						if ( k < 0 || k > len_limit ) //Since more skills use this, check for ending point as well.
							continue;

						if ( k > magnitude2 && !path_search_long(nullptr, m, x0, y0, xi, yi, CELL_CHKWALL) )
							continue; //Targets beyond the initial ending point need the wall check.

						//All these shifts are to increase the precision of the intersection point and distance considering how it's
						//int32 math.
						k  = ( k << 4 ) / magnitude2; //k will be between 1~16 instead of 0~1
						xi <<= 4;
						yi <<= 4;
						xu = ( x0 << 4 ) + k * ( x1 - x0 );
						yu = ( y0 << 4 ) + k * ( y1 - y0 );
						k  = MAGNITUDE2(xi, yi, xu, yu);

						//If all dot coordinates were <<4 the square of the magnitude is <<8
						if ( k > range )
							continue;

						bl_list[ bl_list_count++ ] = bl;
					}
				}
			}
		}
	 if( type&BL_MOB )
		for( by = my0 / BLOCK_SIZE; by <= my1 / BLOCK_SIZE; by++ ) {
			for( bx = mx0 / BLOCK_SIZE; bx <= mx1 / BLOCK_SIZE; bx++ ) {
				for( bl = mapdata->block_mob[ bx + by * mapdata->bxs ]; bl != nullptr; bl = bl->next ) {
					if( bl->prev && bl_list_count < BL_LIST_MAX ) {
						xi = bl->x;
						yi = bl->y;
						k = ( xi - x0 ) * ( x1 - x0 ) + ( yi - y0 ) * ( y1 - y0 );

						if ( k < 0 || k > len_limit )
							continue;

						if ( k > magnitude2 && !path_search_long(nullptr, m, x0, y0, xi, yi, CELL_CHKWALL) )
							continue; //Targets beyond the initial ending point need the wall check.

						k  = ( k << 4 ) / magnitude2; //k will be between 1~16 instead of 0~1
						xi <<= 4;
						yi <<= 4;
						xu = ( x0 << 4 ) + k * ( x1 - x0 );
						yu = ( y0 << 4 ) + k * ( y1 - y0 );
						k  = MAGNITUDE2(xi, yi, xu, yu);

						//If all dot coordinates were <<4 the square of the magnitude is <<8
						if ( k > range )
							continue;

						bl_list[ bl_list_count++ ] = bl;
					}
				}
			}
		}

	if( bl_list_count >= BL_LIST_MAX )
		ShowWarning("map_foreachinpath: block count too many!\n");

	map_freeblock_lock();

	for( i = blockcount; i < bl_list_count; i++ )
		if( bl_list[ i ]->prev ) { //func() may delete this bl_list[] slot, checking for prev ensures it wasn't queued for deletion.
			va_start(ap, type);
			returnCount += func(bl_list[ i ], ap);
			va_end(ap);
		}

	map_freeblock_unlock();

	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]

}

/*========================================== [Playtester]
* Calls the given function for every object of a type that is on a path.
* The path goes into one of the eight directions and the direction is determined by the given coordinates.
* The path has a length, a width and an offset.
* The cost for diagonal movement is the same as for horizontal/vertical movement.
* @param m: ID of map
* @param x0: Start X
* @param y0: Start Y
* @param x1: X to calculate direction against
* @param y1: Y to calculate direction against
* @param range: Determines width of the path (width = range*2+1 cells)
* @param length: Length of the path
* @param offset: Moves the whole path, half-length for diagonal paths
* @param type: Type of bl to search for
*------------------------------------------*/
int32 map_foreachindir(int32 (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int32 length, int32 offset, int32 type, ...)
{
	int32 returnCount = 0;  //Total sum of returned values of func()

	int32 i, blockcount = bl_list_count;
	struct block_list *bl;
	int32 bx, by;
	int32 mx0, mx1, my0, my1, rx, ry;
	uint8 dir = map_calc_dir_xy( x0, y0, x1, y1, DIR_EAST );
	int16 dx = dirx[dir];
	int16 dy = diry[dir];
	va_list ap;

	if (m < 0)
		return 0;

	if (range < 0)
		return 0;
	if (length < 1)
		return 0;
	if (offset < 0)
		return 0;

	//Special offset handling for diagonal paths
	if( offset && direction_diagonal( (directions)dir ) ){
		//So that diagonal paths can attach to each other, we have to work with half-tile offsets
		offset = (2 * offset) - 1;
		//To get the half-tiles we need to increase length by one
		length++;
	}

	struct map_data *mapdata = map_getmapdata(m);

	if( mapdata == nullptr || mapdata->block == nullptr ){
		return 0;
	}

	//Get area that needs to be checked
	mx0 = x0 + dx*(offset / ((dir % 2) + 1));
	my0 = y0 + dy*(offset / ((dir % 2) + 1));
	mx1 = x0 + dx*(offset / ((dir % 2) + 1) + length - 1);
	my1 = y0 + dy*(offset / ((dir % 2) + 1) + length - 1);

	//The following assumes mx0 < mx1 && my0 < my1
	if (mx0 > mx1)
		std::swap(mx0, mx1);
	if (my0 > my1)
		std::swap(my0, my1);

	//Apply width to the path by turning 90 degrees
	mx0 -= abs( range * dirx[( dir + 2 ) % DIR_MAX] );
	my0 -= abs( range * diry[( dir + 2 ) % DIR_MAX] );
	mx1 += abs( range * dirx[( dir + 2 ) % DIR_MAX] );
	my1 += abs( range * diry[( dir + 2 ) % DIR_MAX] );

	mx0 = max(mx0, 0);
	my0 = max(my0, 0);
	mx1 = min(mx1, mapdata->xs - 1);
	my1 = min(my1, mapdata->ys - 1);

	if (type&~BL_MOB) {
		for (by = my0 / BLOCK_SIZE; by <= my1 / BLOCK_SIZE; by++) {
			for (bx = mx0 / BLOCK_SIZE; bx <= mx1 / BLOCK_SIZE; bx++) {
				for (bl = mapdata->block[bx + by * mapdata->bxs]; bl != nullptr; bl = bl->next) {
					if (bl->prev && bl->type&type && bl_list_count < BL_LIST_MAX) {
						//Check if inside search area
						if (bl->x < mx0 || bl->x > mx1 || bl->y < my0 || bl->y > my1)
							continue;
						//What matters now is the relative x and y from the start point
						rx = (bl->x - x0);
						ry = (bl->y - y0);
						//Do not hit source cell
						if (battle_config.skill_eightpath_same_cell == 0 && rx == 0 && ry == 0)
							continue;
						//This turns it so that the area that is hit is always with positive rx and ry
						rx *= dx;
						ry *= dy;
						//These checks only need to be done for diagonal paths
						if( direction_diagonal( (directions)dir ) ){
							//Check for length
							if ((rx + ry < offset) || (rx + ry > 2 * (length + (offset/2) - 1)))
								continue;
							//Check for width
							if (abs(rx - ry) > 2 * range)
								continue;
						}
						//Everything else ok, check for line of sight from source
						if (!path_search_long(nullptr, m, x0, y0, bl->x, bl->y, CELL_CHKWALL))
							continue;
						//All checks passed, add to list
						bl_list[bl_list_count++] = bl;
					}
				}
			}
		}
	}
	if (type&BL_MOB) {
		for (by = my0 / BLOCK_SIZE; by <= my1 / BLOCK_SIZE; by++) {
			for (bx = mx0 / BLOCK_SIZE; bx <= mx1 / BLOCK_SIZE; bx++) {
				for (bl = mapdata->block_mob[bx + by * mapdata->bxs]; bl != nullptr; bl = bl->next) {
					if (bl->prev && bl_list_count < BL_LIST_MAX) {
						//Check if inside search area
						if (bl->x < mx0 || bl->x > mx1 || bl->y < my0 || bl->y > my1)
							continue;
						//What matters now is the relative x and y from the start point
						rx = (bl->x - x0);
						ry = (bl->y - y0);
						//Do not hit source cell
						if (battle_config.skill_eightpath_same_cell == 0 && rx == 0 && ry == 0)
							continue;
						//This turns it so that the area that is hit is always with positive rx and ry
						rx *= dx;
						ry *= dy;
						//These checks only need to be done for diagonal paths
						if( direction_diagonal( (directions)dir ) ){
							//Check for length
							if ((rx + ry < offset) || (rx + ry > 2 * (length + (offset / 2) - 1)))
								continue;
							//Check for width
							if (abs(rx - ry) > 2 * range)
								continue;
						}
						//Everything else ok, check for line of sight from source
						if (!path_search_long(nullptr, m, x0, y0, bl->x, bl->y, CELL_CHKWALL))
							continue;
						//All checks passed, add to list
						bl_list[bl_list_count++] = bl;
					}
				}
			}
		}
	}

	if( bl_list_count >= BL_LIST_MAX )
		ShowWarning("map_foreachindir: block count too many!\n");

	map_freeblock_lock();

	for( i = blockcount; i < bl_list_count; i++ )
		if( bl_list[ i ]->prev ) { //func() may delete this bl_list[] slot, checking for prev ensures it wasn't queued for deletion.
			va_start(ap, type);
			returnCount += func(bl_list[ i ], ap);
			va_end(ap);
		}

	map_freeblock_unlock();

	bl_list_count = blockcount;
	return returnCount;
}

// Copy of map_foreachincell, but applied to the whole map. [Skotlex]
int32 map_foreachinmap(int32 (*func)(struct block_list*,va_list), int16 m, int32 type,...)
{
	int32 b, bsize;
	int32 returnCount = 0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl;
	int32 blockcount = bl_list_count, i;
	struct map_data *mapdata = map_getmapdata(m);
	va_list ap;

	if( mapdata == nullptr || mapdata->block == nullptr ){
		return 0;
	}

	bsize = mapdata->bxs * mapdata->bys;

	if( type&~BL_MOB )
		for( b = 0; b < bsize; b++ )
			for( bl = mapdata->block[ b ]; bl != nullptr; bl = bl->next )
				if( bl->type&type && bl_list_count < BL_LIST_MAX )
					bl_list[ bl_list_count++ ] = bl;

	if( type&BL_MOB )
		for( b = 0; b < bsize; b++ )
			for( bl = mapdata->block_mob[ b ]; bl != nullptr; bl = bl->next )
				if( bl_list_count < BL_LIST_MAX )
					bl_list[ bl_list_count++ ] = bl;

	if( bl_list_count >= BL_LIST_MAX )
		ShowWarning("map_foreachinmap: block count too many!\n");

	map_freeblock_lock();

	for( i = blockcount; i < bl_list_count ; i++ )
		if( bl_list[ i ]->prev ) { //func() may delete this bl_list[] slot, checking for prev ensures it wasn't queued for deletion.
			va_start(ap, type);
			returnCount += func(bl_list[ i ], ap);
			va_end(ap);
		}

	map_freeblock_unlock();

	bl_list_count = blockcount;
	return returnCount;
}


/// Generates a new flooritem object id from the interval [MIN_FLOORITEM, MAX_FLOORITEM).
/// Used for floor items, skill units and chatroom objects.
/// @return The new object id
int32 map_get_new_object_id(void)
{
	static int32 last_object_id = MIN_FLOORITEM - 1;
	int32 i;

	// find a free id
	i = last_object_id + 1;
	while( i != last_object_id ) {
		if( i == MAX_FLOORITEM )
			i = MIN_FLOORITEM;

		if( !idb_exists(id_db, i) )
			break;

		++i;
	}

	if( i == last_object_id ) {
		ShowError("map_addobject: no free object id!\n");
		return 0;
	}

	// update cursor
	last_object_id = i;

	return i;
}

/*==========================================
 * Timered function to clear the floor (remove remaining item)
 * Called each flooritem_lifetime ms
 *------------------------------------------*/
TIMER_FUNC(map_clearflooritem_timer){
	struct flooritem_data* fitem = (struct flooritem_data*)idb_get(id_db, id);

	if (fitem == nullptr || fitem->type != BL_ITEM || (fitem->cleartimer != tid)) {
		ShowError("map_clearflooritem_timer : error\n");
		return 1;
	}


	if (pet_db_search(fitem->item.nameid, PET_EGG))
		intif_delete_petdata(MakeDWord(fitem->item.card[1], fitem->item.card[2]));

	clif_clearflooritem( *fitem );
	map_deliddb(fitem);
	map_delblock(fitem);
	map_freeblock(fitem);
	return 0;
}

/*
 * clears a single bl item out of the map.
 */
void map_clearflooritem(struct block_list *bl) {
	struct flooritem_data* fitem = (struct flooritem_data*)bl;

	if( fitem->cleartimer != INVALID_TIMER )
		delete_timer(fitem->cleartimer,map_clearflooritem_timer);

	clif_clearflooritem( *fitem );
	map_deliddb(fitem);
	map_delblock(fitem);
	map_freeblock(fitem);
}

/**
 * Returns if a cell is passable and not occupied by given types of block
 * Cells 5 cells from the SW edge and 4 cells from the NE edge are never considered as free
 * @param m: Map of cell to check
 * @param x: X-coordinate of cell to check
 * @param y: Y-coordinate of cell to check
 * @param type: Types of block to check for
 * @return True if cell is passable, not on the edge and not occupied by given types of block
 */
bool map_cell_free(int16 m, int16 x, int16 y, int32 type)
{
	struct map_data* mapdata = map_getmapdata(m);
	if (mapdata == nullptr || mapdata->block == nullptr) {
		return false;
	}

	// Cells outside the map or within 4-5 cells of the map edge are considered invalid officially
	// Note that this isn't symmetric (NE - 4 cells, SW - 5 cells)
	// If for some reason edge size was set to below 5 cells, we consider them as valid
	int16 edge_valid = std::min(battle_config.map_edge_size, 5);
	if (x < edge_valid || x > mapdata->xs - edge_valid || y < edge_valid || y > mapdata->ys - edge_valid)
		return false;
	if (map_getcell(m, x, y, CELL_CHKNOPASS))
		return false;
	if (map_count_oncell(m, x, y, type, 0) > 0)
		return false;

	return true;
}

/**
 * Locates a random available free cell around the given coordinates within a given distance range.
 * This uses the official algorithm that checks each quadrant and line once.
 * x and y are modified with the target free cell when successful.
 * @param m: Map to search
 * @param x: X-coordinate around which free cell is searched
 * @param y: Y-coordinate around which free cell is searched
 * @param distmin: Minimum distance from the given cell
 * @param distmax: Maximum distance from the given cell
 * @param type: If the given types of block are present on the cell, it counts as occupied
 * @return True if free cell could be found
 */
bool map_search_freecell_dist(int16 m, int16* x, int16* y, int16 distmin, int16 distmax, int32 type)
{
	// This is to prevent that always the same quadrant is checked first 
	int16 mirrorx = (rnd()%2) ? -1 : 1;
	int16 mirrory = (rnd()%2) ? -1 : 1;

	for (int16 i = -1; i <= 1; i++) {
		for (int16 j = -1; j <= 1; j++) {
			if (i || j)
			{
				int16 checkX = *x + mirrorx * i * rnd_value(distmin, distmax);
				int16 checkY = *y + mirrory * j * rnd_value(distmin, distmax);
				if (map_cell_free(m, checkX, checkY, type))
				{
					*x = checkX;
					*y = checkY;
					return true;
				}
			}
		}
	}
	return false;
}

static int32 map_count_sub(struct block_list *bl,va_list ap)
{
	return 1;
}

/*==========================================
 * Locates a random spare cell around the object given, using range as max
 * distance from that spot. Used for warping functions. Use range < 0 for
 * whole map range.
 * Returns 1 on success. when it fails and src is available, x/y are set to src's
 * src can be null as long as flag&1
 * when ~flag&1, m is not needed.
 * Flag values:
 * &1 = random cell must be around given m,x,y, not around src
 * &2 = the target should be able to walk to the target tile.
 * &4 = there shouldn't be any players around the target tile (use the no_spawn_on_player setting)
 *------------------------------------------*/
int32 map_search_freecell(struct block_list *src, int16 m, int16 *x, int16 *y, int16 rx, int16 ry, int32 flag, int32 tries)
{
	int32 spawn=0;
	int32 bx, by;

	if( !src && (!(flag&1) || flag&2) )
	{
		ShowDebug("map_search_freecell: Incorrect usage! When src is nullptr, flag has to be &1 and can't have &2\n");
		return 0;
	}

	if (flag&1) {
		bx = *x;
		by = *y;
	} else {
		bx = src->x;
		by = src->y;
		m = src->m;
	}
	if (!rx && !ry) {
		//No range? Return the target cell then....
		*x = bx;
		*y = by;
		return map_getcell(m,*x,*y,CELL_CHKREACH);
	}

	struct map_data *mapdata = map_getmapdata(m);

	if( mapdata == nullptr || mapdata->block == nullptr ){
		return 0;
	}

	int16 edge = battle_config.map_edge_size;
	int16 edge_valid = std::min(edge, (int16)5);
	// In most situations there are 50 tries officially (default value)
	while(tries--) {
		// For map-wide search, the configured tiles from the edge are not considered (default: 15)
		*x = (rx >= 0) ? rnd_value(bx - rx, bx + rx) : rnd_value<int16>(edge, mapdata->xs - edge - 1);
		*y = (ry >= 0) ? rnd_value(by - ry, by + ry) : rnd_value<int16>(edge, mapdata->ys - edge - 1);

		if (*x == bx && *y == by)
			continue; //Avoid picking the same target tile.

		// Cells outside the map or within 4-5 cells of the map edge are considered invalid officially
		// Note that unlike the other edge, this isn't symmetric (NE - 4 cells, SW - 5 cells)
		// If for some reason edge size was set to below 5 cells, we consider them as valid
		if (*x < edge_valid || *x > mapdata->xs - edge_valid || *y < edge_valid || *y > mapdata->ys - edge_valid)
			continue;

		if (map_getcell(m,*x,*y,CELL_CHKREACH))
		{
			if(flag&2 && !unit_can_reach_pos(src, *x, *y, 1))
				continue;
			if(flag&4) {
				if (spawn >= 100) return 0; //Limit of retries reached.
				if (spawn++ < battle_config.no_spawn_on_player &&
					map_foreachinallarea(map_count_sub, m,
						*x - AREA_SIZE, *y - AREA_SIZE,
						*x + AREA_SIZE, *y + AREA_SIZE, BL_PC)) {
					tries++; // This failure should not affect the number of official tries
					continue;
				}
			}
			return 1;
		}
	}
	*x = bx;
	*y = by;
	return 0;
}

/*==========================================
 * Locates the closest, walkable cell with no blocks of a certain type on it
 * Returns true on success and sets x and y to cell found.
 * Otherwise returns false and x and y are not changed.
 * type: Types of block to count
 * flag: 
 *		0x1 - only count standing units
 *------------------------------------------*/
bool map_closest_freecell(int16 m, int16 *x, int16 *y, int32 type, int32 flag)
{
	uint8 dir = DIR_EAST;
	int16 tx = *x;
	int16 ty = *y;
	int32 costrange = 10;

	if(!map_count_oncell(m, tx, ty, type, flag))
		return true; //Current cell is free

	//Algorithm only works up to costrange of 34
	while(costrange <= 34) {
		int16 dx = dirx[dir];
		int16 dy = diry[dir];

		//Linear search
		if( !direction_diagonal( (directions)dir ) && costrange % MOVE_COST == 0 ){
			tx = *x+dx*(costrange/MOVE_COST);
			ty = *y+dy*(costrange/MOVE_COST);
			if(!map_count_oncell(m, tx, ty, type, flag) && map_getcell(m,tx,ty,CELL_CHKPASS)) {
				*x = tx;
				*y = ty;
				return true;
			}
		} 
		//Full diagonal search
		else if( direction_diagonal( (directions)dir ) && costrange % MOVE_DIAGONAL_COST == 0 ){
			tx = *x+dx*(costrange/MOVE_DIAGONAL_COST);
			ty = *y+dy*(costrange/MOVE_DIAGONAL_COST);
			if(!map_count_oncell(m, tx, ty, type, flag) && map_getcell(m,tx,ty,CELL_CHKPASS)) {
				*x = tx;
				*y = ty;
				return true;
			}
		}
		//One cell diagonal, rest linear (TODO: Find a better algorithm for this)
		else if( direction_diagonal( (directions)dir ) && costrange % MOVE_COST == 4 ){
			tx = *x+dx*((dir%4==3)?(costrange/MOVE_COST):1);
			ty = *y+dy*((dir%4==1)?(costrange/MOVE_COST):1);
			if(!map_count_oncell(m, tx, ty, type, flag) && map_getcell(m,tx,ty,CELL_CHKPASS)) {
				*x = tx;
				*y = ty;
				return true;
			}
			tx = *x+dx*((dir%4==1)?(costrange/MOVE_COST):1);
			ty = *y+dy*((dir%4==3)?(costrange/MOVE_COST):1);
			if(!map_count_oncell(m, tx, ty, type, flag) && map_getcell(m,tx,ty,CELL_CHKPASS)) {
				*x = tx;
				*y = ty;
				return true;
			}
		}

		//Get next direction
		if( dir == DIR_SOUTHEAST ){
			//Diagonal search complete, repeat with higher cost range
			if(costrange == 14) costrange += 6;
			else if(costrange == 28 || costrange >= 38) costrange += 2;
			else costrange += 4;
			dir = DIR_EAST;
		}else if( dir == DIR_SOUTH ){
			//Linear search complete, switch to diagonal directions
			dir = DIR_NORTHEAST;
		} else {
			dir = ( dir + 2 ) % DIR_MAX;
		}
	}

	return false;
}

/*==========================================
 * Locates a nearby, walkable cell with no blocks of a certain type on it
 * This one uses the official algorithm the find a free cell
 * Returns true on success and sets x and y to cell found.
 * Otherwise returns false and x and y are not changed.
 * type: Types of block to count
 * flag: 
 *		0x1 - only count standing units
 *------------------------------------------*/
bool map_nearby_freecell(int16 m, int16 &x, int16 &y, int32 type, int32 flag)
{
	int16 tx = x;
	int16 ty = y;

	if(!map_count_oncell(m, tx, ty, type, flag))
		return true; //Current cell is free

	// One of two possible orders of direction processing is used at random
	directions dir[2][DIR_MAX] = {
		{DIR_NORTHEAST, DIR_EAST, DIR_SOUTHEAST, DIR_SOUTH, DIR_NORTH, DIR_SOUTHWEST, DIR_WEST, DIR_NORTHWEST},
		{DIR_SOUTHWEST, DIR_WEST, DIR_NORTHWEST, DIR_NORTH, DIR_SOUTH, DIR_NORTHEAST, DIR_EAST, DIR_SOUTHEAST}
	};
	uint16 array_idx = rnd_value<decltype(array_idx)>(0, ARRAYLENGTH(dir) - 1);

	// Try each direction in the selected array in order
	for(uint8 dir_idx = 0; dir_idx < DIR_MAX; dir_idx++) {
		int16 dx = dirx[dir[array_idx][dir_idx]];
		int16 dy = diry[dir[array_idx][dir_idx]];

		tx = x + dx;
		ty = y + dy;
		if (!map_count_oncell(m, tx, ty, type, flag) && map_getcell(m, tx, ty, CELL_CHKPASS)) {
			x = tx;
			y = ty;
			return true;
		}
	}

	return false;
}

/*==========================================
 * Add an item in floor to location (m,x,y) and add restriction for those who could pickup later
 * NB : If charids are null their no restriction for pickup
 * @param item_data : item attributes
 * @param amount : items quantity
 * @param m : mapid
 * @param x : x coordinates
 * @param y : y coordinates
 * @param first_charid : 1st player that could loot the item (only charid that could loot for first_get_tick duration)
 * @param second_charid :  2nd player that could loot the item (2nd charid that could loot for second_get_charid duration)
 * @param third_charid : 3rd player that could loot the item (3rd charid that could loot for third_get_charid duration)
 * @param flag: &1 MVP item. &2 search free cell in 5x5 area instead of 3x3. &4 bypass droppable check.
 * @param mob_id: Monster ID if dropped by monster
 * @param canShowEffect: enable pillar effect on the dropped item (if set in the database)
 * @param dir: where the item should drop around the target (DIR_MAX: random cell around center)
 * @param type: types of block the item should not stack on
 * @return 0:failure, x:item_gid [MIN_FLOORITEM;MAX_FLOORITEM]==[2;START_ACCOUNT_NUM]
 *------------------------------------------*/
int32 map_addflooritem(struct item *item, int32 amount, int16 m, int16 x, int16 y, int32 first_charid, int32 second_charid, int32 third_charid, int32 flags, uint16 mob_id, bool canShowEffect, enum directions dir, int32 type)
{
	struct flooritem_data *fitem = nullptr;

	nullpo_ret(item);

	if (!(flags&4) && battle_config.item_onfloor && (itemdb_traderight(item->nameid).trade))
		return 0; //can't be dropped

	if (dir > DIR_CENTER && dir < DIR_MAX) {
		x += dirx[dir];
		y += diry[dir];
	}
	// If cell occupied and not center cell, drop item around the drop target cell
	if (dir == DIR_MAX || (dir != DIR_CENTER && !map_cell_free(m, x, y, type))) {
		if (!map_search_freecell_dist(m, &x, &y, 1, (flags&2)?2:1, type)) {
			// Only stop here if BL_ITEM shall not stack, otherwise drop on original target cell
			if (type&BL_ITEM)
				return 0;
		}
	}

	CREATE(fitem, struct flooritem_data, 1);
	fitem->type=BL_ITEM;
	fitem->prev = fitem->next = nullptr;
	fitem->m=m;
	fitem->x=x;
	fitem->y=y;
	fitem->id = map_get_new_object_id();
	if (fitem->id==0) {
		aFree(fitem);
		return 0;
	}

	// If item is flagged as MVP item or dropped by bosses, it is protected for longer
	bool extend_protection = (flags&1);
	if (!extend_protection && mob_id > 0) {
		// Boss and MVP drops both have prelonged loot protection
		if (auto mob = mob_db.find(mob_id); mob != nullptr && mob->get_bosstype() != BOSSTYPE_NONE)
			extend_protection = true;
	}
	fitem->first_get_charid = first_charid;
	fitem->second_get_charid = second_charid;
	fitem->third_get_charid = third_charid;
	if (extend_protection) {
		fitem->first_get_tick = gettick() + battle_config.mvp_item_first_get_time;
		fitem->second_get_tick = fitem->first_get_tick + battle_config.mvp_item_second_get_time;
		fitem->third_get_tick = fitem->second_get_tick + battle_config.mvp_item_third_get_time;
	}
	else {
		fitem->first_get_tick = gettick() + battle_config.item_first_get_time;
		fitem->second_get_tick = fitem->first_get_tick + battle_config.item_second_get_time;
		fitem->third_get_tick = fitem->second_get_tick + battle_config.item_third_get_time;
	}
	fitem->mob_id = mob_id;

	memcpy(&fitem->item,item,sizeof(*item));
	fitem->item.amount = amount;
	fitem->subx = rnd_value(1, 4) * 3;
	fitem->suby = rnd_value(1, 4) * 3;
	fitem->cleartimer = add_timer(gettick()+battle_config.flooritem_lifetime,map_clearflooritem_timer,fitem->id,0);

	map_addiddb(fitem);
	if (map_addblock(fitem))
		return 0;
	clif_dropflooritem(fitem,canShowEffect);

	return fitem->id;
}

/**
 * @see DBCreateData
 */
static DBData create_charid2nick(DBKey key, va_list args)
{
	struct charid2nick *p;
	CREATE(p, struct charid2nick, 1);
	return db_ptr2data(p);
}

/// Adds(or replaces) the nick of charid to nick_db and fulfils pending requests.
/// Does nothing if the character is online.
void map_addnickdb(int32 charid, const char* nick)
{
	struct charid2nick* p;
	
	if( map_charid2sd(charid) )
		return;// already online

	p = (struct charid2nick*)idb_ensure(nick_db, charid, create_charid2nick);
	safestrncpy(p->nick, nick, sizeof(p->nick));

	while( p->requests ) {
		map_session_data* sd;
		struct charid_request* req;
		req = p->requests;
		p->requests = req->next;
		sd = map_charid2sd(req->charid);
		if( sd != nullptr )
			clif_solved_charname( *sd, charid, p->nick );
		aFree(req);
	}
}

/// Removes the nick of charid from nick_db.
/// Sends name to all pending requests on charid.
void map_delnickdb(int32 charid, const char* name)
{
	struct charid2nick* p;
	DBData data;

	if (!nick_db->remove(nick_db, db_i2key(charid), &data) || (p = (struct charid2nick*)db_data2ptr(&data)) == nullptr)
		return;

	while( p->requests ) {
		struct charid_request* req;
		map_session_data* sd;
		req = p->requests;
		p->requests = req->next;
		sd = map_charid2sd(req->charid);
		if( sd != nullptr )
			clif_solved_charname( *sd, charid, name );
		aFree(req);
	}
	aFree(p);
}

/// Notifies sd of the nick of charid.
/// Uses the name in the character if online.
/// Uses the name in nick_db if offline.
void map_reqnickdb(map_session_data * sd, int32 charid)
{
	struct charid2nick* p;
	struct charid_request* req;
	map_session_data* tsd;

	nullpo_retv(sd);

	tsd = map_charid2sd(charid);
	if( tsd != nullptr )
	{
		clif_solved_charname( *sd, charid, tsd->status.name );
		return;
	}

	p = (struct charid2nick*)idb_ensure(nick_db, charid, create_charid2nick);
	if( *p->nick )
	{
		clif_solved_charname( *sd, charid, p->nick );
		return;
	}
	// not in cache, request it
	CREATE(req, struct charid_request, 1);
	req->next = p->requests;
	p->requests = req;
	chrif_searchcharid(charid);
}

/*==========================================
 * add bl to id_db
 *------------------------------------------*/
void map_addiddb(struct block_list *bl)
{
	nullpo_retv(bl);

	if( bl->type == BL_PC )
	{
		TBL_PC* sd = (TBL_PC*)bl;
		idb_put(pc_db,sd->id,sd);
		uidb_put(charid_db,sd->status.char_id,sd);
	}
	else if( bl->type == BL_MOB )
	{
		TBL_MOB* md = (TBL_MOB*)bl;
		idb_put(mobid_db,bl->id,bl);

		if( md->state.boss )
			idb_put(bossid_db, bl->id, bl);
	}

	if( bl->type & BL_REGEN )
		idb_put(regen_db, bl->id, bl);

	idb_put(id_db,bl->id,bl);
}

/*==========================================
 * remove bl from id_db
 *------------------------------------------*/
void map_deliddb(struct block_list *bl)
{
	nullpo_retv(bl);

	if( bl->type == BL_PC )
	{
		TBL_PC* sd = (TBL_PC*)bl;
		idb_remove(pc_db,sd->id);
		uidb_remove(charid_db,sd->status.char_id);
	}
	else if( bl->type == BL_MOB )
	{
		idb_remove(mobid_db,bl->id);
		idb_remove(bossid_db,bl->id);
	}

	if( bl->type & BL_REGEN )
		idb_remove(regen_db,bl->id);

	idb_remove(id_db,bl->id);
}

/*==========================================
 * Standard call when a player connection is closed.
 *------------------------------------------*/
int32 map_quit(map_session_data *sd) {
	int32 i;

	if (sd->state.keepshop == false) { // Close vending/buyingstore
		if (sd->state.vending)
			vending_closevending(sd);
		else if (sd->state.buyingstore)
			buyingstore_close(sd);
	}

	if(!sd->state.active) { //Removing a player that is not active.
		struct auth_node *node = chrif_search(sd->status.account_id);
		if (node && node->char_id == sd->status.char_id &&
			node->state != ST_LOGOUT)
			//Except when logging out, clear the auth-connect data immediately.
			chrif_auth_delete(node->account_id, node->char_id, node->state);
		//Non-active players should not have loaded any data yet (or it was cleared already) so no additional cleanups are needed.
		return 0;
	}

	if (sd->expiration_tid != INVALID_TIMER)
		delete_timer(sd->expiration_tid, pc_expiration_timer);

	if (sd->npc_timer_id != INVALID_TIMER) //Cancel the event timer.
		npc_timerevent_quit(sd);

	if (sd->autotrade_tid != INVALID_TIMER)
		delete_timer(sd->autotrade_tid, pc_autotrade_timer);

	if (sd->npc_id)
		npc_event_dequeue(sd);

	if (sd->bg_id)
		bg_team_leave(sd, true, true);

	if (sd->bg_queue_id > 0)
		bg_queue_leave(sd, false);

	if( sd->status.clan_id )
		clan_member_left( *sd );

	pc_itemcd_do(sd,false);

	npc_script_event( *sd, NPCE_LOGOUT );

	//Unit_free handles clearing the player related data,
	//map_quit handles extra specific data which is related to quitting normally
	//(changing map-servers invokes unit_free but bypasses map_quit)
	if( !sd->sc.empty() ) {
		for (const auto &it : status_db) {
			std::bitset<SCF_MAX> &flag = it.second->flag;

			//No need to save infinite status
			if (flag[SCF_NOSAVEINFINITE] && sd->sc.getSCE(it.first) && sd->sc.getSCE(it.first)->val4 > 0) {
				status_change_end(sd, static_cast<sc_type>(it.first));
				continue;
			}

			//Status that are not saved
			if (flag[SCF_NOSAVE]) {
				status_change_end(sd, static_cast<sc_type>(it.first));
				continue;
			}
			//Removes status by config
			if (battle_config.debuff_on_logout&1 && flag[SCF_DEBUFF] || //Removes debuffs
				(battle_config.debuff_on_logout&2 && !(flag[SCF_DEBUFF]))) //Removes buffs
			{
				status_change_end(sd, static_cast<sc_type>(it.first));
				continue;
			}
		}
	}

	for (i = 0; i < EQI_MAX; i++) {
		if (sd->equip_index[i] >= 0)
			if( pc_isequip( sd, sd->equip_index[i] ) != ITEM_EQUIP_ACK_OK ){
				pc_unequipitem(sd,sd->equip_index[i],2);
			}
	}

	// Return loot to owner
	if( sd->pd != nullptr ){
		pet_lootitem_drop( *sd->pd, sd );
	}

	if (sd->ed) // Remove effects here rather than unit_remove_map_pc so we don't clear on Teleport/map change.
		elemental_clean_effect(sd->ed);

	if (sd->state.permanent_speed == 1) sd->state.permanent_speed = 0; // Remove lock so speed is set back to normal at login.

	struct map_data *mapdata = map_getmapdata(sd->m);

	if( mapdata->instance_id > 0 )
		instance_delusers(mapdata->instance_id);

	unit_remove_map_pc(sd,CLR_RESPAWN);

	if (sd->state.vending)
		idb_remove(vending_getdb(), sd->status.char_id);

	if (sd->state.buyingstore)
		idb_remove(buyingstore_getdb(), sd->status.char_id);

	party_booking_delete(sd); // Party Booking [Spiria]
	pc_makesavestatus(sd);
	pc_clean_skilltree(sd);
	pc_crimson_marker_clear(sd);
	pc_macro_detector_disconnect(*sd);
	chrif_save(sd, CSAVE_QUIT|CSAVE_INVENTORY|CSAVE_CART);
	unit_free_pc(sd);
	return 0;
}

/*==========================================
 * Lookup, id to session (player,mob,npc,homon,merc..)
 *------------------------------------------*/
map_session_data * map_id2sd(int32 id){
	if (id <= 0) return nullptr;
	return (map_session_data*)idb_get(pc_db,id);
}

struct mob_data * map_id2md(int32 id){
	if (id <= 0) return nullptr;
	return (struct mob_data*)idb_get(mobid_db,id);
}

struct npc_data * map_id2nd(int32 id){
	struct block_list* bl = map_id2bl(id);
	return BL_CAST(BL_NPC, bl);
}

struct homun_data* map_id2hd(int32 id){
	struct block_list* bl = map_id2bl(id);
	return BL_CAST(BL_HOM, bl);
}

struct s_mercenary_data* map_id2mc(int32 id){
	struct block_list* bl = map_id2bl(id);
	return BL_CAST(BL_MER, bl);
}

struct pet_data* map_id2pd(int32 id){
	struct block_list* bl = map_id2bl(id);
	return BL_CAST(BL_PET, bl);
}

struct s_elemental_data* map_id2ed(int32 id) {
	struct block_list* bl = map_id2bl(id);
	return BL_CAST(BL_ELEM, bl);
}

struct chat_data* map_id2cd(int32 id){
	struct block_list* bl = map_id2bl(id);
	return BL_CAST(BL_CHAT, bl);
}

/// Returns the nick of the target charid or nullptr if unknown (requests the nick to the char server).
const char* map_charid2nick(int32 charid)
{
	struct charid2nick *p;
	map_session_data* sd;

	sd = map_charid2sd(charid);
	if( sd )
		return sd->status.name;// character is online, return it's name

	p = (struct charid2nick*)idb_ensure(nick_db, charid, create_charid2nick);
	if( *p->nick )
		return p->nick;// name in nick_db

	chrif_searchcharid(charid);// request the name
	return nullptr;
}

/// Returns the map_session_data of the charid or nullptr if the char is not online.
map_session_data* map_charid2sd(int32 charid)
{
	return (map_session_data*)uidb_get(charid_db, charid);
}

/*==========================================
 * Search session data from a nick name
 * (without sensitive case if necessary)
 * return map_session_data pointer or nullptr
 *------------------------------------------*/
map_session_data * map_nick2sd(const char *nick, bool allow_partial)
{
	map_session_data* sd;
	map_session_data* found_sd;
	struct s_mapiterator* iter;
	size_t nicklen;
	int32 qty = 0;

	if( nick == nullptr )
		return nullptr;

	nicklen = strlen(nick);
	iter = mapit_getallusers();

	found_sd = nullptr;
	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
	{
		if( allow_partial && battle_config.partial_name_scan )
		{// partial name search
			if( strnicmp(sd->status.name, nick, nicklen) == 0 )
			{
				found_sd = sd;

				if( strcmp(sd->status.name, nick) == 0 )
				{// Perfect Match
					qty = 1;
					break;
				}

				qty++;
			}
		}
		else if( strcasecmp(sd->status.name, nick) == 0 )
		{// exact search only
			found_sd = sd;
			qty = 1;
			break;
		}
	}
	mapit_free(iter);

	if( battle_config.partial_name_scan && qty != 1 )
		found_sd = nullptr;

	return found_sd;
}

/*==========================================
 * Looksup id_db DBMap and returns BL pointer of 'id' or nullptr if not found
 *------------------------------------------*/
struct block_list * map_id2bl(int32 id) {
	return (struct block_list*)idb_get(id_db,id);
}

/**
 * Same as map_id2bl except it only checks for its existence
 **/
bool map_blid_exists( int32 id ) {
	return (idb_exists(id_db,id));
}

/*==========================================
 * Convex Mirror
 *------------------------------------------*/
struct mob_data * map_getmob_boss(int16 m)
{
	DBIterator* iter;
	struct mob_data *md = nullptr;
	bool found = false;

	iter = db_iterator(bossid_db);
	for( md = (struct mob_data*)dbi_first(iter); dbi_exists(iter); md = (struct mob_data*)dbi_next(iter) )
	{
		if( md->m == m )
		{
			found = true;
			break;
		}
	}
	dbi_destroy(iter);

	return (found)? md : nullptr;
}

struct mob_data * map_id2boss(int32 id)
{
	if (id <= 0) return nullptr;
	return (struct mob_data*)idb_get(bossid_db,id);
}

/// Applies func to all the players in the db.
/// Stops iterating if func returns -1.
void map_foreachpc(int32 (*func)(map_session_data* sd, va_list args), ...)
{
	DBIterator* iter;
	map_session_data* sd;

	iter = db_iterator(pc_db);
	for( sd = (map_session_data*)dbi_first(iter); dbi_exists(iter); sd = (map_session_data*)dbi_next(iter) )
	{
		va_list args;
		int32 ret;

		va_start(args, func);
		ret = func(sd, args);
		va_end(args);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Applies func to all the mobs in the db.
/// Stops iterating if func returns -1.
void map_foreachmob(int32 (*func)(struct mob_data* md, va_list args), ...)
{
	DBIterator* iter;
	struct mob_data* md;

	iter = db_iterator(mobid_db);
	for( md = (struct mob_data*)dbi_first(iter); dbi_exists(iter); md = (struct mob_data*)dbi_next(iter) )
	{
		va_list args;
		int32 ret;

		va_start(args, func);
		ret = func(md, args);
		va_end(args);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Applies func to all the npcs in the db.
/// Stops iterating if func returns -1.
void map_foreachnpc(int32 (*func)(struct npc_data* nd, va_list args), ...)
{
	DBIterator* iter;
	struct block_list* bl;

	iter = db_iterator(id_db);
	for( bl = (struct block_list*)dbi_first(iter); dbi_exists(iter); bl = (struct block_list*)dbi_next(iter) )
	{
		if( bl->type == BL_NPC )
		{
			struct npc_data* nd = (struct npc_data*)bl;
			va_list args;
			int32 ret;

			va_start(args, func);
			ret = func(nd, args);
			va_end(args);
			if( ret == -1 )
				break;// stop iterating
		}
	}
	dbi_destroy(iter);
}

/// Applies func to everything in the db.
/// Stops iterating if func returns -1.
void map_foreachregen(int32 (*func)(struct block_list* bl, va_list args), ...)
{
	DBIterator* iter;
	struct block_list* bl;

	iter = db_iterator(regen_db);
	for( bl = (struct block_list*)dbi_first(iter); dbi_exists(iter); bl = (struct block_list*)dbi_next(iter) )
	{
		va_list args;
		int32 ret;

		va_start(args, func);
		ret = func(bl, args);
		va_end(args);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Applies func to everything in the db.
/// Stops iterating if func returns -1.
void map_foreachiddb(int32 (*func)(struct block_list* bl, va_list args), ...)
{
	DBIterator* iter;
	struct block_list* bl;

	iter = db_iterator(id_db);
	for( bl = (struct block_list*)dbi_first(iter); dbi_exists(iter); bl = (struct block_list*)dbi_next(iter) )
	{
		va_list args;
		int32 ret;

		va_start(args, func);
		ret = func(bl, args);
		va_end(args);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Iterator.
/// Can filter by bl type.
struct s_mapiterator
{
	enum e_mapitflags flags;// flags for special behaviour
	enum bl_type types;// what bl types to return
	DBIterator* dbi;// database iterator
};

/// Returns true if the block_list matches the description in the iterator.
///
/// @param _mapit_ Iterator
/// @param _bl_ block_list
/// @return true if it matches
#define MAPIT_MATCHES(_mapit_,_bl_) \
	( \
		( (_bl_)->type & (_mapit_)->types /* type matches */ ) \
	)

/// Allocates a new iterator.
/// Returns the new iterator.
/// types can represent several BL's as a bit field.
/// TODO should this be expanded to allow filtering of map/guild/party/chat/cell/area/...?
///
/// @param flags Flags of the iterator
/// @param type Target types
/// @return Iterator
struct s_mapiterator* mapit_alloc(enum e_mapitflags flags, enum bl_type types)
{
	struct s_mapiterator* mapit;

	CREATE(mapit, struct s_mapiterator, 1);
	mapit->flags = flags;
	mapit->types = types;
	if( types == BL_PC )       mapit->dbi = db_iterator(pc_db);
	else if( types == BL_MOB ) mapit->dbi = db_iterator(mobid_db);
	else                       mapit->dbi = db_iterator(id_db);
	return mapit;
}

/// Frees the iterator.
///
/// @param mapit Iterator
void mapit_free(struct s_mapiterator* mapit)
{
	nullpo_retv(mapit);

	dbi_destroy(mapit->dbi);
	aFree(mapit);
}

/// Returns the first block_list that matches the description.
/// Returns nullptr if not found.
///
/// @param mapit Iterator
/// @return first block_list or nullptr
struct block_list* mapit_first(struct s_mapiterator* mapit)
{
	struct block_list* bl;

	nullpo_retr(nullptr,mapit);

	for( bl = (struct block_list*)dbi_first(mapit->dbi); bl != nullptr; bl = (struct block_list*)dbi_next(mapit->dbi) )
	{
		if( MAPIT_MATCHES(mapit,bl) )
			break;// found match
	}
	return bl;
}

/// Returns the last block_list that matches the description.
/// Returns nullptr if not found.
///
/// @param mapit Iterator
/// @return last block_list or nullptr
struct block_list* mapit_last(struct s_mapiterator* mapit)
{
	struct block_list* bl;

	nullpo_retr(nullptr,mapit);

	for( bl = (struct block_list*)dbi_last(mapit->dbi); bl != nullptr; bl = (struct block_list*)dbi_prev(mapit->dbi) )
	{
		if( MAPIT_MATCHES(mapit,bl) )
			break;// found match
	}
	return bl;
}

/// Returns the next block_list that matches the description.
/// Returns nullptr if not found.
///
/// @param mapit Iterator
/// @return next block_list or nullptr
struct block_list* mapit_next(struct s_mapiterator* mapit)
{
	struct block_list* bl;

	nullpo_retr(nullptr,mapit);

	for( ; ; )
	{
		bl = (struct block_list*)dbi_next(mapit->dbi);
		if( bl == nullptr )
			break;// end
		if( MAPIT_MATCHES(mapit,bl) )
			break;// found a match
		// try next
	}
	return bl;
}

/// Returns the previous block_list that matches the description.
/// Returns nullptr if not found.
///
/// @param mapit Iterator
/// @return previous block_list or nullptr
struct block_list* mapit_prev(struct s_mapiterator* mapit)
{
	struct block_list* bl;

	nullpo_retr(nullptr,mapit);

	for( ; ; )
	{
		bl = (struct block_list*)dbi_prev(mapit->dbi);
		if( bl == nullptr )
			break;// end
		if( MAPIT_MATCHES(mapit,bl) )
			break;// found a match
		// try prev
	}
	return bl;
}

/// Returns true if the current block_list exists in the database.
///
/// @param mapit Iterator
/// @return true if it exists
bool mapit_exists(struct s_mapiterator* mapit)
{
	nullpo_retr(false,mapit);

	return dbi_exists(mapit->dbi);
}

/*==========================================
 * Add npc-bl to id_db, basically register npc to map
 *------------------------------------------*/
bool map_addnpc(int16 m,struct npc_data *nd)
{
	nullpo_ret(nd);

	if( m < 0 )
		return false;

	struct map_data *mapdata = map_getmapdata(m);

	if( mapdata->npc_num == MAX_NPC_PER_MAP )
	{
		ShowWarning("too many NPCs in one map %s\n",mapdata->name);
		return false;
	}

	int32 xs = -1, ys = -1;

	switch (nd->subtype) {
	case NPCTYPE_WARP:
		xs = nd->u.warp.xs;
		ys = nd->u.warp.ys;
		break;
	case NPCTYPE_SCRIPT:
		xs = nd->u.scr.xs;
		ys = nd->u.scr.ys;
		break;
	default:
		break;
	}
	// npcs with trigger area are grouped
	// 0 < npc_num_warp < npc_num_area < npc_num
	if (xs < 0 || ys < 0)
		mapdata->npc[ mapdata->npc_num ] = nd;
	else {
		switch (nd->subtype) {
		case NPCTYPE_WARP:
			mapdata->npc[ mapdata->npc_num ] = mapdata->npc[ mapdata->npc_num_area ];
			mapdata->npc[ mapdata->npc_num_area ] = mapdata->npc[ mapdata->npc_num_warp ];
			mapdata->npc[ mapdata->npc_num_warp ] = nd;
			mapdata->npc_num_warp++;
			mapdata->npc_num_area++;
			break;
		case NPCTYPE_SCRIPT:
			mapdata->npc[ mapdata->npc_num ] = mapdata->npc[ mapdata->npc_num_area ];
			mapdata->npc[ mapdata->npc_num_area ] = nd;
			mapdata->npc_num_area++;
			break;
		default:
			mapdata->npc[ mapdata->npc_num ] = nd;
			break;
		}
	}
	mapdata->npc_num++;
	idb_put(id_db,nd->id,nd);
	return true;
}

/*==========================================
 * Add an instance map
 *------------------------------------------*/
int32 map_addinstancemap(int32 src_m, int32 instance_id, bool no_mapflag)
{
	if(src_m < 0)
		return -1;

	const char *name = map_mapid2mapname(src_m);

	if(strlen(name) > 20) {
		// against buffer overflow
		ShowError("map_addinstancemap: Map name \"%s\" is too long.\n", name);
		return -2;
	}

	int16 dst_m = -1, i;

	for (i = instance_start; i < MAX_MAP_PER_SERVER; i++) {
		if (!map[i].name[0])
			break;
	}
	if (i < map_num) // Destination map value overwrites another
		dst_m = i;
	else if (i < MAX_MAP_PER_SERVER) // Destination map value increments to new map
		dst_m = map_num++;
	else {
		// Out of bounds
		ShowError("map_addinstancemap failed. map_num(%d) > map_max(%d)\n", map_num, MAX_MAP_PER_SERVER);
		return -3;
	}

	struct map_data *src_map = map_getmapdata(src_m);
	struct map_data *dst_map = map_getmapdata(dst_m);

	// Alter the name
	// This also allows us to maintain complete independence with main map functions
	instance_generate_mapname(src_m, instance_id, dst_map->name);

	dst_map->m = dst_m;
	dst_map->instance_id = instance_id;
	dst_map->instance_src_map = src_m;
	dst_map->users = 0;
	dst_map->xs = src_map->xs;
	dst_map->ys = src_map->ys;
	dst_map->bxs = src_map->bxs;
	dst_map->bys = src_map->bys;
	dst_map->iwall_num = src_map->iwall_num;

	memset(dst_map->npc, 0, sizeof(dst_map->npc));
	dst_map->npc_num = 0;
	dst_map->npc_num_area = 0;
	dst_map->npc_num_warp = 0;

	// Reallocate cells
	size_t num_cell = dst_map->xs * dst_map->ys;

	CREATE( dst_map->cell, struct mapcell, num_cell );
	memcpy( dst_map->cell, src_map->cell, num_cell * sizeof(struct mapcell) );

	size_t size = dst_map->bxs * dst_map->bys * sizeof(struct block_list*);

	dst_map->block = (struct block_list **)aCalloc(1,size);
	dst_map->block_mob = (struct block_list **)aCalloc(1,size);

	dst_map->index = mapindex_addmap(-1, dst_map->name);
	dst_map->channel = nullptr;
	dst_map->mob_delete_timer = INVALID_TIMER;

	if(!no_mapflag)
		map_data_copy(dst_map, src_map);

	ShowInfo("[Instance] Created map '%s' (%d) from '%s' (%d).\n", dst_map->name, dst_map->m, name, src_map->m);

	map_addmap2db(dst_map);

	return dst_m;
}

/*==========================================
 * Set player to save point when they leave
 *------------------------------------------*/
static int32 map_instancemap_leave(struct block_list *bl, va_list ap)
{
	map_session_data* sd;

	nullpo_retr(0, bl);
	nullpo_retr(0, sd = (map_session_data *)bl);

	pc_setpos_savepoint( *sd );

	return 1;
}

/*==========================================
 * Remove all units from instance
 *------------------------------------------*/
static int32 map_instancemap_clean(struct block_list *bl, va_list ap)
{
	nullpo_retr(0, bl);
	switch(bl->type) {
		/*case BL_PC:
		// BL_PET, BL_HOM, BL_MER, and BL_ELEM are moved when BL_PC warped out in map_instancemap_leave
			map_quit((map_session_data *) bl);
			break;*/
		case BL_NPC:
			npc_unload((struct npc_data *)bl,true);
			break;
		case BL_MOB:
			unit_free(bl,CLR_OUTSIGHT);
			break;
		case BL_ITEM:
			map_clearflooritem(bl);
			break;
		case BL_SKILL:
			skill_delunit((struct skill_unit *) bl);
			break;
	}

	return 1;
}

static void map_free_questinfo(struct map_data *mapdata);

/*==========================================
 * Deleting an instance map
 *------------------------------------------*/
int32 map_delinstancemap(int32 m)
{
	struct map_data *mapdata = map_getmapdata(m);

	if(m < 0 || mapdata->instance_id <= 0)
		return 0;

	// Kick everyone out
	map_foreachinmap(map_instancemap_leave, m, BL_PC);

	// Do the unit cleanup
	map_foreachinmap(map_instancemap_clean, m, BL_ALL);

	if( mapdata->mob_delete_timer != INVALID_TIMER )
		delete_timer(mapdata->mob_delete_timer, map_removemobs_timer);
	mapdata->mob_delete_timer = INVALID_TIMER;

	// Free memory
	if (mapdata->cell)
		aFree(mapdata->cell);
	mapdata->cell = nullptr;
	if (mapdata->block)
		aFree(mapdata->block);
	mapdata->block = nullptr;
	if (mapdata->block_mob)
		aFree(mapdata->block_mob);
	mapdata->block_mob = nullptr;

	map_free_questinfo(mapdata);
	mapdata->damage_adjust = {};
	mapdata->initMapFlags();
	mapdata->skill_damage.clear();
	mapdata->instance_id = 0;

	mapindex_removemap(mapdata->index);
	map_removemapdb(mapdata);

	mapdata->index = 0;
	memset(&mapdata->name, '\0', sizeof(map[0].name)); // just remove the name
	return 1;
}

/*=========================================
 * Dynamic Mobs [Wizputer]
 *-----------------------------------------*/
// Stores the spawn data entry in the mob list.
// Returns the index of successful, or -1 if the list was full.
int32 map_addmobtolist(uint16 m, struct spawn_data *spawn)
{
	size_t i;
	struct map_data *mapdata = map_getmapdata(m);

	ARR_FIND( 0, MAX_MOB_LIST_PER_MAP, i, mapdata->moblist[i] == nullptr );
	if( i < MAX_MOB_LIST_PER_MAP )
	{
		mapdata->moblist[i] = spawn;
		return static_cast<int32>(i);
	}
	return -1;
}

void map_spawnmobs(int16 m)
{
	int32 i, k=0;
	struct map_data *mapdata = map_getmapdata(m);

	if (mapdata->mob_delete_timer != INVALID_TIMER)
	{	//Mobs have not been removed yet [Skotlex]
		delete_timer(mapdata->mob_delete_timer, map_removemobs_timer);
		mapdata->mob_delete_timer = INVALID_TIMER;
		return;
	}
	for(i=0; i<MAX_MOB_LIST_PER_MAP; i++)
		if(mapdata->moblist[i]!=nullptr)
		{
			k+=mapdata->moblist[i]->num;
			npc_parse_mob2(mapdata->moblist[i]);
		}

	if (battle_config.etc_log && k > 0)
	{
		ShowStatus("Map %s: Spawned '" CL_WHITE "%d" CL_RESET "' mobs.\n",mapdata->name, k);
	}
}

int32 map_removemobs_sub(struct block_list *bl, va_list ap)
{
	struct mob_data *md = (struct mob_data *)bl;
	nullpo_ret(md);

	//When not to remove mob:
	// doesn't respawn and is not a slave
	if( !md->spawn && !md->master_id )
		return 0;
	// respawn data is not in cache
	if( md->spawn && !md->spawn->state.dynamic )
		return 0;
	// hasn't spawned yet
	if( md->spawn_timer != INVALID_TIMER )
		return 0;
	// is damaged and mob_remove_damaged is off
	if( !battle_config.mob_remove_damaged && md->status.hp < md->status.max_hp )
		return 0;
	// is a mvp
	if( md->get_bosstype() == BOSSTYPE_MVP )
		return 0;

	unit_free(md,CLR_OUTSIGHT);

	return 1;
}

TIMER_FUNC(map_removemobs_timer){
	int32 count;
	const int16 m = id;

	if (m < 0)
	{	//Incorrect map id!
		ShowError("map_removemobs_timer error: timer %d points to invalid map %d\n",tid, m);
		return 0;
	}

	struct map_data *mapdata = map_getmapdata(m);

	if (mapdata->mob_delete_timer != tid)
	{	//Incorrect timer call!
		ShowError("map_removemobs_timer mismatch: %d != %d (map %s)\n",mapdata->mob_delete_timer, tid, mapdata->name);
		return 0;
	}
	mapdata->mob_delete_timer = INVALID_TIMER;
	if (mapdata->users > 0) //Map not empty!
		return 1;

	count = map_foreachinmap(map_removemobs_sub, m, BL_MOB);

	if (battle_config.etc_log && count > 0)
		ShowStatus("Map %s: Removed '" CL_WHITE "%d" CL_RESET "' mobs.\n",mapdata->name, count);

	return 1;
}

void map_removemobs(int16 m)
{
	struct map_data *mapdata = map_getmapdata(m);

	if (mapdata->mob_delete_timer != INVALID_TIMER) // should never happen
		return; //Mobs are already scheduled for removal

	// Don't remove mobs on instance map
	if (mapdata->instance_id > 0)
		return;

	mapdata->mob_delete_timer = add_timer(gettick()+battle_config.mob_remove_delay, map_removemobs_timer, m, 0);
}

/*==========================================
 * Check for map_name from map_id
 *------------------------------------------*/
const char* map_mapid2mapname(int32 m)
{
	if (m == -1)
		return "Floating";

	struct map_data *mapdata = map_getmapdata(m);

	if (!mapdata)
		return "";

	if (mapdata->instance_id > 0) { // Instance map check
		std::shared_ptr<s_instance_data> idata = util::umap_find(instances, map[m].instance_id);

		if (!idata) // This shouldn't happen but if it does give them the map we intended to give
			return mapdata->name;
		else {
			for (const auto &it : idata->map) { // Loop to find the src map we want
				if (it.m == m)
					return map_getmapdata(it.src_m)->name;
			}
		}
	}

	return mapdata->name;
}

/*==========================================
 * Hookup, get map_id from map_name
 *------------------------------------------*/
int16 map_mapname2mapid(const char* name)
{
	uint16 map_index;
	map_index = mapindex_name2id(name);
	if (!map_index)
		return -1;
	return map_mapindex2mapid(map_index);
}

/*==========================================
 * Returns the map of the given mapindex. [Skotlex]
 *------------------------------------------*/
int16 map_mapindex2mapid(uint16 mapindex)
{
	struct map_data *md=nullptr;

	if (!mapindex)
		return -1;

	md = (struct map_data*)uidb_get(map_db,(uint32)mapindex);
	if(md==nullptr || md->cell==nullptr)
		return -1;
	return md->m;
}

/*==========================================
 * Switching Ip, port ? (like changing map_server) get ip/port from map_name
 *------------------------------------------*/
int32 map_mapname2ipport(uint16 name, uint32* ip, uint16* port)
{
	struct map_data_other_server *mdos;

	mdos = (struct map_data_other_server*)uidb_get(map_db,(uint32)name);
	if(mdos==nullptr || mdos->cell) //If gat isn't null, this is a local map.
		return -1;
	*ip=mdos->ip;
	*port=mdos->port;
	return 0;
}

/*==========================================
 * Checks if both dirs point in the same direction.
 *------------------------------------------*/
int32 map_check_dir(int32 s_dir,int32 t_dir)
{
	if(s_dir == t_dir)
		return 0;
	switch(s_dir) {
		case DIR_NORTH: if( t_dir == DIR_NORTHEAST || t_dir == DIR_NORTHWEST || t_dir == DIR_NORTH ) return 0; break;
		case DIR_NORTHWEST: if( t_dir == DIR_NORTH || t_dir == DIR_WEST || t_dir == DIR_NORTHWEST ) return 0; break;
		case DIR_WEST: if( t_dir == DIR_NORTHWEST || t_dir == DIR_SOUTHWEST || t_dir == DIR_WEST ) return 0; break;
		case DIR_SOUTHWEST: if( t_dir == DIR_WEST || t_dir == DIR_SOUTH || t_dir == DIR_SOUTHWEST ) return 0; break;
		case DIR_SOUTH: if( t_dir == DIR_SOUTHWEST || t_dir == DIR_SOUTHEAST || t_dir == DIR_SOUTH ) return 0; break;
		case DIR_SOUTHEAST: if( t_dir == DIR_SOUTH || t_dir == DIR_EAST || t_dir == DIR_SOUTHEAST ) return 0; break;
		case DIR_EAST: if( t_dir == DIR_SOUTHEAST || t_dir == DIR_NORTHEAST || t_dir == DIR_EAST ) return 0; break;
		case DIR_NORTHEAST: if( t_dir == DIR_EAST || t_dir == DIR_NORTH || t_dir == DIR_NORTHEAST ) return 0; break;
	}
	return 1;
}

/*==========================================
 * Returns the direction of the given cell, relative to 'src'
 *------------------------------------------*/
uint8 map_calc_dir(struct block_list* src, int16 x, int16 y)
{
	uint8 dir = DIR_NORTH;

	nullpo_retr( dir, src );

	dir = map_calc_dir_xy(src->x, src->y, x, y, unit_getdir(src));

	return dir;
}

/*==========================================
 * Returns the direction of the given cell, relative to source cell
 * Use this if you don't have a block list available to check against
 *------------------------------------------*/
uint8 map_calc_dir_xy(int16 srcx, int16 srcy, int16 x, int16 y, uint8 srcdir) {
	uint8 dir = DIR_NORTH;
	int32 dx, dy;

	dx = x-srcx;
	dy = y-srcy;
	if( dx == 0 && dy == 0 )
	{	// both are standing on the same spot
		// aegis-style, makes knockback default to the left
		// athena-style, makes knockback default to behind 'src'
		dir = ( battle_config.knockback_left ? DIR_EAST : srcdir );
	}
	else if( dx >= 0 && dy >=0 )
	{	// upper-right
		if( dx >= dy*3 )      dir = DIR_EAST;	// right
		else if( dx*3 < dy )  dir = DIR_NORTH;	// up
		else                  dir = DIR_NORTHEAST;	// up-right
	}
	else if( dx >= 0 && dy <= 0 )
	{	// lower-right
		if( dx >= -dy*3 )     dir = DIR_EAST;	// right
		else if( dx*3 < -dy ) dir = DIR_SOUTH;	// down
		else                  dir = DIR_SOUTHEAST;	// down-right
	}
	else if( dx <= 0 && dy <= 0 )
	{	// lower-left
		if( dx*3 >= dy )      dir = DIR_SOUTH;	// down
		else if( dx < dy*3 )  dir = DIR_WEST;	// left
		else                  dir = DIR_SOUTHWEST;	// down-left
	}
	else
	{	// upper-left
		if( -dx*3 <= dy )     dir = DIR_NORTH;	// up
		else if( -dx > dy*3 ) dir = DIR_WEST;	// left
		else                  dir = DIR_NORTHWEST;	// up-left
	}
	return dir;
}

/*==========================================
 * Randomizes target cell x,y to a random walkable cell that
 * has the same distance from object as given coordinates do. [Skotlex]
 *------------------------------------------*/
int32 map_random_dir(struct block_list *bl, int16 *x, int16 *y)
{
	int16 xi = *x-bl->x;
	int16 yi = *y-bl->y;
	int16 i=0;
	int32 dist2 = xi*xi + yi*yi;
	int16 dist = (int16)sqrt((float)dist2);

	if (dist < 1) dist =1;

	do {
		directions j = static_cast<directions>(1 + 2 * rnd_value(0, 3)); //Pick a random diagonal direction
		int16 segment = rnd_value((int16)1, dist); //Pick a random interval from the whole vector in that direction
		xi = bl->x + segment*dirx[j];
		segment = (int16)sqrt((float)(dist2 - segment*segment)); //The complement of the previously picked segment
		yi = bl->y + segment*diry[j];
	} while (
		(map_getcell(bl->m,xi,yi,CELL_CHKNOPASS) || !path_search(nullptr,bl->m,bl->x,bl->y,xi,yi,1,CELL_CHKNOREACH))
		&& (++i)<100 );

	if (i < 100) {
		*x = xi;
		*y = yi;
		return 1;
	}
	return 0;
}

// gat system
inline static struct mapcell map_gat2cell(int32 gat) {
	struct mapcell cell;

	memset(&cell,0,sizeof(struct mapcell));

	switch( gat ) {
		case 0: cell.walkable = 1; cell.shootable = 1; cell.water = 0; break; // walkable ground
		case 1: cell.walkable = 0; cell.shootable = 0; cell.water = 0; break; // non-walkable ground
		case 2: cell.walkable = 1; cell.shootable = 1; cell.water = 0; break; // ???
		case 3: cell.walkable = 1; cell.shootable = 1; cell.water = 1; break; // walkable water
		case 4: cell.walkable = 1; cell.shootable = 1; cell.water = 0; break; // ???
		case 5: cell.walkable = 0; cell.shootable = 1; cell.water = 0; break; // gap (snipable)
		case 6: cell.walkable = 1; cell.shootable = 1; cell.water = 0; break; // ???
		default:
			ShowWarning("map_gat2cell: unrecognized gat type '%d'\n", gat);
			break;
	}

	return cell;
}

static int32 map_cell2gat(struct mapcell cell)
{
	if( cell.walkable == 1 && cell.shootable == 1 && cell.water == 0 ) return 0;
	if( cell.walkable == 0 && cell.shootable == 0 && cell.water == 0 ) return 1;
	if( cell.walkable == 1 && cell.shootable == 1 && cell.water == 1 ) return 3;
	if( cell.walkable == 0 && cell.shootable == 1 && cell.water == 0 ) return 5;

	ShowWarning("map_cell2gat: cell has no matching gat type\n");
	return 1; // default to 'wall'
}

/*==========================================
 * Confirm if celltype in (m,x,y) match the one given in cellchk
 *------------------------------------------*/
int32 map_getcell(int16 m,int16 x,int16 y,cell_chk cellchk)
{
	if (m < 0)
		return 0;
	else
		return map_getcellp(map_getmapdata(m), x, y, cellchk);
}

int32 map_getcellp(struct map_data* m,int16 x,int16 y,cell_chk cellchk)
{
	struct mapcell cell;

	nullpo_ret(m);

	//NOTE: this intentionally overrides the last row and column
	if(x<0 || x>=m->xs-1 || y<0 || y>=m->ys-1)
		return( cellchk == CELL_CHKNOPASS );

	cell = m->cell[x + y*m->xs];

	switch(cellchk)
	{
		// gat type retrieval
		case CELL_GETTYPE:
			return map_cell2gat(cell);

		// base gat type checks
		case CELL_CHKWALL:
			return (!cell.walkable && !cell.shootable);

		case CELL_CHKWATER:
			return (cell.water);

		case CELL_CHKCLIFF:
			return (!cell.walkable && cell.shootable);


		// base cell type checks
		case CELL_CHKNPC:
			return (cell.npc);
		case CELL_CHKBASILICA:
			return (cell.basilica);
		case CELL_CHKLANDPROTECTOR:
			return (cell.landprotector);
		case CELL_CHKNOVENDING:
			return (cell.novending);
		case CELL_CHKNOBUYINGSTORE:
			return (cell.nobuyingstore);
		case CELL_CHKNOCHAT:
			return (cell.nochat);
		case CELL_CHKMAELSTROM:
			return (cell.maelstrom);
		case CELL_CHKICEWALL:
			return (cell.icewall);

		// special checks
		case CELL_CHKPASS:
#ifdef CELL_NOSTACK
			if (cell.cell_bl >= battle_config.custom_cell_stack_limit) return 0;
#endif
		case CELL_CHKREACH:
			return (cell.walkable);

		case CELL_CHKNOPASS:
#ifdef CELL_NOSTACK
			if (cell.cell_bl >= battle_config.custom_cell_stack_limit) return 1;
#endif
		case CELL_CHKNOREACH:
			return (!cell.walkable);

		case CELL_CHKSTACK:
#ifdef CELL_NOSTACK
			return (cell.cell_bl >= battle_config.custom_cell_stack_limit);
#else
			return 0;
#endif

		default:
			return 0;
	}
}

/*==========================================
 * Change the type/flags of a map cell
 * 'cell' - which flag to modify
 * 'flag' - true = on, false = off
 *------------------------------------------*/
void map_setcell(int16 m, int16 x, int16 y, cell_t cell, bool flag)
{
	int32 j;
	struct map_data *mapdata = map_getmapdata(m);

	if( m < 0 || x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys )
		return;

	j = x + y*mapdata->xs;

	switch( cell ) {
		case CELL_WALKABLE:      mapdata->cell[j].walkable = flag;      break;
		case CELL_SHOOTABLE:     mapdata->cell[j].shootable = flag;     break;
		case CELL_WATER:         mapdata->cell[j].water = flag;         break;

		case CELL_NPC:           mapdata->cell[j].npc = flag;           break;
		case CELL_BASILICA:      mapdata->cell[j].basilica = flag;      break;
		case CELL_LANDPROTECTOR: mapdata->cell[j].landprotector = flag; break;
		case CELL_NOVENDING:     mapdata->cell[j].novending = flag;     break;
		case CELL_NOCHAT:        mapdata->cell[j].nochat = flag;        break;
		case CELL_MAELSTROM:	 mapdata->cell[j].maelstrom = flag;	  break;
		case CELL_ICEWALL:		 mapdata->cell[j].icewall = flag;		  break;
		case CELL_NOBUYINGSTORE: mapdata->cell[j].nobuyingstore = flag; break;
		default:
			ShowWarning("map_setcell: invalid cell type '%d'\n", (int32)cell);
			break;
	}
}

void map_setgatcell(int16 m, int16 x, int16 y, int32 gat)
{
	int32 j;
	struct mapcell cell;
	struct map_data *mapdata = map_getmapdata(m);

	if( m < 0 || x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys )
		return;

	j = x + y*mapdata->xs;

	cell = map_gat2cell(gat);
	mapdata->cell[j].walkable = cell.walkable;
	mapdata->cell[j].shootable = cell.shootable;
	mapdata->cell[j].water = cell.water;
}

/*==========================================
 * Invisible Walls
 *------------------------------------------*/
static DBMap* iwall_db;

bool map_iwall_exist(const char* wall_name)
{
	return strdb_exists(iwall_db, wall_name);
}

void map_iwall_nextxy(int16 x, int16 y, int8 dir, int32 pos, int16 *x1, int16 *y1)
{
	if( dir == DIR_NORTH || dir == DIR_SOUTH )
		*x1 = x; // Keep X
	else if( dir > DIR_NORTH && dir < DIR_SOUTH )
		*x1 = x - pos; // Going left
	else
		*x1 = x + pos; // Going right

	if( dir == DIR_WEST || dir == DIR_EAST )
		*y1 = y;
	else if( dir > DIR_WEST && dir < DIR_EAST )
		*y1 = y - pos;
	else
		*y1 = y + pos;
}

bool map_iwall_set(int16 m, int16 x, int16 y, int32 size, int8 dir, bool shootable, const char* wall_name)
{
	struct iwall_data *iwall;
	int32 i;
	int16 x1 = 0, y1 = 0;

	if( size < 1 || !wall_name )
		return false;

	if( (iwall = (struct iwall_data *)strdb_get(iwall_db, wall_name)) != nullptr )
		return false; // Already Exists

	if( map_getcell(m, x, y, CELL_CHKNOREACH) )
		return false; // Starting cell problem

	CREATE(iwall, struct iwall_data, 1);
	iwall->m = m;
	iwall->x = x;
	iwall->y = y;
	iwall->size = size;
	iwall->dir = dir;
	iwall->shootable = shootable;
	safestrncpy(iwall->wall_name, wall_name, sizeof(iwall->wall_name));

	for( i = 0; i < size; i++ )
	{
		map_iwall_nextxy(x, y, dir, i, &x1, &y1);

		if( map_getcell(m, x1, y1, CELL_CHKNOREACH) )
			break; // Collision

		map_setcell(m, x1, y1, CELL_WALKABLE, false);
		map_setcell(m, x1, y1, CELL_SHOOTABLE, shootable);

		clif_changemapcell( m, x1, y1, map_getcell( m, x1, y1, CELL_GETTYPE ) );
	}

	iwall->size = i;

	strdb_put(iwall_db, iwall->wall_name, iwall);
	map_getmapdata(m)->iwall_num++;

	return true;
}

void map_iwall_get(map_session_data *sd) {
	struct iwall_data *iwall;
	DBIterator* iter;
	int16 x1, y1;
	int32 i;

	if( map_getmapdata(sd->m)->iwall_num < 1 )
		return;

	iter = db_iterator(iwall_db);
	for( iwall = (struct iwall_data *)dbi_first(iter); dbi_exists(iter); iwall = (struct iwall_data *)dbi_next(iter) ) {
		if( iwall->m != sd->m )
			continue;

		for( i = 0; i < iwall->size; i++ ) {
			map_iwall_nextxy(iwall->x, iwall->y, iwall->dir, i, &x1, &y1);
			clif_changemapcell( iwall->m, x1, y1, map_getcell( iwall->m, x1, y1, CELL_GETTYPE ), SELF, sd );
		}
	}
	dbi_destroy(iter);
}

bool map_iwall_remove(const char *wall_name)
{
	struct iwall_data *iwall;
	int16 i, x1, y1;

	if( (iwall = (struct iwall_data *)strdb_get(iwall_db, wall_name)) == nullptr )
		return false; // Nothing to do

	for( i = 0; i < iwall->size; i++ ) {
		map_iwall_nextxy(iwall->x, iwall->y, iwall->dir, i, &x1, &y1);

		map_setcell(iwall->m, x1, y1, CELL_SHOOTABLE, true);
		map_setcell(iwall->m, x1, y1, CELL_WALKABLE, true);

		clif_changemapcell( iwall->m, x1, y1, map_getcell( iwall->m, x1, y1, CELL_GETTYPE ) );
	}

	map_getmapdata(iwall->m)->iwall_num--;
	strdb_remove(iwall_db, iwall->wall_name);
	return true;
}

/**
 * @see DBCreateData
 */
static DBData create_map_data_other_server(DBKey key, va_list args)
{
	struct map_data_other_server *mdos;
	uint16 mapindex = (uint16)key.ui;
	mdos=(struct map_data_other_server *)aCalloc(1,sizeof(struct map_data_other_server));
	mdos->index = mapindex;
	memcpy(mdos->name, mapindex_id2name(mapindex), MAP_NAME_LENGTH);
	return db_ptr2data(mdos);
}

/*==========================================
 * Add mapindex to db of another map server
 *------------------------------------------*/
int32 map_setipport(uint16 mapindex, uint32 ip, uint16 port)
{
	struct map_data_other_server *mdos;

	mdos= (struct map_data_other_server *)uidb_ensure(map_db,(uint32)mapindex, create_map_data_other_server);

	if(mdos->cell) //Local map,Do nothing. Give priority to our own local maps over ones from another server. [Skotlex]
		return 0;
	if(ip == clif_getip() && port == clif_getport()) {
		//That's odd, we received info that we are the ones with this map, but... we don't have it.
		ShowFatalError("map_setipport : received info that this map-server SHOULD have map '%s', but it is not loaded.\n",mapindex_id2name(mapindex));
		exit(EXIT_FAILURE);
	}
	mdos->ip   = ip;
	mdos->port = port;
	return 1;
}

/**
 * Delete all the other maps server management
 * @see DBApply
 */
int32 map_eraseallipport_sub(DBKey key, DBData *data, va_list va)
{
	struct map_data_other_server *mdos = (struct map_data_other_server *)db_data2ptr(data);
	if(mdos->cell == nullptr) {
		db_remove(map_db,key);
		aFree(mdos);
	}
	return 0;
}

int32 map_eraseallipport(void)
{
	map_db->foreach(map_db,map_eraseallipport_sub);
	return 1;
}

/*==========================================
 * Delete mapindex from db of another map server
 *------------------------------------------*/
int32 map_eraseipport(uint16 mapindex, uint32 ip, uint16 port)
{
	struct map_data_other_server *mdos;

	mdos = (struct map_data_other_server*)uidb_get(map_db,(uint32)mapindex);
	if(!mdos || mdos->cell) //Map either does not exists or is a local map.
		return 0;

	if(mdos->ip==ip && mdos->port == port) {
		uidb_remove(map_db,(uint32)mapindex);
		aFree(mdos);
		return 1;
	}
	return 0;
}

/*==========================================
 * [Shinryo]: Init the mapcache
 *------------------------------------------*/
static char *map_init_mapcache(FILE *fp)
{
	size_t size = 0;
	char *buffer;

	// No file open? Return..
	nullpo_ret(fp);

	// Get file size
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Allocate enough space
	CREATE(buffer, char, size);

	// No memory? Return..
	nullpo_ret(buffer);

	// Read file into buffer..
	if(fread(buffer, 1, size, fp) != size) {
		ShowError("map_init_mapcache: Could not read entire mapcache file\n");
		return nullptr;
	}

	return buffer;
}

/*==========================================
 * Map cache reading
 * [Shinryo]: Optimized some behaviour to speed this up
 *==========================================*/
int32 map_readfromcache(struct map_data *m, char *buffer, char *decode_buffer)
{
	int32 i;
	struct map_cache_main_header *header = (struct map_cache_main_header *)buffer;
	struct map_cache_map_info *info = nullptr;
	char *p = buffer + sizeof(struct map_cache_main_header);

	for(i = 0; i < header->map_count; i++) {
		info = (struct map_cache_map_info *)p;

		if( strcmp(m->name, info->name) == 0 )
			break; // Map found

		// Jump to next entry..
		p += sizeof(struct map_cache_map_info) + info->len;
	}

	if( info && i < header->map_count ) {
		unsigned long size, xy;

		if( info->xs <= 0 || info->ys <= 0 )
			return 0;// Invalid

		m->xs = info->xs;
		m->ys = info->ys;
		size = (unsigned long)info->xs*(unsigned long)info->ys;

		if(size > MAX_MAP_SIZE) {
			ShowWarning("map_readfromcache: %s exceeded MAX_MAP_SIZE of %d\n", info->name, MAX_MAP_SIZE);
			return 0; // Say not found to remove it from list.. [Shinryo]
		}

		// TO-DO: Maybe handle the scenario, if the decoded buffer isn't the same size as expected? [Shinryo]
		decode_zip(decode_buffer, &size, p+sizeof(struct map_cache_map_info), info->len);

		CREATE(m->cell, struct mapcell, size);


		for( xy = 0; xy < size; ++xy )
			m->cell[xy] = map_gat2cell(decode_buffer[xy]);

		return 1;
	}

	return 0; // Not found
}

int32 map_addmap(char* mapname)
{
	if( strcmpi(mapname,"clear")==0 )
	{
		map_num = 0;
		instance_start = 0;
		return 0;
	}

	if (map_num >= MAX_MAP_PER_SERVER - 1) {
		ShowError("Could not add map '" CL_WHITE "%s" CL_RESET "', the limit of maps has been reached.\n", mapname);
		return 1;
	}

	mapindex_getmapname(mapname, map[map_num].name);
	map_num++;
	return 0;
}

static void map_delmapid(int32 id)
{
	ShowNotice("Removing map [ %s ] from maplist" CL_CLL "\n",map[id].name);
	for (int32 i = id; i < map_num - 1; i++)
		map[i] = map[i + 1];
	map_num--;
}

int32 map_delmap(char* mapname){
	char map_name[MAP_NAME_LENGTH];

	if (strcmpi(mapname, "all") == 0) {
		map_num = 0;
		return 0;
	}

	mapindex_getmapname(mapname, map_name);
	for (int32 i = 0; i < map_num; i++) {
		if (strcmp(map[i].name, map_name) == 0) {
			map_delmapid(i);
			return 1;
		}
	}

	return 0;
}

/// Initializes map flags and adjusts them depending on configuration.
void map_flags_init(void){
	for (int32 i = 0; i < map_num; i++) {
		struct map_data *mapdata = &map[i];
		union u_mapflag_args args = {};

		mapdata->initMapFlags(); // Resize and define default values
		mapdata->drop_list.clear();
		args.flag_val = 100;

		// additional mapflag data
		mapdata->zone = 0; // restricted mapflag zone
		mapdata->setMapFlag(MF_NOCOMMAND, false); // nocommand mapflag level
		map_setmapflag_sub(i, MF_BEXP, true, &args); // per map base exp multiplicator
		map_setmapflag_sub(i, MF_JEXP, true, &args); // per map job exp multiplicator

		// Clear adjustment data, will be reset after loading NPC
		mapdata->damage_adjust = {};
		mapdata->skill_damage.clear();
		mapdata->skill_duration.clear();
		map_free_questinfo(mapdata);

		if (instance_start && i >= instance_start)
			continue;

		// adjustments
		if( battle_config.pk_mode && !mapdata_flag_vs2(mapdata) )
			mapdata->setMapFlag(MF_PVP, true); // make all maps pvp for pk_mode [Valaris]
	}
}

/**
* Copying map data from parent map for instance map
* @param dst_map Mapdata will be copied to
* @param src_map Copying data from
*/
void map_data_copy(struct map_data *dst_map, struct map_data *src_map) {
	nullpo_retv(dst_map);
	nullpo_retv(src_map);

	memcpy(&dst_map->save, &src_map->save, sizeof(struct point));
	memcpy(&dst_map->damage_adjust, &src_map->damage_adjust, sizeof(struct s_skill_damage));

	dst_map->copyFlags(*src_map);
	dst_map->skill_damage.insert(src_map->skill_damage.begin(), src_map->skill_damage.end());
	dst_map->skill_duration.insert(src_map->skill_duration.begin(), src_map->skill_duration.end());

	dst_map->zone = src_map->zone;
}

/**
* Copy map data for instance maps from its parents
* that were cleared in map_flags_init() after reloadscript
*/
void map_data_copyall (void) {
	if (!instance_start)
		return;
	for (int32 i = instance_start; i < map_num; i++) {
		struct map_data *mapdata = &map[i];
		std::shared_ptr<s_instance_data> idata = util::umap_find(instances, mapdata->instance_id);
		if (!mapdata || mapdata->name[0] == '\0' || !mapdata->instance_src_map || (idata && idata->nomapflag))
			continue;
		map_data_copy(mapdata, &map[mapdata->instance_src_map]);
	}
}

/*
 * Reads from the .rsw for each map
 * Returns water height (or NO_WATER if file doesn't exist) or other error is encountered.
 * Assumed path for file is data/mapname.rsw
 * Credits to LittleWolf
 */
int32 map_waterheight(char* mapname)
{
	char fn[256];
 	char *found;

	//Look up for the rsw
	sprintf(fn, "data\\%s.rsw", mapname);

	found = grfio_find_file(fn);
	if (found) strcpy(fn, found); // replace with real name

	// read & convert fn
	return grfio_read_rsw_water_level( fn );
}

/*==================================
 * .GAT format
 *----------------------------------*/
int32 map_readgat (struct map_data* m)
{
	char filename[256];
	uint8* gat;
	int32 water_height;
	size_t xy, off, num_cells;

	sprintf(filename, "data\\%s.gat", m->name);

	gat = (uint8 *) grfio_reads(filename);
	if (gat == nullptr)
		return 0;

	m->xs = *(int32*)(gat+6);
	m->ys = *(int32*)(gat+10);
	num_cells = m->xs * m->ys;
	CREATE(m->cell, struct mapcell, num_cells);

	water_height = map_waterheight(m->name);

	// Set cell properties
	off = 14;
	for( xy = 0; xy < num_cells; ++xy )
	{
		// read cell data
		float height = *(float*)( gat + off      );
		uint32 type = *(uint32*)( gat + off + 16 );
		off += 20;

		if( type == 0 && water_height != RSW_NO_WATER && height > water_height )
			type = 3; // Cell is 0 (walkable) but under water level, set to 3 (walkable water)

		m->cell[xy] = map_gat2cell(type);
	}

	aFree(gat);

	return 1;
}

/*======================================
 * Add/Remove map to the map_db
 *--------------------------------------*/
void map_addmap2db(struct map_data *m)
{
	uidb_put(map_db, (uint32)m->index, m);
}

void map_removemapdb(struct map_data *m)
{
	uidb_remove(map_db, (uint32)m->index);
}

/*======================================
 * Initiate maps loading stage
 *--------------------------------------*/
int32 map_readallmaps (void)
{
	FILE* fp;
	// Has the uncompressed gat data of all maps, so just one allocation has to be made
	std::vector<char *> map_cache_buffer = {};

	if( enable_grf )
		ShowStatus("Loading maps (using GRF files)...\n");
	else {
		// Load the map cache files in reverse order to account for import
		const std::vector<std::string> mapcachefilepath = {
			"db/" DBIMPORT "/map_cache.dat",
			"db/" DBPATH "map_cache.dat",
			"db/map_cache.dat",
		};

		for(const auto &mapdat : mapcachefilepath) {
			ShowStatus( "Loading maps (using %s as map cache)...\n", mapdat.c_str() );

			if( ( fp = fopen(mapdat.c_str(), "rb")) == nullptr) {
				ShowFatalError( "Unable to open map cache file " CL_WHITE "%s" CL_RESET "\n", mapdat.c_str());
				continue;
			}

			// Init mapcache data. [Shinryo]
			map_cache_buffer.push_back(map_init_mapcache(fp));

			if( !map_cache_buffer.back() ) {
				ShowFatalError( "Failed to initialize mapcache data (%s)..\n", mapdat.c_str());
				exit(EXIT_FAILURE);
			}

			fclose(fp);
		}
	}

	int32 maps_removed = 0;

	ShowStatus("Loading %d maps.\n", map_num);

	for (int32 i = 0; i < map_num; i++) {
		size_t size;
		bool success = false;
		uint16 idx = 0;
		struct map_data *mapdata = &map[i];
		char map_cache_decode_buffer[MAX_MAP_SIZE];

#ifdef DETAILED_LOADING_OUTPUT
		// show progress
		ShowStatus("Loading maps [%i/%i]: %s" CL_CLL "\r", i, map_num, mapdata->name);
#endif

		if( enable_grf ){
			// try to load the map
			success = map_readgat(mapdata) != 0;
		}else{
			// try to load the map
			for (const auto &cache : map_cache_buffer) {
				if ((success = map_readfromcache(mapdata, cache, map_cache_decode_buffer)) != 0)
					break;
			}
		}

		// The map was not found - remove it
		if (!(idx = mapindex_name2id(mapdata->name)) || !success) {
			map_delmapid(i);
			maps_removed++;
			i--;
			continue;
		}

		mapdata->index = idx;

		if (uidb_get(map_db,(uint32)mapdata->index) != nullptr) {
			ShowWarning("Map %s already loaded!" CL_CLL "\n", mapdata->name);
			if (mapdata->cell) {
				aFree(mapdata->cell);
				mapdata->cell = nullptr;
			}
			map_delmapid(i);
			maps_removed++;
			i--;
			continue;
		}

		map_addmap2db(mapdata);

		mapdata->m = i;
		memset(mapdata->moblist, 0, sizeof(mapdata->moblist));	//Initialize moblist [Skotlex]
		mapdata->mob_delete_timer = INVALID_TIMER;	//Initialize timer [Skotlex]

		mapdata->bxs = (mapdata->xs + BLOCK_SIZE - 1) / BLOCK_SIZE;
		mapdata->bys = (mapdata->ys + BLOCK_SIZE - 1) / BLOCK_SIZE;

		size = mapdata->bxs * mapdata->bys * sizeof(struct block_list*);
		mapdata->block = (struct block_list**)aCalloc(size, 1);
		mapdata->block_mob = (struct block_list**)aCalloc(size, 1);

		memset(&mapdata->save, 0, sizeof(struct point));
		mapdata->damage_adjust = {};
		mapdata->channel = nullptr;
	}

	// intialization and configuration-dependent adjustments of mapflags
	map_flags_init();

	if( !enable_grf ) {
		// The cache isn't needed anymore, so free it. [Shinryo]
		auto it = map_cache_buffer.begin();

		while (it != map_cache_buffer.end()) {
			aFree(*it);
			it = map_cache_buffer.erase(it);
		}
	}

	if (maps_removed)
		ShowNotice("Maps removed: '" CL_WHITE "%d" CL_RESET "'" CL_CLL ".\n", maps_removed);

	// finished map loading
	ShowInfo("Successfully loaded '" CL_WHITE "%d" CL_RESET "' maps." CL_CLL "\n",map_num);

	return 0;
}

////////////////////////////////////////////////////////////////////////
static int32 map_ip_set = 0;
static int32 char_ip_set = 0;

/*==========================================
 * Console Command Parser [Wizputer]
 *------------------------------------------*/
int32 parse_console(const char* buf){
	char type[64];
	char command[64];
	char mapname[MAP_NAME_LENGTH];
	int16 x = 0;
	int16 y = 0;
	int32 n;
	map_session_data sd;

	strcpy(sd.status.name, "console");

	if( ( n = sscanf(buf, "%63[^:]:%63[^:]:%11s %6hd %6hd[^\n]", type, command, mapname, &x, &y) ) < 5 ){
		if( ( n = sscanf(buf, "%63[^:]:%63[^\n]", type, command) ) < 2 )		{
			if((n = sscanf(buf, "%63[^\n]", type))<1) return -1; //nothing to do no arg
		}
	}

	if( n != 5 ){ //end string and display
		if( n < 2 ) {
			ShowNotice("Type of command: '%s'\n", type);
			command[0] = '\0';
			mapname[0] = '\0';
		}
		else {
			ShowNotice("Type of command: '%s' || Command: '%s'\n", type, command);
			mapname[0] = '\0';
		}
	}
	else
		ShowNotice("Type of command: '%s' || Command: '%s' || Map: '%s' Coords: %d %d\n", type, command, mapname, x, y);

	if(strcmpi("admin",type) == 0 ) {
		if(strcmpi("map",command) == 0){
			int16 m = map_mapname2mapid(mapname);
			if( m < 0 ){
				ShowWarning("Console: Unknown map.\n");
				return 0;
			}
			sd.m = m;
			map_search_freecell(&sd, m, &sd.x, &sd.y, -1, -1, 0);
			if( x > 0 )
				sd.x = x;
			if( y > 0 )
				sd.y = y;
			ShowNotice("Now at: '%s' Coords: %d %d\n", mapname, x, y);
		}
		else if( !is_atcommand(sd.fd, &sd, command, 2) )
			ShowInfo("Console: Invalid atcommand.\n");
	}
	else if( n == 2 && strcmpi("server", type) == 0 ){
		if( strcmpi("shutdown", command) == 0 || strcmpi("exit", command) == 0 || strcmpi("quit", command) == 0 ){
			global_core->signal_shutdown();
		}
	}
	else if( strcmpi("ers_report", type) == 0 ){
		ers_report();
	}
	else if( strcmpi("help", type) == 0 ) {
		ShowInfo("Available commands:\n");
		ShowInfo("\t admin:@<atcommand> => Uses an atcommand. Do NOT use commands requiring an attached player.\n");
		ShowInfo("\t admin:map:<map> <x> <y> => Changes the map from which console commands are executed.\n");
		ShowInfo("\t server:shutdown => Stops the server.\n");
		ShowInfo("\t ers_report => Displays database usage.\n");
	}

	return 0;
}

/*==========================================
 * Read map server configuration files (conf/map_athena.conf...)
 *------------------------------------------*/
int32 map_config_read(const char *cfgName)
{
	char line[1024], w1[32], w2[1024];
	FILE *fp;

	fp = fopen(cfgName,"r");
	if( fp == nullptr )
	{
		ShowError("Map configuration file not found at: %s\n", cfgName);
		return 1;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		char* ptr;

		if( line[0] == '/' && line[1] == '/' )
			continue;
		if( (ptr = strstr(line, "//")) != nullptr )
			*ptr = '\n'; //Strip comments
		if( sscanf(line, "%31[^:]: %1023[^\t\r\n]", w1, w2) < 2 )
			continue;

		//Strip trailing spaces
		ptr = w2 + strlen(w2);
		while (--ptr >= w2 && *ptr == ' ');
		ptr++;
		*ptr = '\0';

		if(strcmpi(w1,"timestamp_format")==0)
			safestrncpy(timestamp_format, w2, 20);
		else if(strcmpi(w1,"stdout_with_ansisequence")==0)
			stdout_with_ansisequence = config_switch(w2);
		else if(strcmpi(w1,"console_silent")==0) {
			msg_silent = atoi(w2);
			if( msg_silent ) // only bother if its actually enabled
				ShowInfo("Console Silent Setting: %d\n", atoi(w2));
		} else if (strcmpi(w1, "userid")==0)
			chrif_setuserid(w2);
		else if (strcmpi(w1, "passwd") == 0)
			chrif_setpasswd(w2);
		else if (strcmpi(w1, "char_ip") == 0)
			char_ip_set = chrif_setip(w2);
		else if (strcmpi(w1, "char_port") == 0)
			chrif_setport(atoi(w2));
		else if (strcmpi(w1, "map_ip") == 0)
			map_ip_set = clif_setip(w2);
		else if (strcmpi(w1, "bind_ip") == 0)
			clif_setbindip(w2);
		else if (strcmpi(w1, "map_port") == 0) {
			clif_setport(atoi(w2));
			map_port = (atoi(w2));
		} else if (strcmpi(w1, "map") == 0)
			map_addmap(w2);
		else if (strcmpi(w1, "delmap") == 0)
			map_delmap(w2);
		else if (strcmpi(w1, "npc") == 0)
			npc_addsrcfile(w2, false);
		else if (strcmpi(w1, "delnpc") == 0)
			npc_delsrcfile(w2);
		else if (strcmpi(w1, "autosave_time") == 0) {
			autosave_interval = atoi(w2);
			if (autosave_interval < 1) //Revert to default saving.
				autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
			else
				autosave_interval *= 1000; //Pass from sec to ms
		} else if (strcmpi(w1, "minsave_time") == 0) {
			minsave_interval= atoi(w2);
			if (minsave_interval < 1)
				minsave_interval = 1;
		} else if (strcmpi(w1, "save_settings") == 0)
			save_settings = cap_value(atoi(w2),CHARSAVE_NONE,CHARSAVE_ALL);
		else if (strcmpi(w1, "motd_txt") == 0)
			safestrncpy(motd_txt, w2, sizeof(motd_txt));
		else if (strcmpi(w1, "charhelp_txt") == 0)
			safestrncpy(charhelp_txt, w2, sizeof(charhelp_txt));
		else if (strcmpi(w1, "channel_conf") == 0)
			safestrncpy(channel_conf, w2, sizeof(channel_conf));
		else if(strcmpi(w1,"db_path") == 0)
			safestrncpy(db_path,w2,ARRAYLENGTH(db_path));
		else if (strcmpi(w1, "console") == 0) {
			console = config_switch(w2);
			if (console)
				ShowNotice("Console Commands are enabled.\n");
		} else if (strcmpi(w1, "enable_spy") == 0)
			enable_spy = config_switch(w2);
		else if (strcmpi(w1, "use_grf") == 0)
			enable_grf = config_switch(w2);
		else if (strcmpi(w1, "console_msg_log") == 0)
			console_msg_log = atoi(w2);//[Ind]
		else if (strcmpi(w1, "console_log_filepath") == 0)
			safestrncpy(console_log_filepath, w2, sizeof(console_log_filepath));
		else if (strcmpi(w1, "import") == 0)
			map_config_read(w2);
		else
			ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
	}

	fclose(fp);
	return 0;
}

void map_reloadnpc_sub(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName,"r");
	if( fp == nullptr )
	{
		ShowError("Map configuration file not found at: %s\n", cfgName);
		return;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		char* ptr;

		if( line[0] == '/' && line[1] == '/' )
			continue;
		if( (ptr = strstr(line, "//")) != nullptr )
			*ptr = '\n'; //Strip comments
		if( sscanf(line, "%1023[^:]: %1023[^\t\r\n]", w1, w2) < 2 )
			continue;

		//Strip trailing spaces
		ptr = w2 + strlen(w2);
		while (--ptr >= w2 && *ptr == ' ');
		ptr++;
		*ptr = '\0';

		if (strcmpi(w1, "npc") == 0)
			npc_addsrcfile(w2, false);
		else if (strcmpi(w1, "delnpc") == 0)
			npc_delsrcfile(w2);
		else if (strcmpi(w1, "import") == 0)
			map_reloadnpc_sub(w2);
		else
			ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
	}

	fclose(fp);
}

void map_reloadnpc(bool clear)
{
	if (clear)
		npc_addsrcfile("clear", false); // this will clear the current script list

#ifdef RENEWAL
	map_reloadnpc_sub("npc/re/scripts_main.conf");
#else
	map_reloadnpc_sub("npc/pre-re/scripts_main.conf");
#endif
}

int32 inter_config_read(const char *cfgName)
{
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	fp=fopen(cfgName,"r");
	if(fp==nullptr){
		ShowError("File not found: %s\n",cfgName);
		return 1;
	}
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;
		if( sscanf(line,"%1023[^:]: %1023[^\r\n]",w1,w2) < 2 )
			continue;

#define RENEWALPREFIX "renewal-"
		if (!strncmpi(w1, RENEWALPREFIX, strlen(RENEWALPREFIX))) {
#ifdef RENEWAL
			// Move the original name to the beginning of the string
			memmove(w1, w1 + strlen(RENEWALPREFIX), strlen(w1) - strlen(RENEWALPREFIX) + 1);
#else
			// In Pre-Renewal the Renewal specific configurations can safely be ignored
			continue;
#endif
		}
#undef RENEWALPREFIX

		if( strcmpi( w1, "barter_table" ) == 0 )
			safestrncpy( barter_table, w2, sizeof(barter_table) );
		else if( strcmpi( w1, "buyingstore_db" ) == 0 )
			safestrncpy( buyingstores_table, w2, sizeof(buyingstores_table) );
		else if( strcmpi( w1, "buyingstore_items_table" ) == 0 )
			safestrncpy( buyingstore_items_table, w2, sizeof(buyingstore_items_table) );
		else if(strcmpi(w1,"item_table")==0)
			safestrncpy(item_table,w2,sizeof(item_table));
		else if(strcmpi(w1,"item2_table")==0)
			safestrncpy(item2_table,w2,sizeof(item2_table));
		else if(strcmpi(w1,"mob_table")==0)
			safestrncpy(mob_table,w2,sizeof(mob_table));
		else if(strcmpi(w1,"mob2_table")==0)
			safestrncpy(mob2_table,w2,sizeof(mob2_table));
		else if(strcmpi(w1,"mob_skill_table")==0)
			safestrncpy(mob_skill_table,w2,sizeof(mob_skill_table));
		else if(strcmpi(w1,"mob_skill2_table")==0)
			safestrncpy(mob_skill2_table,w2,sizeof(mob_skill2_table));
		else if( strcmpi( w1, "vending_db" ) == 0 )
			safestrncpy( vendings_table, w2, sizeof(vendings_table) );
		else if( strcmpi( w1, "vending_items_table" ) == 0 )
			safestrncpy(vending_items_table, w2, sizeof(vending_items_table));
		else if( strcmpi( w1, "partybookings_table" ) == 0 )
			safestrncpy(partybookings_table, w2, sizeof(partybookings_table));
		else if( strcmpi(w1, "roulette_table") == 0)
			safestrncpy(roulette_table, w2, sizeof(roulette_table));
		else if (strcmpi(w1, "market_table") == 0)
			safestrncpy(market_table, w2, sizeof(market_table));
		else if (strcmpi(w1, "sales_table") == 0)
			safestrncpy(sales_table, w2, sizeof(sales_table));
		else if (strcmpi(w1, "guild_storage_log") == 0)
			safestrncpy(guild_storage_log_table, w2, sizeof(guild_storage_log_table));
		else
		//Map Server SQL DB
		if(strcmpi(w1,"map_server_ip")==0)
			map_server_ip = w2;
		else
		if(strcmpi(w1,"map_server_port")==0)
			map_server_port=atoi(w2);
		else
		if(strcmpi(w1,"map_server_id")==0)
			map_server_id = w2;
		else
		if(strcmpi(w1,"map_server_pw")==0)
			map_server_pw = w2;
		else
		if(strcmpi(w1,"map_server_db")==0)
			map_server_db = w2;
		else
		if(strcmpi(w1,"default_codepage")==0)
			default_codepage = w2;
		else
		if(strcmpi(w1,"use_sql_db")==0) {
			db_use_sqldbs = config_switch(w2);
			ShowStatus ("Using SQL dbs: %s\n",w2);
		} else
		if(strcmpi(w1,"log_db_ip")==0)
			log_db_ip = w2;
		else
		if(strcmpi(w1,"log_db_id")==0)
			log_db_id = w2;
		else
		if(strcmpi(w1,"log_db_pw")==0)
			log_db_pw = w2;
		else
		if(strcmpi(w1,"log_db_port")==0)
			log_db_port = (uint16)strtoul( w2, nullptr, 10 );
		else
		if(strcmpi(w1,"log_db_db")==0)
			log_db_db = w2;
		else
		if(strcmpi(w1,"start_status_points")==0)
			inter_config.start_status_points=atoi(w2);
		else
		if(strcmpi(w1, "emblem_woe_change")==0)
			inter_config.emblem_woe_change = config_switch(w2) == 1;
		else
		if (strcmpi(w1, "emblem_transparency_limit") == 0) {
			auto val = atoi(w2);
			val = cap_value(val, 0, 100);
			inter_config.emblem_transparency_limit = val;
		}
		if( mapreg_config_read(w1,w2) )
			continue;
		//support the import command, just like any other config
		else
		if(strcmpi(w1,"import")==0)
			inter_config_read(w2);
	}
	fclose(fp);

	return 0;
}

/*=======================================
 *  MySQL Init
 *---------------------------------------*/
int32 map_sql_init(void)
{
	// main db connection
	mmysql_handle = Sql_Malloc();
	qsmysql_handle = Sql_Malloc();

	ShowInfo("Connecting to the Map DB Server....\n");
	if( SQL_ERROR == Sql_Connect(mmysql_handle, map_server_id.c_str(), map_server_pw.c_str(), map_server_ip.c_str(), map_server_port, map_server_db.c_str()) ||
		SQL_ERROR == Sql_Connect(qsmysql_handle, map_server_id.c_str(), map_server_pw.c_str(), map_server_ip.c_str(), map_server_port, map_server_db.c_str()) )
	{
		ShowError("Couldn't connect with uname='%s',host='%s',port='%d',database='%s'\n",
			map_server_id.c_str(), map_server_ip.c_str(), map_server_port, map_server_db.c_str());
		Sql_ShowDebug(mmysql_handle);
		Sql_Free(mmysql_handle);
		Sql_ShowDebug(qsmysql_handle);
		Sql_Free(qsmysql_handle);
		exit(EXIT_FAILURE);
	}
	ShowStatus("Connect success! (Map Server Connection)\n");

	if( !default_codepage.empty() ) {
		if ( SQL_ERROR == Sql_SetEncoding(mmysql_handle, default_codepage.c_str()) )
			Sql_ShowDebug(mmysql_handle);
		if ( SQL_ERROR == Sql_SetEncoding(qsmysql_handle, default_codepage.c_str()) )
			Sql_ShowDebug(qsmysql_handle);
	}
	return 0;
}

int32 map_sql_close(void)
{
	ShowStatus("Close Map DB Connection....\n");
	Sql_Free(mmysql_handle);
	Sql_Free(qsmysql_handle);
	mmysql_handle = nullptr;
	qsmysql_handle = nullptr;

	if (log_config.sql_logs)
	{
		ShowStatus("Close Log DB Connection....\n");
		Sql_Free(logmysql_handle);
		logmysql_handle = nullptr;
	}

	return 0;
}

int32 log_sql_init(void)
{
	// log db connection
	logmysql_handle = Sql_Malloc();

	ShowInfo("" CL_WHITE "[SQL]" CL_RESET ": Connecting to the Log Database " CL_WHITE "%s" CL_RESET " At " CL_WHITE "%s" CL_RESET "...\n",log_db_db.c_str(), log_db_ip.c_str());
	if ( SQL_ERROR == Sql_Connect(logmysql_handle, log_db_id.c_str(), log_db_pw.c_str(), log_db_ip.c_str(), log_db_port, log_db_db.c_str()) ){
		ShowError("Couldn't connect with uname='%s',host='%s',port='%hu',database='%s'\n",
			log_db_id.c_str(), log_db_ip.c_str(), log_db_port, log_db_db.c_str());
		Sql_ShowDebug(logmysql_handle);
		Sql_Free(logmysql_handle);
		exit(EXIT_FAILURE);
	}
	ShowStatus("" CL_WHITE "[SQL]" CL_RESET ": Successfully '" CL_GREEN "connected" CL_RESET "' to Database '" CL_WHITE "%s" CL_RESET "'.\n", log_db_db.c_str());

	if( !default_codepage.empty() )
		if ( SQL_ERROR == Sql_SetEncoding(logmysql_handle, default_codepage.c_str()) )
			Sql_ShowDebug(logmysql_handle);

	return 0;
}

void map_remove_questinfo(int32 m, struct npc_data *nd) {
	struct map_data *mapdata = map_getmapdata(m);

	nullpo_retv(nd);
	nullpo_retv(mapdata);

	util::vector_erase_if_exists(mapdata->qi_npc, nd->id);
	nd->qi_data.clear();
}

static void map_free_questinfo(struct map_data *mapdata) {
	nullpo_retv(mapdata);

	for (const auto &it : mapdata->qi_npc) {
		struct npc_data *nd = map_id2nd(it);

		if (!nd || nd->qi_data.empty())
			continue;

		nd->qi_data.clear();
	}

	mapdata->qi_npc.clear();
}

/**
 * @see DBApply
 */
int32 map_db_final(DBKey key, DBData *data, va_list ap)
{
	struct map_data_other_server *mdos = (struct map_data_other_server *)db_data2ptr(data);
	if(mdos && mdos->cell == nullptr)
		aFree(mdos);
	return 0;
}

/**
 * @see DBApply
 */
int32 nick_db_final(DBKey key, DBData *data, va_list args)
{
	struct charid2nick* p = (struct charid2nick*)db_data2ptr(data);
	struct charid_request* req;

	if( p == nullptr )
		return 0;
	while( p->requests )
	{
		req = p->requests;
		p->requests = req->next;
		aFree(req);
	}
	aFree(p);
	return 0;
}

int32 cleanup_sub(struct block_list *bl, va_list ap)
{
	nullpo_ret(bl);

	switch(bl->type) {
		case BL_PC:
			map_quit((map_session_data *) bl);
			break;
		case BL_NPC:
			npc_unload((struct npc_data *)bl,false);
			break;
		case BL_MOB:
			unit_free(bl,CLR_OUTSIGHT);
			break;
		case BL_PET:
		//There is no need for this, the pet is removed together with the player. [Skotlex]
			break;
		case BL_ITEM:
			map_clearflooritem(bl);
			break;
		case BL_SKILL:
			skill_delunit((struct skill_unit *) bl);
			break;
	}

	return 1;
}

/**
 * Add new skill damage adjustment entry for a map
 * @param m: Map data
 * @param skill_id: Skill ID
 * @param args: Mapflag arguments
 */
void map_skill_damage_add(struct map_data *m, uint16 skill_id, union u_mapflag_args *args) {
	nullpo_retv(m);
	nullpo_retv(args);

	if (m->skill_damage.find(skill_id) != m->skill_damage.end()) { // Entry exists
		args->skill_damage.caster |= m->skill_damage[skill_id].caster;
		m->skill_damage[skill_id] = args->skill_damage;
		return;
	}

	m->skill_damage.insert({ skill_id, args->skill_damage }); // Add new entry
}

/**
 * Add new skill duration adjustment entry for a map
 * @param mapd: Map data
 * @param skill_id: Skill ID to adjust
 * @param per: Skill duration adjustment value in percent
 */
void map_skill_duration_add(struct map_data *mapd, uint16 skill_id, uint16 per) {
	nullpo_retv(mapd);

	if (mapd->skill_duration.find(skill_id) != mapd->skill_duration.end()) // Entry exists
		mapd->skill_duration[skill_id] += per;
	else // Update previous entry
		mapd->skill_duration.insert({ skill_id, per });
}

/**
 * PvP timer handling (starting)
 * @param bl: Player block object
 * @param ap: func* with va_list values
 * @return 0
 */
static int32 map_mapflag_pvp_start_sub(struct block_list *bl, va_list ap)
{
	map_session_data *sd = map_id2sd(bl->id);

	nullpo_retr(0, sd);

	if (sd->pvp_timer == INVALID_TIMER) {
		sd->pvp_timer = add_timer(gettick() + 200, pc_calc_pvprank_timer, sd->id, 0);
		sd->pvp_rank = 0;
		sd->pvp_lastusers = 0;
		sd->pvp_point = 5;
		sd->pvp_won = 0;
		sd->pvp_lost = 0;
	}

	clif_map_property(sd, MAPPROPERTY_FREEPVPZONE, SELF);
	return 0;
}

/**
 * PvP timer handling (stopping)
 * @param bl: Player block object
 * @param ap: func* with va_list values
 * @return 0
 */
static int32 map_mapflag_pvp_stop_sub(struct block_list *bl, va_list ap)
{
	map_session_data* sd = map_id2sd(bl->id);

	clif_pvpset(sd, 0, 0, 2);

	if (sd->pvp_timer != INVALID_TIMER) {
		delete_timer(sd->pvp_timer, pc_calc_pvprank_timer);
		sd->pvp_timer = INVALID_TIMER;
	}

	return 0;
}

/**
 * Return the mapflag enum from the given name.
 * @param name: Mapflag name
 * @return Mapflag enum value
 */
enum e_mapflag map_getmapflag_by_name(char* name)
{
	char flag_constant[255];
	int64 mapflag;

	safesnprintf(flag_constant, sizeof(flag_constant), "mf_%s", name);

	if (!script_get_constant(flag_constant, &mapflag))
		return MF_INVALID;
	else
		return (enum e_mapflag)mapflag;
}

/**
 * Return the mapflag name from the given enum.
 * @param mapflag: Mapflag enum
 * @param output: Stores the mapflag name
 * @return True on success otherwise false
 */
bool map_getmapflag_name( enum e_mapflag mapflag, char* output ){
	const char* constant;
	const char* prefix = "mf_";
	size_t i;
	size_t len = strlen( prefix );

	// Look it up
	constant = script_get_constant_str( prefix, mapflag );

	// Should never happen
	if (constant == nullptr)
		return false;

	// Begin copy after the prefix
	for(i = len; constant[i]; i++)
		output[i-len] = (char)tolower(constant[i]); // Force lowercase
	output[i - len] = 0; // Terminate it

	return true;
}

/**
 * Get a mapflag value
 * @param m: Map ID
 * @param mapflag: Mapflag ID
 * @param args: Arguments for special flags
 * @return Mapflag value on success or -1 on failure
 */
int32 map_getmapflag_sub(int16 m, enum e_mapflag mapflag, union u_mapflag_args *args)
{
	if (m < 0 || m >= MAX_MAP_PER_SERVER) {
		ShowWarning("map_getmapflag: Invalid map ID %d.\n", m);
		return -1;
	}

	struct map_data *mapdata = &map[m];

	if (mapflag < MF_MIN || mapflag >= MF_MAX) {
		ShowWarning("map_getmapflag: Invalid mapflag %d on map %s.\n", mapflag, mapdata->name);
		return -1;
	}

	switch(mapflag) {
		case MF_RESTRICTED:
			return mapdata->zone;
		case MF_NOLOOT:
			return mapdata->getMapFlag(MF_NOMOBLOOT) && mapdata->getMapFlag(MF_NOMVPLOOT);
		case MF_NOPENALTY:
			return mapdata->getMapFlag(MF_NOEXPPENALTY) && mapdata->getMapFlag(MF_NOZENYPENALTY);
		case MF_NOEXP:
			return mapdata->getMapFlag(MF_NOBASEEXP) && mapdata->getMapFlag(MF_NOJOBEXP);
		case MF_SKILL_DAMAGE:
			nullpo_retr(-1, args);

			switch (args->flag_val) {
				case SKILLDMG_PC:
				case SKILLDMG_MOB:
				case SKILLDMG_BOSS:
				case SKILLDMG_OTHER:
					return mapdata->damage_adjust.rate[args->flag_val];
				case SKILLDMG_CASTER:
					return mapdata->damage_adjust.caster;
				default:
					return mapdata->getMapFlag(mapflag);
			}
		default:
			return mapdata->getMapFlag(mapflag);
	}
}

/**
 * Set a mapflag
 * @param m: Map ID
 * @param mapflag: Mapflag ID
 * @param status: true - Set mapflag, false - Remove mapflag
 * @param args: Arguments for special flags
 * @return True on success or false on failure
 */
bool map_setmapflag_sub(int16 m, enum e_mapflag mapflag, bool status, union u_mapflag_args *args)
{
	if (m < 0 || m >= MAX_MAP_PER_SERVER) {
		ShowWarning("map_setmapflag: Invalid map ID %d.\n", m);
		return false;
	}

	struct map_data *mapdata = &map[m];

	if (mapflag < MF_MIN || mapflag >= MF_MAX) {
		ShowWarning("map_setmapflag: Invalid mapflag %d on map %s.\n", mapflag, mapdata->name);
		return false;
	}

	switch(mapflag) {
		case MF_NOSAVE:
			if (status) {
				nullpo_retr(false, args);

				mapdata->save.map = args->nosave.map;
				mapdata->save.x = args->nosave.x;
				mapdata->save.y = args->nosave.y;
			}
			mapdata->setMapFlag(mapflag, status);
			break;
		case MF_PVP:
			mapdata->setMapFlag(mapflag, status); // Must come first to properly set map property
			if (!status) {
				clif_map_property_mapall(m, MAPPROPERTY_NOTHING);
				map_foreachinmap(map_mapflag_pvp_stop_sub, m, BL_PC);
				map_foreachinmap(unit_stopattack, m, BL_CHAR, 0);
			} else {
				if (!battle_config.pk_mode) {
					clif_map_property_mapall(m, MAPPROPERTY_FREEPVPZONE);
					map_foreachinmap(map_mapflag_pvp_start_sub, m, BL_PC);
				}
				if (mapdata->getMapFlag(MF_GVG)) {
					mapdata->setMapFlag(MF_GVG, false);
					ShowWarning("map_setmapflag: Unable to set GvG and PvP flags for the same map! Removing GvG flag from %s.\n", mapdata->name);
				}
				if (mapdata->getMapFlag(MF_GVG_TE)) {
					mapdata->setMapFlag(MF_GVG_TE, false);
					ShowWarning("map_setmapflag: Unable to set GvG TE and PvP flags for the same map! Removing GvG TE flag from %s.\n", mapdata->name);
				}
				if (mapdata->getMapFlag(MF_GVG_DUNGEON)) {
					mapdata->setMapFlag(MF_GVG_DUNGEON, false);
					ShowWarning("map_setmapflag: Unable to set GvG Dungeon and PvP flags for the same map! Removing GvG Dungeon flag from %s.\n", mapdata->name);
				}
				if (mapdata->getMapFlag(MF_GVG_CASTLE)) {
					mapdata->setMapFlag(MF_GVG_CASTLE, false);
					ShowWarning("map_setmapflag: Unable to set GvG Castle and PvP flags for the same map! Removing GvG Castle flag from %s.\n", mapdata->name);
				}
				if (mapdata->getMapFlag(MF_GVG_TE_CASTLE)) {
					mapdata->setMapFlag(MF_GVG_TE_CASTLE, false);
					ShowWarning("map_setmapflag: Unable to set GvG TE Castle and PvP flags for the same map! Removing GvG TE Castle flag from %s.\n", mapdata->name);
				}
				if (mapdata->getMapFlag(MF_BATTLEGROUND)) {
					mapdata->setMapFlag(MF_BATTLEGROUND, false);
					ShowWarning("map_setmapflag: Unable to set Battleground and PvP flags for the same map! Removing Battleground flag from %s.\n", mapdata->name);
				}
			}
			break;
		case MF_GVG:
		case MF_GVG_TE:
			mapdata->setMapFlag(mapflag, status); // Must come first to properly set map property
			if (!status) {
				clif_map_property_mapall(m, MAPPROPERTY_NOTHING);
				map_foreachinmap(unit_stopattack, m, BL_CHAR, 0);
			} else {
				clif_map_property_mapall(m, MAPPROPERTY_AGITZONE);
				if (mapdata->getMapFlag(MF_PVP)) {
					mapdata->setMapFlag(MF_PVP, false);
					if (!battle_config.pk_mode)
						ShowWarning("map_setmapflag: Unable to set PvP and GvG flags for the same map! Removing PvP flag from %s.\n", mapdata->name);
				}
				if (mapdata->getMapFlag(MF_BATTLEGROUND)) {
					mapdata->setMapFlag(MF_BATTLEGROUND, false);
					ShowWarning("map_setmapflag: Unable to set Battleground and GvG flags for the same map! Removing Battleground flag from %s.\n", mapdata->name);
				}
			}
			break;
		case MF_GVG_CASTLE:
		case MF_GVG_TE_CASTLE:
			if (status) {
				if (mapflag == MF_GVG_CASTLE && mapdata->getMapFlag(MF_GVG_TE_CASTLE)) {
					mapdata->setMapFlag(MF_GVG_TE_CASTLE, false);
					ShowWarning("map_setmapflag: Unable to set GvG TE Castle and GvG Castle flags for the same map! Removing GvG TE Castle flag from %s.\n", mapdata->name);
				}
				if (mapflag == MF_GVG_TE_CASTLE && mapdata->getMapFlag(MF_GVG_CASTLE)) {
					mapdata->setMapFlag(MF_GVG_CASTLE, false);
					ShowWarning("map_setmapflag: Unable to set GvG Castle and GvG TE Castle flags for the same map! Removing GvG Castle flag from %s.\n", mapdata->name);
				}
				if (mapdata->getMapFlag(MF_PVP)) {
					mapdata->setMapFlag(MF_PVP, false);
					if (!battle_config.pk_mode)
						ShowWarning("npc_parse_mapflag: Unable to set PvP and GvG%s Castle flags for the same map! Removing PvP flag from %s.\n", (mapflag == MF_GVG_CASTLE ? nullptr : " TE"), mapdata->name);
				}
			}
			mapdata->setMapFlag(mapflag, status);
			break;
		case MF_GVG_DUNGEON:
			if (status && mapdata->getMapFlag(MF_PVP)) {
				mapdata->setMapFlag(MF_PVP, false);
				if (!battle_config.pk_mode)
					ShowWarning("map_setmapflag: Unable to set PvP and GvG Dungeon flags for the same map! Removing PvP flag from %s.\n", mapdata->name);
			}
			mapdata->setMapFlag(mapflag, status);
			break;
		case MF_NOBASEEXP:
		case MF_NOJOBEXP:
			if (status) {
				if (mapflag == MF_NOBASEEXP && mapdata->getMapFlag(MF_BEXP) != 100) {
					mapdata->setMapFlag(MF_BEXP, false);
					ShowWarning("map_setmapflag: Unable to set BEXP and No Base EXP flags for the same map! Removing BEXP flag from %s.\n", mapdata->name);
				}
				if (mapflag == MF_NOJOBEXP && mapdata->getMapFlag(MF_JEXP) != 100) {
					mapdata->setMapFlag(MF_JEXP, false);
					ShowWarning("map_setmapflag: Unable to set JEXP and No Job EXP flags for the same map! Removing JEXP flag from %s.\n", mapdata->name);
				}
			}
			mapdata->setMapFlag(mapflag, status);
			break;
		case MF_PVP_NIGHTMAREDROP:
			if (status) {
				nullpo_retr(false, args);

				if (mapdata->drop_list.size() == MAX_DROP_PER_MAP) {
					ShowWarning("map_setmapflag: Reached the maximum number of drop list items for mapflag pvp_nightmaredrop on %s. Skipping.\n", mapdata->name);
					break;
				}

				struct s_drop_list entry;

				entry.drop_id = args->nightmaredrop.drop_id;
				entry.drop_type = args->nightmaredrop.drop_type;
				entry.drop_per = args->nightmaredrop.drop_per;
				mapdata->drop_list.push_back(entry);
			}
			mapdata->setMapFlag(mapflag, status);
			break;
		case MF_RESTRICTED:
			if (!status) {
				if (args == nullptr) {
					mapdata->zone = 0;
				} else {
					mapdata->zone ^= (1 << (args->flag_val + 1)) << 3;
				}

				// Don't completely disable the mapflag's status if other zones are active
				if (mapdata->zone == 0) {
					mapdata->setMapFlag(mapflag, status);
				}
			} else {
				nullpo_retr(false, args);

				mapdata->zone |= (1 << (args->flag_val + 1)) << 3;
				mapdata->setMapFlag(mapflag, status);
			}
			break;
		case MF_NOCOMMAND:
			if (status) {
				nullpo_retr(false, args);

				mapdata->setMapFlag(mapflag, ((args->flag_val <= 0) ? 100 : args->flag_val));
			} else
				mapdata->setMapFlag(mapflag, false);
			break;
		case MF_JEXP:
		case MF_BEXP:
			if (status) {
				nullpo_retr(false, args);

				if (mapflag == MF_JEXP && mapdata->getMapFlag(MF_NOJOBEXP)) {
					mapdata->setMapFlag(MF_NOJOBEXP, false);
					ShowWarning("map_setmapflag: Unable to set No Job EXP and JEXP flags for the same map! Removing No Job EXP flag from %s.\n", mapdata->name);
				}
				if (mapflag == MF_BEXP && mapdata->getMapFlag(MF_NOBASEEXP)) {
					mapdata->setMapFlag(MF_NOBASEEXP, false);
					ShowWarning("map_setmapflag: Unable to set No Base EXP and BEXP flags for the same map! Removing No Base EXP flag from %s.\n", mapdata->name);
				}
				mapdata->setMapFlag(mapflag, args->flag_val);
			} else
				mapdata->setMapFlag(mapflag, false);
			break;
		case MF_SPECIALPOPUP:
			if (status) {
				nullpo_retr(false, args);

				mapdata->setMapFlag(mapflag, args->flag_val);
			} else
				mapdata->setMapFlag(mapflag, false);
			break;
		case MF_BATTLEGROUND:
			if (status) {
				nullpo_retr(false, args);

				if (mapdata->getMapFlag(MF_PVP)) {
					mapdata->setMapFlag(MF_PVP, false);
					if (!battle_config.pk_mode)
						ShowWarning("map_setmapflag: Unable to set PvP and Battleground flags for the same map! Removing PvP flag from %s.\n", mapdata->name);
				}
				if (mapdata->getMapFlag(MF_GVG)) {
					mapdata->setMapFlag(MF_GVG, false);
					ShowWarning("map_setmapflag: Unable to set GvG and Battleground flags for the same map! Removing GvG flag from %s.\n", mapdata->name);
				}
				if (mapdata->getMapFlag(MF_GVG_DUNGEON)) {
					mapdata->setMapFlag(MF_GVG_DUNGEON, false);
					ShowWarning("map_setmapflag: Unable to set GvG Dungeon and Battleground flags for the same map! Removing GvG Dungeon flag from %s.\n", mapdata->name);
				}
				if (mapdata->getMapFlag(MF_GVG_CASTLE)) {
					mapdata->setMapFlag(MF_GVG_CASTLE, false);
					ShowWarning("map_setmapflag: Unable to set GvG Castle and Battleground flags for the same map! Removing GvG Castle flag from %s.\n", mapdata->name);
				}
				mapdata->setMapFlag(mapflag, ((args->flag_val <= 0 || args->flag_val > 2) ? 1 : args->flag_val));
			} else
				mapdata->setMapFlag(mapflag, false);
			break;
		case MF_NOLOOT:
			mapdata->setMapFlag(MF_NOMOBLOOT, status);
			mapdata->setMapFlag(MF_NOMVPLOOT, status);
			break;
		case MF_NOPENALTY:
			mapdata->setMapFlag(MF_NOEXPPENALTY, status);
			mapdata->setMapFlag(MF_NOZENYPENALTY, status);
			break;
		case MF_NOEXP:
			mapdata->setMapFlag(MF_NOBASEEXP, status);
			mapdata->setMapFlag(MF_NOJOBEXP, status);
			break;
		case MF_SKILL_DAMAGE:
			if (!status) {
				mapdata->damage_adjust = {};
				mapdata->skill_damage.clear();
			} else {
				nullpo_retr(false, args);

				if (!args->flag_val) { // Signifies if it's a single skill or global damage adjustment
					if (args->skill_damage.caster == 0) {
						ShowError("map_setmapflag: Skill damage adjustment without casting type for map %s.\n", mapdata->name);
						return false;
					}

					mapdata->damage_adjust.caster |= args->skill_damage.caster;
					for (int32 i = SKILLDMG_PC; i < SKILLDMG_MAX; i++)
						mapdata->damage_adjust.rate[i] = cap_value(args->skill_damage.rate[i], -100, 100000);
				}
			}
			mapdata->setMapFlag(mapflag, status);
			break;
		case MF_SKILL_DURATION:
			if (!status)
				mapdata->skill_duration.clear();
			else {
				nullpo_retr(false, args);

				map_skill_duration_add(mapdata, args->skill_duration.skill_id, args->skill_duration.per);
			}
			mapdata->setMapFlag(mapflag, status);
			break;
		default:
			mapdata->setMapFlag(mapflag, status);
			break;
	}

	return true;
}

/**
 * @see DBApply
 */
static int32 cleanup_db_sub(DBKey key, DBData *data, va_list va)
{
	return cleanup_sub((struct block_list *)db_data2ptr(data), va);
}

/*==========================================
 * map destructor
 *------------------------------------------*/
void MapServer::finalize(){
	ShowStatus("Terminating...\n");
	channel_config.closing = true;

	//Ladies and babies first.
	struct s_mapiterator* iter = mapit_getallusers();
	for( map_session_data* sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
		map_quit(sd);
	mapit_free(iter);

	for (int32 i = 0; i < map_num; i++) {
		map_free_questinfo(map_getmapdata(i));
	}

	/* prepares npcs for a faster shutdown process */
	do_clear_npc();

	// remove all objects on maps
	ShowStatus("Cleaning up %d maps.\n", map_num);
	for (int32 i = 0; i < map_num; i++) {
		struct map_data *mapdata = map_getmapdata(i);
#ifdef DEBUG
		ShowStatus("Cleaning up maps [%d/%d]: %s..." CL_CLL "\r", i++, map_num, mapdata->name);
#endif
		map_foreachinmap(cleanup_sub, i, BL_ALL);
		channel_delete(mapdata->channel,false);
	}
	ShowStatus("Cleaned up %d maps." CL_CLL "\n", map_num);

	id_db->foreach(id_db,cleanup_db_sub);
	chrif_char_reset_offline();
	chrif_flush_fifo();

	do_final_atcommand();
	do_final_battle();
	do_final_chrif();
	do_final_clan();
#ifndef MAP_GENERATOR
	do_final_clif();
#endif
	do_final_npc();
	do_final_quest();
	do_final_achievement();
	do_final_script();
	do_final_instance();
	do_final_itemdb();
	do_final_storage();
	do_final_guild();
	do_final_party();
	do_final_pc();
	do_final_pet();
	do_final_homunculus();
	do_final_mercenary();
	do_final_mob(false);
	do_final_msg();
	do_final_skill();
	do_final_status();
	do_final_unit();
	do_final_battleground();
	do_final_duel();
	do_final_elemental();
	do_final_cashshop();
	do_final_channel(); //should be called after final guild
	do_final_vending();
	do_final_buyingstore();
	do_final_path();

	map_db->destroy(map_db, map_db_final);

	for (int32 i = 0; i < map_num; i++) {
		struct map_data *mapdata = map_getmapdata(i);

		if(mapdata->cell) aFree(mapdata->cell);
		if(mapdata->block) aFree(mapdata->block);
		if(mapdata->block_mob) aFree(mapdata->block_mob);
		if(battle_config.dynamic_mobs) { //Dynamic mobs flag by [random]
			if(mapdata->mob_delete_timer != INVALID_TIMER)
				delete_timer(mapdata->mob_delete_timer, map_removemobs_timer);
			for (int32 j=0; j<MAX_MOB_LIST_PER_MAP; j++)
				if (mapdata->moblist[j]) aFree(mapdata->moblist[j]);
		}
		mapdata->damage_adjust = {};
	}

	mapindex_final();
	if(enable_grf)
		grfio_final();

	id_db->destroy(id_db, nullptr);
	pc_db->destroy(pc_db, nullptr);
	mobid_db->destroy(mobid_db, nullptr);
	bossid_db->destroy(bossid_db, nullptr);
	nick_db->destroy(nick_db, nick_db_final);
	charid_db->destroy(charid_db, nullptr);
	iwall_db->destroy(iwall_db, nullptr);
	regen_db->destroy(regen_db, nullptr);

	map_sql_close();

	ShowStatus("Finished.\n");
}

static int32 map_abort_sub(map_session_data* sd, va_list ap)
{
	chrif_save(sd, CSAVE_QUIT|CSAVE_INVENTORY|CSAVE_CART);
	return 1;
}


//------------------------------
// Function called when the server
// has received a crash signal.
//------------------------------
void MapServer::handle_crash(){
	static int32 run = 0;
	//Save all characters and then flush the inter-connection.
	if (run) {
		ShowFatalError("Server has crashed while trying to save characters. Character data can't be saved!\n");
		return;
	}
	run = 1;
	if (!chrif_isconnected())
	{
		if (pc_db->size(pc_db))
			ShowFatalError("Server has crashed without a connection to the char-server, %u characters can't be saved!\n", pc_db->size(pc_db));
		return;
	}
	ShowError("Server received crash signal! Attempting to save all online characters!\n");
	map_foreachpc(map_abort_sub);
	chrif_flush_fifo();
}

/*======================================================
 * Map-Server help options screen
 *------------------------------------------------------*/
void display_helpscreen(bool do_exit)
{
	ShowInfo("Usage: %s [options]\n", SERVER_NAME);
	ShowInfo("\n");
	ShowInfo("Options:\n");
	ShowInfo("  -?, -h [--help]\t\tDisplays this help screen.\n");
	ShowInfo("  -v [--version]\t\tDisplays the server's version.\n");
	ShowInfo("  --run-once\t\t\tCloses server after loading (testing).\n");
	ShowInfo("  --map-config <file>\t\tAlternative map-server configuration.\n");
	ShowInfo("  --battle-config <file>\tAlternative battle configuration.\n");
	ShowInfo("  --atcommand-config <file>\tAlternative atcommand configuration.\n");
	ShowInfo("  --script-config <file>\tAlternative script configuration.\n");
	ShowInfo("  --msg-config <file>\t\tAlternative message configuration.\n");
	ShowInfo("  --grf-path <file>\t\tAlternative GRF path configuration.\n");
	ShowInfo("  --inter-config <file>\t\tAlternative inter-server configuration.\n");
	ShowInfo("  --log-config <file>\t\tAlternative logging configuration.\n");
	if( do_exit )
		exit(EXIT_SUCCESS);
}

/*======================================================
 * Message System
 *------------------------------------------------------*/
struct msg_data {
	char* msg[MAP_MAX_MSG];
};
struct msg_data *map_lang2msgdb(uint8 lang){
	return (struct msg_data*)idb_get(map_msg_db, lang);
}

void map_do_init_msg(void){
	int32 test=0, i=0, size;
	const char * listelang[] = {
		MSG_CONF_NAME_EN,	//default
		MSG_CONF_NAME_RUS,
		MSG_CONF_NAME_SPN,
		MSG_CONF_NAME_GRM,
		MSG_CONF_NAME_CHN,
		MSG_CONF_NAME_MAL,
		MSG_CONF_NAME_IDN,
		MSG_CONF_NAME_FRN,
		MSG_CONF_NAME_POR,
		MSG_CONF_NAME_THA
	};

	map_msg_db = idb_alloc(DB_OPT_BASE);
	size = ARRAYLENGTH(listelang); //avoid recalc
	while(test!=-1 && size>i){ //for all enable lang +(English default)
		test = msg_checklangtype(i,false);
		if(test == 1) msg_config_read(listelang[i],i); //if enabled read it and assign i to langtype
		i++;
	}
}
void map_do_final_msg(void){
	DBIterator *iter = db_iterator(map_msg_db);
	struct msg_data *mdb;

	for (mdb = (struct msg_data *)dbi_first(iter); dbi_exists(iter); mdb = (struct msg_data *)dbi_next(iter)) {
		_do_final_msg(MAP_MAX_MSG,mdb->msg);
		aFree(mdb);
	}
	dbi_destroy(iter);
	map_msg_db->destroy(map_msg_db, nullptr);
}
void map_msg_reload(void){
	map_do_final_msg(); //clear data
	map_do_init_msg();
}
int32 map_msg_config_read(const char *cfgName, int32 lang){
	struct msg_data *mdb;

	if( (mdb = map_lang2msgdb(lang)) == nullptr )
		CREATE(mdb, struct msg_data, 1);
	else
		idb_remove(map_msg_db, lang);
	idb_put(map_msg_db, lang, mdb);

	if(_msg_config_read(cfgName,MAP_MAX_MSG,mdb->msg)!=0){ //an error occur
		idb_remove(map_msg_db, lang); //@TRYME
		aFree(mdb);
	}
	return 0;
}
const char* map_msg_txt(map_session_data *sd, int32 msg_number){
	struct msg_data *mdb;
	uint8 lang = 0; //default
	if(sd && sd->langtype) lang = sd->langtype;

	if( (mdb = map_lang2msgdb(lang)) != nullptr){
		const char *tmp = _msg_txt(msg_number,MAP_MAX_MSG,mdb->msg);
		if(strcmp(tmp,"??")) //to verify result
			return tmp;
		ShowDebug("Message #%d not found for langtype %d.\n",msg_number,lang);
	}
	ShowDebug("Selected langtype %d not loaded, trying fallback...\n",lang);
	if(lang != 0 && (mdb = map_lang2msgdb(0)) != nullptr) //fallback
		return _msg_txt(msg_number,MAP_MAX_MSG,mdb->msg);
	return "??";
}

/**
 * Read the option specified in command line
 *  and assign the confs used by the different server.
 * @param argc: Argument count
 * @param argv: Argument values
 * @return true or Exit on failure.
 */
int32 mapgenerator_get_options(int32 argc, char** argv) {
#ifdef MAP_GENERATOR
	bool optionSet = false;
	for (int32 i = 1; i < argc; i++) {
		const char *arg = argv[i];
		if (arg[0] != '-' && (arg[0] != '/' || arg[1] == '-')) {// -, -- and /
		} else if (arg[0] == '/' || (++arg)[0] == '-') {// long option
			arg++;

			if (strcmp(arg, "generate-navi") == 0) {
				gen_options.navi = true;
			} else if (strcmp(arg, "generate-itemmoveinfo") == 0) {
				gen_options.itemmoveinfo = true;
			} else if (strcmp(arg, "generate-reputation") == 0) {
				gen_options.reputation = true;
			} else {
				// pass through to default get_options
				continue;
			}

			// clear option
			argv[i] = nullptr;
			optionSet = true;
		}
	}
	if (!optionSet) {
		ShowError("No options passed to the map generator, you must set at least one.\n");
		exit(1);
	}
#endif
	return 1;
}

int32 map_data::getMapFlag(int32 flag) const {
#ifdef DEBUG
	if (flag < 0 || flag > flags.size()) {
		// This is debugged because we didn't previously check for out-of-bounds
		ShowError("map_data::getMapFlag: flag %d out of bounds (0-%d)\n", flag, flags.size() - 1);
		return 0;
	}
#endif
	return flags[flag];
}

void map_data::setMapFlag(int32 flag, int32 value) {
#ifdef DEBUG
	if (flag < 0 || flag > flags.size()) {
		// This is debugged because we didn't previously check for out-of-bounds
		ShowError("map_data::getMapFlag: flag %d out of bounds (0-%d)\n", flag, flags.size() - 1);
		return;
	}
#endif
	flags[flag] = value;
}

void map_data::initMapFlags() {
	flags.clear();
	flags.resize(MF_MAX, 0);
}

void map_data::copyFlags(const map_data& other) {
	flags = other.flags;
}

/// Called when a terminate signal is received.
void MapServer::handle_shutdown(){
	ShowStatus("Shutting down...\n");

	map_session_data* sd;
	struct s_mapiterator* iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
		clif_GM_kick(nullptr, sd);
	mapit_free(iter);
	flush_fifos();
}

bool MapServer::initialize( int32 argc, char *argv[] ){
#ifdef GCOLLECT
	GC_enable_incremental();
#endif

	safestrncpy(console_log_filepath, "./log/map-msg_log.log", sizeof(console_log_filepath));

	/* Multilanguage */
	MSG_CONF_NAME_EN = "conf/msg_conf/map_msg.conf"; // English (default)
	MSG_CONF_NAME_RUS = "conf/msg_conf/map_msg_rus.conf";	// Russian
	MSG_CONF_NAME_SPN = "conf/msg_conf/map_msg_spn.conf";	// Spanish
	MSG_CONF_NAME_GRM = "conf/msg_conf/map_msg_grm.conf";	// German
	MSG_CONF_NAME_CHN = "conf/msg_conf/map_msg_chn.conf";	// Chinese
	MSG_CONF_NAME_MAL = "conf/msg_conf/map_msg_mal.conf";	// Malaysian
	MSG_CONF_NAME_IDN = "conf/msg_conf/map_msg_idn.conf";	// Indonesian
	MSG_CONF_NAME_FRN = "conf/msg_conf/map_msg_frn.conf";	// French
	MSG_CONF_NAME_POR = "conf/msg_conf/map_msg_por.conf";	// Brazilian Portuguese
	MSG_CONF_NAME_THA = "conf/msg_conf/map_msg_tha.conf";	// Thai
	/* Multilanguage */

	// default inter_config
	inter_config.start_status_points = 48;
	inter_config.emblem_woe_change = true;
	inter_config.emblem_transparency_limit = 80;

#ifdef MAP_GENERATOR
	mapgenerator_get_options(argc, argv);
#endif
	cli_get_options(argc,argv);

	map_config_read(MAP_CONF_NAME);

	if (save_settings == CHARSAVE_NONE)
		ShowWarning("Value of 'save_settings' is not set, player's data only will be saved every 'autosave_time' (%d seconds).\n", autosave_interval/1000);

	// loads npcs
	map_reloadnpc(false);

	chrif_checkdefaultlogin();

	if (!map_ip_set || !char_ip_set) {
		char ip_str[16];
		ip2str(addr_[0], ip_str);

#if !defined(BUILDBOT)
		ShowWarning( "Not all IP addresses in map_athena.conf configured, autodetecting...\n" );
#endif

		if (naddr_ == 0)
			ShowError("Unable to determine your IP address...\n");
		else if (naddr_ > 1)
			ShowNotice("Multiple interfaces detected...\n");

		ShowInfo("Defaulting to %s as our IP address\n", ip_str);

		if (!map_ip_set)
			clif_setip(ip_str);
		if (!char_ip_set)
			chrif_setip(ip_str);
	}

	battle_config_read(BATTLE_CONF_FILENAME);
	script_config_read(SCRIPT_CONF_NAME);
	inter_config_read(INTER_CONF_NAME);
	log_config_read(LOG_CONF_NAME);

	id_db = idb_alloc(DB_OPT_BASE);
	pc_db = idb_alloc(DB_OPT_BASE);	//Added for reliable map_id2sd() use. [Skotlex]
	mobid_db = idb_alloc(DB_OPT_BASE);	//Added to lower the load of the lazy mob ai. [Skotlex]
	bossid_db = idb_alloc(DB_OPT_BASE); // Used for Convex Mirror quick MVP search
	map_db = uidb_alloc(DB_OPT_BASE);
	nick_db = idb_alloc(DB_OPT_BASE);
	charid_db = uidb_alloc(DB_OPT_BASE);
	regen_db = idb_alloc(DB_OPT_BASE); // efficient status_natural_heal processing
	iwall_db = strdb_alloc(DB_OPT_RELEASE_DATA,2*NAME_LENGTH+2+1); // [Zephyrus] Invisible Walls

	map_sql_init();
	if (log_config.sql_logs)
		log_sql_init();

	mapindex_init();
	if(enable_grf)
		grfio_init(GRF_PATH_FILENAME);

	map_readallmaps();

	add_timer_func_list(map_freeblock_timer, "map_freeblock_timer");
	add_timer_func_list(map_clearflooritem_timer, "map_clearflooritem_timer");
	add_timer_func_list(map_removemobs_timer, "map_removemobs_timer");
	add_timer_interval(gettick()+1000, map_freeblock_timer, 0, 0, 60*1000);
	
	map_do_init_msg();
	do_init_path();
	do_init_atcommand();
	do_init_battle();
	do_init_instance();
	do_init_chrif();
	do_init_clan();
#ifndef MAP_GENERATOR
	do_init_clif();
#endif
	do_init_script();
	do_init_itemdb();
	do_init_channel();
	do_init_cashshop();
	do_init_skill();
	do_init_mob();
	do_init_pc();
	do_init_status();
	do_init_party();
	do_init_guild();
	do_init_storage();
	do_init_pet();
	do_init_homunculus();
	do_init_mercenary();
	do_init_elemental();
	do_init_quest();
	do_init_achievement();
	do_init_battleground();
	do_init_npc();
	do_init_unit();
	do_init_duel();
	do_init_vending();
	do_init_buyingstore();

	npc_event_do_oninit();	// Init npcs (OnInit)

	if (battle_config.pk_mode)
		ShowNotice("Server is running on '" CL_WHITE "PK Mode" CL_RESET "'.\n");

#ifndef MAP_GENERATOR
	ShowStatus("Server is '" CL_GREEN "ready" CL_RESET "' and listening on port '" CL_WHITE "%d" CL_RESET "'.\n\n", map_port);
#else
	// depending on gen_options, generate the correct things
	if (gen_options.navi)
		navi_create_lists();
	if (gen_options.itemmoveinfo)
		itemdb_gen_itemmoveinfo();
	if (gen_options.reputation)
		pc_reputation_generate();
	this->signal_shutdown();
#endif

	if( console ){ //start listening
		add_timer_func_list(parse_console_timer, "parse_console_timer");
		add_timer_interval(gettick()+1000, parse_console_timer, 0, 0, 1000); //start in 1s each 1sec
	}

	return true;
}

int32 main( int32 argc, char *argv[] ){
	return main_core<MapServer>( argc, argv );
}
