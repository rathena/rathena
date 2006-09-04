// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "../common/core.h"
#include "../common/timer.h"
#include "../common/grfio.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/showmsg.h"
#include "../common/version.h"
#include "../common/nullpo.h"

#include "map.h"
#include "chrif.h"
#include "clif.h"
#include "intif.h"
#include "npc.h"
#include "pc.h"
#include "status.h"
#include "mob.h"
#include "chat.h"
#include "itemdb.h"
#include "storage.h"
#include "skill.h"
#include "trade.h"
#include "party.h"
#include "unit.h"
#include "battle.h"
#include "script.h"
#include "guild.h"
#include "pet.h"
#include "mercenary.h"	//[orn]
#include "atcommand.h"
#include "charcommand.h"

#include "log.h"

#include "charsave.h"

#include "irc.h"

// maybe put basic macros to somewhere else
#define swap(a,b) ((a == b) || ((a ^= b), (b ^= a), (a ^= b)))

#ifndef TXT_ONLY

#include "mail.h" // mail system [Valaris]

MYSQL mmysql_handle;
MYSQL_RES* 	sql_res ;
MYSQL_ROW	sql_row ;
char tmp_sql[65535]="";

MYSQL logmysql_handle; //For the log database - fix by [Maeki]
MYSQL_RES* logsql_res ;
MYSQL_ROW  logsql_row ;

MYSQL mail_handle; // mail system [Valaris]
MYSQL_RES* 	mail_res ;
MYSQL_ROW	mail_row ;

int map_server_port = 3306;
char map_server_ip[16] = "127.0.0.1";
char map_server_id[32] = "ragnarok";
char map_server_pw[32] = "ragnarok";
char map_server_db[32] = "ragnarok";
char default_codepage[32] = ""; //Feature by irmin.
int db_use_sqldbs = 0;
int connection_ping_interval = 0;

int login_server_port = 3306;
char login_server_ip[16] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "ragnarok";
char login_server_db[32] = "ragnarok";

char item_db_db[32] = "item_db";
char item_db2_db[32] = "item_db2";
char mob_db_db[32] = "mob_db";
char mob_db2_db[32] = "mob_db2";

char login_db[32] = "login";
char login_db_level[32] = "level";
char login_db_account_id[32] = "account_id";

int log_db_port = 3306;
char log_db_ip[16] = "127.0.0.1";
char log_db_id[32] = "ragnarok";
char log_db_pw[32] = "ragnarok";
char log_db[32] = "log";

int mail_server_port = 3306;
char mail_server_ip[16] = "127.0.0.1";
char mail_server_id[32] = "ragnarok";
char mail_server_pw[32] = "ragnarok";
char mail_server_db[32] = "ragnarok";
int mail_server_enable = 0;

char gm_db[32] = "login";
char gm_db_level[32] = "level";
char gm_db_account_id[32] = "account_id";

int read_gm_interval = 600000;

char char_db[32] = "char";

char mail_db[32] = "mail";

char charsql_host[40] = "localhost";
int charsql_port = 3306;
char charsql_user[32] = "ragnarok";
char charsql_pass[32] = "eAthena";
char charsql_db[40] = "ragnarok";
MYSQL charsql_handle;
MYSQL_RES* charsql_res;
MYSQL_ROW charsql_row;

#endif /* not TXT_ONLY */

int lowest_gm_level = 1;

// This param using for sending mainchat
// messages like whispers to this nick. [LuzZza]
char main_chat_nick[16] = "Main";

char *INTER_CONF_NAME;
char *LOG_CONF_NAME;
char *MAP_CONF_NAME;
char *BATTLE_CONF_FILENAME;
char *ATCOMMAND_CONF_FILENAME;
char *CHARCOMMAND_CONF_FILENAME;
char *SCRIPT_CONF_NAME;
char *MSG_CONF_NAME;
char *GRF_PATH_FILENAME;

// 極力 staticでロ?カルに?める
static struct dbt * id_db=NULL;
static struct dbt * pc_db=NULL;
static struct dbt * map_db=NULL;
static struct dbt * charid_db=NULL;

static int map_users=0;
static struct block_list *objects[MAX_FLOORITEM];
static int first_free_object_id=0,last_object_id=0;

#define block_free_max 1048576
struct block_list *block_free[block_free_max];
static int block_free_count = 0, block_free_lock = 0;

#define BL_LIST_MAX 1048576
static struct block_list *bl_list[BL_LIST_MAX];
static int bl_list_count = 0;

static char afm_dir[1024] = ""; // [Valaris]

struct map_data map[MAX_MAP_PER_SERVER];
int map_num = 0;

int map_port=0;

int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
int minsave_interval = 100;
int save_settings = 0xFFFF;
int charsave_method = 0; //Default 'OLD' Save method (SQL ONLY!) [Sirius]
int agit_flag = 0;
int night_flag = 0; // 0=day, 1=night [Yor]

struct charid2nick {
	char nick[NAME_LENGTH];
	int req_id;
};

// ｫﾞｫﾃｫﾗｫｭｫ罩ﾃｫｷｫ袮ﾗ鯑ｫﾕｫ鬮ｰ(map_athana.conf?ｪﾎread_map_from_cacheｪﾇ・)
// 0:ﾗﾗ鯑ｪｷｪﾊｪ､ 1:ﾞｪ?ﾜﾁ 2:?ﾜﾁ
int  map_read_flag = READ_FROM_GAT;
char map_cache_file[256]="db/map.info"; // ｫﾞｫﾃｫﾗｫｭｫ罩ﾃｫｷｫ雖ﾕｫ｡ｫ､ｫ・｣

char db_path[256] = "db";
char motd_txt[256] = "conf/motd.txt";
char help_txt[256] = "conf/help.txt";
char help2_txt[256] = "conf/help2.txt";
char charhelp_txt[256] = "conf/charhelp.txt";

char wisp_server_name[NAME_LENGTH] = "Server"; // can be modified in char-server configuration file

int console = 0;
int enable_spy = 0; //To enable/disable @spy commands, which consume too much cpu time when sending packets. [Skotlex]

/*==========================================
 * 全map鯖?計での接??設定
 * (char鯖から送られてくる)
 *------------------------------------------
 */
void map_setusers(int fd)
{
	RFIFOHEAD(fd);
	WFIFOHEAD(fd, 2);

	map_users = RFIFOL(fd,2);
	// send some anser
	WFIFOW(fd,0) = 0x2718;
	WFIFOSET(fd,2);
}

/*==========================================
 * 全map鯖?計での接??取得 (/wへの?答用)
 *------------------------------------------
 */
int map_getusers(void) {
	return map_users;
}

//Distance functions, taken from http://www.flipcode.com/articles/article_fastdistance.shtml
int check_distance(int dx, int dy, int distance) {
#ifdef CIRCULAR_AREA
	//In this case, we just do a square comparison. Add 1 tile grace for diagonal range checks.
	return (dx*dx + dy*dy <= distance*distance + (dx&&dy?1:0));
#else
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	return ((dx<dy?dy:dx) <= distance);
#endif
}

unsigned int distance(int dx, int dy) {
#ifdef CIRCULAR_AREA
	unsigned int min, max;

	if ( dx < 0 ) dx = -dx;
	if ( dy < 0 ) dy = -dy;
	//There appears to be something wrong with the aproximation below when either dx/dy is 0! [Skotlex]
	if ( dx == 0 ) return dy;
	if ( dy == 0 ) return dx;
	
	if ( dx < dy )
	{
		min = dx;
		max = dy;
	} else {
		min = dy;
		max = dx;
	}
   // coefficients equivalent to ( 123/128 * max ) and ( 51/128 * min )
	return ((( max << 8 ) + ( max << 3 ) - ( max << 4 ) - ( max << 1 ) +
		( min << 7 ) - ( min << 5 ) + ( min << 3 ) - ( min << 1 )) >> 8 );
#else
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	return (dx<dy?dy:dx);
#endif
}

//
// block削除の安全性確保?理
//

/*==========================================
 * blockをfreeするときfreeの?わりに呼ぶ
 * ロックされているときはバッファにためる
 *------------------------------------------
 */
int map_freeblock (struct block_list *bl)
{
	if (block_free_lock == 0 || block_free_count >= block_free_max)
	{
		aFree(bl);
		bl = NULL;
		if (block_free_count >= block_free_max)
			if (battle_config.error_log)
				ShowWarning("map_freeblock: too many free block! %d %d\n",
					block_free_count, block_free_lock);
	} else
		block_free[block_free_count++] = bl;

	return block_free_lock;
}
/*==========================================
 * blockのfreeを一市Iに禁止する
 *------------------------------------------
 */
int map_freeblock_lock (void)
{
	return ++block_free_lock;
}

/*==========================================
 * blockのfreeのロックを解除する
 * このとき、ロックが完全になくなると
 * バッファにたまっていたblockを全部削除
 *------------------------------------------
 */
//int map_freeblock_unlock (void)
int map_freeblock_unlock_sub(char *file, int lineno)
{
	if ((--block_free_lock) == 0) {
		int i;
		for (i = 0; i < block_free_count; i++)
		{	//Directly calling aFree shouldn't be a leak, as Free remembers the size the original pointed to memory was allocated with? [Skotlex]
//			aFree(block_free[i]);
//			_mfree(block_free[i], file, lineno, __func__);
			_mfree(block_free[i], file, block_free[i]->type*100000+lineno, __func__);
			block_free[i] = NULL;
		}
		block_free_count = 0;
	} else if (block_free_lock < 0) {
		if (battle_config.error_log)
			ShowError("map_freeblock_unlock: lock count < 0 !\n");
		block_free_lock = 0; // 次回以降のロックに支障が出てくるのでリセット
	}

	return block_free_lock;
}

// map_freeblock_lock() を呼んで map_freeblock_unlock() を呼ばない
// 関数があったので、定期的にblock_free_lockをリセットするようにする。
// この関数は、do_timer() のトップレベルから呼ばれるので、
// block_free_lock を直接いじっても支障無いはず。

int map_freeblock_timer (int tid, unsigned int tick, int id, int data)
{
	if (block_free_lock > 0) {
		ShowError("map_freeblock_timer: block_free_lock(%d) is invalid.\n", block_free_lock);
		block_free_lock = 1;
		map_freeblock_unlock();
	}

	return 0;
}

//
// block化?理
//
/*==========================================
 * map[]のblock_listから?がっている場合に
 * bl->prevにbl_headのアドレスを入れておく
 *------------------------------------------
 */
static struct block_list bl_head;

#ifdef CELL_NOSTACK
/*==========================================
 * These pair of functions update the counter of how many objects
 * lie on a tile.
 *------------------------------------------
 */
void map_addblcell(struct block_list *bl)
{
	if(bl->m<0 || bl->x<0 || bl->x>=map[bl->m].xs
		|| bl->y<0 || bl->y>=map[bl->m].ys || !(bl->type&BL_CHAR))
		return;
	map[bl->m].cell_bl[bl->x+bl->y*map[bl->m].xs]++;
	return;
}

void map_delblcell(struct block_list *bl)
{
	if(bl->m <0 || bl->x<0 || bl->x>=map[bl->m].xs
		|| bl->y<0 || bl->y>=map[bl->m].ys || !(bl->type&BL_CHAR))
		return;
	map[bl->m].cell_bl[bl->x+bl->y*map[bl->m].xs]--;
}
#endif

/*==========================================
 * Adds a block to the map.
 * If flag is 1, then the block was just added
 * otherwise it is part of a transition.
 *------------------------------------------
 */
int map_addblock_sub (struct block_list *bl, int flag)
{
	int m, x, y, pos;

	nullpo_retr(0, bl);

	if (bl->prev != NULL) {
		if(battle_config.error_log)
			ShowError("map_addblock error : bl->prev != NULL\n");
		return 0;
	}

	m = bl->m;
	x = bl->x;
	y = bl->y;
	if (m < 0 || m >= map_num ||
		x < 0 || x >= map[m].xs ||
		y < 0 || y >= map[m].ys)
		return 1;
	
#ifdef CELL_NOSTACK
	map_addblcell(bl);
#endif
	
	pos = x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs;
	if (bl->type == BL_MOB) {
		bl->next = map[m].block_mob[pos];
		bl->prev = &bl_head;
		if (bl->next) bl->next->prev = bl;
		map[m].block_mob[pos] = bl;
		map[m].block_mob_count[pos]++;
	} else {
		bl->next = map[m].block[pos];
		bl->prev = &bl_head;
		if (bl->next) bl->next->prev = bl;
		map[m].block[pos] = bl;
		map[m].block_count[pos]++;
		if (bl->type == BL_PC && flag)
		{
			struct map_session_data* sd;
			if (map[m].users++ == 0 && battle_config.dynamic_mobs)	//Skotlex
				map_spawnmobs(m);
			sd = (struct map_session_data*)bl;
			if (battle_config.pet_no_gvg && map_flag_gvg(m) && sd->pd)
			{	//Return the pet to egg. [Skotlex]
				clif_displaymessage(sd->fd, "Pets are not allowed in Guild Wars.");
				pet_menu(sd, 3); //Option 3 is return to egg.
			}
		}
	}

	return 0;
}

/*==========================================
 * Removes a block from the map.
 * If flag is 1, then the block is removed for good
 * otherwise it is part of a transition.
 *------------------------------------------
 */
int map_delblock_sub (struct block_list *bl, int flag)
{
	int b;
	nullpo_retr(0, bl);

	// ?にblocklistから?けている
	if (bl->prev == NULL) {
		if (bl->next != NULL) {
			// prevがNULLでnextがNULLでないのは有ってはならない
			if(battle_config.error_log)
				ShowError("map_delblock error : bl->next!=NULL\n");
		}
		return 0;
	}

#ifdef CELL_NOSTACK
	map_delblcell(bl);
#endif
	
	b = bl->x/BLOCK_SIZE+(bl->y/BLOCK_SIZE)*map[bl->m].bxs;

	if (bl->type == BL_PC && flag)
		if (--map[bl->m].users == 0 && battle_config.dynamic_mobs)	//[Skotlex]
			map_removemobs(bl->m);

	if (bl->next)
		bl->next->prev = bl->prev;
	if (bl->prev == &bl_head) {
		// リストの頭なので、map[]のblock_listを更新する
		if (bl->type == BL_MOB) {
			map[bl->m].block_mob[b] = bl->next;
			if ((map[bl->m].block_mob_count[b]--) < 0)
				map[bl->m].block_mob_count[b] = 0;
		} else {
			map[bl->m].block[b] = bl->next;
			if((map[bl->m].block_count[b]--) < 0)
				map[bl->m].block_count[b] = 0;
		}
	} else {
		bl->prev->next = bl->next;
	}
	bl->next = NULL;
	bl->prev = NULL;

	return 0;
}

/*==========================================
 * Moves a block a x/y target position. [Skotlex]
 * Pass flag as 1 to prevent doing skill_unit_move checks
 * (which are executed by default on BL_CHAR types)
 *------------------------------------------
 */
