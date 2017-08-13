// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INTER_SQL_H_
#define _INTER_SQL_H_

#include "../common/cbasetypes.h"
#include "../common/conf.h"
#include "../common/mmo.h"
#include "../common/sql.h"

#ifdef __cplusplus // C codes can't see this
#include <memory>
#include <unordered_map>

extern "C" {

struct Inter_Config {
	std::string cfgFile; ///< Inter-Config file
	std::unordered_map< uint8, std::shared_ptr<s_storage_table> > storages; ///< Storage name & table information
};
#endif

extern struct Inter_Config interserv_config;

int inter_init_sql(const char *file);
void inter_final(void);
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);
int mapif_disconnectplayer(int fd, uint32 account_id, uint32 char_id, int reason);
void mapif_accinfo_ack(bool success, int map_fd, int u_fd, int u_aid, int account_id, int8 type,
	int group_id, int logincount, int state, const char *email, const char *last_ip, const char *lastlogin,
	const char *birthdate, const char *user_pass, const char *pincode, const char *userid);

int inter_log(char *fmt,...);

#define inter_cfgName "conf/inter_athena.conf"

extern unsigned int party_share_level;

extern Sql* sql_handle;
extern Sql* lsql_handle;

void inter_savereg(uint32 account_id, uint32 char_id, const char *key, unsigned int index, intptr_t val, bool is_string);
int inter_accreg_fromsql(uint32 account_id, uint32 char_id, int fd, int type);

#ifdef __cplusplus
}
#endif

#endif /* _INTER_SQL_H_ */
