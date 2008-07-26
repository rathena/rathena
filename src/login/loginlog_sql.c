// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/socket.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include <string.h>
#include <stdlib.h> // exit

char   log_db_ip[32] = "127.0.0.1";
uint16 log_db_port = 3306;
char   log_db_id[32] = "ragnarok";
char   log_db_pw[32] = "ragnarok";
char   log_db_db[32] = "ragnarok";
char   log_db[32] = "log";

char loginlog_db[256] = "loginlog";
Sql* sql_handle;
bool enabled = false;


/*=============================================
 * Records an event in the login log
 *---------------------------------------------*/
void login_log(uint32 ip, const char* username, int rcode, const char* message)
{
	char esc_username[NAME_LENGTH*2+1];
	char esc_message[255*2+1];
	int retcode;

	if( !enabled )
		return;

	Sql_EscapeStringLen(sql_handle, esc_username, username, strnlen(username, NAME_LENGTH));
	Sql_EscapeStringLen(sql_handle, esc_message, message, strnlen(message, 255));

	retcode = Sql_Query(sql_handle,
		"INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%s', '%s', '%d', '%s')",
		loginlog_db, ip2str(ip,NULL), esc_username, rcode, message);

	if( retcode != SQL_SUCCESS )
		Sql_ShowDebug(sql_handle);
}

bool loginlog_init(void)
{
	sql_handle = Sql_Malloc();

	if( SQL_ERROR == Sql_Connect(sql_handle, log_db_id, log_db_pw, log_db_ip, log_db_port, log_db_db) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	enabled = true;

	return true;
}

bool loginlog_final(void)
{
	Sql_Free(sql_handle);
	sql_handle = NULL;
	return true;
}

bool loginlog_config_read(const char* key, const char* value)
{
	if( strcmpi(key, "log_db_ip") == 0 )
		safestrncpy(log_db_ip, value, sizeof(log_db_ip));
	else
	if( strcmpi(key, "log_db_port") == 0 )
		log_db_port = (uint16)strtoul(value, NULL, 10);
	else
	if( strcmpi(key, "log_db_id") == 0 )
		safestrncpy(log_db_id, value, sizeof(log_db_id));
	else
	if( strcmpi(key, "log_db_pw") == 0 )
		safestrncpy(log_db_pw, value, sizeof(log_db_pw));
	else
	if( strcmpi(key, "log_db") == 0 )
		safestrncpy(log_db, value, sizeof(log_db));
	else
	if( strcmpi(key, "loginlog_db") == 0 )
		safestrncpy(loginlog_db, value, sizeof(loginlog_db));
	else
		return false;

	return true;
}
