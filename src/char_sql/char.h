#include "../common/core.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/mmo.h"
#include "../common/version.h"
#include "../common/db.h"

#ifndef _CHAR_H_
#define _CHAR_H_

#define MAX_MAP_SERVERS 30

#define LAN_CONF_NAME	"conf/lan_support.conf"

#define DEFAULT_AUTOSAVE_INTERVAL 300*1000

struct mmo_map_server{
  long ip;
  short port;
  int users;
  char map[MAX_MAP_PER_SERVER][16];
};
struct itemtmp {
	int flag;//checked = 1 else 0
	int id;
	short nameid;
	short amount;
	unsigned short equip;
	char identify;
	char refine;
	char attribute;
	short card[4];
};
enum {
	TABLE_INVENTORY,
	TABLE_CART,
	TABLE_STORAGE,
	TABLE_GUILD_STORAGE,
};
struct itemtemp{
	struct itemtmp equip[MAX_GUILD_STORAGE],notequip[MAX_GUILD_STORAGE];
};
int memitemdata_to_sql(struct itemtemp mapitem, int eqcount, int noteqcount, int char_id,int tableswitch);
int mapif_sendall(unsigned char *buf,unsigned int len);
int mapif_sendallwos(int fd,unsigned char *buf,unsigned int len);
int mapif_send(int fd,unsigned char *buf,unsigned int len);

extern int autosave_interval;
extern char db_path[];
extern char char_db[256];
extern char cart_db[256];
extern char inventory_db[256];
extern char charlog_db[256];
extern char storage_db[256];
extern char interlog_db[256];
extern char reg_db[256];
extern char skill_db[256];
extern char memo_db[256];
extern char guild_db[256];
extern char guild_alliance_db[256];
extern char guild_castle_db[256];
extern char guild_expulsion_db[256];
extern char guild_member_db[256];
extern char guild_position_db[256];
extern char guild_skill_db[256];
extern char guild_storage_db[256];
extern char party_db[256];
extern char pet_db[256];

int db_use_sqldbs; // added for sql item_db read for char server [Valaris]
extern char login_db_level[32];
extern char login_db_account_id[32];

extern int lowest_gm_level;

#endif

#include "inter.h"
#include "int_pet.h"
#include "int_guild.h"
#include "int_party.h"
#include "int_storage.h"