int map_moveblock(struct block_list *bl, int x1, int y1, unsigned int tick) {
	int x0 = bl->x, y0 = bl->y;
	struct status_change *sc = NULL;
	int moveblock = ( x0/BLOCK_SIZE != x1/BLOCK_SIZE || y0/BLOCK_SIZE != y1/BLOCK_SIZE);

	if (!bl->prev) {
		//Block not in map, just update coordinates, but do naught else.
		bl->x = x1;
		bl->y = y1;
		return 0;	
	}
	//TODO: Perhaps some outs of bounds checking should be placed here?
	if (bl->type&BL_CHAR) {
		skill_unit_move(bl,tick,2);
		sc = status_get_sc(bl);
		if (sc && sc->count) {
			if (sc->data[SC_CLOSECONFINE].timer != -1)
				status_change_end(bl, SC_CLOSECONFINE, -1);
			if (sc->data[SC_CLOSECONFINE2].timer != -1)
				status_change_end(bl, SC_CLOSECONFINE2, -1);
//			if (sc->data[SC_BLADESTOP].timer != -1) //Won't stop when you are knocked away, go figure...
//				status_change_end(bl, SC_BLADESTOP, -1);
			if (sc->data[SC_BASILICA].timer != -1)
				status_change_end(bl, SC_BASILICA, -1);
		}
	}
	if (moveblock) map_delblock_sub(bl,0);
#ifdef CELL_NOSTACK
	else map_delblcell(bl);
#endif
	bl->x = x1;
	bl->y = y1;
	if (moveblock) map_addblock_sub(bl,0);
#ifdef CELL_NOSTACK
	else map_addblcell(bl);
#endif
	if (bl->type&BL_CHAR) {
		skill_unit_move(bl,tick,3);
		if (sc) {
			if (sc->count) {
				if (sc->data[SC_CLOAKING].timer != -1)
					skill_check_cloaking(bl, sc);
				if (sc->data[SC_DANCING].timer != -1)
					skill_unit_move_unit_group((struct skill_unit_group *)sc->data[SC_DANCING].val2, bl->m, x1-x0, y1-y0);
				if (sc->data[SC_WARM].timer != -1)
					skill_unit_move_unit_group((struct skill_unit_group *)sc->data[SC_WARM].val4, bl->m, x1-x0, y1-y0);
			}
		}
	}
	return 0;
}
	
/*==========================================
 * 周?のPC人?を?える (unused)
 *------------------------------------------
 */
int map_countnearpc (int m, int x, int y)
{
	int bx, by, c = 0;
	struct block_list *bl=NULL;

	if (map[m].users == 0)
		return 0;
	for (by = y/BLOCK_SIZE-AREA_SIZE/BLOCK_SIZE-1; by<=y/BLOCK_SIZE+AREA_SIZE/BLOCK_SIZE+1; by++) {
		if (by < 0 || by >= map[m].bys)
			continue;
		for (bx = x/BLOCK_SIZE-AREA_SIZE/BLOCK_SIZE-1; bx <= x/BLOCK_SIZE+AREA_SIZE/BLOCK_SIZE+1; bx++) {
			if (bx < 0 || bx >= map[m].bxs)
				continue;
			bl = map[m].block[bx+by*map[m].bxs];
			while(bl) {
				if (bl->type == BL_PC)
					c++;
				bl = bl->next;
			}
		}
	}

	return c;
}

/*==========================================
 * Counts specified number of objects on given cell.
 *------------------------------------------
 */
int map_count_oncell(int m, int x, int y, int type) {
	int bx,by;
	struct block_list *bl=NULL;
	int i,c;
	int count = 0;

	if (x < 0 || y < 0 || (x >= map[m].xs) || (y >= map[m].ys))
		return 0;
	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	if (type&~BL_MOB)
	{
		bl = map[m].block[bx+by*map[m].bxs];
		c = map[m].block_count[bx+by*map[m].bxs];
		for(i=0;i<c && bl;i++,bl=bl->next)
			if(bl->x == x && bl->y == y && bl->type&type) count++;
	}
	
	if (type&BL_MOB)
	{
		bl = map[m].block_mob[bx+by*map[m].bxs];
		c = map[m].block_mob_count[bx+by*map[m].bxs];
		for(i=0;i<c && bl;i++,bl=bl->next)
			if(bl->x == x && bl->y == y) count++;
	}
	return count;
}
/*
 * ｫｻｫ・ｾｪﾎｪﾋﾌｸｪﾄｪｱｪｿｫｹｫｭｫ・讚ﾋｫﾃｫﾈｪｹ
 */
struct skill_unit *map_find_skill_unit_oncell(struct block_list *target,int x,int y,int skill_id,struct skill_unit *out_unit)
{
	int m,bx,by;
	struct block_list *bl;
	int i,c;
	struct skill_unit *unit;
	m = target->m;

	if (x < 0 || y < 0 || (x >= map[m].xs) || (y >= map[m].ys))
		return NULL;
	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	bl = map[m].block[bx+by*map[m].bxs];
	c = map[m].block_count[bx+by*map[m].bxs];
	for(i=0;i<c && bl;i++,bl=bl->next){
		if (bl->x != x || bl->y != y || bl->type != BL_SKILL)
			continue;
		unit = (struct skill_unit *) bl;
		if (unit==out_unit || !unit->alive ||
				!unit->group || unit->group->skill_id!=skill_id)
			continue;
		if (battle_check_target(&unit->bl,target,unit->group->target_flag)>0)
			return unit;
	}
	return NULL;
}

/*==========================================
 * Adapted from foreachinarea for an easier invocation. [Skotlex]
 *------------------------------------------
 */

int map_foreachinrange(int (*func)(struct block_list*,va_list),struct block_list *center, int range,int type,...) {
	va_list ap;
	int bx,by,m;
	int returnCount =0;	//total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;
	int x0,x1,y0,y1;
	m = center->m;
	if (m < 0)
		return 0;
	va_start(ap,type);
	x0 = center->x-range;
	x1 = center->x+range;
	y0 = center->y-range;
	y1 = center->y+range;
	
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= map[m].xs) x1 = map[m].xs-1;
	if (y1 >= map[m].ys) y1 = map[m].ys-1;
	
	if (type&~BL_MOB)
		for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && bl->type&type
						&& bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1
#ifdef CIRCULAR_AREA
						&& check_distance_bl(center, bl, range)
#endif
					  	&& bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	if(type&BL_MOB)
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl
						&& bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1
#ifdef CIRCULAR_AREA
						&& check_distance_bl(center, bl, range)
#endif
						&& bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachinrange: block count too many!\n");
	}

	map_freeblock_lock();	// メモリからの解放を禁止する

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// 有?かどうかチェック
			returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();	// 解放を許可する

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]
}

/*==========================================
 * Same as foreachinrange, but there must be a shoot-able range between center and target to be counted in. [Skotlex]
 *------------------------------------------
 */
int map_foreachinshootrange(int (*func)(struct block_list*,va_list),struct block_list *center, int range,int type,...) {
	va_list ap;
	int bx,by,m;
	int returnCount =0;	//total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;
	int x0,x1,y0,y1;
	m = center->m;
	if (m < 0)
		return 0;
	va_start(ap,type);
	x0 = center->x-range;
	x1 = center->x+range;
	y0 = center->y-range;
	y1 = center->y+range;
	
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= map[m].xs) x1 = map[m].xs-1;
	if (y1 >= map[m].ys) y1 = map[m].ys-1;
	
	if (type&~BL_MOB)
		for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && bl->type&type
						&& bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1
#ifdef CIRCULAR_AREA
						&& check_distance_bl(center, bl, range)
#endif
						&& path_search_long(NULL,center->m,center->x,center->y,bl->x,bl->y)
					  	&& bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	if(type&BL_MOB)
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl
						&& bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1
#ifdef CIRCULAR_AREA
						&& check_distance_bl(center, bl, range)
#endif
						&& path_search_long(NULL,center->m,center->x,center->y,bl->x,bl->y)
						&& bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachinrange: block count too many!\n");
	}

	map_freeblock_lock();	// メモリからの解放を禁止する

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// 有?かどうかチェック
			returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();	// 解放を許可する

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]
}

/*==========================================
 * map m (x0,y0)-(x1,y1)?の全objに?して
 * funcを呼ぶ
 * type!=0 ならその種類のみ
 *------------------------------------------
 */
int map_foreachinarea(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int type,...) {
	va_list ap;
	int bx,by;
	int returnCount =0;	//total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;

	if (m < 0)
		return 0;
	va_start(ap,type);
	if (x1 < x0)
	{	//Swap range
		bx = x0;
		x0 = x1;
		x1 = bx;
	}
	if (y1 < y0)
	{
		bx = y0;
		y0 = y1;
		y1 = bx;
	}
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= map[m].xs) x1 = map[m].xs-1;
	if (y1 >= map[m].ys) y1 = map[m].ys-1;
	
	if (type&~BL_MOB)
		for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && bl->type&type && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	if(type&BL_MOB)
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachinarea: block count too many!\n");
	}

	map_freeblock_lock();	// メモリからの解放を禁止する

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// 有?かどうかチェック
			returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();	// 解放を許可する

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]
}

/*==========================================
 * 矩形(x0,y0)-(x1,y1)が(dx,dy)移動した暫ﾌ
 * 領域外になる領域(矩形かL字形)?のobjに
 * ?してfuncを呼ぶ
 *
 * dx,dyは-1,0,1のみとする（どんな値でもいいっぽい？）
 *------------------------------------------
 */
int map_foreachinmovearea(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int dx,int dy,int type,...) {
	int bx,by;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	va_list ap;
	int blockcount=bl_list_count,i,c;

	va_start(ap,type);
	if (x1 < x0)
	{	//Swap range
		bx = x0;
		x0 = x1;
		x1 = bx;
	}
	if (y1 < y0)
	{
		bx = y0;
		y0 = y1;
		y1 = bx;
	}
	if(dx==0 || dy==0){
		// 矩形領域の場合
		if(dx==0){
			if(dy<0){
				y0=y1+dy+1;
			} else {
				y1=y0+dy-1;
			}
		} else if(dy==0){
			if(dx<0){
				x0=x1+dx+1;
			} else {
				x1=x0+dx-1;
			}
		}
		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=map[m].xs) x1=map[m].xs-1;
		if(y1>=map[m].ys) y1=map[m].ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				if (type&~BL_MOB) {
					bl = map[m].block[bx+by*map[m].bxs];
					c = map[m].block_count[bx+by*map[m].bxs];
					for(i=0;i<c && bl;i++,bl=bl->next){
						if(bl && bl->type&type && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
							bl_list[bl_list_count++]=bl;
					}
				}
				if (type&BL_MOB) {
					bl = map[m].block_mob[bx+by*map[m].bxs];
					c = map[m].block_mob_count[bx+by*map[m].bxs];
					for(i=0;i<c && bl;i++,bl=bl->next){
						if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
							bl_list[bl_list_count++]=bl;
					}
				}
			}
		}
	}else{
		// L字領域の場合

		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=map[m].xs) x1=map[m].xs-1;
		if(y1>=map[m].ys) y1=map[m].ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				if (type & ~BL_MOB) {
					bl = map[m].block[bx+by*map[m].bxs];
					c = map[m].block_count[bx+by*map[m].bxs];
					for(i=0;i<c && bl;i++,bl=bl->next){
						if(!bl || !(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
							continue;
						if(bl && bl->type&type && ((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
							(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
							bl_list_count<BL_LIST_MAX)
								bl_list[bl_list_count++]=bl;
					}
				}
				if (type & BL_MOB) {
					bl = map[m].block_mob[bx+by*map[m].bxs];
					c = map[m].block_mob_count[bx+by*map[m].bxs];
					for(i=0;i<c && bl;i++,bl=bl->next){
						if(!bl || !(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
							continue;
						if(bl && ((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
							(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
							bl_list_count<BL_LIST_MAX)
								bl_list[bl_list_count++]=bl;
					}
				}
			}
		}

	}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachinmovearea: block count too many!\n");
	}

	map_freeblock_lock();	// メモリからの解放を禁止する

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev) {	// 有?かどうかチェック
			if (bl_list[i]->type == BL_PC
			  && session[((struct map_session_data *) bl_list[i])->fd] == NULL)
				continue;
			returnCount += func(bl_list[i],ap);
		}

	map_freeblock_unlock();	// 解放を許可する

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;
}

// -- moonsoul	(added map_foreachincell which is a rework of map_foreachinarea but
//			 which only checks the exact single x/y passed to it rather than an
//			 area radius - may be more useful in some instances)
//
int map_foreachincell(int (*func)(struct block_list*,va_list),int m,int x,int y,int type,...) {
	int bx,by;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	va_list ap;
	int blockcount=bl_list_count,i,c;

	va_start(ap,type);

	by=y/BLOCK_SIZE;
	bx=x/BLOCK_SIZE;

	if(type&~BL_MOB)
	{
		bl = map[m].block[bx+by*map[m].bxs];
		c = map[m].block_count[bx+by*map[m].bxs];
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if(bl && bl->type&type && bl->x==x && bl->y==y && bl_list_count<BL_LIST_MAX)
				bl_list[bl_list_count++]=bl;
		}
	}

	if(type&BL_MOB)
	{
		bl = map[m].block_mob[bx+by*map[m].bxs];
		c = map[m].block_mob_count[bx+by*map[m].bxs];
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if(bl && bl->x==x && bl->y==y && bl_list_count<BL_LIST_MAX)
				bl_list[bl_list_count++]=bl;
		}
	}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachincell: block count too many!\n");
	}

	map_freeblock_lock();	// メモリからの解放を禁止する

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// 有?かどうかチェック
			returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();	// 解放を許可する

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;
}

/*============================================================
* For checking a path between two points (x0, y0) and (x1, y1)
*------------------------------------------------------------
 */
int map_foreachinpath(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int range,int type,...)
{
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
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
// My trigonometrics and math is a little rusty... so the approach I am writing 
// here is basicly do a double for to check for all targets in the square that 
// contains the initial and final positions (area range increased to match the 
// radius given), then for each object to test, calculate the distance to the 
// path and include it if the range fits and the target is in the line (0<k<1,
// as they call it).
// The implementation I took as reference is found at 
// http://astronomy.swin.edu.au/~pbourke/geometry/pointline/ 
// (they have a link to a C implementation, too)
// This approach is a lot like #2 commented on this function, which I have no 
// idea why it was commented. I won't use doubles/floats, but pure int math for speed purposes
// The range considered is always the same no matter how close/far the target is because that's
// how SharpShooting works currently in kRO.

	//Generic map_foreach* variables.
	va_list ap;
	int i, blockcount = bl_list_count;
	struct block_list *bl;
	int c, bx, by;
	//method specific variables
	int magnitude2; //The square of the magnitude
	int k, xi, yi, xu, yu;
	int mx0 = x0, mx1 = x1, my0 = y0, my1 = y1;
	
	//Avoid needless calculations by not getting the sqrt right away.
	#define MAGNITUDE2(x0, y0, x1, y1) (((x1)-(x0))*((x1)-(x0)) + ((y1)-(y0))*((y1)-(y0)))
	
	if (m < 0)
		return 0;
		
	va_start(ap,type);

	//Expand target area to cover range.
	if (mx0 > mx1)
	{
		mx0+=range;
		mx1-=range;
	} else {
		mx0-=range;
		mx1+=range;
	}
	if (my0 > my1)
	{
		my0+=range;
		my1-=range;
	} else {
		my0-=range;
		my1+=range;
	}

	//The two fors assume mx0 < mx1 && my0 < my1
	if (mx0 > mx1)
	{
		k = mx1;
		mx1 = mx0;
		mx0 = k;
	}
	if (my0 > my1)
	{
		k = my1;
		my1 = my0;
		my0 = k;
	}
	
	if (mx0 < 0) mx0 = 0;
	if (my0 < 0) my0 = 0;
	if (mx1 >= map[m].xs) mx1 = map[m].xs-1;
	if (my1 >= map[m].ys) my1 = map[m].ys-1;
	
	range*=range<<8; //Values are shifted later on for higher precision using int math.
	magnitude2 = MAGNITUDE2(x0,y0, x1,y1);
	if (magnitude2 < 1) //Same begin and ending point, can't trace path.
		return 0;
	
	if (type & ~BL_MOB)
		for (by = my0 / BLOCK_SIZE; by <= my1 / BLOCK_SIZE; by++) {
			for(bx=mx0/BLOCK_SIZE;bx<=mx1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && bl->prev && bl->type&type && bl_list_count<BL_LIST_MAX)
					{
						xi = bl->x;
						yi = bl->y;
					
						k = (xi-x0)*(x1-x0) + (yi-y0)*(y1-y0);
						if (k < 0 || k > magnitude2) //Since more skills use this, check for ending point as well.
							continue;
					
						//All these shifts are to increase the precision of the intersection point and distance considering how it's
						//int math.
						k = (k<<4)/magnitude2; //k will be between 1~16 instead of 0~1
						xi<<=4;
						yi<<=4;
						xu= (x0<<4) +k*(x1-x0);
						yu= (y0<<4) +k*(y1-y0);
						k = MAGNITUDE2(xi, yi, xu, yu);
						
						//If all dot coordinates were <<4 the square of the magnitude is <<8
						if (k > range)
							continue;

						bl_list[bl_list_count++]=bl;
					}
				}
			}
		}
	if(type&BL_MOB)
		for(by=my0/BLOCK_SIZE;by<=my1/BLOCK_SIZE;by++){
			for(bx=mx0/BLOCK_SIZE;bx<=mx1/BLOCK_SIZE;bx++){
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && bl->prev && bl_list_count<BL_LIST_MAX)
					{
						xi = bl->x;
						yi = bl->y;
						k = (xi-x0)*(x1-x0) + (yi-y0)*(y1-y0);
						if (k < 0 || k > magnitude2)
							continue;
					
						k = (k<<4)/magnitude2; //k will be between 1~16 instead of 0~1
						xi<<=4;
						yi<<=4;
						xu= (x0<<4) +k*(x1-x0);
						yu= (y0<<4) +k*(y1-y0);
						k = MAGNITUDE2(xi, yi, xu, yu);
						
						//If all dot coordinates were <<4 the square of the magnitude is <<8
						if (k > range)
							continue;

						bl_list[bl_list_count++]=bl;
					}
				}
			}
		}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachinpath: block count too many!\n");
	}

	map_freeblock_lock();

	for(i=blockcount;i<bl_list_count;i++)
		returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]

}

