// $Id: map.c,v 1.6 2004/09/25 17:37:01 MouseJstr Exp $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <netdb.h>
#endif

#include "core.h"
#include "timer.h"
#include "db.h"
#include "grfio.h"
#include "malloc.h"
#include "version.h"

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
#include "battle.h"
#include "script.h"
#include "guild.h"
#include "pet.h"
#include "atcommand.h"
#include "charcommand.h"
#include "nullpo.h"
#include "socket.h"
#include "log.h"
#include "showmsg.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

unsigned long ticks = 0; // by MC Cameri

#ifndef TXT_ONLY

#include "mail.h" // mail system [Valaris]

MYSQL mmysql_handle;
MYSQL_RES* 	sql_res ;
MYSQL_ROW	sql_row ;
char tmp_sql[65535]="";

MYSQL lmysql_handle;
MYSQL_RES* lsql_res ;
MYSQL_ROW  lsql_row ;
char tmp_lsql[65535]="";

MYSQL mail_handle; // mail system [Valaris]
MYSQL_RES* 	mail_res ;
MYSQL_ROW	mail_row ;
char tmp_msql[65535]="";

int map_server_port = 3306;
char map_server_ip[16] = "127.0.0.1";
char map_server_id[32] = "ragnarok";
char map_server_pw[32] = "ragnarok";
char map_server_db[32] = "ragnarok";
int db_use_sqldbs = 0;

int login_server_port = 3306;
char login_server_ip[16] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "ragnarok";
char login_server_db[32] = "ragnarok";

char item_db_db[32] = "item_db";
char mob_db_db[32] = "mob_db";
char login_db[32] = "login";
char login_db_level[32] = "level";
char login_db_account_id[32] = "account_id";

char log_db[32] = "log";
char log_db_ip[16] = "127.0.0.1";
char log_db_id[32] = "ragnarok";
char log_db_pw[32] = "ragnarok";
int log_db_port = 3306;

char gm_db[32] = "login";
char gm_db_level[32] = "level";
char gm_db_account_id[32] = "account_id";

int lowest_gm_level = 1;
int read_gm_interval = 600000;

char char_db[32] = "char";

static int online_timer(int,unsigned int,int,int);

int CHECK_INTERVAL = 3600000; // [Valaris]
int check_online_timer=0; // [Valaris]

#endif /* not TXT_ONLY */

#define USE_AFM
#define USE_AF2

// ‹É—Í static‚Åƒ?ƒJƒ‹‚É?‚ß‚é
static struct dbt * id_db=NULL;
static struct dbt * map_db=NULL;
static struct dbt * nick_db=NULL;
static struct dbt * charid_db=NULL;

static int users=0;
static struct block_list *objects[MAX_FLOORITEM];
static int first_free_object_id=0,last_object_id=0;

#define block_free_max 1048576
static void *block_free[block_free_max];
static int block_free_count = 0, block_free_lock = 0;

#define BL_LIST_MAX 1048576
static struct block_list *bl_list[BL_LIST_MAX];
static int bl_list_count = 0;

static char afm_dir[1024] = ""; // [Valaris]

struct map_data map[MAX_MAP_PER_SERVER];
int map_num = 0;

int map_port=0;

int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
int agit_flag = 0;
int night_flag = 0; // 0=day, 1=night [Yor]

//Added for Mugendai's I'm Alive mod
int imalive_on=0;
int imalive_time=60;
//Added by Mugendai for GUI
int flush_on=1;
int flush_time=100;

struct charid2nick {
	char nick[24];
	int req_id;
};

// «Ş«Ã«×«­«ã«Ã«·«å××éÄ«Õ«é«°(map_athana.conf?ªÎread_map_from_cacheªÇò¦ïÒ)
// 0:××éÄª·ªÊª¤ 1:Şª?õêÜÁğí 2:?õêÜÁğí
int  map_read_flag = READ_FROM_GAT;
char map_cache_file[256]="db/map.info"; // «Ş«Ã«×«­«ã«Ã«·«å«Õ«¡«¤«ëÙ£

char motd_txt[256] = "conf/motd.txt";
char help_txt[256] = "conf/help.txt";

char wisp_server_name[24] = "Server"; // can be modified in char-server configuration file

int console = 0;
/*==========================================
 * ‘SmapI?Œv‚Å‚ÌÚ??İ’è
 * (charI‚©‚ç‘—‚ç‚ê‚Ä‚­‚é)
 *------------------------------------------
 */
void map_setusers(int n) {
	users = n;
}

/*==========================================
 * ‘SmapI?Œv‚Å‚ÌÚ??æ“¾ (/w‚Ö‚Ì?“š—p)
 *------------------------------------------
 */
int map_getusers(void) {
	return users;
}

//
// blockíœ‚ÌˆÀ‘S«Šm•Û?—
//

/*==========================================
 * block‚ğfree‚·‚é‚Æ‚«free‚Ì?‚í‚è‚ÉŒÄ‚Ô
 * ƒƒbƒN‚³‚ê‚Ä‚¢‚é‚Æ‚«‚Íƒoƒbƒtƒ@‚É‚½‚ß‚é
 *------------------------------------------
 */
int map_freeblock( void *bl )
{
	if(block_free_lock==0){
		aFree(bl);
		bl = NULL;
	}
	else{
		if( block_free_count>=block_free_max ) {
			if(battle_config.error_log)
				printf("map_freeblock: *WARNING* too many free block! %d %d\n",
			block_free_count,block_free_lock);
		}
		else
			block_free[block_free_count++]=bl;
	}
	return block_free_lock;
}
/*==========================================
 * block‚Ìfree‚ğˆê“I‚É‹Ö~‚·‚é
 *------------------------------------------
 */
int map_freeblock_lock(void) {
	return ++block_free_lock;
}

/*==========================================
 * block‚Ìfree‚ÌƒƒbƒN‚ğ‰ğœ‚·‚é
 * ‚±‚Ì‚Æ‚«AƒƒbƒN‚ªŠ®‘S‚É‚È‚­‚È‚é‚Æ
 * ƒoƒbƒtƒ@‚É‚½‚Ü‚Á‚Ä‚¢‚½block‚ğ‘S•”íœ
 *------------------------------------------
 */
int map_freeblock_unlock(void) {
	if ((--block_free_lock) == 0) {
		int i;
//		if(block_free_count>0) {
//			if(battle_config.error_log)
//				printf("map_freeblock_unlock: free %d object\n",block_free_count);
//		}
		for(i=0;i<block_free_count;i++){
			aFree(block_free[i]);
			block_free[i] = NULL;
		}
		block_free_count=0;
	}else if(block_free_lock<0){
		if(battle_config.error_log)
			printf("map_freeblock_unlock: lock count < 0 !\n");
	}
	return block_free_lock;
}


//
// block‰»?—
//
/*==========================================
 * map[]‚Ìblock_list‚©‚ç?‚ª‚Á‚Ä‚¢‚éê‡‚É
 * bl->prev‚Ébl_head‚ÌƒAƒhƒŒƒX‚ğ“ü‚ê‚Ä‚¨‚­
 *------------------------------------------
 */
static struct block_list bl_head;

/*==========================================
 * map[]‚Ìblock_list‚É’Ç‰Á
 * mob‚Í?‚ª‘½‚¢‚Ì‚Å•ÊƒŠƒXƒg
 *
 * ?‚Élink?‚İ‚©‚ÌŠm”F‚ª–³‚¢BŠë?‚©‚à
 *------------------------------------------
 */
int map_addblock(struct block_list *bl)
{
	int m,x,y;

	nullpo_retr(0, bl);

	if(bl->prev != NULL){
			if(battle_config.error_log)
				printf("map_addblock error : bl->prev!=NULL\n");
		return 0;
	}

	m=bl->m;
	x=bl->x;
	y=bl->y;
	if(m<0 || m>=map_num ||
	   x<0 || x>=map[m].xs ||
	   y<0 || y>=map[m].ys)
		return 1;

	if(bl->type==BL_MOB){
		bl->next = map[m].block_mob[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs];
		bl->prev = &bl_head;
		if(bl->next) bl->next->prev = bl;
		map[m].block_mob[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs] = bl;
		map[m].block_mob_count[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs]++;
	} else {
		bl->next = map[m].block[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs];
		bl->prev = &bl_head;
		if(bl->next) bl->next->prev = bl;
		map[m].block[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs] = bl;
		map[m].block_count[x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs]++;
		if(bl->type==BL_PC)
			map[m].users++;
	}

	return 0;
}

/*==========================================
 * map[]‚Ìblock_list‚©‚çŠO‚·
 * prev‚ªNULL‚Ìê‡list‚É?‚ª‚Á‚Ä‚È‚¢
 *------------------------------------------
 */
