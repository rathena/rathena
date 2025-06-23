// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ipban.hpp"

#include <cstdlib>
#include <cstring>

#include <common/cbasetypes.hpp>
#include <common/showmsg.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "login.hpp"
#include "loginlog.hpp"


std::string ipban_db_hostname = "127.0.0.1";
uint16 ipban_db_port = 3306;
std::string ipban_db_username = "ragnarok";
std::string ipban_db_password = "";
std::string ipban_db_database = "ragnarok";
std::string ipban_codepage = "";
std::string ipban_table = "ipbanlist";

// globals
static Sql* sql_handle = nullptr;
static int32 cleanup_timer_id = INVALID_TIMER;
static bool ipban_inited = false;

//early declaration
TIMER_FUNC(ipban_cleanup);

/**
 * Check if ip is in the active bans list.
 * @param ip: ipv4 ip to check if ban
 * @return true if found or error, false if not in list
 */
bool ipban_check(uint32 ip) {
	uint8* p = (uint8*)&ip;
	char* data = nullptr;
	int32 matches;

	if( !login_config.ipban )
		return false;// ipban disabled

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `rtime` > NOW() AND (`list` = '%u.*.*.*' OR `list` = '%u.%u.*.*' OR `list` = '%u.%u.%u.*' OR `list` = '%u.%u.%u.%u')",
		ipban_table.c_str(), p[3], p[3], p[2], p[3], p[2], p[1], p[3], p[2], p[1], p[0]) )
	{
		Sql_ShowDebug(sql_handle);
		// close connection because we can't verify their connectivity.
		return true;
	}

	if( SQL_ERROR == Sql_NextRow(sql_handle) )
		return true;// Shouldn't happen, but just in case...

	Sql_GetData(sql_handle, 0, &data, nullptr);
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
			ipban_table.c_str(), p[3], p[2], p[1], login_config.dynamic_pass_failure_ban_duration) )
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
TIMER_FUNC(ipban_cleanup){
	if( !login_config.ipban )
		return 0;// ipban disabled

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `rtime` <= NOW()", ipban_table.c_str()) )
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
			ipban_db_hostname = value;
		else
		if( strcmpi(key, "port") == 0 )
			ipban_db_port = (uint16)strtoul( value, nullptr, 10 );
		else
		if( strcmpi(key, "id") == 0 )
			ipban_db_username = value;
		else
		if( strcmpi(key, "pw") == 0 )
			ipban_db_password = value;
		else
		if( strcmpi(key, "db") == 0 )
			ipban_db_database = value;
		else
			return false;// not found
		return true;
	}

	signature = "ipban_";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "codepage") == 0 )
			ipban_codepage = value;
		else
		if( strcmpi(key, "ipban_table") == 0 )
			ipban_table = value;
		else
		if( strcmpi(key, "enable") == 0 )
			login_config.ipban = (config_switch(value) != 0);
		else
		if( strcmpi(key, "dynamic_pass_failure_ban") == 0 )
			login_config.dynamic_pass_failure_ban = (config_switch(value) != 0);
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
	ipban_inited = true;

	if( !login_config.ipban )
		return;// ipban disabled

	// establish connections
	sql_handle = Sql_Malloc();
	if( SQL_ERROR == Sql_Connect(sql_handle, ipban_db_username.c_str(), ipban_db_password.c_str(), ipban_db_hostname.c_str(), ipban_db_port, ipban_db_database.c_str()) )
	{
		ShowError("Couldn't connect with uname='%s',host='%s',port='%hu',database='%s'\n",
			ipban_db_username.c_str(), ipban_db_hostname.c_str(), ipban_db_port, ipban_db_database.c_str());
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	ShowInfo("Ipban connection made.\n");

	if( !ipban_codepage.empty() && SQL_ERROR == Sql_SetEncoding(sql_handle, ipban_codepage.c_str()) )
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
	sql_handle = nullptr;
}
