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

// globals
static Sql* sql_handle = NULL;

// login sql settings
struct Ipban_Config {
	uint16 ipban_db_port;
	StringBuf *ipban_db_hostname;
	StringBuf *ipban_db_username;
	StringBuf *ipban_db_password;
	StringBuf *ipban_db_database;
	StringBuf *ipban_codepage;
	StringBuf *ipban_table;

	int cleanup_timer_id;
	bool ipban_inited;
};

struct Ipban_Config ipban_config;

//early declaration
int ipban_cleanup(int tid, unsigned int tick, int id, intptr_t data);

void ipban_config_init(void) {
	ipban_config.ipban_db_port = 3306;
	ipban_config.ipban_db_hostname = StringBuf_FromStr("127.0.0.1");
	ipban_config.ipban_db_username = StringBuf_FromStr("ragnarok");
	ipban_config.ipban_db_password = StringBuf_FromStr("");
	ipban_config.ipban_db_database = StringBuf_FromStr("ragnarok");
	ipban_config.ipban_codepage    = StringBuf_FromStr("");
	ipban_config.ipban_table       = StringBuf_FromStr("ipbanlist");

	ipban_config.cleanup_timer_id = INVALID_TIMER;
	ipban_config.ipban_inited = false;
}

static void ipban_config_final(void) {
	StringBuf_Free(ipban_config.ipban_db_hostname);
	StringBuf_Free(ipban_config.ipban_db_username);
	StringBuf_Free(ipban_config.ipban_db_password);
	StringBuf_Free(ipban_config.ipban_db_database);
	StringBuf_Free(ipban_config.ipban_codepage);
	StringBuf_Free(ipban_config.ipban_table);
}

static bool ipban_check_table(void) {
	ShowInfo("Start checking DB integrity (IP Ban)\n");

	// IP ban List
	if( SQL_ERROR == Sql_Query(sql_handle,
		"SELECT `list`, `btime`, `rtime`, `reason` "
		"FROM `%s`;", StringBuf_Value(ipban_config.ipban_table)) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}
	return true;
}

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
		StringBuf_Value(ipban_config.ipban_table), p[3], p[3], p[2], p[3], p[2], p[1], p[3], p[2], p[1], p[0]) )
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
			StringBuf_Value(ipban_config.ipban_table), p[3], p[2], p[1], login_config.dynamic_pass_failure_ban_duration) )
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

	if( ipban_config.ipban_inited )
		return false;// settings can only be changed before init

	signature = "ipban_db_";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "ip") == 0 )
			StringBuf_PrintfClear(ipban_config.ipban_db_hostname, value);
		else
		if( strcmpi(key, "port") == 0 )
			ipban_config.ipban_db_port = (uint16)strtoul(value, NULL, 10);
		else
		if( strcmpi(key, "id") == 0 )
			StringBuf_PrintfClear(ipban_config.ipban_db_username, value);
		else
		if( strcmpi(key, "pw") == 0 )
			StringBuf_PrintfClear(ipban_config.ipban_db_password, value);
		else
		if( strcmpi(key, "db") == 0 )
			StringBuf_PrintfClear(ipban_config.ipban_db_database, value);
		else
			return false;// not found
		return true;
	}

	signature = "ipban_";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "codepage") == 0 )
			StringBuf_PrintfClear(ipban_config.ipban_codepage, value);
		else
		if( strcmpi(key, "list_table") == 0 )
			StringBuf_PrintfClear(ipban_config.ipban_table, value);
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
	ipban_config.ipban_inited = true;

	if( !login_config.ipban )
		return;// ipban disabled

	// establish connections
	sql_handle = Sql_Malloc();
	if( SQL_ERROR == Sql_Connect(sql_handle, StringBuf_Value(ipban_config.ipban_db_username), StringBuf_Value(ipban_config.ipban_db_password), StringBuf_Value(ipban_config.ipban_db_hostname), ipban_config.ipban_db_port, StringBuf_Value(ipban_config.ipban_db_database)) )
	{
		ShowError("Couldn't connect with uname='%s',passwd='%s',host='%s',port='%d',database='%s'\n",
			StringBuf_Value(ipban_config.ipban_db_username), StringBuf_Value(ipban_config.ipban_db_password), StringBuf_Value(ipban_config.ipban_db_hostname), ipban_config.ipban_db_port, StringBuf_Value(ipban_config.ipban_db_database));
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}
	ShowInfo("Ipban conection made\n");

	if( StringBuf_Length(ipban_config.ipban_codepage) && SQL_ERROR == Sql_SetEncoding(sql_handle, StringBuf_Value(ipban_config.ipban_codepage)) )
		Sql_ShowDebug(sql_handle);

	if (!ipban_check_table()) {
		ShowFatalError("login-server (ipban) : A table is missing in sql-server, please fix it, see (sql-files/main.sql for structure) \n");
		exit(EXIT_FAILURE);
	}

	ShowStatus("Ipban connection: Database '"CL_WHITE"%s"CL_RESET"' at '"CL_WHITE"%s"CL_RESET"'\n", StringBuf_Value(ipban_config.ipban_db_database), StringBuf_Value(ipban_config.ipban_db_hostname));

	if( login_config.ipban_cleanup_interval > 0 )
	{ // set up periodic cleanup of connection history and active bans
		add_timer_func_list(ipban_cleanup, "ipban_cleanup");
		ipban_config.cleanup_timer_id = add_timer_interval(gettick()+10, ipban_cleanup, 0, 0, login_config.ipban_cleanup_interval*1000);
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
		delete_timer(ipban_config.cleanup_timer_id, ipban_cleanup);

	ipban_cleanup(0,0,0,0); // always clean up on login-server stop
	ipban_config_final();

	// close connections
	Sql_Free(sql_handle);
	sql_handle = NULL;
}
