// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/sql.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "login.h"
#include "ipban.h"
#include <stdlib.h>
#include <string.h>

// database options
static char   ipban_db_hostname[32] = "127.0.0.1";
static uint16 ipban_db_port = 3306;
static char   ipban_db_username[32] = "ragnarok";
static char   ipban_db_password[32] = "ragnarok";
static char   ipban_db_database[32] = "ragnarok";
static char   ipban_table[32] = "ipbanlist";

static char   log_db_hostname[32] = "127.0.0.1";
static uint16 log_db_port = 3306;
static char   log_db_username[32] = "ragnarok";
static char   log_db_password[32] = "ragnarok";
static char   log_db_database[32] = "ragnarok";
static char   loginlog_table[32] = "loginlog";

static char default_codepage[32] = "";

// globals
static Sql* sql_handle;
static Sql* logsql_handle;
static int cleanup_timer_id = INVALID_TIMER;

int ipban_cleanup(int tid, unsigned int tick, int id, intptr data);


// initialize
void ipban_init(void)
{
	// establish connections
	sql_handle = Sql_Malloc();
	if( SQL_ERROR == Sql_Connect(sql_handle, ipban_db_username, ipban_db_password, ipban_db_hostname, ipban_db_port, ipban_db_database) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}
	if( default_codepage[0] != '\0' && SQL_ERROR == Sql_SetEncoding(sql_handle, default_codepage) )
		Sql_ShowDebug(sql_handle);

	logsql_handle = Sql_Malloc();
	if( SQL_ERROR == Sql_Connect(logsql_handle, log_db_username, log_db_password, log_db_hostname, log_db_port, log_db_database) )
	{
		Sql_ShowDebug(logsql_handle);
		Sql_Free(logsql_handle);
		exit(EXIT_FAILURE);
	}
	if( default_codepage[0] != '\0' && SQL_ERROR == Sql_SetEncoding(logsql_handle, default_codepage) )
		Sql_ShowDebug(logsql_handle);

	// set up periodic cleanup of connection history and active bans
	add_timer_func_list(ipban_cleanup, "ipban_cleanup");
	cleanup_timer_id = add_timer_interval(gettick()+10, ipban_cleanup, 0, 0, 60*1000);
}

// finalize
void ipban_final(void)
{
	// release data
	delete_timer(cleanup_timer_id, ipban_cleanup);

	// close connections
	Sql_Free(sql_handle);
	sql_handle = NULL;
	Sql_Free(logsql_handle);
	logsql_handle = NULL;
}