int map_delblock(struct block_list *bl)
{
	int b;
	nullpo_retr(0, bl);

	// ?‚Éblocklist‚©‚ç?‚¯‚Ä‚¢‚é
	if(bl->prev==NULL){
		if(bl->next!=NULL){
			// prev‚ªNULL‚Ånext‚ªNULL‚Å‚È‚¢‚Ì‚Í—L‚Á‚Ä‚Í‚È‚ç‚È‚¢
			if(battle_config.error_log)
				printf("map_delblock error : bl->next!=NULL\n");
		}
		return 0;
	}

	b = bl->x/BLOCK_SIZE+(bl->y/BLOCK_SIZE)*map[bl->m].bxs;

	if(bl->type==BL_PC)
		map[bl->m].users--;
	if(bl->next)
		bl->next->prev = bl->prev;
	if(bl->prev==&bl_head){
		// ƒŠƒXƒg‚Ì“ª‚È‚Ì‚ÅAmap[]‚Ìblock_list‚ğXV‚·‚é
		if(bl->type==BL_MOB){
			map[bl->m].block_mob[b] = bl->next;
			if((map[bl->m].block_mob_count[b]--) < 0)
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
 * ü?‚ÌPCl?‚ğ?‚¦‚é (Œ»İ–¢g—p)
 *------------------------------------------
 */
int map_countnearpc(int m, int x, int y) {
	int bx,by,c=0;
	struct block_list *bl=NULL;

	if(map[m].users==0)
		return 0;
	for(by=y/BLOCK_SIZE-AREA_SIZE/BLOCK_SIZE-1;by<=y/BLOCK_SIZE+AREA_SIZE/BLOCK_SIZE+1;by++){
		if(by<0 || by>=map[m].bys)
			continue;
		for(bx=x/BLOCK_SIZE-AREA_SIZE/BLOCK_SIZE-1;bx<=x/BLOCK_SIZE+AREA_SIZE/BLOCK_SIZE+1;bx++){
			if(bx<0 || bx>=map[m].bxs)
				continue;
			bl = map[m].block[bx+by*map[m].bxs];
			for(;bl;bl=bl->next){
				if(bl->type==BL_PC)
					c++;
			}
		}
	}
	return c;
}

/*==========================================
 * ƒZƒ‹ã‚ÌPC‚ÆMOB‚Ì?‚ğ?‚¦‚é (ƒOƒ‰ƒ“ƒhƒNƒƒX—p)
 *------------------------------------------
 */
int map_count_oncell(int m, int x, int y) {
	int bx,by;
	struct block_list *bl=NULL;
	int i,c;
	int count = 0;

	if (x < 0 || y < 0 || (x >= map[m].xs) || (y >= map[m].ys))
		return 1;
	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	bl = map[m].block[bx+by*map[m].bxs];
	c = map[m].block_count[bx+by*map[m].bxs];
	for(i=0;i<c && bl;i++,bl=bl->next){
		if(bl->x == x && bl->y == y && bl->type == BL_PC) count++;
	}
	bl = map[m].block_mob[bx+by*map[m].bxs];
	c = map[m].block_mob_count[bx+by*map[m].bxs];
	for(i=0;i<c && bl;i++,bl=bl->next){
		if(bl->x == x && bl->y == y) count++;
	}
	if(!count) count = 1;
	return count;
}
/*
 * «»«ëß¾ªÎõÌôøªËÌ¸ªÄª±ª¿«¹«­«ë«æ«Ë«Ã«ÈªòÚ÷ª¹
 */
struct skill_unit *map_find_skill_unit_oncell(int m,int x,int y,int skill_id)
{
	int bx,by;
	struct block_list *bl;
	int i,c;
	struct skill_unit *unit;

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
		if (unit->alive && unit->group->skill_id == skill_id)
			return unit;
	}
	return NULL;
}

/*==========================================
 * map m (x0,y0)-(x1,y1)?‚Ì‘Sobj‚É?‚µ‚Ä
 * func‚ğŒÄ‚Ô
 * type!=0 ‚È‚ç‚»‚Ìí—Ş‚Ì‚İ
 *------------------------------------------
 */
void map_foreachinarea(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int type,...) {
	va_list ap;
	int bx,by;
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;

	if(m < 0)
		return;
	va_start(ap,type);
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= map[m].xs) x1 = map[m].xs-1;
	if (y1 >= map[m].ys) y1 = map[m].ys-1;
	if (type == 0 || type != BL_MOB)
		for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	if(type==0 || type==BL_MOB)
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
			printf("map_foreachinarea: *WARNING* block count too many!\n");
	}

	map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			func(bl_list[i],ap);

	map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é

	va_end(ap);
	bl_list_count = blockcount;
}

/*==========================================
 * ‹éŒ`(x0,y0)-(x1,y1)‚ª(dx,dy)ˆÚ“®‚µ‚½‚Ì
 * —ÌˆæŠO‚É‚È‚é—Ìˆæ(‹éŒ`‚©LšŒ`)?‚Ìobj‚É
 * ?‚µ‚Äfunc‚ğŒÄ‚Ô
 *
 * dx,dy‚Í-1,0,1‚Ì‚İ‚Æ‚·‚éi‚Ç‚ñ‚È’l‚Å‚à‚¢‚¢‚Á‚Û‚¢Hj
 *------------------------------------------
 */
