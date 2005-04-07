#ifndef _INTER_H_
#define _INTER_H_

int inter_init(const char *file);
void inter_final();
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);
int mapif_send_gmaccounts();

int inter_check_length(int fd,int length);

int inter_log(char *fmt,...);

#define inter_cfgName "conf/inter_athena.conf"

extern int party_share_level;
extern char inter_log_filename[1024];

//add include for DBMS(mysql)
#include <mysql.h>

extern MYSQL mysql_handle;
extern char tmp_sql[65535];
extern MYSQL_RES* 	sql_res ;
extern MYSQL_ROW	sql_row ;
extern int 	sql_cnt;

extern MYSQL lmysql_handle;
extern char tmp_lsql[65535];
extern MYSQL_RES* 	lsql_res ;
extern MYSQL_ROW	lsql_row ;

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

extern int log_inter;

#endif