// load configuration options
bool ipban_config_read(const char* key, const char* value)
{
	// login server settings
	if( strcmpi(key, "ipban.enable") == 0 )
		login_config.ipban = (bool)config_switch(value);
	else
	if( strcmpi(key, "ipban.dynamic_pass_failure_ban") == 0 )
		login_config.dynamic_pass_failure_ban = (bool)config_switch(value);
	else
	if( strcmpi(key, "ipban.dynamic_pass_failure_ban_interval") == 0 )
		login_config.dynamic_pass_failure_ban_interval = atoi(value);
	else
	if( strcmpi(key, "ipban.dynamic_pass_failure_ban_limit") == 0 )
		login_config.dynamic_pass_failure_ban_limit = atoi(value);
	else
	if( strcmpi(key, "ipban.dynamic_pass_failure_ban_duration") == 0 )
		login_config.dynamic_pass_failure_ban_duration = atoi(value);
	else

	// ipban table settings
	if( strcmpi(key, "ipban.sql.db_hostname") == 0 )
		safestrncpy(ipban_db_hostname, value, sizeof(ipban_db_hostname));
	else
	if( strcmpi(key, "ipban.sql.db_port") == 0 )
		ipban_db_port = (uint16)strtoul(value, NULL, 10);
	else
	if( strcmpi(key, "ipban.sql.db_username") == 0 )
		safestrncpy(ipban_db_username, value, sizeof(ipban_db_username));
	else
	if( strcmpi(key, "ipban.sql.db_password") == 0 )
		safestrncpy(ipban_db_password, value, sizeof(ipban_db_password));
	else
	if( strcmpi(key, "ipban.sql.db_database") == 0 )
		safestrncpy(ipban_db_database, value, sizeof(ipban_db_database));
	else
	if( strcmpi(key, "ipban.sql.ipban_table") == 0 )
		safestrncpy(ipban_table, value, sizeof(ipban_table));
	else

	// interserver settings
	if( strcmpi(key, "log_db_ip") == 0 )
		safestrncpy(log_db_hostname, value, sizeof(log_db_hostname));
	else
	if( strcmpi(key, "log_db_port") == 0 )
		log_db_port = (uint16)strtoul(value, NULL, 10);
	else
	if( strcmpi(key, "log_db_id") == 0 )
		safestrncpy(log_db_username, value, sizeof(log_db_username));
	else
	if( strcmpi(key, "log_db_pw") == 0 )
		safestrncpy(log_db_password, value, sizeof(log_db_password));
	else
	if( strcmpi(key, "log_db") == 0 )
		safestrncpy(log_db_database, value, sizeof(log_db_database));
	else
	if( strcmpi(key, "loginlog_db") == 0 )
		safestrncpy(loginlog_table, value, sizeof(loginlog_table));
	else
	if( strcmpi(key, "default_codepage") == 0 )
		safestrncpy(default_codepage, value, sizeof(default_codepage));
	else
		return false;

	return true;
}

// check ip against active bans list
bool ipban_check(uint32 ip)
{
	uint8* p = (uint8*)&ip;
	char* data = NULL;
	int matches;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `list` = '%u.*.*.*' OR `list` = '%u.%u.*.*' OR `list` = '%u.%u.%u.*' OR `list` = '%u.%u.%u.%u'",
		ipban_table, p[3], p[3], p[2], p[3], p[2], p[1], p[3], p[2], p[1], p[0]) )
	{
		Sql_ShowDebug(sql_handle);
		// close connection because we can't verify their connectivity.
		return true;
	}

	if( SQL_ERROR == Sql_NextRow(sql_handle) )
		return true;// Shouldn't happen, but just in case...

	Sql_GetData(sql_handle, 0, &data, NULL);
	matches = atoi(data);
	Sql_FreeResult(sql_handle);

	return( matches > 0 );
}

// log failed attempt
void ipban_log(uint32 ip)
{
	unsigned long failures = 0;
	if( SQL_ERROR == Sql_Query(logsql_handle, "SELECT count(*) FROM `%s` WHERE `ip` = '%s' AND `rcode` = '1' AND `time` > NOW() - INTERVAL %d MINUTE",
		loginlog_table, ip2str(ip,NULL), login_config.dynamic_pass_failure_ban_interval) )// how many times failed account? in one ip.
		Sql_ShowDebug(sql_handle);

	//check query result
	if( SQL_SUCCESS == Sql_NextRow(logsql_handle) )
	{
		char* data;
		Sql_GetData(logsql_handle, 0, &data, NULL);
		failures = strtoul(data, NULL, 10);
		Sql_FreeResult(logsql_handle);
	}

	// if over the limit, add a temporary ban entry
	if( failures >= login_config.dynamic_pass_failure_ban_limit )
	{
		uint8* p = (uint8*)&ip;
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s`(`list`,`btime`,`rtime`,`reason`) VALUES ('%u.%u.%u.*', NOW() , NOW() +  INTERVAL %d MINUTE ,'Password error ban')",
			ipban_table, p[3], p[2], p[1], login_config.dynamic_pass_failure_ban_duration) )
			Sql_ShowDebug(sql_handle);
	}
}

// remove expired bans
int ipban_cleanup(int tid, unsigned int tick, int id, intptr data)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `ipbanlist` WHERE `rtime` <= NOW()") )
		Sql_ShowDebug(sql_handle);

	return 0;
}