void map_foreachinmovearea(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int dx,int dy,int type,...) {
	int bx,by;
	struct block_list *bl=NULL;
	va_list ap;
	int blockcount=bl_list_count,i,c;

	va_start(ap,type);
	if(dx==0 || dy==0){
		// ‹éŒ`—Ìˆæ‚Ìê‡
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
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	}else{
		// Lš—Ìˆæ‚Ìê‡

		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=map[m].xs) x1=map[m].xs-1;
		if(y1>=map[m].ys) y1=map[m].ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if((bl) && !(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
						continue;
					if((bl) && ((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
						(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
						bl_list_count<BL_LIST_MAX)
							bl_list[bl_list_count++]=bl;
				}
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if((bl) && !(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
						continue;
					if((bl) && ((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
						(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
						bl_list_count<BL_LIST_MAX)
							bl_list[bl_list_count++]=bl;
				}
			}
		}

	}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			printf("map_foreachinarea: *WARNING* block count too many!\n");
	}

	map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev) {	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			if (bl_list[i]->type == BL_PC
			  && session[((struct map_session_data *) bl_list[i])->fd] == NULL)
				continue;
			func(bl_list[i],ap);
		}

	map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é

	va_end(ap);
	bl_list_count = blockcount;
}

// -- moonsoul	(added map_foreachincell which is a rework of map_foreachinarea but
//			 which only checks the exact single x/y passed to it rather than an
//			 area radius - may be more useful in some instances)
//
void map_foreachincell(int (*func)(struct block_list*,va_list),int m,int x,int y,int type,...) {
	int bx,by;
	struct block_list *bl=NULL;
	va_list ap;
	int blockcount=bl_list_count,i,c;

	va_start(ap,type);

	by=y/BLOCK_SIZE;
	bx=x/BLOCK_SIZE;

	if(type==0 || type!=BL_MOB)
	{
		bl = map[m].block[bx+by*map[m].bxs];
		c = map[m].block_count[bx+by*map[m].bxs];
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if(type && bl && bl->type!=type)
				continue;
			if(bl && bl->x==x && bl->y==y && bl_list_count<BL_LIST_MAX)
				bl_list[bl_list_count++]=bl;
		}
	}

	if(type==0 || type==BL_MOB)
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
			printf("map_foreachincell: *WARNING* block count too many!\n");
	}

	map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			func(bl_list[i],ap);

	map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é

	va_end(ap);
	bl_list_count = blockcount;
}

/*==========================================
 * °ƒAƒCƒeƒ€‚âƒGƒtƒFƒNƒg—p‚ÌˆêobjŠ„‚è?‚Ä
 * object[]‚Ö‚Ì•Û‘¶‚Æid_db“o?‚Ü‚Å
 *
 * bl->id‚à‚±‚Ì’†‚Åİ’è‚µ‚Ä–â‘è–³‚¢?
 *------------------------------------------
 */
int map_addobject(struct block_list *bl) {
	int i;
	if( bl == NULL ){
		printf("map_addobject nullpo?\n");
		return 0;
	}
	if(first_free_object_id<2 || first_free_object_id>=MAX_FLOORITEM)
		first_free_object_id=2;
	for(i=first_free_object_id;i<MAX_FLOORITEM;i++)
		if(objects[i]==NULL)
			break;
	if(i>=MAX_FLOORITEM){
		if(battle_config.error_log)
			printf("no free object id\n");
		return 0;
	}
	first_free_object_id=i;
	if(last_object_id<i)
		last_object_id=i;
	objects[i]=bl;
	numdb_insert(id_db,i,bl);
	return i;
}

/*==========================================
 * ˆêobject‚Ì‰ğ•ú
 *	map_delobject‚Ìfree‚µ‚È‚¢ƒo?ƒWƒ‡ƒ“
 *------------------------------------------
 */
int map_delobjectnofree(int id) {
	if(objects[id]==NULL)
		return 0;

	map_delblock(objects[id]);
	numdb_erase(id_db,id);
//	map_freeblock(objects[id]);
	objects[id]=NULL;

	if(first_free_object_id>id)
		first_free_object_id=id;

	while(last_object_id>2 && objects[last_object_id]==NULL)
		last_object_id--;

	return 0;
}

/*==========================================
 * ˆêobject‚Ì‰ğ•ú
 * block_list‚©‚ç‚ÌíœAid_db‚©‚ç‚Ìíœ
 * object data‚ÌfreeAobject[]‚Ö‚ÌNULL‘ã“ü
 *
 * add‚Æ‚Ì??«‚ª–³‚¢‚Ì‚ª?‚É‚È‚é
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
 * ‘Sˆêobj‘Šè‚Éfunc‚ğŒÄ‚Ô
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
			if(type && objects[i]->type!=type)
				continue;
			if(bl_list_count>=BL_LIST_MAX) {
				if(battle_config.error_log)
					printf("map_foreachobject: too many block !\n");
			}
			else
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
 * °ƒAƒCƒeƒ€‚ğÁ‚·
 *
 * data==0‚Ì‚Ítimer‚ÅÁ‚¦‚½
 * data!=0‚Ì‚ÍE‚¤“™‚ÅÁ‚¦‚½‚Æ‚µ‚Ä“®ì
 *
 * ŒãÒ‚ÍAmap_clearflooritem(id)‚Ö
 * map.h?‚Å#define‚µ‚Ä‚ ‚é
 *------------------------------------------
 */
int map_clearflooritem_timer(int tid,unsigned int tick,int id,int data) {
	struct flooritem_data *fitem=NULL;

	fitem = (struct flooritem_data *)objects[id];
	if(fitem==NULL || fitem->bl.type!=BL_ITEM || (!data && fitem->cleartimer != tid)){
		if(battle_config.error_log)
			printf("map_clearflooritem_timer : error\n");
		return 1;
	}
	if(data)
		delete_timer(fitem->cleartimer,map_clearflooritem_timer);
	else if(fitem->item_data.card[0] == (short)0xff00)
		intif_delete_petdata(*((long *)(&fitem->item_data.card[1])));
	clif_clearflooritem(fitem,0);
	map_delobject(fitem->bl.id);

	return 0;
}

/*==========================================
 * (m,x,y)‚Ìü?rangeƒ}ƒX?‚Ì‹ó‚«(=N“ü‰Â”\)cell‚Ì
 * ?‚©‚ç“K?‚Èƒ}ƒX–Ú‚ÌÀ•W‚ğx+(y<<16)‚Å•Ô‚·
 *
 * Œ»?range=1‚ÅƒAƒCƒeƒ€ƒhƒƒbƒv—p“r‚Ì‚İ
 *------------------------------------------
 */
int map_searchrandfreecell(int m,int x,int y,int range) {
	int free_cell,i,j;

	for(free_cell=0,i=-range;i<=range;i++){
		if(i+y<0 || i+y>=map[m].ys)
			continue;
		for(j=-range;j<=range;j++){
			if(j+x<0 || j+x>=map[m].xs)
				continue;
			if(map_getcell(m,j+x,i+y,CELL_CHKNOPASS))
				continue;
			free_cell++;
		}
	}
	if(free_cell==0)
		return -1;
	free_cell=rand()%free_cell;
	for(i=-range;i<=range;i++){
		if(i+y<0 || i+y>=map[m].ys)
			continue;
		for(j=-range;j<=range;j++){
			if(j+x<0 || j+x>=map[m].xs)
				continue;
			if(map_getcell(m,j+x,i+y,CELL_CHKNOPASS))
				continue;
			if(free_cell==0){
				x+=j;
				y+=i;
				i=range+1;
				break;
			}
			free_cell--;
		}
	}

	return x+(y<<16);
}

/*==========================================
 * (m,x,y)‚ğ’†S‚É3x3ˆÈ?‚É°ƒAƒCƒeƒ€İ’u
 *
 * item_data‚ÍamountˆÈŠO‚ğcopy‚·‚é
 *------------------------------------------
 */
int map_addflooritem(struct item *item_data,int amount,int m,int x,int y,struct map_session_data *first_sd,
    struct map_session_data *second_sd,struct map_session_data *third_sd,int type) {
	int xy,r;
	unsigned int tick;
	struct flooritem_data *fitem=NULL;

	nullpo_retr(0, item_data);

	if((xy=map_searchrandfreecell(m,x,y,1))<0)
		return 0;
	r=rand();

	fitem = (struct flooritem_data *)aCalloc(1,sizeof(*fitem));
	fitem->bl.type=BL_ITEM;
	fitem->bl.prev = fitem->bl.next = NULL;
	fitem->bl.m=m;
	fitem->bl.x=xy&0xffff;
	fitem->bl.y=(xy>>16)&0xffff;
	fitem->first_get_id = 0;
	fitem->first_get_tick = 0;
	fitem->second_get_id = 0;
	fitem->second_get_tick = 0;
	fitem->third_get_id = 0;
	fitem->third_get_tick = 0;

	fitem->bl.id = map_addobject(&fitem->bl);
	if(fitem->bl.id==0){
		aFree(fitem);
		return 0;
	}

	tick = gettick();
	if(first_sd) {
		fitem->first_get_id = first_sd->bl.id;
		if(type)
			fitem->first_get_tick = tick + battle_config.mvp_item_first_get_time;
		else
			fitem->first_get_tick = tick + battle_config.item_first_get_time;
	}
	if(second_sd) {
		fitem->second_get_id = second_sd->bl.id;
		if(type)
			fitem->second_get_tick = tick + battle_config.mvp_item_first_get_time + battle_config.mvp_item_second_get_time;
		else
			fitem->second_get_tick = tick + battle_config.item_first_get_time + battle_config.item_second_get_time;
	}
	if(third_sd) {
		fitem->third_get_id = third_sd->bl.id;
		if(type)
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

/*==========================================
 * charid_db‚Ö’Ç‰Á(•ÔM‘Ò‚¿‚ª‚ ‚ê‚Î•ÔM)
 *------------------------------------------
 */
void map_addchariddb(int charid, char *name) {
	struct charid2nick *p=NULL;
	int req=0;

	p = (struct charid2nick*)numdb_search(charid_db,charid);
	if(p==NULL){	// ƒf?ƒ^ƒx?ƒX‚É‚È‚¢
		p = (struct charid2nick *)aCallocA(1,sizeof(struct charid2nick));
		p->req_id=0;
	}else
		numdb_erase(charid_db,charid);

	req=p->req_id;
	memcpy(p->nick,name,24);
	p->req_id=0;
	numdb_insert(charid_db,charid,p);
	if(req){	// •ÔM‘Ò‚¿‚ª‚ ‚ê‚Î•ÔM
		struct map_session_data *sd = map_id2sd(req);
		if(sd!=NULL)
			clif_solved_charname(sd,charid);
	}
}

/*==========================================
 * charid_db‚Ö’Ç‰Ái•ÔM—v‹‚Ì‚İj
 *------------------------------------------
 */
int map_reqchariddb(struct map_session_data * sd,int charid) {
	struct charid2nick *p=NULL;

	nullpo_retr(0, sd);

	p = (struct charid2nick*)numdb_search(charid_db,charid);
	if(p!=NULL)	// ƒf?ƒ^ƒx?ƒX‚É‚·‚Å‚É‚ ‚é
		return 0;
	p = (struct charid2nick *)aCalloc(1,sizeof(struct charid2nick));
	p->req_id=sd->bl.id;
	numdb_insert(charid_db,charid,p);
	return 0;
}

/*==========================================
 * id_db‚Öbl‚ğ’Ç‰Á
 *------------------------------------------
 */
void map_addiddb(struct block_list *bl) {
	nullpo_retv(bl);

	numdb_insert(id_db,bl->id,bl);
}

/*==========================================
 * id_db‚©‚çbl‚ğíœ
 *------------------------------------------
 */
void map_deliddb(struct block_list *bl) {
	nullpo_retv(bl);

	numdb_erase(id_db,bl->id);
}

/*==========================================
 * nick_db‚Ösd‚ğ’Ç‰Á
 *------------------------------------------
 */
void map_addnickdb(struct map_session_data *sd) {
	nullpo_retv(sd);

	strdb_insert(nick_db,sd->status.name,sd);
}

/*==========================================
 * PC‚Ìquit?— map.c?•ª
 *
 * quit?—‚Ìå?‚ªˆá‚¤‚æ‚¤‚È?‚à‚µ‚Ä‚«‚½
 *------------------------------------------
 */
int map_quit(struct map_session_data *sd) {
	nullpo_retr(0, sd);

	if (sd->state.event_disconnect) {
		struct npc_data *npc;
		if ((npc = npc_name2id("PCLogoutEvent"))) {
			run_script(npc->u.scr.script,0,sd->bl.id,npc->bl.id); // PCLogoutNPC
			ShowStatus("Event '"CL_WHITE"PCLogoutEvent"CL_RESET"' executed.\n");
		}
	}

	if(sd->chatID)	// ƒ`ƒƒƒbƒg‚©‚ço‚é
		chat_leavechat(sd);

	if(sd->trade_partner)	// æˆø‚ğ’†?‚·‚é
		trade_tradecancel(sd);

	if(sd->party_invite>0)	// ƒp?ƒeƒB?—U‚ğ‹‘”Û‚·‚é
		party_reply_invite(sd,sd->party_invite_account,0);

	if(sd->guild_invite>0)	// ƒMƒ‹ƒh?—U‚ğ‹‘”Û‚·‚é
		guild_reply_invite(sd,sd->guild_invite,0);
	if(sd->guild_alliance>0)	// ƒMƒ‹ƒh“¯–¿?—U‚ğ‹‘”Û‚·‚é
		guild_reply_reqalliance(sd,sd->guild_alliance_account,0);

	party_send_logout(sd);	// ƒp?ƒeƒB‚ÌƒƒOƒAƒEƒgƒƒbƒZ?ƒW‘—M

	guild_send_memberinfoshort(sd,0);	// ƒMƒ‹ƒh‚ÌƒƒOƒAƒEƒgƒƒbƒZ?ƒW‘—M

	pc_cleareventtimer(sd);	// ƒCƒxƒ“ƒgƒ^ƒCƒ}‚ğ”jŠü‚·‚é

	if(sd->state.storage_flag)
		storage_guild_storage_quit(sd,0);
	else
		storage_storage_quit(sd);	// ‘qŒÉ‚ğŠJ‚¢‚Ä‚é‚È‚ç•Û‘¶‚·‚é

	// check if we've been authenticated [celest]
	if (sd->state.auth)
		skill_castcancel(&sd->bl,0);	// ‰r¥‚ğ’†?‚·‚é

	skill_stop_dancing(&sd->bl,1);// ƒ_ƒ“ƒX/‰‰‘t’†?

	if(sd->sc_data && sd->sc_data[SC_BERSERK].timer!=-1) //ƒo?ƒT?ƒN’†‚ÌI—¹‚ÍHP‚ğ100‚É
		sd->status.hp = 100;

	status_change_clear(&sd->bl,1);	// ƒXƒe?ƒ^ƒXˆÙí‚ğ‰ğœ‚·‚é
	skill_clear_unitgroup(&sd->bl);	// ƒXƒLƒ‹ƒ†ƒjƒbƒgƒOƒ‹?ƒv‚Ìíœ
	skill_cleartimerskill(&sd->bl);

	// check if we've been authenticated [celest]
	if (sd->state.auth) {
		pc_stop_walking(sd,0);
		pc_stopattack(sd);
		pc_delinvincibletimer(sd);
	}
	pc_delspiritball(sd,sd->spiritball,1);
	skill_gangsterparadise(sd,0);

	if (sd->state.auth)
		status_calc_pc(sd,4);
//	skill_clear_unitgroup(&sd->bl);	// [Sara-chan]

	clif_clearchar_area(&sd->bl,2);

	if(sd->status.pet_id && sd->pd) {
		pet_lootitem_drop(sd->pd,sd);
		pet_remove_map(sd);
		if(sd->pet.intimate <= 0) {
			intif_delete_petdata(sd->status.pet_id);
			sd->status.pet_id = 0;
			sd->pd = NULL;
			sd->petDB = NULL;
		}
		else
			intif_save_petdata(sd->status.account_id,&sd->pet);
	}

	if(pc_isdead(sd))
		pc_setrestartvalue(sd,2);

	pc_makesavestatus(sd);
	chrif_save(sd);
	storage_storage_dirty(sd);
	storage_storage_save(sd);

	if( sd->npc_stackbuf && sd->npc_stackbuf != NULL) {
		aFree( sd->npc_stackbuf );
		sd->npc_stackbuf = NULL;
	}

	map_delblock(&sd->bl);

#ifndef TXT_ONLY
	chrif_char_offline(sd);
#endif

	{
		void *p = numdb_search(charid_db,sd->status.char_id);
		if(p) {
			numdb_erase(charid_db,sd->status.char_id);
			free(p);
		}
	}
	strdb_erase(nick_db,sd->status.name);
	numdb_erase(charid_db,sd->status.char_id);
	numdb_erase(id_db,sd->bl.id);
	free(sd->reg);
	free(sd->regstr);

	return 0;
}

/*==========================================
 * id”Ô?‚ÌPC‚ğ’T‚·B‹‚È‚¯‚ê‚ÎNULL
 *------------------------------------------
 */
struct map_session_data * map_id2sd(int id) {
// remove search from db, because:
// 1 - all players, npc, items and mob are in this db (to search, it's not speed, and search in session is more sure)
// 2 - DB seems not always correct. Sometimes, when a player disconnects, its id (account value) is not removed and structure
//     point to a memory area that is not more a session_data and value are incorrect (or out of available memory) -> crash
// replaced by searching in all session.
// by searching in session, we are sure that fd, session, and account exist.
/*
	struct block_list *bl;

	bl=numdb_search(id_db,id);
	if(bl && bl->type==BL_PC)
		return (struct map_session_data*)bl;
	return NULL;
*/
	int i;
	struct map_session_data *sd=NULL;

	for(i = 0; i < fd_max; i++)
		if (session[i] && (sd = (struct map_session_data*)session[i]->session_data) && sd->bl.id == id)
			return sd;

	return NULL;
}

/*==========================================
 * char_id”Ô?‚Ì–¼‘O‚ğ’T‚·
 *------------------------------------------
 */
char * map_charid2nick(int id) {
	struct charid2nick *p = (struct charid2nick*)numdb_search(charid_db,id);

	if(p==NULL)
		return NULL;
	if(p->req_id!=0)
		return NULL;
	return p->nick;
}


/*==========================================
 * Search session data from a nick name
 * (without sensitive case if necessary)
 * return map_session_data pointer or NULL
 *------------------------------------------
 */
struct map_session_data * map_nick2sd(char *nick) {
	int i, quantity=0, nicklen;
	struct map_session_data *sd = NULL;
	struct map_session_data *pl_sd = NULL;

	if (nick == NULL)
		return NULL;

    nicklen = strlen(nick);

	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data*)session[i]->session_data) && pl_sd->state.auth)
			// Without case sensitive check (increase the number of similar character names found)
			if (strnicmp(pl_sd->status.name, nick, nicklen) == 0) {
				// Strict comparison (if found, we finish the function immediatly with correct value)
				if (strcmp(pl_sd->status.name, nick) == 0)
					return pl_sd;
				quantity++;
				sd = pl_sd;
			}
	}
	// Here, the exact character name is not found
	// We return the found index of a similar account ONLY if there is 1 similar character
	if (quantity == 1)
		return sd;

	// Exact character name is not found and 0 or more than 1 similar characters have been found ==> we say not found
	return NULL;
}

