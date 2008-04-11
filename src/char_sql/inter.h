// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INTER_SQL_H_
#define _INTER_SQL_H_

struct accreg;
#include "../common/sql.h"

int inter_init_sql(const char *file);
void inter_final(void);
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);
int mapif_send_gmaccounts(void);
int mapif_disconnectplayer(int fd, int account_id, int char_id, int reason);

int inter_check_length(int fd,int length);

int inter_log(char *fmt,...);

#define inter_cfgName "conf/inter_athena.conf"

extern unsigned int party_share_level;
extern char inter_log_filename[1024];

extern Sql* sql_handle;
extern Sql* lsql_handle;

extern int char_server_port;
extern char char_server_ip[32];
extern char char_server_id[32];
extern char char_server_pw[32];
extern char char_server_db[32];

extern int login_db_server_port;
extern char login_db_server_ip[32];
extern char login_db_server_id[32];
extern char login_db_server_pw[32];
extern char login_db_server_db[32];

extern char main_chat_nick[16];

int inter_accreg_tosql(int account_id, int char_id, struct accreg *reg, int type);

#endif /* _INTER_SQL_H_ */