// Copy of map_foreachincell, but applied to the whole map. [Skotlex]
int map_foreachinmap(int (*func)(struct block_list*,va_list),int m,int type,...) {
	int b, bsize;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	va_list ap;
	int blockcount=bl_list_count,i,c;

	va_start(ap,type);

	bsize = map[m].bxs * map[m].bys;
	if(type&~BL_MOB)
	{
		for(b=0;b<bsize;b++){
			bl = map[m].block[b];
			c = map[m].block_count[b];
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && bl->type&type && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
		}
	}

	if(type&BL_MOB)
	{
		for(b=0;b<bsize;b++){
			bl = map[m].block_mob[b];
			c = map[m].block_mob_count[b];
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
		}
	}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachinmap: block count too many!\n");
	}

	map_freeblock_lock();	// メモリからの解放を禁止する

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// 有?かどうかチェック
			returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();	// 解放を許可する

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;
}

/*==========================================
 * 床アイテムやエフェクト用の一三bj割り?て
 * object[]への保存とid_db登?まで
 *
 * bl->idもこの中で設定して問題無い?
 *------------------------------------------
 */
int map_addobject(struct block_list *bl) {
	int i;
	if( bl == NULL ){
		ShowWarning("map_addobject nullpo?\n");
		return 0;
	}
	if(first_free_object_id<2 || first_free_object_id>=MAX_FLOORITEM)
		first_free_object_id=2;
	for(i=first_free_object_id;i<MAX_FLOORITEM && objects[i];i++);
	if(i>=MAX_FLOORITEM){
		if(battle_config.error_log)
			ShowWarning("no free object id\n");
		return 0;
	}
	first_free_object_id=i;
	if(last_object_id<i)
		last_object_id=i;
	objects[i]=bl;
	idb_put(id_db,i,bl);
	return i;
}

/*==========================================
 * 一三bjectの解放
 *	map_delobjectのfreeしないバ?ジョン
 *------------------------------------------
 */
int map_delobjectnofree(int id) {
	if(objects[id]==NULL)
		return 0;

	map_delblock(objects[id]);
	idb_remove(id_db,id);
	objects[id]=NULL;

	if(first_free_object_id>id)
		first_free_object_id=id;

	while(last_object_id>2 && objects[last_object_id]==NULL)
		last_object_id--;

	return 0;
}

/*==========================================
 * 一三bjectの解放
 * block_listからの削除、id_dbからの削除
 * object dataのfree、object[]へのNULL代入
 *
 * addとの??性が無いのが?になる
 *------------------------------------------
 */
int map_delobject(int id) {
	struct block_list *obj = objects[id];

	if(obj==NULL)
		return 0;

	map_delobjectnofree(id);
	map_freeblock(obj);

	return 0;
}

/*==========================================
 * 全一三bj相手にfuncを呼ぶ
 *
 *------------------------------------------
 */
void map_foreachobject(int (*func)(struct block_list*,va_list),int type,...) {
	int i;
	int blockcount=bl_list_count;
	va_list ap;

	va_start(ap,type);

	for(i=2;i<=last_object_id;i++){
		if(objects[i]){
			if(!(objects[i]->type==type)) // Fixed [Lance]
				continue;
			if(bl_list_count>=BL_LIST_MAX) {
				if(battle_config.error_log)
					ShowWarning("map_foreachobject: too many blocks !\n");
				break;
			}
			bl_list[bl_list_count++]=objects[i];
		}
	}

	map_freeblock_lock();

	for(i=blockcount;i<bl_list_count;i++)
		if( bl_list[i]->prev || bl_list[i]->next )
			func(bl_list[i],ap);

	map_freeblock_unlock();

	va_end(ap);
	bl_list_count = blockcount;
}

/*==========================================
 * 床アイテムを消す
 *
 * data==0の暫ﾍtimerで消えた殊 * data!=0の暫ﾍ拾う等で消えた暫ﾆして動作
 *
 * 後者は、map_clearflooritem(id)へ
 * map.h?で#defineしてある
 *------------------------------------------
 */
int map_clearflooritem_timer(int tid,unsigned int tick,int id,int data) {
	struct flooritem_data *fitem=NULL;

	fitem = (struct flooritem_data *)objects[id];
	if(fitem==NULL || fitem->bl.type!=BL_ITEM || (!data && fitem->cleartimer != tid)){
		if(battle_config.error_log)
			ShowError("map_clearflooritem_timer : error\n");
		return 1;
	}
	if(data)
		delete_timer(fitem->cleartimer,map_clearflooritem_timer);
	else if(fitem->item_data.card[0] == CARD0_PET)
		intif_delete_petdata( MakeDWord(fitem->item_data.card[1],fitem->item_data.card[2]) );
	clif_clearflooritem(fitem,0);
	map_delobject(fitem->bl.id);

	return 0;
}

/*==========================================
 * (m,x,y) locates a random available free cell around the given coordinates
 * to place an BL_ITEM object. Scan area is 9x9, returns 1 on success.
 * x and y are modified with the target cell when successful.
 *------------------------------------------
 */
int map_searchrandfreecell(int m,int *x,int *y,int stack) {
	int free_cell,i,j;
	int free_cells[9][2];

	for(free_cell=0,i=-1;i<=1;i++){
		if(i+*y<0 || i+*y>=map[m].ys)
			continue;
		for(j=-1;j<=1;j++){
			if(j+*x<0 || j+*x>=map[m].xs)
				continue;
			if(map_getcell(m,j+*x,i+*y,CELL_CHKNOPASS))
				continue;
			//Avoid item stacking to prevent against exploits. [Skotlex]
			if(stack && map_count_oncell(m,j+*x,i+*y, BL_ITEM) > stack)
				continue;
			free_cells[free_cell][0] = j+*x;
			free_cells[free_cell++][1] = i+*y;
		}
	}
	if(free_cell==0)
		return 0;
	free_cell = rand()%free_cell;
	*x = free_cells[free_cell][0];
	*y = free_cells[free_cell][1];
	return 1;
}


static int map_count_sub(struct block_list *bl,va_list ap)
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
 *------------------------------------------
 */
int map_search_freecell(struct block_list *src, int m, short *x,short *y, int rx, int ry, int flag) {
	int tries, spawn=0;
	int bx, by;
	int rx2 = 2*rx+1;
	int ry2 = 2*ry+1;

	if (!src && (!(flag&1) || flag&2))
	{
		ShowDebug("map_search_freecell: Incorrect usage! When src is NULL, flag has to be &1 and can't have &2\n");
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
	
	if (rx >= 0 && ry >= 0) {
		tries = rx2*ry2;
		if (tries > 100) tries = 100;
	} else {
		tries = map[m].xs*map[m].ys;
		if (tries > 500) tries = 500;
	}
	
	while(tries--) {
		*x = (rx >= 0)?(rand()%rx2-rx+bx):(rand()%(map[m].xs-2)+1);
		*y = (ry >= 0)?(rand()%ry2-ry+by):(rand()%(map[m].ys-2)+1);
		
		if (*x == bx && *y == by)
			continue; //Avoid picking the same target tile.
		
		if (map_getcell(m,*x,*y,CELL_CHKREACH))
		{
			if(flag&2 && !unit_can_reach_pos(src, *x, *y, 1))
				continue;
			if(flag&4) {
				if (spawn >= 100) return 0; //Limit of retries reached.
				if (spawn++ < battle_config.no_spawn_on_player &&
					map_foreachinarea(map_count_sub, m,
						*x-AREA_SIZE, *y-AREA_SIZE,
					  	*x+AREA_SIZE, *y+AREA_SIZE, BL_PC)
				)
				continue;
			}
			return 1;
		}
	}
	*x = bx;
	*y = by;
	return 0;
}

/*==========================================
 * (m,x,y)を中心に3x3以?に床アイテム設置
 *
 * item_dataはamount以外をcopyする
 * type flag: &1 MVP item. &2 do stacking check.
 *------------------------------------------
 */
int map_addflooritem(struct item *item_data,int amount,int m,int x,int y,struct map_session_data *first_sd,
    struct map_session_data *second_sd,struct map_session_data *third_sd,int type) {
	int r;
	unsigned int tick;
	struct flooritem_data *fitem=NULL;

	nullpo_retr(0, item_data);

	if(!map_searchrandfreecell(m,&x,&y,type&2?1:0))
		return 0;
	r=rand();

	fitem = (struct flooritem_data *)aCalloc(1,sizeof(*fitem));
	fitem->bl.type=BL_ITEM;
	fitem->bl.prev = fitem->bl.next = NULL;
	fitem->bl.m=m;
	fitem->bl.x=x;
	fitem->bl.y=y;
	fitem->bl.id = map_addobject(&fitem->bl);
	if(fitem->bl.id==0){
		aFree(fitem);
		return 0;
	}

	tick = gettick();
	if(first_sd) {
		fitem->first_get_id = first_sd->bl.id;
		if(type&1)
			fitem->first_get_tick = tick + battle_config.mvp_item_first_get_time;
		else
			fitem->first_get_tick = tick + battle_config.item_first_get_time;
	}
	if(second_sd) {
		fitem->second_get_id = second_sd->bl.id;
		if(type&1)
			fitem->second_get_tick = tick + battle_config.mvp_item_first_get_time + battle_config.mvp_item_second_get_time;
		else
			fitem->second_get_tick = tick + battle_config.item_first_get_time + battle_config.item_second_get_time;
	}
	if(third_sd) {
		fitem->third_get_id = third_sd->bl.id;
		if(type&1)
			fitem->third_get_tick = tick + battle_config.mvp_item_first_get_time + battle_config.mvp_item_second_get_time + battle_config.mvp_item_third_get_time;
		else
			fitem->third_get_tick = tick + battle_config.item_first_get_time + battle_config.item_second_get_time + battle_config.item_third_get_time;
	}

	memcpy(&fitem->item_data,item_data,sizeof(*item_data));
	fitem->item_data.amount=amount;
	fitem->subx=(r&3)*3+3;
	fitem->suby=((r>>2)&3)*3+3;
	fitem->cleartimer=add_timer(gettick()+battle_config.flooritem_lifetime,map_clearflooritem_timer,fitem->bl.id,0);

	map_addblock(&fitem->bl);
	clif_dropflooritem(fitem);

	return fitem->bl.id;
}

static void* create_charid2nick(DBKey key, va_list args) {
	struct charid2nick *p;
	p = (struct charid2nick *)aCallocA(1, sizeof (struct charid2nick));
	return p;
}
/*==========================================
 * charid_dbへ追加(返信待ちがあれば返信)
 *------------------------------------------
 */
void map_addchariddb(int charid, char *name) {
	struct charid2nick *p;
	int req = 0;

	p = idb_ensure(charid_db,charid,create_charid2nick);
	req = p->req_id;
	p->req_id = 0;
	//We overwrite the nick anyway in case a different one arrived.
	memcpy(p->nick, name, NAME_LENGTH);

	if (req) {
		struct map_session_data *sd = map_id2sd(req);
		if (sd) clif_solved_charname(sd,charid);
	}
}

/*==========================================
 * charid_dbへ追加（返信要求のみ）
 *------------------------------------------
 */
int map_reqchariddb(struct map_session_data * sd,int charid) {
	struct charid2nick *p=NULL;

	nullpo_retr(0, sd);

	p = (struct charid2nick*)idb_get(charid_db,charid);
	if(p) return 0; //Nothing to request, we already have the name!
	p = (struct charid2nick *)aCalloc(1,sizeof(struct charid2nick));
	p->req_id=sd->bl.id;
	idb_put(charid_db,charid,p);
	return 0;
}

/*==========================================
 * id_dbへblを追加
 *------------------------------------------
 */
void map_addiddb(struct block_list *bl) {
	nullpo_retv(bl);

	if (bl->type == BL_PC)
		idb_put(pc_db,bl->id,bl);
	idb_put(id_db,bl->id,bl);
}

/*==========================================
 * id_dbからblを削除
 *------------------------------------------
 */
void map_deliddb(struct block_list *bl) {
	nullpo_retv(bl);

	if (bl->type == BL_PC)
		idb_remove(pc_db,bl->id);
	idb_remove(id_db,bl->id);
}

/*==========================================
 * PCのquit?理 map.c?分
 *
 * quit?理の主?が違うような?もしてきた
 *------------------------------------------
 */
int map_quit(struct map_session_data *sd) {

	//nullpo_retr(0, sd); //Utterly innecessary, all invokations to this function already have an SD non-null check.
	//Learn to use proper coding and stop relying on nullpo_'s for safety :P [Skotlex]
	if(!sd->state.waitingdisconnect) {
		if (sd->npc_timer_id != -1) //Cancel the event timer.
			npc_timerevent_quit(sd);
		if (sd->state.event_disconnect)
			npc_script_event(sd, NPCE_LOGOUT);

		sd->state.waitingdisconnect = 1;
		if (sd->pd) unit_free(&sd->pd->bl,0);
		if (sd->hd) unit_free(&sd->hd->bl,0);
		unit_free(&sd->bl,3);
		chrif_save(sd,1);
	} else { //Try to free some data, without saving anything (this could be invoked on map server change. [Skotlex]
		if (sd->bl.prev != NULL)
		{	//Remove from map...
			unit_remove_map(&sd->bl, 0);
			if (sd->pd && sd->pd->bl.prev != NULL)
				unit_remove_map(&sd->pd->bl, 0);
		}
	}

	//Do we really need to remove the name?
	idb_remove(charid_db,sd->status.char_id);
	idb_remove(id_db,sd->bl.id);

	if(sd->reg)
	{	//Double logout already freed pointer fix... [Skotlex]
		aFree(sd->reg);
		sd->reg = NULL;
		sd->reg_num = 0;
	}
	if(sd->regstr)
	{
		aFree(sd->regstr);
		sd->regstr = NULL;
		sd->regstr_num = 0;
	}
	if (sd->st) {
		if (sd->st->stack)
			script_free_stack (sd->st->stack);
		aFree(sd->st);
		sd->st = NULL;
		sd->npc_id = 0;
	}
#ifndef TXT_ONLY
	if(charsave_method)
	{	//Let player be free'd on closing the connection.
		idb_remove(pc_db,sd->status.account_id);
		if (!(sd->fd && session[sd->fd]->session_data))
			aFree(sd); //In case player was not attached to session.
		return 0;	
	}
#endif
	if(sd->fd)
  	{	//Player will be free'd on save-ack. [Skotlex]
		if (session[sd->fd])
			session[sd->fd]->session_data = NULL;
		sd->fd = 0;
	}
	return 0;
}

void map_quit_ack(struct map_session_data *sd) {
	if (sd && sd->state.finalsave) {
		idb_remove(pc_db,sd->status.account_id);
		aFree(sd);
	}
}

static int do_reconnect_map_sub(DBKey key,void *data,va_list va) {
	struct map_session_data *sd = (TBL_PC*)data;
	if (sd->state.finalsave) {
		sd->state.finalsave = 0;
		chrif_save(sd, 1); //Resend to save!
		return 1;
	}
	return 0;
}

void do_reconnect_map(void) {
	pc_db->foreach(pc_db,do_reconnect_map_sub);
}

/*==========================================
 * id番?のPCを探す。居なければNULL
 *------------------------------------------
 */
struct map_session_data * map_id2sd(int id) {
// Now using pc_db to handle all players, should be quicker than both previous methods at a small expense of more memory. [Skotlex]
	if (id <= 0) return NULL;
	return (struct map_session_data*)idb_get(pc_db,id);
}

/*==========================================
 * char_id番?の名前を探す
 *------------------------------------------
 */
char * map_charid2nick(int id) {
	struct charid2nick *p = (struct charid2nick*)idb_get(charid_db,id);

	if(p==NULL)
		return NULL;
	return p->nick;
}

struct map_session_data * map_charid2sd(int id) {
	int i, users;
	struct map_session_data **all_sd;

	if (id <= 0) return 0;

	all_sd = map_getallusers(&users);
	for(i = 0; i < users; i++)
		if (all_sd[i] && all_sd[i]->status.char_id == id)
			return all_sd[i];

	return NULL;
}

/*==========================================
 * Search session data from a nick name
 * (without sensitive case if necessary)
 * return map_session_data pointer or NULL
 *------------------------------------------
 */
struct map_session_data * map_nick2sd(char *nick) {
	int i, users;
	struct map_session_data *pl_sd = NULL, **pl_allsd;

	if (nick == NULL)
		return NULL;

	pl_allsd = map_getallusers(&users);
	if (battle_config.partial_name_scan)
	{
		int qty = 0, nicklen = strlen(nick);
		struct map_session_data *sd = NULL;
		for (i = 0; i < users; i++) {
			pl_sd = pl_allsd[i];
			// Without case sensitive check (increase the number of similar character names found)
			if (strnicmp(pl_sd->status.name, nick, nicklen) == 0) {
				// Strict comparison (if found, we finish the function immediatly with correct value)
				if (strcmp(pl_sd->status.name, nick) == 0)
					return pl_sd;
				qty++;
				sd = pl_sd;
			}
		}
		// We return the found index of a similar account ONLY if there is 1 similar character
		if (qty == 1)
			return sd;
	} else { //Exact Search
		for (i = 0; i < users; i++) {
			if (strcasecmp(pl_allsd[i]->status.name, nick) == 0)
				return pl_allsd[i];
		}
	}
	//Not found.
	return NULL;
}

/*==========================================
 * id番?の物を探す
 * 一三bjectの場合は配列を引くのみ
 *------------------------------------------
 */
struct block_list * map_id2bl(int id)
{
	struct block_list *bl=NULL;
	if(id >= 0 && id < sizeof(objects)/sizeof(objects[0]))
		bl = objects[id];
	else
		bl = idb_get(id_db,id);

	return bl;
}

static int map_getallpc_sub(DBKey key,void * data,va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*) data;
	if (!sd->state.auth || sd->state.waitingdisconnect || sd->state.finalsave)
		return 1; //Do not count in not-yet authenticated characters or ready to disconnect ones.

	return 0;
}