/*==========================================
 * id”Ô?‚Ì•¨‚ğ’T‚·
 * ˆêobject‚Ìê‡‚Í”z—ñ‚ğˆø‚­‚Ì‚İ
 *------------------------------------------
 */
struct block_list * map_id2bl(int id)
{
	struct block_list *bl=NULL;
	if(id<sizeof(objects)/sizeof(objects[0]))
		bl = objects[id];
	else
		bl = (struct block_list*)numdb_search(id_db,id);

	return bl;
}

/*==========================================
 * id_db?‚Ì‘S‚Ä‚Éfunc‚ğ?s
 *------------------------------------------
 */
int map_foreachiddb(int (*func)(void*,void*,va_list),...) {
	va_list ap;

	va_start(ap,func);
	numdb_foreach(id_db,func,ap);
	va_end(ap);
	return 0;
}

/*==========================================
 * map.npc‚Ö’Ç‰Á (warp“™‚Ì—Ìˆæ‚¿‚Ì‚İ)
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
			printf("too many NPCs in one map %s\n",map[m].name);
		return -1;
	}
	if(i==map[m].npc_num){
		map[m].npc_num++;
	}

	nullpo_retr(0, nd);

	map[m].npc[i]=nd;
	nd->n = i;
	numdb_insert(id_db,nd->bl.id,nd);

	return i;
}

void map_removenpc(void) {
    int i,m,n=0;

    for(m=0;m<map_num;m++) {
        for(i=0;i<map[m].npc_num && i<MAX_NPC_PER_MAP;i++) {
            if(map[m].npc[i]!=NULL) {
                clif_clearchar_area(&map[m].npc[i]->bl,2);
                map_delblock(&map[m].npc[i]->bl);
                numdb_erase(id_db,map[m].npc[i]->bl.id);
                if(map[m].npc[i]->bl.subtype==SCRIPT) {
//                    aFree(map[m].npc[i]->u.scr.script);
//                    aFree(map[m].npc[i]->u.scr.label_list);
                }
                aFree(map[m].npc[i]);
                map[m].npc[i] = NULL;
                n++;
            }
        }
    }

	sprintf(tmp_output,"Successfully removed and freed from memory '"CL_WHITE"%d"CL_RESET"' NPCs.\n",n);
	ShowStatus(tmp_output);
}

/*==========================================
 * map–¼‚©‚çmap”Ô?‚Ö?Š·
 *------------------------------------------
 */
int map_mapname2mapid(char *name) {
	struct map_data *md=NULL;

	md = (struct map_data*)strdb_search(map_db,name);

	#ifdef USE_AFM
		// If we can't find the .gat map try .afm instead [celest]
		if(md==NULL && strstr(name,".gat")) {
			char afm_name[16] = "";
			strncpy(afm_name, name, strlen(name) - 4);
			strcat(afm_name, ".afm");
			md = (struct map_data*)strdb_search(map_db,afm_name);
		}
	#endif

	if(md==NULL || md->gat==NULL)
		return -1;
	return md->m;
}

/*==========================================
 * ‘¼Imap–¼‚©‚çip,port?Š·
 *------------------------------------------
 */
int map_mapname2ipport(char *name,int *ip,int *port) {
	struct map_data_other_server *mdos=NULL;

	mdos = (struct map_data_other_server*)strdb_search(map_db,name);
	if(mdos==NULL || mdos->gat)
		return -1;
	*ip=mdos->ip;
	*port=mdos->port;
	return 0;
}

/*==========================================
 *
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
 * ”Ş‰ä‚Ì•ûŒü‚ğŒvZ
 *------------------------------------------
 */
int map_calc_dir( struct block_list *src,int x,int y) {
	int dir=0;
	int dx,dy;

	nullpo_retr(0, src);

	dx=x-src->x;
	dy=y-src->y;
	if( dx==0 && dy==0 ){	// ”Ş‰ä‚ÌêŠˆê’v
		dir=0;	// ã
	}else if( dx>=0 && dy>=0 ){	// •ûŒü“I‚É‰Eã
		dir=7;						// ‰Eã
		if( dx*3-1<dy ) dir=0;		// ã
		if( dx>dy*3 ) dir=6;		// ‰E
	}else if( dx>=0 && dy<=0 ){	// •ûŒü“I‚É‰E‰º
		dir=5;						// ‰E‰º
		if( dx*3-1<-dy ) dir=4;		// ‰º
		if( dx>-dy*3 ) dir=6;		// ‰E
	}else if( dx<=0 && dy<=0 ){ // •ûŒü“I‚É¶‰º
		dir=3;						// ¶‰º
		if( dx*3+1>dy ) dir=4;		// ‰º
		if( dx<dy*3 ) dir=2;		// ¶
	}else{						// •ûŒü“I‚É¶ã
		dir=1;						// ¶ã
		if( -dx*3-1<dy ) dir=0;		// ã
		if( -dx>dy*3 ) dir=2;		// ¶
	}
	return dir;
}

// gatŒn
/*==========================================
 * (m,x,y)‚Ìó‘Ô‚ğ’²‚×‚é
 *------------------------------------------
 */

int map_getcell(int m,int x,int y,cell_t cellchk)
{
	return (m < 0 || m > MAX_MAP_PER_SERVER) ? 0 : map_getcellp(&map[m],x,y,cellchk);
}

int map_getcellp(struct map_data* m,int x,int y,cell_t cellchk)
{
	int j;
	nullpo_ret(m);

	if(x<0 || x>=m->xs-1 || y<0 || y>=m->ys-1)
	{
		if(cellchk==CELL_CHKNOPASS) return 1;
		return 0;
	}
	j=x+y*m->xs;

	switch(cellchk)
	{
		case CELL_CHKPASS:
			return (m->gat[j] != 1 && m->gat[j] != 5);
		case CELL_CHKNOPASS:
			return (m->gat[j] == 1 || m->gat[j] == 5);
		case CELL_CHKWALL:
			return (m->gat[j] == 1);
		case CELL_CHKNPC:
			return (m->gat[j]&0x80);
		case CELL_CHKWATER:
			return (m->gat[j] == 3);
		case CELL_CHKGROUND:
			return (m->gat[j] == 5);
		case CELL_GETTYPE:
			return m->gat[j];
		default:
			return 0;
	}
}

/*==========================================
 * (m,x,y)‚Ìó‘Ô‚ğİ’è‚·‚é
 *------------------------------------------
 */
void map_setcell(int m,int x,int y,int cell)
{
	int j;
	if(x<0 || x>=map[m].xs || y<0 || y>=map[m].ys)
		return;
	j=x+y*map[m].xs;

	if (cell == CELL_SETNPC)
		map[m].gat[j] |= 0x80;
	else
		map[m].gat[j] = cell;
}

/*==========================================
 * ‘¼IŠÇ—‚Ìƒ}ƒbƒv‚ğdb‚É’Ç‰Á
 *------------------------------------------
 */
int map_setipport(char *name,unsigned long ip,int port) {
	struct map_data *md=NULL;
	struct map_data_other_server *mdos=NULL;

	md = (struct map_data*)strdb_search(map_db,name);
	if(md==NULL){ // not exist -> add new data
		mdos=(struct map_data_other_server *)aCalloc(1,sizeof(struct map_data_other_server));
		memcpy(mdos->name,name,24);
		mdos->gat  = NULL;
		mdos->ip   = ip;
		mdos->port = port;
		strdb_insert(map_db,mdos->name,mdos);
	} else {
		if(md->gat){ // local -> check data
			if(ip!=clif_getip() || port!=clif_getport()){
				printf("from char server : %s -> %08lx:%d\n",name,ip,port);
				return 1;
			}
		} else { // update
			mdos=(struct map_data_other_server *)md;
			mdos->ip   = ip;
			mdos->port = port;
		}
	}
	return 0;
}

// ‰Šú‰»ü‚è
/*==========================================
 * …ê‚‚³İ’è
 *------------------------------------------
 */
