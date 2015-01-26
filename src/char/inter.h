// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INTER_SQL_H_
#define _INTER_SQL_H_

#include "../common/sql.h"

int inter_init_sql(const char *file);
void inter_final(void);
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);
int mapif_send_gmaccounts(void);
int mapif_disconnectplayer(int fd, uint32 account_id, uint32 char_id, int reason);
void mapif_accinfo_ack(bool success, int map_fd, int u_fd, int u_aid, int account_id, int8 type,
	int group_id, int logincount, int state, const char *email, const char *last_ip, const char *lastlogin,
	const char *birthdate, const char *user_pass, const char *pincode, const char *userid);

int inter_log(char *fmt,...);

#define inter_cfgName "conf/inter_athena.conf"

struct Inter_Config {
	unsigned short char_server_port;  ///< char-server port
	StringBuf *char_server_ip;        ///< char-server hostname/ip
	StringBuf *char_server_id;        ///< char-server username
	StringBuf *char_server_pw;        ///< char-server password
	StringBuf *char_server_db;        ///< char-server database
	StringBuf *default_codepage;      ///< Codepage [irmin]

	unsigned short party_share_level; ///< Party share level
};
extern struct Inter_Config inter_config; /// Inter/char-server configuration with database

extern Sql* sql_handle;
extern Sql* lsql_handle;

void inter_savereg(uint32 account_id, uint32 char_id, const char *key, unsigned int index, intptr_t val, bool is_string);
int inter_accreg_fromsql(uint32 account_id, uint32 char_id, int fd, int type);

#endif /* _INTER_SQL_H_ */