/*==========================================
 * Returns an array of all players in the server (includes non connected ones) [Skotlex]
 * The int pointer given returns the count of elements in the array.
 * If null is passed, it is requested that the memory be freed (for shutdown), and null is returned.
 *------------------------------------------
 */
struct map_session_data** map_getallusers(int *users) {
	static struct map_session_data **all_sd=NULL;
	static unsigned int all_count = 0;
	
	if (users == NULL)
	{	//Free up data
		if (all_sd) aFree(all_sd);
		all_sd = NULL;
		return NULL;
	}

	if (all_sd == NULL)
	{	//Init data
		all_count = pc_db->size(pc_db); //This is the real number of chars in the db, better use this than the actual "online" count.
		if (all_count < 1)
			all_count = 10; //Allow room for at least 10 chars.
		all_sd = aCalloc(all_count, sizeof(struct map_session_data*)); //it's actually just the size of a pointer.
	}

	if (all_count < pc_db->size(pc_db))
	{
		all_count = pc_db->size(pc_db)+10; //Give some room to prevent doing reallocs often.
		all_sd = aRealloc(all_sd, all_count*sizeof(struct map_session_data*));
	}
	*users = pc_db->getall(pc_db,(void**)all_sd,all_count,map_getallpc_sub);
	if (*users > (signed int)all_count) //Which should be impossible...
		*users = all_count;
	return all_sd;
}

void map_foreachpc(int (*func)(DBKey,void*,va_list),...) {
	va_list ap;
	va_start(ap,func);
	pc_db->foreach(pc_db,func,ap);
	va_end(ap);
}

/*==========================================
 * id_db?の全てにfuncを?行
 *------------------------------------------
 */
int map_foreachiddb(int (*func)(DBKey,void*,va_list),...) {
	va_list ap;

	va_start(ap,func);
	id_db->foreach(id_db,func,ap);
	va_end(ap);
	return 0;
}

/*==========================================
 * map.npcへ追加 (warp等の領域持ちのみ)
 *------------------------------------------
 */
int map_addnpc(int m,struct npc_data *nd) {
	int i;
	if(m<0 || m>=map_num)
		return -1;
	for(i=0;i<map[m].npc_num && i<MAX_NPC_PER_MAP;i++)
		if(map[m].npc[i]==NULL)
			break;
	if(i==MAX_NPC_PER_MAP){
		if(battle_config.error_log)
			ShowWarning("too many NPCs in one map %s\n",map[m].name);
		return -1;
	}
	if(i==map[m].npc_num){
		map[m].npc_num++;
	}

	nullpo_retr(0, nd);

	map[m].npc[i]=nd;
	nd->n = i;
	idb_put(id_db,nd->bl.id,nd);

	return i;
}

void map_removenpc(void) {
	int i,m,n=0;

	for(m=0;m<map_num;m++) {
		for(i=0;i<map[m].npc_num && i<MAX_NPC_PER_MAP;i++) {
			if(map[m].npc[i]!=NULL) {
				clif_clearchar_area(&map[m].npc[i]->bl,2);
				map_delblock(&map[m].npc[i]->bl);
				idb_remove(id_db,map[m].npc[i]->bl.id);
				if(map[m].npc[i]->bl.subtype==SCRIPT) {
					aFree(map[m].npc[i]->u.scr.script);
					aFree(map[m].npc[i]->u.scr.label_list);
				}
				aFree(map[m].npc[i]);
				map[m].npc[i] = NULL;
				n++;
			}
		}
	}
	
	ShowStatus("Successfully removed and freed from memory '"CL_WHITE"%d"CL_RESET"' NPCs.\n",n);
}

/*=========================================
 * Dynamic Mobs [Wizputer]
 *-----------------------------------------
 */

// allocates a struct when it there is place free in the cache,
// and returns NULL otherwise
// -- i'll just leave the old code in case it's needed ^^;
int map_addmobtolist(unsigned short m, struct spawn_data *spawn)
{
	size_t i;
	for (i = 0; i < MAX_MOB_LIST_PER_MAP; i++) {
		if (map[m].moblist[i] == NULL) {
			map[m].moblist[i] = spawn;
			return i;
		}
	}
	return -1;
}

void map_spawnmobs(int m)
{
	int i, k=0;
	if (map[m].mob_delete_timer != -1)
	{	//Mobs have not been removed yet [Skotlex]
		delete_timer(map[m].mob_delete_timer, map_removemobs_timer);
		map[m].mob_delete_timer = -1;
		return;
	}
	for(i=0; i<MAX_MOB_LIST_PER_MAP; i++)
		if(map[m].moblist[i]!=NULL)
		{
			k+=map[m].moblist[i]->num;
			npc_parse_mob2(map[m].moblist[i],i);
		}

	if (battle_config.etc_log && k > 0)
	{
		ShowStatus("Map %s: Spawned '"CL_WHITE"%d"CL_RESET"' mobs.\n",map[m].name, k);
	}
}

int mob_cache_cleanup_sub(struct block_list *bl, va_list ap) {
	struct mob_data *md = (struct mob_data *)bl;
	nullpo_retr(0, md);

	//When not to remove:
	//Mob has the cached flag on 0
	if (!md->special_state.cached)
		return 0;
	if (!battle_config.mob_remove_damaged && 
		md->status.hp < md->status.max_hp)
		return 0; //Do not remove damaged mobs.
	
	unit_free(&md->bl,0);

	return 1;
}

int map_removemobs_timer(int tid, unsigned int tick, int id, int data)
{
	int k;
	if (id < 0 || id >= MAX_MAP_PER_SERVER)
	{	//Incorrect map id!
		if (battle_config.error_log)
			ShowError("map_removemobs_timer error: timer %d points to invalid map %d\n",tid, id);
		return 0;
	}
	if (map[id].mob_delete_timer != tid)
	{	//Incorrect timer call!
		if (battle_config.error_log)
			ShowError("map_removemobs_timer mismatch: %d != %d (map %s)\n",map[id].mob_delete_timer, tid, map[id].name);
		return 0;
	}
	map[id].mob_delete_timer = -1;
	if (map[id].users > 0) //Map not empty!
		return 1;
	k = map_foreachinmap(mob_cache_cleanup_sub, id, BL_MOB);

	if (battle_config.etc_log && k > 0)
		ShowStatus("Map %s: Removed '"CL_WHITE"%d"CL_RESET"' mobs.\n",map[id].name, k);
	
	return 1;
}

void map_removemobs(int m)
{
	if (map[m].mob_delete_timer != -1)
		return; //Mobs are already scheduled for removal

	map[m].mob_delete_timer = add_timer(gettick()+battle_config.mob_remove_delay, map_removemobs_timer, m, 0);
}

/*==========================================
 * map名からmap番?へ?換
 *------------------------------------------
 */
int map_mapname2mapid(char *name) {
	unsigned short map_index;
	map_index = mapindex_name2id(name);
	if (!map_index)
		return -1;
	return map_mapindex2mapid(map_index);
}

/*==========================================
 * Returns the map of the given mapindex. [Skotlex]
 *------------------------------------------
 */
int map_mapindex2mapid(unsigned short mapindex) {
	struct map_data *md=NULL;
	
	if (!mapindex)
		return -1;
	
	md = (struct map_data*)uidb_get(map_db,(unsigned int)mapindex);
	if(md==NULL || md->gat==NULL)
		return -1;
	return md->m;
}

/*==========================================
 * 他鯖map名からip,port?換
 *------------------------------------------
 */
int map_mapname2ipport(unsigned short name,int *ip,int *port) {
	struct map_data_other_server *mdos=NULL;

	mdos = (struct map_data_other_server*)uidb_get(map_db,(unsigned int)name);
	if(mdos==NULL || mdos->gat) //If gat isn't null, this is a local map.
		return -1;
	*ip=mdos->ip;
	*port=mdos->port;
	return 0;
}

/*==========================================
 * Checks if both dirs point in the same direction.
 *------------------------------------------
 */
int map_check_dir(int s_dir,int t_dir) {
	if(s_dir == t_dir)
		return 0;
	switch(s_dir) {
		case 0:
			if(t_dir == 7 || t_dir == 1 || t_dir == 0)
				return 0;
			break;
		case 1:
			if(t_dir == 0 || t_dir == 2 || t_dir == 1)
				return 0;
			break;
		case 2:
			if(t_dir == 1 || t_dir == 3 || t_dir == 2)
				return 0;
			break;
		case 3:
			if(t_dir == 2 || t_dir == 4 || t_dir == 3)
				return 0;
			break;
		case 4:
			if(t_dir == 3 || t_dir == 5 || t_dir == 4)
				return 0;
			break;
		case 5:
			if(t_dir == 4 || t_dir == 6 || t_dir == 5)
				return 0;
			break;
		case 6:
			if(t_dir == 5 || t_dir == 7 || t_dir == 6)
				return 0;
			break;
		case 7:
			if(t_dir == 6 || t_dir == 0 || t_dir == 7)
				return 0;
			break;
	}
	return 1;
}

/*==========================================
 * Returns the direction of the given cell in absolute relation to the char
 * (regardless of where the char is facing)
 *------------------------------------------
 */
int map_calc_dir( struct block_list *src,int x,int y) {
	int dir=0;
	int dx,dy;

	nullpo_retr(0, src);

	dx=x-src->x;
	dy=y-src->y;
	if( dx==0 && dy==0 ){	// 彼我の場所一致
		dir=0;	// 上
	}else if( dx>=0 && dy>=0 ){	// 方向的に右上
		dir=7;						// 右上
		if( dx*2-1<dy ) dir=0;		// 上
		if( dx>dy*2 ) dir=6;		// 右
	}else if( dx>=0 && dy<=0 ){	// 方向的に右下
		dir=5;						// 右下
		if( dx*2-1<-dy ) dir=4;		// 下
		if( dx>-dy*2 ) dir=6;		// 右
	}else if( dx<=0 && dy<=0 ){ // 方向的に左下
		dir=3;						// 左下
		if( dx*2+1>dy ) dir=4;		// 下
		if( dx<dy*2 ) dir=2;		// 左
	}else{						// 方向的に左上
		dir=1;						// 左上
		if( -dx*2-1<dy ) dir=0;		// 上
		if( -dx>dy*2 ) dir=2;		// 左
	}
	return dir;
}

/*==========================================
 * Randomizes target cell x,y to a random walkable cell that 
 * has the same distance from object as given coordinates do. [Skotlex]
 *------------------------------------------
 */
int map_random_dir(struct block_list *bl, short *x, short *y) {
	struct walkpath_data wpd;
	short xi = *x-bl->x;
	short yi = *y-bl->y;
	short i=0, j;
	int dist2 = xi*xi + yi*yi;
	short dist = (short)sqrt(dist2);
	short segment;
	
	if (dist < 1) dist =1;
	
	do {
		j = rand()%8; //Pick a random direction
		segment = 1+(rand()%dist); //Pick a random interval from the whole vector in that direction
		xi = bl->x + segment*dirx[j];
		segment = (short)sqrt(dist2 - segment*segment); //The complement of the previously picked segment
		yi = bl->y + segment*diry[j];
	} while ((
		map_getcell(bl->m,xi,yi,CELL_CHKNOPASS)  ||
		path_search_real(&wpd,bl->m,bl->x,bl->y,xi,yi,1,CELL_CHKNOREACH) == -1)
		&& (++i)<100);
	
	if (i < 100) {
		*x = xi;
		*y = yi;
		return 1;
	}
	return 0;
}
// gat系
/*==========================================
 * (m,x,y)の状態を調べる
 *------------------------------------------
 */

int map_getcell(int m,int x,int y,cell_t cellchk)
{
	return (m < 0 || m >= MAX_MAP_PER_SERVER) ? 0 : map_getcellp(&map[m],x,y,cellchk);
}

