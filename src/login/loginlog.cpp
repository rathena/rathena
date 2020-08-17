// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "loginlog.hpp"

#include <stdlib.h> // exit
#include <string>

#include "../common/cbasetypes.hpp"
#include "../common/mmo.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/sql.hpp"
#include "../common/strlib.hpp"

static Sql* sql_handle = NULL;

struct Loginlog_Config {
	// local sql settings
	uint16 log_db_port;
	std::string log_db_hostname;
	std::string log_db_username;
	std::string log_db_password;
	std::string log_db_database;
	std::string log_codepage;
	std::string log_login_table;

	bool enabled;
};
struct Loginlog_Config loginlog_config; /// LoginLog config

/**
 * Initialize default configurations
 **/
void loginlog_config_init(void) {
	loginlog_config.log_db_port = 3306;
	loginlog_config.log_db_hostname = "127.0.0.1";
	loginlog_config.log_db_username = "ragnarok";
	loginlog_config.log_db_password = "";
	loginlog_config.log_db_database = "ragnarok";
	loginlog_config.log_codepage    = "";
	loginlog_config.log_login_table = "loginlog";

	loginlog_config.enabled = false;;
}

/**
 * Finalize default configurations
 **/
void loginlog_config_final(void) {
}

static bool loginlog_check_table(void) {
	ShowInfo("Start checking DB integrity (Login Log)\n");

	// IP ban List
	if( SQL_ERROR == Sql_Query(sql_handle,
		"SELECT `time`, `ip`, `user`, `rcode`, `log` "
		"FROM `%s`;", loginlog_config.log_login_table.c_str()) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}
	return true;
}

/**
 * Get the number of failed login attempts by the ip in the last minutes.
 * @param ip: ip to search attempt from
 * @param minutes: intervall to search
 * @return number of failed attempts
 */
unsigned long loginlog_failedattempts(uint32 ip, unsigned int minutes) {
	unsigned long failures = 0;

	if( !loginlog_config.enabled )
		return 0;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `ip` = '%s' AND (`rcode` = '0' OR `rcode` = '1') AND `time` > NOW() - INTERVAL %d MINUTE",
		loginlog_config.log_login_table.c_str(), ip2str(ip,NULL), minutes) )// how many times failed account? in one ip.
		Sql_ShowDebug(sql_handle);

	if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		char* data;
		Sql_GetData(sql_handle, 0, &data, NULL);
		failures = strtoul(data, NULL, 10);
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
void login_log(uint32 ip, const char* username, int rcode, const char* message) {
	char esc_username[NAME_LENGTH*2+1];
	char esc_message[255*2+1];
	int retcode;

	if( !loginlog_config.enabled )
		return;

	Sql_EscapeStringLen(sql_handle, esc_username, username, strnlen(username, NAME_LENGTH));
	Sql_EscapeStringLen(sql_handle, esc_message, message, strnlen(message, 255));

	retcode = Sql_Query(sql_handle,
		"INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%s', '%s', '%d', '%s')",
		loginlog_config.log_login_table.c_str(), ip2str(ip,NULL), esc_username, rcode, esc_message);

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
		loginlog_config.log_db_hostname = value;
	else
	if( strcmpi(key, "log_db_port") == 0 )
		loginlog_config.log_db_port = (uint16)strtoul(value, NULL, 10);
	else
	if( strcmpi(key, "log_db_id") == 0 )
		loginlog_config.log_db_username = value;
	else
	if( strcmpi(key, "log_db_pw") == 0 )
		loginlog_config.log_db_password = value;
	else
	if( strcmpi(key, "log_db_db") == 0 )
		loginlog_config.log_db_database = value;
	else
	if( strcmpi(key, "log_codepage") == 0 )
		loginlog_config.log_codepage = value;
	else
	if( strcmpi(key, "log_login_table") == 0 )
		loginlog_config.log_login_table = value;
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

	if( SQL_ERROR == Sql_Connect(sql_handle, loginlog_config.log_db_username.c_str(), loginlog_config.log_db_password.c_str(), loginlog_config.log_db_hostname.c_str(), loginlog_config.log_db_port, loginlog_config.log_db_database.c_str()) )
	{
		ShowError("Couldn't connect with uname='%s',passwd='%s',host='%s',port='%d',database='%s'\n",
			loginlog_config.log_db_username.c_str(), loginlog_config.log_db_password.c_str(), loginlog_config.log_db_hostname.c_str(), loginlog_config.log_db_port, loginlog_config.log_db_database.c_str());
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	if( !loginlog_config.log_codepage.empty() && SQL_ERROR == Sql_SetEncoding(sql_handle, loginlog_config.log_codepage.c_str()) )
		Sql_ShowDebug(sql_handle);

	if (!loginlog_check_table()) {
		ShowFatalError("login-server (loginlog) : A table is missing in sql-server, please fix it, see (sql-files/logs.sql for structure) \n");
		exit(EXIT_FAILURE);
	}

	loginlog_config.enabled = true;

	ShowStatus("Loginlog connection: Database '" CL_WHITE "%s" CL_RESET "' at '" CL_WHITE "%s" CL_RESET "'\n", loginlog_config.log_db_database.c_str(), loginlog_config.log_db_hostname.c_str());
	return true;
}


/**
 * Handler to cleanup module, called when login-server stops.
 * Currently closing sql connection to log schema.
 * @return true success
 */
bool loginlog_final(void) {
	Sql_Free(sql_handle);
	sql_handle = NULL;
	loginlog_config_final();
	return true;
}