static struct waterlist_ {
	char mapname[24];
	int waterheight;
} *waterlist=NULL;

#define NO_WATER 1000000

static int map_waterheight(char *mapname) {
	if(waterlist){
		int i;
		for(i=0;waterlist[i].mapname[0] && i < MAX_MAP_PER_SERVER;i++)
			if(strcmp(waterlist[i].mapname,mapname)==0)
				return waterlist[i].waterheight;
	}
	return NO_WATER;
}

static void map_readwater(char *watertxt) {
	char line[1024],w1[1024];
	FILE *fp=NULL;
	int n=0;

	fp=fopen(watertxt,"r");
	if(fp==NULL){
		printf("file not found: %s\n",watertxt);
		return;
	}
	if(waterlist==NULL)
		waterlist = (struct waterlist_*)aCallocA(MAX_MAP_PER_SERVER,sizeof(*waterlist));
	while(fgets(line,1020,fp) && n < MAX_MAP_PER_SERVER){
		int wh,count;
		if(line[0] == '/' && line[1] == '/')
			continue;
		if((count=sscanf(line,"%s%d",w1,&wh)) < 1){
			continue;
		}
		strcpy(waterlist[n].mapname,w1);
		if(count >= 2)
			waterlist[n].waterheight = wh;
		else
			waterlist[n].waterheight = 3;
		n++;
	}
	fclose(fp);
}
/*==========================================
* ƒ}ƒbƒvƒLƒƒƒbƒVƒ…‚É’Ç‰Á‚·‚é
*===========================================*/

// ƒ}ƒbƒvƒLƒƒƒbƒVƒ…‚ÌÅ‘å’l
#define MAX_MAP_CACHE 768

//Šeƒ}ƒbƒv‚²‚Æ‚ÌÅ¬ŒÀî•ñ‚ğ“ü‚ê‚é‚à‚ÌAREAD_FROM_BITMAP—p
struct map_cache_info {
	char fn[32];//ƒtƒ@ƒCƒ‹–¼
	int xs,ys; //•‚Æ‚‚³
	int water_height;
	int pos;  // ƒf[ƒ^‚ª“ü‚ê‚Ä‚ ‚éêŠ
	int compressed;     // zilb’Ê‚¹‚é‚æ‚¤‚É‚·‚éˆ×‚Ì—\–ñ
	int compressed_len; // zilb’Ê‚¹‚é‚æ‚¤‚É‚·‚éˆ×‚Ì—\–ñ
}; // 56 byte

struct map_cache_head {
    int sizeof_header;
    int sizeof_map;
    // ã‚Ì‚Q‚Â‰ü•Ï•s‰Â
    int nmaps; // ƒ}ƒbƒv‚ÌŒÂ”
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
	atexit(map_cache_close);
	if(map_cache.fp) {
		map_cache_close();
	}
	map_cache.fp = fopen(fn,"r+b");
	if(map_cache.fp) {
		fread(&map_cache.head,1,sizeof(struct map_cache_head),map_cache.fp);
		fseek(map_cache.fp,0,SEEK_END);
		if(
			map_cache.head.sizeof_header == sizeof(struct map_cache_head) &&
			map_cache.head.sizeof_map    == sizeof(struct map_cache_info) &&
			map_cache.head.nmaps         == MAX_MAP_CACHE &&
			map_cache.head.filesize      == ftell(map_cache.fp)
		) {
			// ƒLƒƒƒbƒVƒ…“Ç‚İ‚İ¬Œ÷
			map_cache.map = (struct map_cache_info *) aMalloc(sizeof(struct map_cache_info) * map_cache.head.nmaps);
			fseek(map_cache.fp,sizeof(struct map_cache_head),SEEK_SET);
			fread(map_cache.map,sizeof(struct map_cache_info),map_cache.head.nmaps,map_cache.fp);
			return 1;
		}
		fclose(map_cache.fp);
	}
	// “Ç‚İ‚İ‚É¸”s‚µ‚½‚Ì‚ÅV‹K‚Éì¬‚·‚é
	map_cache.fp = fopen(fn,"wb");
	if(map_cache.fp) {
		memset(&map_cache.head,0,sizeof(struct map_cache_head));
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
	free(map_cache.map);
	map_cache.fp = NULL;
	return;
}

int map_cache_read(struct map_data *m)
{
	int i;
	if(!map_cache.fp) { return 0; }
	for(i = 0;i < map_cache.head.nmaps ; i++) {
		if(!strcmp(m->name,map_cache.map[i].fn)) {
			if(map_cache.map[i].water_height != map_waterheight(m->name)) {
				// …ê‚Ì‚‚³‚ªˆá‚¤‚Ì‚Å“Ç‚İ’¼‚µ
				return 0;
			} else if(map_cache.map[i].compressed == 0) {
				// ”ñˆ³kƒtƒ@ƒCƒ‹
				int size = map_cache.map[i].xs * map_cache.map[i].ys;
				m->xs = map_cache.map[i].xs;
				m->ys = map_cache.map[i].ys;
				m->gat = (unsigned char *)aCalloc(m->xs * m->ys,sizeof(unsigned char));
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				if(fread(m->gat,1,size,map_cache.fp) == size) {
					// ¬Œ÷
					return 1;
				} else {
					// ‚È‚º‚©ƒtƒ@ƒCƒ‹Œã”¼‚ªŒ‡‚¯‚Ä‚é‚Ì‚Å“Ç‚İ’¼‚µ
					m->xs = 0; m->ys = 0; m->gat = NULL; free(m->gat);
					return 0;
				}
			} else if(map_cache.map[i].compressed == 1) {
				// ˆ³kƒtƒ‰ƒO=1 : zlib
				unsigned char *buf;
				unsigned long dest_len;
				int size_compress = map_cache.map[i].compressed_len;
				m->xs = map_cache.map[i].xs;
				m->ys = map_cache.map[i].ys;
				m->gat = (unsigned char *)aMalloc(m->xs * m->ys * sizeof(unsigned char));
				buf = (unsigned char*)aMalloc(size_compress);
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				if(fread(buf,1,size_compress,map_cache.fp) != size_compress) {
					// ‚È‚º‚©ƒtƒ@ƒCƒ‹Œã”¼‚ªŒ‡‚¯‚Ä‚é‚Ì‚Å“Ç‚İ’¼‚µ
					printf("fread error\n");
					m->xs = 0; m->ys = 0; m->gat = NULL;
					free(m->gat); free(buf);
					return 0;
				}
				dest_len = m->xs * m->ys;
				decode_zip(m->gat,&dest_len,buf,size_compress);
				if(dest_len != map_cache.map[i].xs * map_cache.map[i].ys) {
					// ³í‚É‰ğ“€‚ªo—ˆ‚Ä‚È‚¢
					m->xs = 0; m->ys = 0; m->gat = NULL;
					free(m->gat); free(buf);
					return 0;
				}
				free(buf);
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
			// “¯‚¶ƒGƒ“ƒgƒŠ[‚ª‚ ‚ê‚Îã‘‚«
			if(map_cache.map[i].compressed == 0) {
				len_old = map_cache.map[i].xs * map_cache.map[i].ys;
			} else if(map_cache.map[i].compressed == 1) {
				len_old = map_cache.map[i].compressed_len;
			} else {
				// ƒTƒ|[ƒg‚³‚ê‚Ä‚È‚¢Œ`®‚È‚Ì‚Å’·‚³‚O
				len_old = 0;
			}
			if(map_read_flag == 2) {
				// ˆ³k•Û‘¶
				// ‚³‚·‚ª‚É‚Q”{‚É–c‚ê‚é–‚Í‚È‚¢‚Æ‚¢‚¤–‚Å
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
				// ƒTƒCƒY‚ª“¯‚¶‚©¬‚³‚­‚È‚Á‚½‚Ì‚ÅêŠ‚Í•Ï‚í‚ç‚È‚¢
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);
			} else {
				// V‚µ‚¢êŠ‚É“o˜^
				fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);
				map_cache.map[i].pos = map_cache.head.filesize;
				map_cache.head.filesize += len_new;
			}
			map_cache.map[i].xs  = m->xs;
			map_cache.map[i].ys  = m->ys;
			map_cache.map[i].water_height = map_waterheight(m->name);
			map_cache.dirty = 1;
			if(map_read_flag == 2) {
				free(write_buf);
			}
			return 0;
		}
	}
	// “¯‚¶ƒGƒ“ƒgƒŠ‚ª–³‚¯‚ê‚Î‘‚«‚ß‚éêŠ‚ğ’T‚·
	for(i = 0;i < map_cache.head.nmaps ; i++) {
		if(map_cache.map[i].fn[0] == 0) {
			// V‚µ‚¢êŠ‚É“o˜^
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
			map_cache.map[i].water_height = map_waterheight(m->name);
			map_cache.head.filesize += len_new;
			map_cache.dirty = 1;
			if(map_read_flag == 2) {
				free(write_buf);
			}
			return 0;
		}
	}
	// ‘‚«‚ß‚È‚©‚Á‚½
	return 1;
}

#ifdef USE_AFM
static int map_readafm(int m,char *fn) {

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


	int s;
	int x,y,xs,ys;
	size_t size;

	char afm_line[65535];
	int afm_size[1];
	FILE *afm_file;
	char *str;

	afm_file = fopen(fn, "r");
	if (afm_file != NULL) {

//		printf("\rLoading Maps [%d/%d]: %-50s  ",m,map_num,fn);
//		fflush(stdout);

		str=fgets(afm_line, sizeof(afm_line)-1, afm_file);
		str=fgets(afm_line, sizeof(afm_line)-1, afm_file);
		str=fgets(afm_line, sizeof(afm_line)-1, afm_file);
		sscanf(str , "%d%d", &afm_size[0], &afm_size[1]);

		map[m].m = m;
		xs = map[m].xs = afm_size[0];
		ys = map[m].ys = afm_size[1];
		// check this, unsigned where it might not need to be
		map[m].gat = (unsigned char*)aCallocA(s = map[m].xs * map[m].ys, 1);

		if(map[m].gat==NULL){
			printf("out of memory : map_readmap gat\n");
			exit(1);
		}

		map[m].npc_num=0;
		map[m].users=0;
		memset(&map[m].flag,0,sizeof(map[m].flag));

		if(battle_config.pk_mode) map[m].flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]

		for (y = 0; y < ys; y++) {
			str=fgets(afm_line, sizeof(afm_line)-1, afm_file);
			for (x = 0; x < xs; x++) {
				map[m].gat[x+y*xs] = str[x]-48;
			}
		}

		map[m].bxs=(xs+BLOCK_SIZE-1)/BLOCK_SIZE;
		map[m].bys=(ys+BLOCK_SIZE-1)/BLOCK_SIZE;
		size = map[m].bxs * map[m].bys * sizeof(struct block_list*);
		map[m].block = (struct block_list**)aCalloc(size, 1);

		if(map[m].block == NULL){
			printf("out of memory : map_readmap block\n");
			exit(1);
		}

		map[m].block_mob = (struct block_list**)aCalloc(size, 1);
		if (map[m].block_mob == NULL) {
			printf("out of memory : map_readmap block_mob\n");
			exit(1);
		}

		size = map[m].bxs*map[m].bys*sizeof(int);

		map[m].block_count = (int*)aCallocA(size, 1);
		if(map[m].block_count==NULL){
			printf("out of memory : map_readmap block\n");
			exit(1);
		}
		memset(map[m].block_count,0,size);

		map[m].block_mob_count = (int*)aCallocA(size, 1);
		if(map[m].block_mob_count==NULL){
			printf("out of memory : map_readmap block_mob\n");
			exit(1);
		}
		memset(map[m].block_mob_count,0,size);

		strdb_insert(map_db,map[m].name,&map[m]);

		fclose(afm_file);

	}

	return 0;
}
#endif