int map_getcellp(struct map_data* m,int x,int y,cell_t cellchk)
{
	int type, type2;
#ifdef CELL_NOSTACK
	int type3;
#endif

	nullpo_ret(m);

	if(x<0 || x>=m->xs-1 || y<0 || y>=m->ys-1)
	{
		if(cellchk==CELL_CHKNOPASS) return 1;
		return 0;
	}
	type = m->gat[x+y*m->xs];
	type2 = m->cell[x+y*m->xs];
#ifdef CELL_NOSTACK
	type3 = m->cell_bl[x+y*m->xs];
#endif

	switch(cellchk)
	{
		case CELL_CHKPASS:
#ifdef CELL_NOSTACK
			if (type3 >= battle_config.cell_stack_limit) return 0;
#endif
		case CELL_CHKREACH:
			return (type!=1 && type!=5 && !(type2&CELL_ICEWALL));
		case CELL_CHKNOPASS:
#ifdef CELL_NOSTACK
			if (type3 >= battle_config.cell_stack_limit) return 1;
#endif
		case CELL_CHKNOREACH:
			return (type==1 || type==5 || type2&CELL_ICEWALL);
		case CELL_CHKSTACK:
#ifdef CELL_NOSTACK
			return (type3 >= battle_config.cell_stack_limit);
#else
			return 0;
#endif
		case CELL_CHKWALL:
			return (type==1/* || type2&CELL_ICEWALL*/); //Uncomment to prevent sniping/casting through the icewall. [Skotlex]
		case CELL_CHKWATER:
			return (type==3);
		case CELL_CHKGROUND:
			return (type==5 || type2&CELL_ICEWALL);
		case CELL_GETTYPE:
			return type;
		case CELL_GETCELLTYPE:
			return type2;
		case CELL_CHKNPC:
			return (type2&CELL_NPC);
		case CELL_CHKPNEUMA:
			return (type2&CELL_PNEUMA);
		case CELL_CHKSAFETYWALL:
			return (type2&CELL_SAFETYWALL);
		case CELL_CHKBASILICA:
			return (type2&CELL_BASILICA);
		case CELL_CHKLANDPROTECTOR:
			return (type2&CELL_LANDPROTECTOR);
		case CELL_CHKREGEN:
			return (type2&CELL_REGEN);
		case CELL_CHKICEWALL:
			return (type2&CELL_ICEWALL);
		default:
			return 0;
	}
}

/*==========================================
 * (m,x,y)の状態を設定する
 *------------------------------------------
 */
void map_setcell(int m,int x,int y,int cell)
{
	int j;
	if(x<0 || x>=map[m].xs || y<0 || y>=map[m].ys)
		return;
	j=x+y*map[m].xs;

	switch (cell) {
		case CELL_SETNPC:
			map[m].cell[j] |= CELL_NPC;
			break;
		case CELL_CLRNPC:
			map[m].cell[j] &= ~CELL_NPC;
			break;
		case CELL_SETICEWALL:
			map[m].cell[j] |= CELL_ICEWALL;
			break;
		case CELL_CLRICEWALL:
			map[m].cell[j] &= ~CELL_ICEWALL;
			break;
		case CELL_SETBASILICA:
			map[m].cell[j] |= CELL_BASILICA;
			break;
		case CELL_CLRBASILICA:
			map[m].cell[j] &= ~CELL_BASILICA;
			break;
		case CELL_SETPNEUMA:
			map[m].cell[j] |= CELL_PNEUMA;
			break;
		case CELL_CLRPNEUMA:
			map[m].cell[j] &= ~CELL_PNEUMA;
			break;
		case CELL_SETSAFETYWALL:
			map[m].cell[j] |= CELL_SAFETYWALL;
			break;
		case CELL_CLRSAFETYWALL:
			map[m].cell[j] &= ~CELL_SAFETYWALL;
			break;
		case CELL_SETLANDPROTECTOR:
			map[m].cell[j] |= CELL_LANDPROTECTOR;
			break;
		case CELL_CLRLANDPROTECTOR:
			map[m].cell[j] &= ~CELL_LANDPROTECTOR;
			break;
		case CELL_SETREGEN:
			map[m].cell[j] |= CELL_REGEN;
			break;
		default:
			map[m].gat[j] = cell;
			break;
	}
}
static void* create_map_data_other_server(DBKey key, va_list args) {
	struct map_data_other_server *mdos;
	unsigned short mapindex = (unsigned short)key.ui;
	mdos=(struct map_data_other_server *)aCalloc(1,sizeof(struct map_data_other_server));
	mdos->index = mapindex;
	memcpy(mdos->name, mapindex_id2name(mapindex), MAP_NAME_LENGTH);
	return mdos;
}
/*==========================================
 * 他鯖管理のマップをdbに追加
 *------------------------------------------
 */
int map_setipport(unsigned short mapindex,unsigned long ip,int port) {
	struct map_data_other_server *mdos=NULL;

	mdos=(struct map_data_other_server *)uidb_ensure(map_db,(unsigned int)mapindex, create_map_data_other_server);
	
	if(mdos->gat) //Local map,Do nothing. Give priority to our own local maps over ones from another server. [Skotlex]
		return 0;
	if(ip == clif_getip_long() && port == clif_getport()) {
		//That's odd, we received info that we are the ones with this map, but... we don't have it.
		ShowFatalError("map_setipport : received info that this map-server SHOULD have map '%s', but it is not loaded.\n",mapindex_id2name(mapindex));
		exit(1);
	}
	mdos->ip   = ip;
	mdos->port = port;
	return 1;
}

/*==========================================
 * 他鯖管理のマップを全て削除
 *------------------------------------------
 */
int map_eraseallipport_sub(DBKey key,void *data,va_list va) {
	struct map_data_other_server *mdos = (struct map_data_other_server*)data;
	if(mdos->gat == NULL) {
		db_remove(map_db,key);
		aFree(mdos);
	}
	return 0;
}

int map_eraseallipport(void) {
	map_db->foreach(map_db,map_eraseallipport_sub);
	return 1;
}

/*==========================================
 * 他鯖管理のマップをdbから削除
 *------------------------------------------
 */
int map_eraseipport(unsigned short mapindex,unsigned long ip,int port)
{
	struct map_data_other_server *mdos;
//	unsigned char *p=(unsigned char *)&ip;

	mdos = uidb_get(map_db,(unsigned int)mapindex);
	if(!mdos || mdos->gat) //Map either does not exists or is a local map.
		return 0;

	if(mdos->ip==ip && mdos->port == port) {
		uidb_remove(map_db,(unsigned int)mapindex);
		aFree(mdos);
		return 1;
	}
	return 0;
}

#define NO_WATER 1000000

static int map_setwaterheight_sub(int m) {
	char fn[256];
	char *gat;
	int x,y;
	int wh;
	struct gat_1cell {float high[4]; int type;} *p = NULL;
	
	if (m < 0)
		return 0;
	wh = map[m].water_height;
		
	sprintf(fn,"data\\%s",mapindex_id2name(map[m].index));

	// read & convert fn
	// again, might not need to be unsigned char
	gat = (char *) grfio_read (fn);
	if (gat == NULL)
		return 0;

	for (y = 0; y < map[m].ys; y++) {
		p = (struct gat_1cell*)(gat+y*map[m].xs*20+14);
		for (x = 0; x < map[m].xs; x++) {
			if (wh != NO_WATER && p->type == 0) //Set water cell
				map[m].gat[x+y*map[m].xs] = (p->high[0]>wh || p->high[1]>wh || p->high[2]>wh || p->high[3]>wh) ? 3 : 0;
			else //Remove water cell
				map[m].gat[x+y*map[m].xs] = p->type==3?0:p->type;
			p++;
		}
	}
	aFree(gat);
	return 1;
}
int map_setwaterheight(int m, char *mapname, int height) {
	if (height < 0)
		height = NO_WATER;
	map[m].water_height = height;
	return map_setwaterheight_sub(m);
}

/* map_readwaterheight
 * Reads from the .rsw for each map
 * Returns water height (or NO_WATER if file doesn't exist)
 * or other error is encountered.
 * This receives a map-name, and changes the extension to rsw if it isn't set already.
 * Assumed path for file is data/mapname.rsw
 * Credits to LittleWolf
 */
int map_waterheight(char *mapname) {
	char fn[256];
 	char *rsw, *found;
	float whtemp;
	int wh;

	//Look up for the rsw
	if(!strstr(mapname,"data\\"))
		sprintf(fn,"data\\%s", mapname);
	else
		strcpy(fn, mapname);

	found = grfio_find_file(fn);
	if (!found)
		; //Stick to the current fn
	else if(!strstr(found,"data\\"))
		sprintf(fn,"data\\%s", found);
	else
		strcpy(fn, found);
	
	rsw = strstr(fn, ".");
	if (rsw && strstr(fn, ".rsw") == NULL)
		sprintf(rsw,".rsw");
	// read & convert fn
	// again, might not need to be unsigned char
	rsw = (char *) grfio_read (fn);
	if (rsw)
	{	//Load water height from file
		whtemp = *(float*)(rsw+166);
		wh = (int) whtemp;
		aFree(rsw);
		return wh;
	}
	ShowWarning("Failed to find water level for (%s)\n", mapname, fn);
	return NO_WATER;
}

/*==========================================
* マップキャッシュに追加する
*===========================================*/

// マップキャッシュの最大値
#define MAX_MAP_CACHE 768

//各マップごとの最小限情報を入れるもの、READ_FROM_BITMAP用
struct map_cache_info {
	char fn[32];//ファイル名
	int xs,ys; //幅と高さ
	int water_height;
	int pos;  // データが入れてある場所
	int compressed;     // zilb通せるようにする為の予約
	int compressed_len; // zilb通せるようにする為の予約
}; // 56 byte

struct map_cache_head {
	int sizeof_header;
	int sizeof_map;
	// 上の２つ改変不可
	int nmaps; // マップの個数
	int filesize;
};

struct {
	struct map_cache_head head;
	struct map_cache_info *map;
	FILE *fp;
	int dirty;
} map_cache;

static int map_cache_open(char *fn);
static void map_cache_close(void);
static int map_cache_read(struct map_data *m);
static int map_cache_write(struct map_data *m);

static int map_cache_open(char *fn)
{
	if (map_cache.fp)
		map_cache_close();
	map_cache.fp = fopen(fn, "r+b");
	if (map_cache.fp) {
		fread(&map_cache.head,1,sizeof(struct map_cache_head),map_cache.fp);
		fseek(map_cache.fp,0,SEEK_END);
		if(
			map_cache.head.sizeof_header == sizeof(struct map_cache_head) &&
			map_cache.head.sizeof_map    == sizeof(struct map_cache_info) &&
			map_cache.head.filesize      == ftell(map_cache.fp)
		) {
			// キャッシュ読み甲ﾝ成功
			map_cache.map = (struct map_cache_info *) aMalloc(sizeof(struct map_cache_info) * map_cache.head.nmaps);
			fseek(map_cache.fp,sizeof(struct map_cache_head),SEEK_SET);
			fread(map_cache.map,sizeof(struct map_cache_info),map_cache.head.nmaps,map_cache.fp);
			return 1;
		}
		fclose(map_cache.fp);
	}
	// 読み甲ﾝに失敗したので新規に作成する
	map_cache.fp = fopen(fn,"wb");
	if(map_cache.fp) {
		malloc_set(&map_cache.head,0,sizeof(struct map_cache_head));
		map_cache.map   = (struct map_cache_info *) aCalloc(sizeof(struct map_cache_info),MAX_MAP_CACHE);
		map_cache.head.nmaps         = MAX_MAP_CACHE;
		map_cache.head.sizeof_header = sizeof(struct map_cache_head);
		map_cache.head.sizeof_map    = sizeof(struct map_cache_info);

		map_cache.head.filesize  = sizeof(struct map_cache_head);
		map_cache.head.filesize += sizeof(struct map_cache_info) * map_cache.head.nmaps;

		map_cache.dirty = 1;
		return 1;
	}
	return 0;
}

static void map_cache_close(void)
{
	if(!map_cache.fp) { return; }
	if(map_cache.dirty) {
		fseek(map_cache.fp,0,SEEK_SET);
		fwrite(&map_cache.head,1,sizeof(struct map_cache_head),map_cache.fp);
		fwrite(map_cache.map,map_cache.head.nmaps,sizeof(struct map_cache_info),map_cache.fp);
	}
	fclose(map_cache.fp);
	aFree(map_cache.map);
	map_cache.fp = NULL;
	return;
}

int map_cache_read(struct map_data *m)
{
	int i;
	if(!map_cache.fp) { return 0; }
	for(i = 0;i < map_cache.head.nmaps ; i++) {
		if(!strcmp(m->name,map_cache.map[i].fn)) {
			if(map_cache.map[i].compressed == 0) {
				// 非圧縮ファイル
				int size = map_cache.map[i].xs * map_cache.map[i].ys;
				m->xs = map_cache.map[i].xs;
				m->ys = map_cache.map[i].ys;
				m->water_height = map_cache.map[i].water_height;
				m->gat = (unsigned char *)aCalloc(m->xs * m->ys,sizeof(unsigned char));
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				if(fread(m->gat,1,size,map_cache.fp) == size) {
					// 成功
					return 1;
				} else {
					// なぜかファイル後半が欠けてるので読み直し
					m->xs = 0; m->ys = 0; aFree(m->gat); m->gat = NULL;
					return 0;
				}
			} else if(map_cache.map[i].compressed == 1) {
				// 圧縮フラグ=1 : zlib
				unsigned char *buf;
				unsigned long dest_len;
				int size_compress = map_cache.map[i].compressed_len;
				m->xs = map_cache.map[i].xs;
				m->ys = map_cache.map[i].ys;
				m->water_height = map_cache.map[i].water_height;
				m->gat = (unsigned char *)aMalloc(m->xs * m->ys * sizeof(unsigned char));
				buf = (unsigned char*)aMalloc(size_compress);
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				if(fread(buf,1,size_compress,map_cache.fp) != size_compress) {
					// なぜかファイル後半が欠けてるので読み直し
					ShowError("fread error\n");
					aFree(m->gat); m->xs = 0; m->ys = 0; m->gat = NULL;
					aFree(buf);
					return 0;
				}
				dest_len = m->xs * m->ys;
				decode_zip(m->gat,&dest_len,buf,size_compress);
				if(dest_len != map_cache.map[i].xs * map_cache.map[i].ys) {
					// 正常に解凍が出来てない
					aFree(m->gat); m->xs = 0; m->ys = 0; m->gat = NULL;
					aFree(buf);
					return 0;
				}
				aFree(buf);
				return 1;
			}
		}
	}
	return 0;
}

static int map_cache_write(struct map_data *m)
{
	int i;
	unsigned long len_new , len_old;
	char *write_buf;
	if(!map_cache.fp) { return 0; }
	for(i = 0;i < map_cache.head.nmaps ; i++) {
		if(!strcmp(m->name,map_cache.map[i].fn)) {
			// 同じエントリーがあれば上書き
			if(map_cache.map[i].compressed == 0) {
				len_old = map_cache.map[i].xs * map_cache.map[i].ys;
			} else if(map_cache.map[i].compressed == 1) {
				len_old = map_cache.map[i].compressed_len;
			} else {
				// サポートされてない形式なので長さ０
				len_old = 0;
			}
			if(map_read_flag == 2) {
				// 圧縮保存
				// さすがに２倍に膨れる事はないという事で
				write_buf = (char *) aMalloc(m->xs * m->ys * 2);
				len_new = m->xs * m->ys * 2;
				encode_zip((unsigned char *) write_buf,&len_new,m->gat,m->xs * m->ys);
				map_cache.map[i].compressed     = 1;
				map_cache.map[i].compressed_len = len_new;
			} else {
				len_new = m->xs * m->ys;
				write_buf = (char *) m->gat;
				map_cache.map[i].compressed     = 0;
				map_cache.map[i].compressed_len = 0;
			}
			if(len_new <= len_old) {
				// サイズが同じか小さくなったので場所は変わらない
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);
			} else {
				// 新しい場所に登録
				fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);
				map_cache.map[i].pos = map_cache.head.filesize;
				map_cache.head.filesize += len_new;
			}
			map_cache.map[i].xs  = m->xs;
			map_cache.map[i].ys  = m->ys;
			map_cache.map[i].water_height = m->water_height;
			map_cache.dirty = 1;
			if(map_read_flag == 2) {
				aFree(write_buf);
			}
			return 0;
		}
	}
	// 同じエントリが無ければ書き甲ﾟる場所を探す
	for(i = 0;i < map_cache.head.nmaps ; i++) {
		if(map_cache.map[i].fn[0] == 0) {
			// 新しい場所に登録
			if(map_read_flag == 2) {
				write_buf = (char *) aMalloc(m->xs * m->ys * 2);
				len_new = m->xs * m->ys * 2;
				encode_zip((unsigned char *) write_buf,&len_new,m->gat,m->xs * m->ys);
				map_cache.map[i].compressed     = 1;
				map_cache.map[i].compressed_len = len_new;
			} else {
				len_new = m->xs * m->ys;
				write_buf = (char *) m->gat;
				map_cache.map[i].compressed     = 0;
				map_cache.map[i].compressed_len = 0;
			}
			strncpy(map_cache.map[i].fn,m->name,sizeof(map_cache.map[0].fn));
			fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
			fwrite(write_buf,1,len_new,map_cache.fp);
			map_cache.map[i].pos = map_cache.head.filesize;
			map_cache.map[i].xs  = m->xs;
			map_cache.map[i].ys  = m->ys;
			map_cache.map[i].water_height = m->water_height;
			map_cache.head.filesize += len_new;
			map_cache.dirty = 1;
			if(map_read_flag == 2)
				aFree(write_buf);
			return 0;
		}
	}
	// 書き甲ﾟなかった
	return 1;
}

