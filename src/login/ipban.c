/**
 * @file ipban.c
 * Module purpose is to read configuration for login-server and handle accounts,
 *  and also to synchronize all login interfaces: loginchrif, loginclif, logincnslif.
 * Licensed under GNU GPL.
 *  For more information, see LICENCE in the main folder.
 * @author Athena Dev Teams < r15k
 * @author rAthena Dev Team
 */

#include "../common/cbasetypes.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "login.h"
#include "ipban.h"
#include "loginlog.h"
#include <stdlib.h>

// login sql settings
static char   ipban_db_hostname[32] = "127.0.0.1";
static uint16 ipban_db_port = 3306;
static char   ipban_db_username[32] = "ragnarok";
static char   ipban_db_password[32] = "";
static char   ipban_db_database[32] = "ragnarok";
static char   ipban_codepage[32] = "";
static char   ipban_table[32] = "ipbanlist";

// globals
static Sql* sql_handle = NULL;
static int cleanup_timer_id = INVALID_TIMER;
static bool ipban_inited = false;

//early declaration
int ipban_cleanup(int tid, unsigned int tick, int id, intptr_t data);

/**
 * Check if ip is in the active bans list.
 * @param ip: ipv4 ip to check if ban
 * @return true if found or error, false if not in list
 */
bool ipban_check(uint32 ip) {
	uint8* p = (uint8*)&ip;
	char* data = NULL;
	int matches;

	if( !login_config.ipban )
		return false;// ipban disabled

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `rtime` > NOW() AND (`list` = '%u.*.*.*' OR `list` = '%u.%u.*.*' OR `list` = '%u.%u.%u.*' OR `list` = '%u.%u.%u.%u')",
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

/**
 * Log a failed attempt.
 *  Also bans the user if too many failed attempts are made.
 * @param ip: ipv4 ip to record the failure
 */
void ipban_log(uint32 ip) {
	unsigned long failures;

	if( !login_config.ipban )
		return;// ipban disabled

	failures = loginlog_failedattempts(ip, login_config.dynamic_pass_failure_ban_interval);// how many times failed account? in one ip.

	// if over the limit, add a temporary ban entry
	if( failures >= login_config.dynamic_pass_failure_ban_limit )
	{
		uint8* p = (uint8*)&ip;
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s`(`list`,`btime`,`rtime`,`reason`) VALUES ('%u.%u.%u.*', NOW() , NOW() +  INTERVAL %d MINUTE ,'Password error ban')",
			ipban_table, p[3], p[2], p[1], login_config.dynamic_pass_failure_ban_duration) )
			Sql_ShowDebug(sql_handle);
	}
}

/**
 * Timered function to remove expired bans.
 *  Request all characters to update their registered ip and transmit their new ip.
 *  Performed each ip_sync_interval.
 * @param tid: timer id
 * @param tick: tick of execution
 * @param id: unused
 * @param data: unused
 * @return 0
 */
int ipban_cleanup(int tid, unsigned int tick, int id, intptr_t data) {
	if( !login_config.ipban )
		return 0;// ipban disabled

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `ipbanlist` WHERE `rtime` <= NOW()") )
		Sql_ShowDebug(sql_handle);

	return 0;
}

/**
 * Read configuration options.
 * @param key: config keyword
 * @param value: config value for keyword
 * @return true if successful, false if config not complete or server already running
 */
bool ipban_config_read(const char* key, const char* value) {
	const char* signature;

	if( ipban_inited )
		return false;// settings can only be changed before init

	signature = "ipban_db_";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "ip") == 0 )
			safestrncpy(ipban_db_hostname, value, sizeof(ipban_db_hostname));
		else
		if( strcmpi(key, "port") == 0 )
			ipban_db_port = (uint16)strtoul(value, NULL, 10);
		else
		if( strcmpi(key, "id") == 0 )
			safestrncpy(ipban_db_username, value, sizeof(ipban_db_username));
		else
		if( strcmpi(key, "pw") == 0 )
			safestrncpy(ipban_db_password, value, sizeof(ipban_db_password));
		else
		if( strcmpi(key, "db") == 0 )
			safestrncpy(ipban_db_database, value, sizeof(ipban_db_database));
		else
			return false;// not found
		return true;
	}

	signature = "ipban_";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "codepage") == 0 )
			safestrncpy(ipban_codepage, value, sizeof(ipban_codepage));
		else
		if( strcmpi(key, "ipban_table") == 0 )
			safestrncpy(ipban_table, value, sizeof(ipban_table));
		else
		if( strcmpi(key, "enable") == 0 )
			login_config.ipban = (bool)config_switch(value);
		else
		if( strcmpi(key, "dynamic_pass_failure_ban") == 0 )
			login_config.dynamic_pass_failure_ban = (bool)config_switch(value);
		else
		if( strcmpi(key, "dynamic_pass_failure_ban_interval") == 0 )
			login_config.dynamic_pass_failure_ban_interval = atoi(value);
		else
		if( strcmpi(key, "dynamic_pass_failure_ban_limit") == 0 )
			login_config.dynamic_pass_failure_ban_limit = atoi(value);
		else
		if( strcmpi(key, "dynamic_pass_failure_ban_duration") == 0 )
			login_config.dynamic_pass_failure_ban_duration = atoi(value);
		else
			return false;// not found
		return true;
	}

	return false;// not found
}


/// Constructor destructor

/**
 * Initialize the module.
 * Launched at login-serv start, create db or other long scope variable here.
 */
void ipban_init(void) {
	const char* username = ipban_db_username;
	const char* password = ipban_db_password;
	const char* hostname = ipban_db_hostname;
	uint16      port     = ipban_db_port;
	const char* database = ipban_db_database;
	const char* codepage = ipban_codepage;

	ipban_inited = true;

	if( !login_config.ipban )
		return;// ipban disabled

	if( ipban_db_hostname[0] != '\0' )
	{// local settings
		username = ipban_db_username;
		password = ipban_db_password;
		hostname = ipban_db_hostname;
		port     = ipban_db_port;
		database = ipban_db_database;
		codepage = ipban_codepage;
	}

	// establish connections
	sql_handle = Sql_Malloc();
	if( SQL_ERROR == Sql_Connect(sql_handle, username, password, hostname, port, database) )
	{
                ShowError("Couldn't connect with uname='%s',passwd='%s',host='%s',port='%d',database='%s'\n",
                        username, password, hostname, port, database);
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}
        ShowInfo("Ipban conection made\n");
        
	if( codepage[0] != '\0' && SQL_ERROR == Sql_SetEncoding(sql_handle, codepage) )
		Sql_ShowDebug(sql_handle);

	if( login_config.ipban_cleanup_interval > 0 )
	{ // set up periodic cleanup of connection history and active bans
		add_timer_func_list(ipban_cleanup, "ipban_cleanup");
		cleanup_timer_id = add_timer_interval(gettick()+10, ipban_cleanup, 0, 0, login_config.ipban_cleanup_interval*1000);
	} else // make sure it gets cleaned up on login-server start regardless of interval-based cleanups
		ipban_cleanup(0,0,0,0);
}

/**
 * Destroy the module.
 * Launched at login-serv end, cleanup db connection or other thing here.
 */
void ipban_final(void) {
	if( !login_config.ipban )
		return;// ipban disabled

	if( login_config.ipban_cleanup_interval > 0 )
		// release data
		delete_timer(cleanup_timer_id, ipban_cleanup);

	ipban_cleanup(0,0,0,0); // always clean up on login-server stop

	// close connections
	Sql_Free(sql_handle);
	sql_handle = NULL;
}