/*==========================================
 * ƒ}ƒbƒv1–‡“Ç‚İ‚İ
 * ===================================================*/
static int map_readmap(int m,char *fn, char *alias, int *map_cache, int maxmap) {
	char *gat="";
	size_t size;

	int i = 0;
	int e = 0;
	char progress[21] = "                    ";

	//printf("\rLoading Maps [%d/%d]: %-50s  ",m,map_num,fn);
	if (maxmap) { //avoid map-server crashing if there are 0 maps
		char c = '-';
		static int lasti = -1;
		static int last_time = -1;
		i=m*20/maxmap;
		if ((i != lasti) || (last_time != time(0))) {
			lasti = i;
			printf("\r");
			ShowStatus("Progress: ");
			printf("[");
			for (e=0;e<i;e++) progress[e] = '#';
			printf(progress);
			printf("] Working: [");
			last_time = time(0);
			switch(last_time % 4) {
				case 0: c='\\'; break;
				case 1: c='|'; break;
				case 2: c='/'; break;
				case 3: c='-'; break;
			}
			printf("%c]",c);
			fflush(stdout);
		}
	}

	if(map_cache_read(&map[m])) {
		// ƒLƒƒƒbƒVƒ…‚©‚ç“Ç‚İ‚ß‚½
		(*map_cache)++;
	} else {
		int s;
		int wh;
		int x,y,xs,ys;
		struct gat_1cell {float high[4]; int type;} *p=NULL;
		// read & convert fn
		// again, might not need to be unsigned char
		gat = (char*)grfio_read(fn);
		if(gat==NULL) {
			return -1;
			// ‚³‚·‚ª‚Éƒ}ƒbƒv‚ª“Ç‚ß‚È‚¢‚Ì‚Í‚Ü‚¸‚¢‚Ì‚ÅI—¹‚·‚é
			//printf("Can't load map %s\n",fn);
			//exit(1);
		}

		xs=map[m].xs=*(int*)(gat+6);
		ys=map[m].ys=*(int*)(gat+10);
		map[m].gat = (unsigned char *)aCallocA(s = map[m].xs * map[m].ys,sizeof(unsigned char));
		wh=map_waterheight(map[m].name);
		for(y=0;y<ys;y++){
			p=(struct gat_1cell*)(gat+y*xs*20+14);
			for(x=0;x<xs;x++){
				if(wh!=NO_WATER && p->type==0){
					// …ê”»’è
					map[m].gat[x+y*xs]=(p->high[0]>wh || p->high[1]>wh || p->high[2]>wh || p->high[3]>wh) ? 3 : 0;
				} else {
					map[m].gat[x+y*xs]=p->type;
				}
				p++;
			}
		}
		map_cache_write(&map[m]);
		aFree(gat);
	}

	map[m].m=m;
	map[m].npc_num=0;
	map[m].users=0;
	memset(&map[m].flag,0,sizeof(map[m].flag));
	if(battle_config.pk_mode)
		map[m].flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]
	map[m].bxs=(map[m].xs+BLOCK_SIZE-1)/BLOCK_SIZE;
	map[m].bys=(map[m].ys+BLOCK_SIZE-1)/BLOCK_SIZE;
	size = map[m].bxs * map[m].bys * sizeof(struct block_list*);
	map[m].block = (struct block_list **)aCalloc(1,size);
	map[m].block_mob = (struct block_list **)aCalloc(1,size);
	size = map[m].bxs*map[m].bys*sizeof(int);
	map[m].block_count = (int *)aCallocA(1,size);
	map[m].block_mob_count=(int *)aCallocA(1,size);
	if (alias)
           strdb_insert(map_db,alias,&map[m]);
        else
           strdb_insert(map_db,map[m].name,&map[m]);

//	printf("%s read done\n",fn);

	return 0;
}

/*==========================================
 * ‘S‚Ä‚Ìmapƒf?ƒ^‚ğ?‚İ?‚Ş
 *------------------------------------------
 */