/*==========================================
 * ?み?むmapを追加する
 *------------------------------------------
 */
int map_addmap(char *mapname) {
	if (strcmpi(mapname,"clear")==0) {
		map_num=0;
		return 0;
	}

	if (map_num >= MAX_MAP_PER_SERVER - 1) {
		ShowError("Could not add map '"
		CL_WHITE"%s"CL_RESET"', the limit of maps has been reached.\n",mapname);
		return 1;
	}
	memcpy(map[map_num].name, mapname, MAP_NAME_LENGTH-1);
	map_num++;
	return 0;
}

/*==========================================
 * Removes the map in the index passed.
 *------------------------------------------
 */
static void map_delmapid(int id)
{
	ShowNotice("Removing map [ %s ] from maplist\n",map[id].name);
	memmove(map+id, map+id+1, sizeof(map[0])*(map_num-id-1));
	map_num--;
}

/*==========================================
 * ?み?むmapを削除する
 *------------------------------------------
 */
int map_delmap(char *mapname) {

	int i;

	if (strcmpi(mapname, "all") == 0) {
		map_num = 0;
		return 0;
	}

	for(i = 0; i < map_num; i++) {
		if (strcmp(map[i].name, mapname) == 0) {
			map_delmapid(i);
			return 1;
		}
	}
	return 0;
}

////////////////////////////////////////////////

/*
	Advanced Fusion Maps Support
	(c) 2003-2004, The Fusion Project
	- AlexKreuz

	The following code has been provided by me for eAthena
	under the GNU GPL.  It provides Advanced Fusion
	Map, the map format desgined by me for Fusion, support
	for the eAthena emulator.

	I understand that because it is under the GPL
	that other emulators may very well use this code in their
	GNU project as well.

	The AFM map format was not originally a part of the GNU
	GPL. It originated from scratch by my own hand.  I understand
	that distributing this code to read the AFM maps with eAthena
	causes the GPL to apply to this code.  But the actual AFM
	maps are STILL copyrighted to the Fusion Project.  By choosing

	In exchange for that 'act of faith' I ask for the following.

	A) Give credit where it is due.  If you use this code, do not
	   place your name on the changelog.  Credit should be given
	   to AlexKreuz.
	B) As an act of courtesy, ask me and let me know that you are putting
	   AFM support in your project.  You will have my blessings if you do.
	C) Use the code in its entirety INCLUDING the copyright message.
	   Although the code provided may now be GPL, the AFM maps are not
	   and so I ask you to display the copyright message on the STARTUP
	   SCREEN as I have done here. (refer to core.c)
	   "Advanced Fusion Maps (c) 2003-2004 The Fusion Project"

	Without this copyright, you are NOT entitled to bundle or distribute
	the AFM maps at all.  On top of that, your "support" for AFM maps
	becomes just as shady as your "support" for Gravity GRF files.

	The bottom line is this.  I know that there are those of you who
	would like to use this code but aren't going to want to provide the
	proper credit.  I know this because I speak frome experience.  If
	you are one of those people who is going to try to get around my
	requests, then save your breath because I don't want to hear it.

	I have zero faith in GPL and I know and accept that if you choose to
	not display the copyright for the AFMs then there is absolutely nothing
	I can do about it.  I am not about to start a legal battle over something
	this silly.

	Provide the proper credit because you believe in the GPL.  If you choose
	not to and would rather argue about it, consider the GPL failed.

	October 18th, 2004
	- AlexKreuz
	- The Fusion Project
	*/
static int map_loadafm (struct map_data *m, char *fn)
{
	// check if .afm file exists
	FILE *afm_file = fopen(fn, "r");
	if (afm_file != NULL) {
		int x,y,xs,ys;
		char afm_line[65535];
		int afm_size[2];
		char *str;

		//Gotta skip the first two lines which are just a header of sorts.
		str = fgets(afm_line, sizeof(afm_line)-1, afm_file);
		str = fgets(afm_line, sizeof(afm_line)-1, afm_file);
		str = fgets(afm_line, sizeof(afm_line)-1, afm_file);
		if (!str) return 0;
		sscanf(str , "%d%d", &afm_size[0], &afm_size[1]);

		xs = m->xs = afm_size[0];
		ys = m->ys = afm_size[1];
		m->water_height = map_waterheight(m->name);
		// check this, unsigned where it might not need to be
		m->gat = (unsigned char*)aMallocA(xs * ys);

		for (y = 0; y < ys; y++) {
			str = fgets(afm_line, sizeof(afm_line)-1, afm_file);
			for (x = 0; x < xs; x++)
				m->gat[x+y*xs] = str[x]-48;
		}

		fclose(afm_file);
		return 1;
	}

	return 0;
}
/*==================================
 * .AFM format
 *----------------------------------
 */
int map_readafm (struct map_data *m)
{
	char afm_name[256] = "";
	char fn[256], *p;

	// convert map name to .afm
	if(!strstr(m->name, ".afm")) {
		// check if it's necessary to replace the extension - speeds up loading a bit
		strncpy(afm_name, m->name, strlen(m->name) - 4);
		strcat(afm_name, ".afm");
	} else {
		strcpy(afm_name, m->name);
	}
	
	sprintf(fn, "%s\\%s", afm_dir, afm_name);
	for (p = &fn[0]; *p != 0; p++)
		if (*p == '\\') *p = '/';	// * At the time of Unix

	return map_loadafm(m, fn);
}
/*==================================
 * .AF2 format
 *----------------------------------
 */
int map_readaf2 (struct map_data *m)
{
	FILE *af2_file;
	char af2_name[256] = "";
	char fn[256], *p, *out;
	
	// convert map name to .af2
	p = out = m->name;
	while ((p = strchr(p, '/')) != NULL)
		out = ++p;
	strncpy (af2_name, out, strlen(out));
	// grr, this is so troublesome >.< [celest]
	p = strrchr (af2_name, '.');
	if (p) *p++ = 0;
	strcat(af2_name, ".af2");	
	sprintf(fn, "%s\\%s", afm_dir, af2_name);
	for (p = &fn[0]; *p != 0; p++)
		if (*p == '\\') *p = '/';	// * At the time of Unix

	// check if .af2 file exists
	af2_file = fopen(fn, "r");
	if (af2_file != NULL) {
		char out_file[256];

		fclose(af2_file);
		
		// convert map name to .out
		strncpy (out_file, out, strlen(out));
		p = strrchr (out_file, '.');
		if (p) *p++ = 0;
		strcat(out_file, ".out");

		// unzip .out file and use loadafm()
		if (deflate_file(fn, out_file) &&
			map_loadafm(m, out_file))
		{
			unlink (out_file);
			return 1;
		}
	}

	return 0;
}


/*==========================================
 * マップ1枚読み甲ﾝ
 * ===================================================*/
//static int map_readmap(int m,char *fn, char *alias, int *map_cache, int maxmap) {

/*==================================
 * .GAT format
 *----------------------------------
 */
int map_readgat (struct map_data *m)
{
	char fn[256];
	char *gat;
	int wh,x,y,xs,ys;
	struct gat_1cell {float high[4]; int type;} *p = NULL;
	
	if (strstr(m->name,".gat") == NULL)
		return 0;

	sprintf(fn,"data\\%s",m->name);

	// read & convert fn
	// again, might not need to be unsigned char
	gat = (char *) grfio_read (fn);
	if (gat == NULL)
		return 0;

	xs = m->xs = *(int*)(gat+6);
	ys = m->ys = *(int*)(gat+10);
	m->gat = (unsigned char *)aMallocA((m->xs * m->ys)*sizeof(unsigned char));

	m->water_height = wh = map_waterheight(m->name);
	for (y = 0; y < ys; y++) {
		p = (struct gat_1cell*)(gat+y*xs*20+14);
		for (x = 0; x < xs; x++) {
			if (wh != NO_WATER && p->type == 0)
				// 水場判定
				m->gat[x+y*xs] = (p->high[0]>wh || p->high[1]>wh || p->high[2]>wh || p->high[3]>wh) ? 3 : 0;
			else
				m->gat[x+y*xs] = p->type;
			p++;
		}
	}

	aFree(gat);

	return 1;
}

//////////////////////////////////////////////////////

static int map_cache_init (void);
static int map_readafm_init (void);
static int map_readaf2_init (void);
static int map_readgat_init (void);

// Todo: Properly implement this system as plugins/safer code [Celest]
enum {
	MAP_CACHE = 0,	// jAthena map cache
	MAP_AFM,	// Advanced Fusion Map
	MAP_AF2,	// Advanced Fusion Map
	MAP_GAT,	// GRF map
	MAP_MAXSOURCE
};
// in descending order
int (*mapsource_init[MAP_MAXSOURCE])(void) = {
	map_cache_init,
	map_readafm_init,
	map_readaf2_init,
	map_readgat_init
};
int (*mapsource_read[MAP_MAXSOURCE])(struct map_data *) = {
	map_cache_read,
	map_readafm,
	map_readaf2,
	map_readgat
};
void (*mapsource_final[MAP_MAXSOURCE])(void) = {
	map_cache_close,
	NULL,
	NULL,
	NULL
};

static int map_cache_init (void)
{
	if (map_read_flag >= READ_FROM_BITMAP && map_cache_open(map_cache_file)) {
		ShowMessage("[cache] ");
		return 1;
	}

	return 0;
}
static int map_readafm_init (void)
{
	ShowMessage("[afm] ");
	return 1;
}
static int map_readaf2_init (void)
{
	// check if AFM loading is available,
	// otherwise disable AF2 loading
	if (mapsource_read[1] != NULL) {
		ShowMessage("[af2] ");
		return 1;
	}

	return 0;
}
static int map_readgat_init (void)
{
	ShowMessage("[gat] ");
	return 1;
}

/*======================================
 * Initiate maps loading stage
 *--------------------------------------
 */
int map_readallmaps (void)
{
	// pre-loading stage
	int i;
	int maps_removed = 0;
	int maps_cached = 0;

	ShowMessage(CL_GREEN"[Status]"CL_RESET": Loading Maps with... "CL_WHITE);

	for (i = 0; i < MAP_MAXSOURCE; i++) {
		if (mapsource_init[i] &&	// check if source requires initialisation
			mapsource_init[i]() == 0)	// if init failed
		{
			// remove all loading methods associated with this source
			mapsource_init[i] = NULL;
			mapsource_read[i] = NULL;
			mapsource_final[i] = NULL;
		}
	}

	ShowMessage(CL_RESET"\n");

	// initiate map loading
	for (i = 0; i < map_num; i++)
	{
		int success = 0;
		static int lasti = -1;
		static int last_time = -1;
		int j = i*20/map_num;

		// show progress
		if (map_num &&	//avoid map-server crashing if there are 0 maps
			(j != lasti || last_time != time(0)))
		{
			char progress[21] = "                    ";
			char c = '-';
			int k;

			lasti = j;
			printf("\r");
			ShowStatus("Progress: [");
			for (k=0; k < j; k++) progress[k] = '#';
			printf(progress);
			last_time = (int)time(0);
			switch(last_time % 4) {
				case 0: c='\\'; break;
				case 1: c='|'; break;
				case 2: c='/'; break;
				case 3: c='-'; break;
			}
			printf("] Working: [%c]",c);
			fflush(stdout);
		}

		// pre-init some data
		map[i].alias = NULL;
		map[i].m = i;
		malloc_set (map[i].moblist, 0, sizeof(map[i].moblist));	//Initialize moblist [Skotlex]
		map[i].mob_delete_timer = -1;	//Initialize timer [Skotlex]
		if (battle_config.pk_mode)
			map[i].flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]

		for (j = 0; j < MAP_MAXSOURCE; j++)
		{
			if (mapsource_read[j] &&	// check if map source is valid
				mapsource_read[j](&map[i]))	// check if map source is available
			{
				// successful, now initialise map
				size_t size;
				char *alias;

				if (map[i].alias && (alias = strstr(map[i].name, "<")) != NULL) {	// alias has been set by one of the sources
					*alias++ = '\0';
				}
				if (map[i].alias)
					map[i].index = mapindex_name2id(map[i].alias);
				else
					map[i].index = mapindex_name2id(map[i].name);
				
				if (!map[i].index) {
					if (map[i].alias)
						ShowWarning("Map %s (alias %s) is not in the map-index cache!\n", map[i].name, map[i].alias);
					else
						ShowWarning("Map %s is not in the map-index cache!\n", map[i].name);
					success = 0; //Can't load a map that isn't in our cache.
					if (map[i].gat) {
						aFree(map[i].gat);
						map[i].gat = NULL;	
					}
					break;
				}
				if (uidb_get(map_db,(unsigned int)map[i].index) != NULL) {
					ShowWarning("Map %s already loaded!\n", map[i].name);
					success = 0; //Can't load a map already in the db
					if (map[i].gat) {
						aFree(map[i].gat);
						map[i].gat = NULL;	
					}
					break;
				}

				map[i].cell = (unsigned char *)aCalloc(map[i].xs * map[i].ys, sizeof(unsigned char));
#ifdef CELL_NOSTACK
				map[i].cell_bl = (unsigned char *)aCalloc(map[i].xs * map[i].ys, sizeof(unsigned char));
#endif

				map[i].bxs = (map[i].xs + BLOCK_SIZE - 1) / BLOCK_SIZE;
				map[i].bys = (map[i].ys + BLOCK_SIZE - 1) / BLOCK_SIZE;
				
				// default experience multiplicator
				map[i].jexp = 100;
				map[i].bexp = 100;
				
				size = map[i].bxs * map[i].bys * sizeof(struct block_list*);
				map[i].block = (struct block_list**)aCalloc(size, 1);
				map[i].block_mob = (struct block_list**)aCalloc(size, 1);

				size = map[i].bxs * map[i].bys * sizeof(int);
				map[i].block_count = (int*)aCallocA(size, 1);
				map[i].block_mob_count = (int*)aCallocA(size, 1);

				uidb_put(map_db, (unsigned int)map[i].index, &map[i]);

				// cache our map if necessary
				if (j != MAP_CACHE && mapsource_read[MAP_CACHE] != NULL) {	// map data is not cached yet
					map_cache_write(&map[i]);
					maps_cached++;
				}

				// next map
				success = 1;
				break;
			}
		}

		// no sources have been found, so remove map from list
		if (!success) {
			map_delmapid(i);
			maps_removed++;
			i--;
		}
	}

	// unload map sources
	for (i = 0; i < MAP_MAXSOURCE; i++) {
		if (mapsource_final[i])
			mapsource_final[i]();
	}

	// finished map loading
	printf("\r");
	ShowInfo("Successfully loaded '"CL_WHITE"%d"CL_RESET"' maps.%30s\n",map_num,"");

	if (maps_removed)
		ShowNotice("Maps Removed: '"CL_WHITE"%d"CL_RESET"'\n",maps_removed);
	if (maps_cached)
		ShowNotice("Maps Added to Cache: '"CL_WHITE"%d"CL_RESET"'\n",maps_cached);

	return 0;
}

