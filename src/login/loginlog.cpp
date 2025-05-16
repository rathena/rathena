// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "loginlog.hpp"

#include <cstdlib> // exit
#include <string>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>


std::string log_db_hostname = "127.0.0.1";
uint16 log_db_port = 3306;
std::string log_db_username = "ragnarok";
std::string log_db_password = "";
std::string log_db_database = "ragnarok";
std::string log_login_db = "loginlog";
std::string log_codepage = "";

static Sql* sql_handle = nullptr;
static bool enabled = false;


/**
 * Get the number of failed login attempts by the ip in the last minutes.
 * @param ip: ip to search attempt from
 * @param minutes: intervall to search
 * @return number of failed attempts
 */
unsigned long loginlog_failedattempts(uint32 ip, uint32 minutes) {
	unsigned long failures = 0;

	if( !enabled )
		return 0;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `ip` = '%s' AND (`rcode` = '0' OR `rcode` = '1') AND `time` > NOW() - INTERVAL %d MINUTE",
		log_login_db.c_str(), ip2str(ip,nullptr), minutes) )// how many times failed account? in one ip.
		Sql_ShowDebug(sql_handle);

	if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		char* data;
		Sql_GetData(sql_handle, 0, &data, nullptr);
		failures = strtoul(data, nullptr, 10);
		Sql_FreeResult(sql_handle);
	}
	return failures;
}


/**
 * Records an event in the login log.
 * @param ip:
 * @param username:
 * @param rcode:
 * @param message:
 */
void login_log(uint32 ip, const char* username, int32 rcode, const char* message) {
	char esc_username[NAME_LENGTH*2+1];
	char esc_message[255*2+1];
	int32 retcode;

	if( !enabled )
		return;

	Sql_EscapeStringLen(sql_handle, esc_username, username, strnlen(username, NAME_LENGTH));
	Sql_EscapeStringLen(sql_handle, esc_message, message, strnlen(message, 255));

	retcode = Sql_Query(sql_handle,
		"INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%s', '%s', '%d', '%s')",
		log_login_db.c_str(), ip2str(ip,nullptr), esc_username, rcode, esc_message);

	if( retcode != SQL_SUCCESS )
		Sql_ShowDebug(sql_handle);
}

/**
 * Read configuration options.
 * @param key: config keyword
 * @param value: config value for keyword
 * @return true if successful, false if config not complete or server already running
 */
bool loginlog_config_read(const char* key, const char* value) {
	if( strcmpi(key, "log_db_ip") == 0 )
		log_db_hostname = value;
	else
	if( strcmpi(key, "log_db_port") == 0 )
		log_db_port = (uint16)strtoul(value, nullptr, 10);
	else
	if( strcmpi(key, "log_db_id") == 0 )
		log_db_username = value;
	else
	if( strcmpi(key, "log_db_pw") == 0 )
		log_db_password = value;
	else
	if( strcmpi(key, "log_db_db") == 0 )
		log_db_database = value;
	else
	if( strcmpi(key, "log_codepage") == 0 )
		log_codepage = value;
	else
	if( strcmpi(key, "log_login_db") == 0 )
		log_login_db = value;
	else
		return false;

	return true;
}


/// Constructor destructor

/**
 * Initialize the module.
 * Launched at login-serv start, create db or other long scope variable here.
 * @return true if success else exit execution
 */
bool loginlog_init(void) {
	sql_handle = Sql_Malloc();

	if( SQL_ERROR == Sql_Connect(sql_handle, log_db_username.c_str(), log_db_password.c_str(), log_db_hostname.c_str(), log_db_port, log_db_database.c_str()) )
	{
		ShowError("Couldn't connect with uname='%s',host='%s',port='%hu',database='%s'\n",
			log_db_username.c_str(), log_db_hostname.c_str(), log_db_port, log_db_database.c_str());
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	if( !log_codepage.empty() && SQL_ERROR == Sql_SetEncoding(sql_handle, log_codepage.c_str()) )
		Sql_ShowDebug(sql_handle);

	enabled = true;

	return true;
}


/**
 * Handler to cleanup module, called when login-server stops.
 * Currently closing sql connection to log schema.
 * @return true success
 */
bool loginlog_final(void) {
	Sql_Free(sql_handle);
	sql_handle = nullptr;
	return true;
}
