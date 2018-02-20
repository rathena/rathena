// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INTER_HPP_
#define _INTER_HPP_

#include "../common/cbasetypes.h"
#include "../common/sql.h"

#include <memory>
#include <string>
#include <unordered_map>

struct s_storage_table;
struct Inter_Config {
	std::string cfgFile; ///< Inter-Config file
	std::unordered_map< uint8, std::shared_ptr<s_storage_table> > storages; ///< Storage name & table information
};

extern struct Inter_Config interserv_config;

int inter_init_sql(const char *file);
void inter_final(void);
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);
int mapif_disconnectplayer(int fd, uint32 account_id, uint32 char_id, int reason);
void mapif_accinfo_ack(bool success, int map_fd, int u_fd, int u_aid, int account_id, int8 type,
	int group_id, int logincount, int state, const char *email, const char *last_ip, const char *lastlogin,
	const char *birthdate, const char *userid);

int inter_log(const char *fmt,...);

#define inter_cfgName "conf/inter_athena.conf"

extern unsigned int party_share_level;

extern Sql* sql_handle;
extern Sql* lsql_handle;

void inter_savereg(uint32 account_id, uint32 char_id, const char *key, unsigned int index, intptr_t val, bool is_string);
int inter_accreg_fromsql(uint32 account_id, uint32 char_id, int fd, int type);

#endif /* _INTER_HPP_ */