////////////////////////////////////////////////////////////////////////
static int map_ip_set = 0;
static int char_ip_set = 0;

/*==========================================
 * Console Command Parser [Wizputer]
 *------------------------------------------
 */
int parse_console(char *buf) {
	char type[64],command[64],map[64], buf2[72];
	int x = 0, y = 0;
	int m, n;
	struct map_session_data sd;

	malloc_set(&sd, 0, sizeof(struct map_session_data));
	strcpy( sd.status.name , "console");

	if ( ( n = sscanf(buf, "%[^:]:%[^:]:%99s %d %d[^\n]", type , command , map , &x , &y )) < 5 )
		if ( ( n = sscanf(buf, "%[^:]:%[^\n]", type , command )) < 2 )
			n = sscanf(buf,"%[^\n]",type);

	if ( n == 5 ) {
		m = map_mapname2mapid(map);
		if ( m < 0 ) {
			ShowWarning("Console: Unknown map\n");
			return 0;
		}
		sd.bl.m = m;
		map_search_freecell(&sd.bl, m, &sd.bl.x, &sd.bl.y, -1, -1, 0); 
		if (x > 0)
			sd.bl.x = x;

		if (y > 0)
			sd.bl.y = y;
	}

	ShowInfo("Type of command: %s || Command: %s || Map: %s Coords: %d %d\n",type,command,map,x,y);

	if ( strcmpi("admin",type) == 0 && n == 5 ) {
		sprintf(buf2,"console: %s",command);
		if( is_atcommand(sd.fd,&sd,buf2,99) == AtCommand_None )
			printf("Console: not atcommand\n");
	} else if ( strcmpi("server",type) == 0 && n == 2 ) {
		if ( strcmpi("shutdown", command) == 0 || strcmpi("exit",command) == 0 || strcmpi("quit",command) == 0 ) {
			runflag = 0;
		}
	} else if ( strcmpi("help",type) == 0 ) {
		ShowNotice("To use GM commands:\n");
		printf("admin:<gm command>:<map of \"gm\"> <x> <y>\n");
		printf("You can use any GM command that doesn't require the GM.\n");
		printf("No using @item or @warp however you can use @charwarp\n");
		printf("The <map of \"gm\"> <x> <y> is for commands that need coords of the GM\n");
		printf("IE: @spawn\n");
		printf("To shutdown the server:\n");
		printf("server:shutdown\n");
	}

	return 0;
}

/*==========================================
 * 設定ファイルを?み?む
 *------------------------------------------
 */
int map_config_read(char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName,"r");
	if (fp == NULL) {
		ShowFatalError("Map configuration file not found at: %s\n", cfgName);
		exit(1);
	}
	while(fgets(line, sizeof(line) -1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
			if(strcmpi(w1,"timestamp_format")==0){
				strncpy(timestamp_format, w2, 20);
			} else if(strcmpi(w1,"console_silent")==0){
				msg_silent = 0; //To always allow the next line to show up.
				ShowInfo("Console Silent Setting: %d\n", atoi(w2));
				msg_silent = atoi(w2);
			} else if (strcmpi(w1, "userid")==0){
				chrif_setuserid(w2);
			} else if (strcmpi(w1, "passwd") == 0) {
				chrif_setpasswd(w2);
			} else if (strcmpi(w1, "char_ip") == 0) {
				char_ip_set = chrif_setip(w2);
			} else if (strcmpi(w1, "char_port") == 0) {
				chrif_setport(atoi(w2));
			} else if (strcmpi(w1, "map_ip") == 0) {
				map_ip_set = clif_setip(w2);
			} else if (strcmpi(w1, "bind_ip") == 0) {
				clif_setbindip(w2);
			} else if (strcmpi(w1, "map_port") == 0) {
				clif_setport(atoi(w2));
				map_port = (atoi(w2));
			} else if (strcmpi(w1, "map") == 0) {
				map_addmap(w2);
			} else if (strcmpi(w1, "delmap") == 0) {
				map_delmap(w2);
			} else if (strcmpi(w1, "npc") == 0) {
				npc_addsrcfile(w2);
			} else if (strcmpi(w1, "delnpc") == 0) {
				npc_delsrcfile(w2);
			} else if (strcmpi(w1, "autosave_time") == 0) {
				autosave_interval = atoi(w2);
				if (autosave_interval < 1) //Revert to default saving.
					autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
				else
					autosave_interval *= 1000; //Pass from sec to ms
			} else if (strcmpi(w1, "minsave_time") == 0) {
				minsave_interval= atoi(w2);
				if (minsave_interval < 1)
					minsave_interval = 1;
			} else if (strcmpi(w1, "save_settings") == 0) {
				save_settings = atoi(w2);
			} else if (strcmpi(w1, "motd_txt") == 0) {
				strcpy(motd_txt, w2);
			} else if (strcmpi(w1, "help_txt") == 0) {
				strcpy(help_txt, w2);
			} else if (strcmpi(w1, "help2_txt") == 0) {
				strcpy(help2_txt, w2);
			} else if (strcmpi(w1, "charhelp_txt") == 0) {
				strcpy(charhelp_txt, w2);
			} else if (strcmpi(w1, "mapreg_txt") == 0) {
				strcpy(mapreg_txt, w2);
			} else if(strcmpi(w1,"read_map_from_cache") == 0){
				if (atoi(w2) == 2)
					map_read_flag = READ_FROM_BITMAP_COMPRESSED;
				else if (atoi(w2) == 1)
					map_read_flag = READ_FROM_BITMAP;
				else
					map_read_flag = READ_FROM_GAT;
			} else if(strcmpi(w1,"map_cache_file") == 0) {
				strncpy(map_cache_file,w2,255);
			} else if(strcmpi(w1,"db_path") == 0) {
				strncpy(db_path,w2,255);
			} else if(strcmpi(w1,"afm_dir") == 0) {
				strcpy(afm_dir, w2);
			} else if (strcmpi(w1, "console") == 0) {
				if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 ) {
					console = 1;
					ShowNotice("Console Commands are enabled.\n");
				}
			} else if (strcmpi(w1, "enable_spy") == 0) {
				if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 )
					enable_spy = 1;
				else
					enable_spy = 0;
			} else if (strcmpi(w1, "import") == 0) {
				map_config_read(w2);
			}
		}
	}
	fclose(fp);

	return 0;
}

int inter_config_read(char *cfgName)
{
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowError("File not found: '%s'.\n",cfgName);
		return 1;
	}
	while(fgets(line,1020,fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if(strcmpi(w1,"party_share_level")==0){
			party_share_level = battle_config_switch(w2);
		} else if(strcmpi(w1,"lowest_gm_level")==0){
			lowest_gm_level = atoi(w2);
		
		/* Main chat nick [LuzZza] */
		} else if(strcmpi(w1, "main_chat_nick")==0){
			strcpy(main_chat_nick, w2);
			
	#ifndef TXT_ONLY
		} else if(strcmpi(w1,"charsave_method")==0){
			charsave_method = atoi(w2); //New char saving method.
		} else if(strcmpi(w1,"item_db_db")==0){
			strcpy(item_db_db,w2);
		} else if(strcmpi(w1,"mob_db_db")==0){
			strcpy(mob_db_db,w2);
		} else if(strcmpi(w1,"item_db2_db")==0){
			strcpy(item_db2_db,w2);
		} else if(strcmpi(w1,"mob_db2_db")==0){
			strcpy(mob_db2_db,w2);
		} else if(strcmpi(w1,"login_db_level")==0){
			strcpy(login_db_level,w2);
		} else if(strcmpi(w1,"login_db_account_id")==0){
		    strcpy(login_db_account_id,w2);
		} else if(strcmpi(w1,"login_db")==0){
			strcpy(login_db,w2);
		} else if (strcmpi(w1, "char_db") == 0) {
			strcpy(char_db, w2);
		} else if(strcmpi(w1,"gm_db_level")==0){
			strcpy(gm_db_level,w2);
		} else if(strcmpi(w1,"gm_db_account_id")==0){
		    strcpy(gm_db_account_id,w2);
		} else if(strcmpi(w1,"gm_db")==0){
			strcpy(gm_db,w2);
		//Map Server SQL DB
		} else if(strcmpi(w1,"map_server_ip")==0){
			strcpy(map_server_ip, w2);
		} else if(strcmpi(w1,"map_server_port")==0){
			map_server_port=atoi(w2);
		} else if(strcmpi(w1,"map_server_id")==0){
			strcpy(map_server_id, w2);
		} else if(strcmpi(w1,"map_server_pw")==0){
			strcpy(map_server_pw, w2);
		} else if(strcmpi(w1,"map_server_db")==0){
			strcpy(map_server_db, w2);
		} else if(strcmpi(w1,"default_codepage")==0){
			strcpy(default_codepage, w2);
		} else if(strcmpi(w1,"use_sql_db")==0){
			db_use_sqldbs = battle_config_switch(w2);
			ShowStatus ("Using SQL dbs: %s\n",w2);
		} else if(strcmpi(w1,"connection_ping_interval")==0) {
			connection_ping_interval = battle_config_switch(w2);
		//Login Server SQL DB
		} else if(strcmpi(w1,"login_server_ip")==0){
			strcpy(login_server_ip, w2);
        } else if(strcmpi(w1,"login_server_port")==0){
			login_server_port = atoi(w2);
		} else if(strcmpi(w1,"login_server_id")==0){
			strcpy(login_server_id, w2);
		} else if(strcmpi(w1,"login_server_pw")==0){
			strcpy(login_server_pw, w2);
		} else if(strcmpi(w1,"login_server_db")==0){
			strcpy(login_server_db, w2);
		} else if(strcmpi(w1,"read_gm_interval")==0){
			read_gm_interval = ( atoi(w2) * 60 * 1000 ); // Minutes multiplied by 60 secs per min by 1000 milliseconds per second
		}else if(strcmpi(w1, "char_server_ip") == 0){
			strcpy(charsql_host, w2);
		}else if(strcmpi(w1, "char_server_port") == 0){
			charsql_port = atoi(w2);
		}else if(strcmpi(w1, "char_server_id") == 0){
			strcpy(charsql_user, w2);
		}else if(strcmpi(w1, "char_server_pw") == 0){
			strcpy(charsql_pass, w2);
		}else if(strcmpi(w1, "char_server_db") == 0){
			strcpy(charsql_db, w2);
		} else if(strcmpi(w1,"log_db")==0) {
			strcpy(log_db, w2);
		} else if(strcmpi(w1,"log_db_ip")==0) {
			strcpy(log_db_ip, w2);
		} else if(strcmpi(w1,"log_db")==0) {
			strcpy(log_db, w2);
		} else if(strcmpi(w1,"log_db_id")==0) {
			strcpy(log_db_id, w2);
		} else if(strcmpi(w1,"log_db_pw")==0) {
			strcpy(log_db_pw, w2);
		} else if(strcmpi(w1,"log_db_port")==0) {
			log_db_port = atoi(w2);
		// Mail Server SQL 
		} else if(strcmpi(w1,"mail_server_enable")==0){
			mail_server_enable = battle_config_switch(w2);
			ShowStatus ("Using Mail Server: %s\n",w2);
		} else if(strcmpi(w1,"mail_server_ip")==0){
			strcpy(mail_server_ip, w2);
		} else if(strcmpi(w1,"mail_server_port")==0){
			mail_server_port=atoi(w2);
		} else if(strcmpi(w1,"mail_server_id")==0){
			strcpy(mail_server_id, w2);
		} else if(strcmpi(w1,"mail_server_pw")==0){
			strcpy(mail_server_pw, w2);
		} else if(strcmpi(w1,"mail_server_db")==0){
			strcpy(mail_server_db, w2);
		} else if(strcmpi(w1,"mail_db")==0) {
			strcpy(mail_db, w2);
	#endif
		//support the import command, just like any other config
		} else if(strcmpi(w1,"import")==0){
			inter_config_read(w2);
		}
	}
	fclose(fp);

	return 0;
}

#ifndef TXT_ONLY
/*=======================================
 *  MySQL Init
 *---------------------------------------
 */

