// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAR_SQL_H_
#define _CHAR_SQL_H_

#include "../common/core.h" // CORE_ST_LAST

enum E_CHARSERVER_ST
{
	CHARSERVER_ST_RUNNING = CORE_ST_LAST,
	CHARSERVER_ST_SHUTDOWN,
	CHARSERVER_ST_LAST
};

struct mmo_charstatus;

#define MAX_MAP_SERVERS 30

#define DEFAULT_AUTOSAVE_INTERVAL 300*1000

enum {
	TABLE_INVENTORY,
	TABLE_CART,
	TABLE_STORAGE,
	TABLE_GUILD_STORAGE,
};

int memitemdata_to_sql(const struct item items[], int max, int id, int tableswitch);

int mapif_sendall(unsigned char *buf,unsigned int len);
int mapif_sendallwos(int fd,unsigned char *buf,unsigned int len);
int mapif_send(int fd,unsigned char *buf,unsigned int len);

int char_married(int pl1,int pl2);
int char_child(int parent_id, int child_id);
int char_family(int pl1,int pl2,int pl3);

int request_accreg2(int account_id, int char_id);
int save_accreg2(unsigned char* buf, int len);

extern int char_name_option;
extern char char_name_letters[];
extern bool char_gm_read;
extern int autosave_interval;
extern int save_log;
extern char db_path[];
extern char char_db[256];
extern char scdata_db[256];
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
extern char mail_db[256];
extern char auction_db[256];
extern char quest_db[256];
extern char homunculus_db[256];
extern char skill_homunculus_db[256];
extern char mercenary_db[256];
extern char mercenary_owner_db[256];
extern char ragsrvinfo_db[256];

extern int db_use_sqldbs; // added for sql item_db read for char server [Valaris]

extern int guild_exp_rate;
extern int log_inter;

#endif /* _CHAR_SQL_H_ */