int map_readallmap(void) {
	int i,maps_removed=0;
	char fn[256];
#ifdef USE_AFM
	FILE *afm_file;
#endif
	int map_cache = 0;

	// ƒ}ƒbƒvƒLƒƒƒbƒVƒ…‚ğŠJ‚­
	if(map_read_flag >= READ_FROM_BITMAP) {
		map_cache_open(map_cache_file);
	}

	sprintf(tmp_output, "Loading Maps%s...\n",
		(map_read_flag == CREATE_BITMAP_COMPRESSED ? " (Generating Map Cache w/ Compression)" :
		map_read_flag == CREATE_BITMAP ? " (Generating Map Cache)" :
		map_read_flag >= READ_FROM_BITMAP ? " (w/ Map Cache)" :
		map_read_flag == READ_FROM_AFM ? " (w/ AFM)" : ""));
	ShowStatus(tmp_output);

	// æ‚É‘S•”‚Ìƒƒbƒv‚Ì‘¶İ‚ğŠm”F
	for(i=0;i<map_num;i++){

#ifdef USE_AFM
		char afm_name[256] = "";
		char *p;
		if(!strstr(map[i].name,".afm")) {
		// check if it's necessary to replace the extension - speeds up loading abit
			strncpy(afm_name, map[i].name, strlen(map[i].name) - 4);
			strcat(afm_name, ".afm");
		}
		map[i].alias = NULL;
		sprintf(fn,"%s\\%s",afm_dir,afm_name);
		for(p=&fn[0];*p!=0;p++) if (*p=='\\') *p = '/';	// * At the time of Unix

		afm_file = fopen(fn, "r");
		if (afm_file != NULL) {
			map_readafm(i,fn);
			fclose(afm_file);
		}
		else if(strstr(map[i].name,".gat")!=NULL) {
#else
		if(strstr(map[i].name,".gat")!=NULL) {
#endif
			char *p = strstr(map[i].name, "<"); // [MouseJstr]
			if (p != NULL) {
				char buf[64];
				*p++ = '\0';
				sprintf(buf,"data\\%s", p);
				map[i].alias = aStrdup(buf);
			} else
				map[i].alias = NULL;

			sprintf(fn,"data\\%s",map[i].name);
			if(map_readmap(i,fn, p, &map_cache, map_num) == -1) {
				map_delmap(map[i].name);
				maps_removed++;
				i--;
			}
		}
	}

	aFree(waterlist);
	printf("\r");
	snprintf(tmp_output,sizeof(tmp_output),"Successfully loaded '"CL_WHITE"%d"CL_RESET"' maps.%30s\n",map_num,"");
	ShowInfo(tmp_output);

	map_cache_close();
	if(map_read_flag == CREATE_BITMAP || map_read_flag == CREATE_BITMAP_COMPRESSED) {
		--map_read_flag;
	}

	if (maps_removed) {
		snprintf(tmp_output,sizeof(tmp_output),"Maps Removed: '"CL_WHITE"%d"CL_RESET"'\n",maps_removed);
		ShowNotice(tmp_output);
	}
	return 0;
}

/*==========================================
 * ?‚İ?‚Şmap‚ğ’Ç‰Á‚·‚é
 *------------------------------------------
 */
int map_addmap(char *mapname) {
	if (strcmpi(mapname,"clear")==0) {
		map_num=0;
		return 0;
	}

	if (map_num >= MAX_MAP_PER_SERVER - 1) {
		snprintf(tmp_output,sizeof(tmp_output),"Could not add map '"
		CL_WHITE"%s"CL_RESET"', the limit of maps has been reached.\n",mapname);
		ShowError(tmp_output);
		return 1;
	}
	memcpy(map[map_num].name, mapname, 24);
	map_num++;
	return 0;
}

/*==========================================
 * ?‚İ?‚Şmap‚ğíœ‚·‚é
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
		    printf("Removing map [ %s ] from maplist\n",map[i].name);
			memmove(map+i, map+i+1, sizeof(map[0])*(map_num-i-1));
			map_num--;
		}
	}
	return 0;
}

static int map_ip_set_ = 0;
static int char_ip_set_ = 0;
//static int bind_ip_set_ = 0;

/*==========================================
 * Console Command Parser [Wizputer]
 *------------------------------------------
 */
int parse_console(char *buf) {
    char *type,*command,*map, *buf2;
    int x = 0, y = 0;
    int m, n;
    struct map_session_data *sd;

    sd = (struct map_session_data*)aCalloc(sizeof(*sd), 1);

    sd->fd = 0;
    strcpy( sd->status.name , "console");

    type = (char *)aMallocA(64);
    command = (char *)aMallocA(64);
    map = (char *)aMallocA(64);
    buf2 = (char *)aMallocA(72);

    memset(type,0,64);
    memset(command,0,64);
    memset(map,0,64);
    memset(buf2,0,72);

    if ( ( n = sscanf(buf, "%[^:]:%[^:]:%99s %d %d[^\n]", type , command , map , &x , &y )) < 5 )
        if ( ( n = sscanf(buf, "%[^:]:%[^\n]", type , command )) < 2 )
            n = sscanf(buf,"%[^\n]",type);

    if ( n == 5 ) {
        if (x <= 0) {
            x = rand() % 399 + 1;
            sd->bl.x = x;
        } else {
            sd->bl.x = x;
        }

        if (y <= 0) {
            y = rand() % 399 + 1;
            sd->bl.y = y;
        } else {
            sd->bl.y = y;
        }

        m = map_mapname2mapid(map);
        if ( m >= 0 )
            sd->bl.m = m;
        else {
            printf("Console: Unknown map\n");
            goto end;
        }
    }

    printf("Type of command: %s || Command: %s || Map: %s Coords: %d %d\n",type,command,map,x,y);

    if ( strcmpi("admin",type) == 0 && n == 5 ) {
        sprintf(buf2,"console: %s",command);
        if( is_atcommand(sd->fd,sd,buf2,99) == AtCommand_None )
            printf("Console: not atcommand\n");
    } else if ( strcmpi("server",type) == 0 && n == 2 ) {
        if ( strcmpi("shutdown", command) == 0 || strcmpi("exit",command) == 0 || strcmpi("quit",command) == 0 ) {
            exit(0);
        }
    } else if ( strcmpi("help",type) == 0 ) {
        printf("To use GM commands:\n");
        printf("admin:<gm command>:<map of \"gm\"> <x> <y>\n");
        printf("You can use any GM command that doesn't require the GM.\n");
        printf("No using @item or @warp however you can use @charwarp\n");
        printf("The <map of \"gm\"> <x> <y> is for commands that need coords of the GM\n");
        printf("IE: @spawn\n");
        printf("To shutdown the server:\n");
        printf("server:shutdown\n");
    }

    end:
    aFree(buf);
    aFree(type);
    aFree(command);
    aFree(map);
    aFree(buf2);
    aFree(sd);

    return 0;
}

/*==========================================
 * İ’èƒtƒ@ƒCƒ‹‚ğ?‚İ?‚Ş
 *------------------------------------------
 */
int map_config_read(char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	struct hostent *h = NULL;

	fp = fopen(cfgName,"r");
	if (fp == NULL) {
		printf("Map configuration file not found at: %s\n", cfgName);
		exit(1);
	}
	while(fgets(line, sizeof(line) -1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
			if (strcmpi(w1, "userid")==0){
				chrif_setuserid(w2);
			} else if (strcmpi(w1, "passwd") == 0) {
				chrif_setpasswd(w2);
			} else if (strcmpi(w1, "char_ip") == 0) {
				char_ip_set_ = 1;
				h = gethostbyname (w2);
				if(h != NULL) {
					snprintf(tmp_output,sizeof(tmp_output),"Char Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%d.%d.%d.%d"CL_RESET"'.\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
					ShowInfo(tmp_output);
					sprintf(w2,"%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				}
				chrif_setip(w2);
			} else if (strcmpi(w1, "char_port") == 0) {
				chrif_setport(atoi(w2));
			} else if (strcmpi(w1, "map_ip") == 0) {
				map_ip_set_ = 1;
				h = gethostbyname (w2);
				if (h != NULL) {
					snprintf(tmp_output,sizeof(tmp_output),"Map Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%d.%d.%d.%d"CL_RESET"'.\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
					ShowInfo(tmp_output);
					sprintf(w2, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				}
				clif_setip(w2);
			} else if (strcmpi(w1, "bind_ip") == 0) {
				//bind_ip_set_ = 1;
				h = gethostbyname (w2);
				if (h != NULL) {
					snprintf(tmp_output,sizeof(tmp_output),"Map Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%d.%d.%d.%d"CL_RESET"'.\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
					ShowInfo(tmp_output);
					sprintf(w2, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				}
				clif_setbindip(w2);
			} else if (strcmpi(w1, "map_port") == 0) {
				clif_setport(atoi(w2));
				map_port = (atoi(w2));
			} else if (strcmpi(w1, "water_height") == 0) {
				map_readwater(w2);
			} else if (strcmpi(w1, "map") == 0) {
				map_addmap(w2);
			} else if (strcmpi(w1, "delmap") == 0) {
				map_delmap(w2);
			} else if (strcmpi(w1, "npc") == 0) {
				npc_addsrcfile(w2);
			} else if (strcmpi(w1, "delnpc") == 0) {
				npc_delsrcfile(w2);
			} else if (strcmpi(w1, "data_grf") == 0) {
				grfio_setdatafile(w2);
			} else if (strcmpi(w1, "sdata_grf") == 0) {
				grfio_setsdatafile(w2);
			} else if (strcmpi(w1, "adata_grf") == 0) {
				grfio_setadatafile(w2);
			} else if (strcmpi(w1, "autosave_time") == 0) {
				autosave_interval = atoi(w2) * 1000;
				if (autosave_interval <= 0)
					autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
			} else if (strcmpi(w1, "motd_txt") == 0) {
				strcpy(motd_txt, w2);
			} else if (strcmpi(w1, "help_txt") == 0) {
				strcpy(help_txt, w2);
			} else if (strcmpi(w1, "mapreg_txt") == 0) {
				strcpy(mapreg_txt, w2);
			}else if(strcmpi(w1,"read_map_from_cache")==0){
				if (atoi(w2) == 2)
					map_read_flag = READ_FROM_BITMAP_COMPRESSED;
				else if (atoi(w2) == 1)
					map_read_flag = READ_FROM_BITMAP;
				else
					map_read_flag = READ_FROM_GAT;
			}else if(strcmpi(w1,"map_cache_file")==0){
				strncpy(map_cache_file,w2,255);
			} else if (strcmpi(w1, "import") == 0) {
				map_config_read(w2);
			} else if (strcmpi(w1, "console") == 0) {
			    if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 ) {
			        console = 1;
					ShowNotice("Console Commands is enabled.\n");
				}
            } else if(strcmpi(w1,"imalive_on")==0){		//Added by Mugendai for I'm Alive mod
				imalive_on = atoi(w2);					//Added by Mugendai for I'm Alive mod
			} else if(strcmpi(w1,"imalive_time")==0){	//Added by Mugendai for I'm Alive mod
				imalive_time = atoi(w2);				//Added by Mugendai for I'm Alive mod
			} else if(strcmpi(w1,"flush_on")==0){		//Added by Mugendai for GUI
				flush_on = atoi(w2);					//Added by Mugendai for GUI
			} else if(strcmpi(w1,"flush_time")==0){		//Added by Mugendai for GUI
				flush_time = atoi(w2);					//Added by Mugendai for GUI
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
		snprintf(tmp_output,sizeof(tmp_output),"File not found: '%s'.\n",cfgName);
		ShowError(tmp_output);
		return 1;
	}
	while(fgets(line,1020,fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if(strcmpi(w1,"stall_time")==0){
			stall_time_ = atoi(w2);
	#ifndef TXT_ONLY
		} else if(strcmpi(w1,"item_db_db")==0){
			strcpy(item_db_db,w2);
		} else if(strcmpi(w1,"mob_db_db")==0){
			strcpy(mob_db_db,w2);
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
		} else if(strcmpi(w1,"use_sql_db")==0){
			if (strcmpi(w2,"yes")){db_use_sqldbs=0;} else if (strcmpi(w2,"no")){db_use_sqldbs=1;}
			printf ("Using SQL dbs: %s\n",w2);
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
		} else if(strcmpi(w1,"lowest_gm_level")==0){
			lowest_gm_level = atoi(w2);
		} else if(strcmpi(w1,"read_gm_interval")==0){
			read_gm_interval = ( atoi(w2) * 60 * 1000 ); // Minutes multiplied by 60 secs per min by 1000 milliseconds per second
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
	printf("Connect Map DB Server....\n");
	if(!mysql_real_connect(&mmysql_handle, map_server_ip, map_server_id, map_server_pw,
		map_server_db ,map_server_port, (char *)NULL, 0)) {
			//pointer check
			printf("%s\n",mysql_error(&mmysql_handle));
			exit(1);
	}
	else {
		printf ("connect success! (Map Server Connection)\n");
	}

    mysql_init(&lmysql_handle);

    //DB connection start
    printf("Connect Login DB Server....\n");
    if(!mysql_real_connect(&lmysql_handle, login_server_ip, login_server_id, login_server_pw,
        login_server_db ,login_server_port, (char *)NULL, 0)) {
	        //pointer check
			printf("%s\n",mysql_error(&lmysql_handle));
			exit(1);
	}
	 else {
		printf ("connect success! (Login Server Connection)\n");
	 }

	if(battle_config.mail_system) { // mail system [Valaris]
		mysql_init(&mail_handle);
		if(!mysql_real_connect(&mail_handle, map_server_ip, map_server_id, map_server_pw,
			map_server_db ,map_server_port, (char *)NULL, 0)) {
				printf("%s\n",mysql_error(&mail_handle));
				exit(1);
		}
	}

	return 0;
}

int map_sql_close(void){
	mysql_close(&mmysql_handle);
	printf("Close Map DB Connection....\n");

	mysql_close(&lmysql_handle);
	printf("Close Login DB Connection....\n");
	return 0;
}

int log_sql_init(void){

    mysql_init(&mmysql_handle);

	//DB connection start
	printf(""CL_WHITE"[SQL]"CL_RESET": Connecting to Log Database "CL_WHITE"%s"CL_RESET" At "CL_WHITE"%s"CL_RESET"...\n",log_db,log_db_ip);
	if(!mysql_real_connect(&mmysql_handle, log_db_ip, log_db_id, log_db_pw,
		log_db ,log_db_port, (char *)NULL, 0)) {
			//pointer check
			printf(""CL_WHITE"[SQL Error]"CL_RESET": %s\n",mysql_error(&mmysql_handle));
			exit(1);
	} else {
		printf(""CL_WHITE"[SQL]"CL_RESET": Successfully '"CL_GREEN"connected"CL_RESET"' to Database '"CL_WHITE"%s"CL_RESET"'.\n", log_db);
	}

	return 0;
}

int online_timer(int tid,unsigned int tick,int id,int data)
{
	if(check_online_timer != tid)
		return 0;

	char_online_check();

	check_online_timer=add_timer(gettick()+CHECK_INTERVAL,online_timer,0,0);

	return 0;
}

void char_online_check(void)
{
	int i;
	struct map_session_data *sd=NULL;

	chrif_char_reset_offline();

	for(i=0;i<fd_max;i++){
		if (session[i] && (sd = (struct map_session_data*)session[i]->session_data) && sd && sd->state.auth &&
		 !(battle_config.hide_GM_session && pc_isGM(sd)))
			if(sd->status.char_id) {
				 chrif_char_online(sd);
			}
	}


	if(check_online_timer && check_online_timer != -1) {
		delete_timer(check_online_timer,online_timer);
		add_timer(gettick()+CHECK_INTERVAL,online_timer,0,0);
	}

}

#endif /* not TXT_ONLY */

//-----------------------------------------------------
//I'm Alive Alert
//Used to output 'I'm Alive' every few seconds
//Intended to let frontends know if the app froze
//-----------------------------------------------------
int imalive_timer(int tid, unsigned int tick, int id, int data){
	printf("I'm Alive\n");
	return 0;
}

//-----------------------------------------------------
//Flush stdout
//stdout buffer needs flushed to be seen in GUI
//-----------------------------------------------------
int flush_timer(int tid, unsigned int tick, int id, int data){
	fflush(stdout);
	return 0;
}

int id_db_final(void *k,void *d,va_list ap)
{
	struct mob_data *id;
	nullpo_retr(0, id = (struct mob_data*)d);
	if(id->lootitem)
		aFree(id->lootitem);
	if(id)
		aFree(id);
	return 0;
}

int map_db_final(void *k,void *d,va_list ap)
{
	struct map_data *id;
	nullpo_retr(0, id = (struct map_data*)d);
	if(id->gat)
		aFree(id->gat);
	if(id)
		aFree(id);
	return 0;
}
int nick_db_final(void *k,void *d,va_list ap){ return 0; }
int charid_db_final(void *k,void *d,va_list ap){ return 0; }
static int cleanup_sub(struct block_list *bl, va_list ap) {
	nullpo_retr(0, bl);

        switch(bl->type) {
        case BL_PC:
            map_quit((struct map_session_data *) bl);
            break;
        case BL_NPC:
            npc_delete((struct npc_data *)bl);
            break;
        case BL_MOB:
            mob_delete((struct mob_data *)bl);
            break;
        case BL_PET:
            pet_remove_map((struct map_session_data *)bl);
            break;
        case BL_ITEM:
            map_clearflooritem(bl->id);
            break;
        case BL_SKILL:
            skill_delunit((struct skill_unit *) bl);
            break;
        }

	return 0;
}

/*==========================================
 * mapII—¹?—
 *------------------------------------------
 */
void do_final(void) {
    int map_id, i;
	ShowStatus("Terminating...\n");

    for (map_id = 0; map_id < map_num;map_id++)
		if(map[map_id].m)
			map_foreachinarea(cleanup_sub, map_id, 0, 0, map[map_id].xs, map[map_id].ys, 0, 0);

#ifndef TXT_ONLY
    chrif_char_reset_offline();
#endif

    chrif_flush_fifo();

    for (i = 0; i < fd_max; i++)
        delete_session(i);

#if 0
    map_removenpc();

//    do_final_timer(); (we used timer_final() instead)
    timer_final();
//    numdb_final(id_db, id_db_final);
    strdb_final(map_db, map_db_final);
    strdb_final(nick_db, nick_db_final);
    numdb_final(charid_db, charid_db_final);


	do_final_script();
	do_final_itemdb();
	do_final_storage();
	do_final_guild();
	do_final_pet();
/*
	for(i=0;i<map_num;i++){
		if(map[i].gat) {
			aFree(map[i].gat);
			map[i].gat=NULL; //isn't it NULL already o_O?
		}
		if(map[i].block) aFree(map[i].block);
		if(map[i].block_mob) aFree(map[i].block_mob);
		if(map[i].block_count) aFree(map[i].block_count);
		if(map[i].block_mob_count) aFree(map[i].block_mob_count);
	}
*/
#endif

#ifndef TXT_ONLY
    map_sql_close();
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

/*======================================================
 * Map-Server Init and Command-line Arguments [Valaris]
 *------------------------------------------------------
 */
int do_init(int argc, char *argv[]) {
	int i;
	FILE *data_conf;
	char line[1024], w1[1024], w2[1024];

#ifdef GCOLLECT
	GC_enable_incremental();
#endif

	char *INTER_CONF_NAME="conf/inter_athena.conf";
	char *LOG_CONF_NAME="conf/log_athena.conf";
	char *MAP_CONF_NAME = "conf/map_athena.conf";
	char *BATTLE_CONF_FILENAME = "conf/battle_athena.conf";
	char *ATCOMMAND_CONF_FILENAME = "conf/atcommand_athena.conf";
	char *CHARCOMMAND_CONF_FILENAME = "conf/charcommand_athena.conf";
	char *SCRIPT_CONF_NAME = "conf/script_athena.conf";
	char *MSG_CONF_NAME = "conf/msg_athena.conf";
	char *GRF_PATH_FILENAME = "conf/grf-files.txt";

	chrif_connected = 0;

	srand(gettick());

	for (i = 1; i < argc ; i++) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--h") == 0 || strcmp(argv[i], "--?") == 0 || strcmp(argv[i], "/?") == 0)
			map_helpscreen(1);
		if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "--v") == 0 || strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "/v") == 0)
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
#endif /* not TXT_ONLY */
		else if (strcmp(argv[i],"--log_config") == 0 || strcmp(argv[i],"--log-config") == 0)
		    LOG_CONF_NAME = argv[i+1];
		else if (strcmp(argv[i],"--run_once") == 0)	// close the map-server as soon as its done.. for testing [Celest]
			runflag = 0;
	}

	map_config_read(MAP_CONF_NAME);

        if ((naddr_ == 0) && (map_ip_set_ == 0 || char_ip_set_ == 0)) {
            printf("\nUnable to determine your IP address... please edit\n");
            printf("the map_athena.conf file and set it.\n");
            printf("(127.0.0.1 is valid if you have no network interface)\n");
        }

        if (map_ip_set_ == 0 || char_ip_set_ == 0) {
          // The map server should know what IP address it is running on
          //   - MouseJstr
          int localaddr = ntohl(addr_[0]);
          unsigned char *ptr = (unsigned char *) &localaddr;
          char buf[16];
          sprintf(buf, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);;
          if (naddr_ != 1)
            printf("Multiple interfaces detected..  using %s as our IP address\n", buf);
          else
            printf("Defaulting to %s as our IP address\n", buf);
          if (map_ip_set_ == 0)
          	clif_setip(buf);
          if (char_ip_set_ == 0)
		chrif_setip(buf);

          if (ptr[0] == 192 && ptr[1] == 168)
            printf("\nFirewall detected.. \n    edit lan_support.conf and map_athena.conf\n\n");
        }
	if (SHOW_DEBUG_MSG) ShowNotice("Server running in '"CL_WHITE"Debug Mode"CL_RESET"'.\n");
	battle_config_read(BATTLE_CONF_FILENAME);
	msg_config_read(MSG_CONF_NAME);
	atcommand_config_read(ATCOMMAND_CONF_FILENAME);
	charcommand_config_read(CHARCOMMAND_CONF_FILENAME);
	script_config_read(SCRIPT_CONF_NAME);
	inter_config_read(INTER_CONF_NAME);
	log_config_read(LOG_CONF_NAME);

	atexit(do_final);

	id_db = numdb_init();
	map_db = strdb_init(16);
	nick_db = strdb_init(24);
	charid_db = numdb_init();
#ifndef TXT_ONLY
	map_sql_init();
#endif /* not TXT_ONLY */

	grfio_init(GRF_PATH_FILENAME);

	data_conf = fopen(GRF_PATH_FILENAME, "r");
	// It will read, if there is grf-files.txt.
	if (data_conf) {
		while(fgets(line, 1020, data_conf)) {
			if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
				if(strcmp(w1,"afm_dir") == 0)
					strcpy(afm_dir, w2);
			}
		}
		fclose(data_conf);
	} // end of reading grf-files.txt for AFMs


	map_readallmap();

	add_timer_func_list(map_clearflooritem_timer, "map_clearflooritem_timer");

	//Added by Mugendai for GUI support
	if (flush_on)
		add_timer_interval(gettick()+10, flush_timer,0,0,flush_time);

#ifndef TXT_ONLY // online status timer, checks every hour [Valaris]
	add_timer_func_list(online_timer, "online_timer");
	check_online_timer=add_timer(gettick()+CHECK_INTERVAL,online_timer,0,0);
#endif /* not TXT_ONLY */

	do_init_chrif();
	do_init_clif();
	do_init_itemdb();
	do_init_mob();	// npc‚Ì‰Šú‰»?‚Åmob_spawn‚µ‚ÄAmob_db‚ğ?Æ‚·‚é‚Ì‚Åinit_npc‚æ‚èæ
	do_init_script();
	do_init_pc();
	do_init_status();
	do_init_party();
	do_init_guild();
	do_init_storage();
	do_init_skill();
	do_init_pet();
	do_init_npc();

#ifndef TXT_ONLY /* mail system [Valaris] */
	if(battle_config.mail_system)
		do_init_mail();

	if (log_config.branch || log_config.drop || log_config.mvpdrop ||
		log_config.present || log_config.produce || log_config.refine ||
		log_config.trade)
	{
		log_sql_init();
	}
#endif /* not TXT_ONLY */

	npc_event_do_oninit();	// npc‚ÌOnInitƒCƒxƒ“ƒg?s

	if ( console ) {
	    set_defaultconsoleparse(parse_console);
	   	start_console();
	}

	if (battle_config.pk_mode == 1)
		ShowNotice("Server is running on '"CL_WHITE"PK Mode"CL_RESET"'.\n");

	//Added for Mugendais I'm Alive mod
	if (imalive_on)
		add_timer_interval(gettick()+10, imalive_timer,0,0,imalive_time*1000);

	sprintf(tmp_output,"Server is '"CL_GREEN"ready"CL_RESET"' and listening on port '"CL_WHITE"%d"CL_RESET"'.\n\n", map_port);
	ShowStatus(tmp_output);

	ticks = gettick();

	return 0;
}

int compare_item(struct item *a, struct item *b) {
  return (
          (a->nameid == b->nameid) &&
          (a->identify == b->identify) &&
          (a->refine == b->refine) &&
          (a->attribute == b->attribute) &&
          (a->card[0] == b->card[0]) &&
          (a->card[1] == b->card[1]) &&
          (a->card[2] == b->card[2]) &&
          (a->card[3] == b->card[3]));
}