int map_sql_init(void){

    mysql_init(&mmysql_handle);

	//DB connection start
	ShowInfo("Connecting to the Map DB Server....\n");
	if(!mysql_real_connect(&mmysql_handle, map_server_ip, map_server_id, map_server_pw,
		map_server_db ,map_server_port, (char *)NULL, 0)) {
			//pointer check
			ShowSQL("DB error - %s\n",mysql_error(&mmysql_handle));
			exit(1);
	}
	else {
		ShowStatus("connect success! (Map Server Connection)\n");
	}

	if(mail_server_enable) { // mail system [Valaris]
		mysql_init(&mail_handle);
	        ShowInfo("Connecting to the Mail DB Server....\n");
		if(!mysql_real_connect(&mail_handle, mail_server_ip, mail_server_id, mail_server_pw,
			mail_server_db ,mail_server_port, (char *)NULL, 0)) {
				ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
				exit(1);
		}
		if( strlen(default_codepage) > 0 ) {
			sprintf( tmp_sql, "SET NAMES %s", default_codepage );
			if (mysql_query(&mail_handle, tmp_sql)) {
				ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			}
		}
	}
	if( strlen(default_codepage) > 0 ) {
		sprintf( tmp_sql, "SET NAMES %s", default_codepage );
		if (mysql_query(&mmysql_handle, tmp_sql)) {
			ShowSQL("DB error - %s\n",mysql_error(&mmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
	}
	return 0;
}

int map_sql_close(void){
	mysql_close(&mmysql_handle);
	ShowStatus("Close Map DB Connection....\n");

	if (log_config.sql_logs)
	{
		mysql_close(&logmysql_handle);
		ShowStatus("Close Log DB Connection....\n");
	}

	return 0;
}

int log_sql_init(void){

    mysql_init(&logmysql_handle);

	//DB connection start
	ShowInfo(""CL_WHITE"[SQL]"CL_RESET": Connecting to the Log Database "CL_WHITE"%s"CL_RESET" At "CL_WHITE"%s"CL_RESET"...\n",log_db,log_db_ip);
	if(!mysql_real_connect(&logmysql_handle, log_db_ip, log_db_id, log_db_pw,
		log_db ,log_db_port, (char *)NULL, 0)) {
			//pointer check
			ShowSQL("DB error - %s\n",mysql_error(&logmysql_handle));
			exit(1);
	}
  
	ShowStatus(""CL_WHITE"[SQL]"CL_RESET": Successfully '"CL_GREEN"connected"CL_RESET"' to Database '"CL_WHITE"%s"CL_RESET"'.\n", log_db);
	if( strlen(default_codepage) > 0 ) {
		sprintf( tmp_sql, "SET NAMES %s", default_codepage );
		if (mysql_query(&logmysql_handle, tmp_sql)) {
			ShowSQL("DB error - %s\n",mysql_error(&logmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
	}
	return 0;
}
#endif /* not TXT_ONLY */

int map_db_final(DBKey k,void *d,va_list ap)
{
	struct map_data_other_server *mdos = (struct map_data_other_server*)d;
	if(mdos && mdos->gat == NULL)
		aFree(mdos);
	return 0;
}
int nick_db_final(void *k,void *d,va_list ap)
{
	char *p = (char *) d;
	if (p) aFree(p);
	return 0;
}

int cleanup_sub(struct block_list *bl, va_list ap) {
	nullpo_retr(0, bl);

	switch(bl->type) {
		case BL_PC:
			map_quit((struct map_session_data *) bl);
			break;
		case BL_NPC:
			npc_unload((struct npc_data *)bl);
			break;
		case BL_MOB:
			unit_free(bl,0);
			break;
		case BL_PET:
		//There is no need for this, the pet is removed together with the player. [Skotlex]
			break;
		case BL_ITEM:
			map_clearflooritem(bl->id);
			break;
		case BL_SKILL:
			skill_delunit((struct skill_unit *) bl, 1);
			break;
	}

	return 1;
}

static int cleanup_db_sub(DBKey key,void *data,va_list va) {
	return cleanup_sub((struct block_list*)data, NULL);
}

static int cleanup_db_subpc(DBKey key,void *data,va_list va) {
	struct map_session_data *sd = (TBL_PC*)data;
	if (!sd->state.finalsave)
  	{	//Error?
		ShowError("do_final: Player character in DB which was not sent to save! %d:%d\n", sd->status.account_id, sd->status.char_id);
		map_quit(sd); //Attempt force-save
	}
	//Force remove from memory...
	map_quit_ack(sd);
	return 1;
}

/*==========================================
 * map鯖終了・理
 *------------------------------------------
 */
void do_final(void) {
	int i, j;
	struct map_session_data **pl_allsd;

	ShowStatus("Terminating...\n");

	//we probably don't need the cache open at all times 'yet', so this is closed by mapsource_final [celest]
	//map_cache_close();

	// We probably don't need the grfio after server bootup 'yet' too. So this is closed near the end of do_init [Lance]
	if((battle_config.cardillust_read_grffile || battle_config.item_equip_override_grffile || 
		battle_config.item_slots_override_grffile || battle_config.item_name_override_grffile ||
		battle_config.skill_sp_override_grffile))
		grfio_final();

	for (i = 0; i < map_num; i++)
		if (map[i].m >= 0)
			map_foreachinmap(cleanup_sub, i, BL_ALL);

	//Scan any remaining players (between maps?) to kick them out. [Skotlex]
	pl_allsd = map_getallusers(&j);
	for (i = 0; i < j; i++)
		map_quit(pl_allsd[i]);
		
	id_db->foreach(id_db,cleanup_db_sub);
	chrif_char_reset_offline();
	chrif_flush_fifo();
  	//Online players were sent to save, but the ack will not arrive on time!
	//They have to be removed from memory, and assume the char-server saved them.
	pc_db->foreach(pc_db,cleanup_db_subpc);

	do_final_atcommand();
	do_final_battle();
	do_final_chrif(); // この内部でキャラを全て切断する
	do_final_npc();
//	map_removenpc();
	do_final_script();
	do_final_itemdb();
	do_final_storage();
	do_final_guild();
	do_final_party();
	do_final_pc();
	do_final_pet();
	do_final_mob();
	do_final_msg();
	do_final_skill();
	do_final_unit();
	if(use_irc)
		do_final_irc();

	map_getallusers(NULL); //Clear the memory allocated for this array.
	
	map_db->destroy(map_db, map_db_final);
	
	for (i=0; i<map_num; i++) {
		if(map[i].gat) aFree(map[i].gat);
		if(map[i].cell) aFree(map[i].cell);
#ifdef CELL_NOSTACK
		if(map[i].cell_bl) aFree(map[i].cell_bl);
#endif
		if(map[i].block) aFree(map[i].block);
		if(map[i].block_mob) aFree(map[i].block_mob);
		if(map[i].block_count) aFree(map[i].block_count);
		if(map[i].block_mob_count) aFree(map[i].block_mob_count);
		if(battle_config.dynamic_mobs) { //Dynamic mobs flag by [random]
			for (j=0; j<MAX_MOB_LIST_PER_MAP; j++)
				if (map[i].moblist[j]) aFree(map[i].moblist[j]);
		}
	}

	mapindex_final();
	
	id_db->destroy(id_db, NULL);
	pc_db->destroy(pc_db, NULL);
	charid_db->destroy(charid_db, NULL);

//#endif

#ifndef TXT_ONLY
    map_sql_close();
	if(charsave_method)
		charsql_db_init(0); //Connecting to chardb
#endif /* not TXT_ONLY */
	ShowStatus("Successfully terminated.\n");
}

/*======================================================
 * Map-Server Version Screen [MC Cameri]
 *------------------------------------------------------
 */
void map_helpscreen(int flag) { // by MC Cameri
	puts("Usage: map-server [options]");
	puts("Options:");
	puts(CL_WHITE"  Commands\t\t\tDescription"CL_RESET);
	puts("-----------------------------------------------------------------------------");
	puts("  --help, --h, --?, /?		Displays this help screen");
	puts("  --map-config <file>		Load map-server configuration from <file>");
	puts("  --battle-config <file>	Load battle configuration from <file>");
	puts("  --atcommand-config <file>	Load atcommand configuration from <file>");
	puts("  --charcommand-config <file>	Load charcommand configuration from <file>");
	puts("  --script-config <file>	Load script configuration from <file>");
	puts("  --msg-config <file>		Load message configuration from <file>");
	puts("  --grf-path-file <file>	Load grf path file configuration from <file>");
	puts("  --sql-config <file>		Load inter-server configuration from <file>");
	puts("				(SQL Only)");
	puts("  --log-config <file>		Load logging configuration from <file>");
	puts("				(SQL Only)");
	puts("  --version, --v, -v, /v	Displays the server's version");
	puts("\n");
	if (flag) exit(1);
}

/*======================================================
 * Map-Server Version Screen [MC Cameri]
 *------------------------------------------------------
 */
void map_versionscreen(int flag) {
	printf("CL_WHITE" "eAthena version %d.%02d.%02d, Athena Mod version %d" CL_RESET"\n",
		ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION,
		ATHENA_MOD_VERSION);
	puts(CL_GREEN "Website/Forum:" CL_RESET "\thttp://eathena.deltaanime.net/");
	puts(CL_GREEN "Download URL:" CL_RESET "\thttp://eathena.systeminplace.net/");
	puts(CL_GREEN "IRC Channel:" CL_RESET "\tirc://irc.deltaanime.net/#athena");
	puts("\nOpen " CL_WHITE "readme.html" CL_RESET " for more information.");
	if (ATHENA_RELEASE_FLAG) ShowNotice("This version is not for release.\n");
	if (flag) exit(1);
}


#ifndef TXT_ONLY
/*======================================================
 * Does a mysql_ping to all connection handles. [Skotlex]
 *------------------------------------------------------
 */
int map_sql_ping(int tid, unsigned int tick, int id, int data) 
{
	ShowInfo("Pinging SQL server to keep connection alive...\n");
	mysql_ping(&mmysql_handle);
	if (log_config.sql_logs)
		mysql_ping(&logmysql_handle);
	if(mail_server_enable)
		mysql_ping(&mail_handle);
	return 0;
}
#endif

/*======================================================
 * Map-Server Init and Command-line Arguments [Valaris]
 *------------------------------------------------------
 */
void set_server_type(void)
{
	SERVER_TYPE = ATHENA_SERVER_MAP;
}
int do_init(int argc, char *argv[]) {
	int i;

#ifdef GCOLLECT
	GC_enable_incremental();
#endif

	INTER_CONF_NAME="conf/inter_athena.conf";
	LOG_CONF_NAME="conf/log_athena.conf";
	MAP_CONF_NAME = "conf/map_athena.conf";
	BATTLE_CONF_FILENAME = "conf/battle_athena.conf";
	ATCOMMAND_CONF_FILENAME = "conf/atcommand_athena.conf";
	CHARCOMMAND_CONF_FILENAME = "conf/charcommand_athena.conf";
	SCRIPT_CONF_NAME = "conf/script_athena.conf";
	MSG_CONF_NAME = "conf/msg_athena.conf";
	GRF_PATH_FILENAME = "conf/grf-files.txt";

	srand(gettick());

	for (i = 1; i < argc ; i++) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--h") == 0 || strcmp(argv[i], "--?") == 0 || strcmp(argv[i], "/?") == 0)
			map_helpscreen(1);
		else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "--v") == 0 || strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "/v") == 0)
			map_versionscreen(1);
		else if (strcmp(argv[i], "--map_config") == 0 || strcmp(argv[i], "--map-config") == 0)
			MAP_CONF_NAME=argv[i+1];
		else if (strcmp(argv[i],"--battle_config") == 0 || strcmp(argv[i],"--battle-config") == 0)
			BATTLE_CONF_FILENAME = argv[i+1];
		else if (strcmp(argv[i],"--atcommand_config") == 0 || strcmp(argv[i],"--atcommand-config") == 0)
			ATCOMMAND_CONF_FILENAME = argv[i+1];
		else if (strcmp(argv[i],"--charcommand_config") == 0 || strcmp(argv[i],"--charcommand-config") == 0)
			CHARCOMMAND_CONF_FILENAME = argv[i+1];
		else if (strcmp(argv[i],"--script_config") == 0 || strcmp(argv[i],"--script-config") == 0)
			SCRIPT_CONF_NAME = argv[i+1];
		else if (strcmp(argv[i],"--msg_config") == 0 || strcmp(argv[i],"--msg-config") == 0)
			MSG_CONF_NAME = argv[i+1];
		else if (strcmp(argv[i],"--grf_path_file") == 0 || strcmp(argv[i],"--grf-path-file") == 0)
			GRF_PATH_FILENAME = argv[i+1];
#ifndef TXT_ONLY
		else if (strcmp(argv[i],"--inter_config") == 0 || strcmp(argv[i],"--inter-config") == 0)
			INTER_CONF_NAME = argv[i+1];
#endif
		else if (strcmp(argv[i],"--log_config") == 0 || strcmp(argv[i],"--log-config") == 0)
			LOG_CONF_NAME = argv[i+1];
		else if (strcmp(argv[i],"--run_once") == 0)	// close the map-server as soon as its done.. for testing [Celest]
			runflag = 0;
	}

	map_config_read(MAP_CONF_NAME);
	irc_read_conf(IRC_CONF); // [Zido]
	chrif_checkdefaultlogin();

	if (!map_ip_set || !char_ip_set) {
		// The map server should know what IP address it is running on
		//   - MouseJstr
		int localaddr = ntohl(addr_[0]);
		unsigned char *ptr = (unsigned char *) &localaddr;
		char buf[16];
		if (naddr_ == 0) {
			ShowError("\nUnable to determine your IP address... please edit the map_athena.conf file and set it.\n");
			ShowError("(127.0.0.1 is valid if you have no network interface)\n");
		}
		sprintf(buf, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);;
		if (naddr_ != 1)
			ShowNotice("Multiple interfaces detected..  using %s as our IP address\n", buf);
		else
			ShowInfo("Defaulting to %s as our IP address\n", buf);
		if (!map_ip_set)
			clif_setip(buf);
		if (!char_ip_set)
			chrif_setip(buf);
		if (ptr[0] == 192 && ptr[1] == 168)
			ShowNotice("\nFirewall detected.. \n    edit subnet_athena.conf and map_athena.conf\n\n");
	}

	if (SHOW_DEBUG_MSG)
		ShowNotice("Server running in '"CL_WHITE"Debug Mode"CL_RESET"'.\n");


	battle_config_read(BATTLE_CONF_FILENAME);
	msg_config_read(MSG_CONF_NAME);
	atcommand_config_read(ATCOMMAND_CONF_FILENAME);
	charcommand_config_read(CHARCOMMAND_CONF_FILENAME);
	script_config_read(SCRIPT_CONF_NAME);
	inter_config_read(INTER_CONF_NAME);
	log_config_read(LOG_CONF_NAME);

	id_db = db_alloc(__FILE__,__LINE__,DB_INT,DB_OPT_BASE,sizeof(int));
	pc_db = db_alloc(__FILE__,__LINE__,DB_INT,DB_OPT_BASE,sizeof(int));	//Added for reliable map_id2sd() use. [Skotlex]
	map_db = db_alloc(__FILE__,__LINE__,DB_UINT,DB_OPT_BASE,sizeof(int));
	charid_db = db_alloc(__FILE__,__LINE__,DB_INT,DB_OPT_RELEASE_DATA,sizeof(int));
#ifndef TXT_ONLY
	map_sql_init();
	if(charsave_method)
		charsql_db_init(1); //Connecting to chardb
#endif /* not TXT_ONLY */

	mapindex_init();
	grfio_init(GRF_PATH_FILENAME);

	map_readallmaps();

	add_timer_func_list(map_freeblock_timer, "map_freeblock_timer");
	add_timer_func_list(map_clearflooritem_timer, "map_clearflooritem_timer");
	add_timer_func_list(map_removemobs_timer, "map_removemobs_timer");
	add_timer_interval(gettick()+1000, map_freeblock_timer, 0, 0, 60*1000);

	if(use_irc)
		do_init_irc();
	do_init_atcommand();
	do_init_battle();
	do_init_chrif();
	do_init_clif();
	do_init_script();
	do_init_itemdb();
	do_init_mob();	// npcの初期化・でmob_spawnして、mob_dbを?照するのでinit_npcより先
	do_init_pc();
	do_init_status();
	do_init_party();
	do_init_guild();
	do_init_storage();
	do_init_skill();
	do_init_pet();
	do_init_merc();	//[orn]
	do_init_npc();
	do_init_unit();
#ifndef TXT_ONLY /* mail system [Valaris] */
	if(mail_server_enable)
		do_init_mail();

	if (log_config.sql_logs)
	{
		log_sql_init();
	}
	
	if (connection_ping_interval) {
		add_timer_func_list(map_sql_ping, "map_sql_ping");
		add_timer_interval(gettick()+connection_ping_interval*60*60*1000,
				map_sql_ping, 0, 0, connection_ping_interval*60*60*1000);
	}
#endif /* not TXT_ONLY */

	npc_event_do_oninit();	// npcのOnInitイベント?行

	if ( console ) {
		set_defaultconsoleparse(parse_console);
		start_console();
	}

	if (battle_config.pk_mode == 1)
		ShowNotice("Server is running on '"CL_WHITE"PK Mode"CL_RESET"'.\n");

	if(!(battle_config.cardillust_read_grffile || battle_config.item_equip_override_grffile || 
		battle_config.item_slots_override_grffile || battle_config.item_name_override_grffile ||
		battle_config.skill_sp_override_grffile))
		grfio_final(); // Unused after reading all maps.

	//However, some reload functions still use it,disable them.
	//battle_config.cardillust_read_grffile =
	//battle_config.item_equip_override_grffile =
	//battle_config.item_slots_override_grffile =
	//battle_config.item_name_override_grffile = 0;

	ShowStatus("Server is '"CL_GREEN"ready"CL_RESET"' and listening on port '"CL_WHITE"%d"CL_RESET"'.\n\n", map_port);

	return 0;
}

int compare_item(struct item *a, struct item *b) {

	if (a->nameid == b->nameid &&
		a->identify == b->identify &&
		a->refine == b->refine &&
		a->attribute == b->attribute)
	{
		int i;
		for (i = 0; i < MAX_SLOTS && (a->card[i] == b->card[i]); i++);
		return (i == MAX_SLOTS);
	}
	return 0;
}

#ifndef TXT_ONLY
int charsql_db_init(int method){

	if(method == 1){ //'INIT / START'
		ShowInfo("Connecting to 'character' Database... ");
		mysql_init(&charsql_handle);

		if(!mysql_real_connect(&charsql_handle, charsql_host, charsql_user, charsql_pass, charsql_db, charsql_port, (char *)NULL, 0)){
			ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
			exit(1);
		}else{
			printf("success.\n");
			if( strlen(default_codepage) > 0 ) {
				sprintf( tmp_sql, "SET NAMES %s", default_codepage );
				if (mysql_query(&charsql_handle, tmp_sql)) {
					ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				}
			}
		}
	}else if(method == 0){ //'FINAL' / Shutdown
		ShowInfo("Closing 'character' Database connection ... ");
		mysql_close(&charsql_handle);
		printf("done.\n");
	}
	return 0;
}
#endif
