#ifndef _INTER_H_
#define _INTER_H_

int inter_init(const char *file);
int inter_save();
int inter_parse_frommap(int fd);

int inter_check_length(int fd,int length);

#define INTER_CONF_NAME "conf/inter_athena.conf"


//add include for DBMS(mysql)
#include <mysql.h>

extern MYSQL mysql_handle;
extern char tmp_sql[65535];
extern MYSQL_RES* 	sql_res ;
extern MYSQL_ROW	sql_row ;
extern int 	sql_cnt;

extern int db_server_port;
extern char db_server_ip[16];
extern char db_server_id[32];
extern char db_server_pw[32];
extern char db_server_logindb[32];

#endif
