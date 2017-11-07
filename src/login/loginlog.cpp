/**
 * @file loginlog.c
 * Module purpose is to register (log) events into a file or sql database.
 * Licensed under GNU GPL.
 *  For more information, see LICENCE in the main folder.
 * @author Athena Dev Teams rev < 15k
 * @author rAthena Dev Team
 */

#include "loginlog.hpp"

#include <stdlib.h> // exit
#include <string.h>

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/socket.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"

// global sql settings (in ipban_sql.c)
static char   global_db_hostname[64] = "127.0.0.1"; // Doubled to reflect the change on commit #0f2dd7f
static uint16 global_db_port = 3306;
static char   global_db_username[32] = "ragnarok";
static char   global_db_password[32] = ""; //empty by default since mysql is empty by default as well
static char   global_db_database[32] = "ragnarok";
static char   global_codepage[32] = "";
// local sql settings
static char   log_db_hostname[64] = ""; // Doubled to reflect the change on commit #0f2dd7f
static uint16 log_db_port = 0;
static char   log_db_username[32] = "";
static char   log_db_password[32] = "";
static char   log_db_database[32] = "";
static char   log_codepage[32] = "";
static char   log_login_db[256] = "loginlog";

static Sql* sql_handle = NULL;
static bool enabled = false;


/**
 * Get the number of failed login attempts by the ip in the last minutes.
 * @param ip: ip to search attempt from
 * @param minutes: intervall to search
 * @return number of failed attempts
 */
unsigned long loginlog_failedattempts(uint32 ip, unsigned int minutes) {
	unsigned long failures = 0;

	if( !enabled )
		return 0;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `ip` = '%s' AND (`rcode` = '0' OR `rcode` = '1') AND `time` > NOW() - INTERVAL %d MINUTE",
		log_login_db, ip2str(ip,NULL), minutes) )// how many times failed account? in one ip.
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

	if( !enabled )
		return;

	Sql_EscapeStringLen(sql_handle, esc_username, username, strnlen(username, NAME_LENGTH));
	Sql_EscapeStringLen(sql_handle, esc_message, message, strnlen(message, 255));

	retcode = Sql_Query(sql_handle,
		"INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%s', '%s', '%d', '%s')",
		log_login_db, ip2str(ip,NULL), esc_username, rcode, esc_message);

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
	const char* signature;

	signature = "sql.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "db_hostname") == 0 )
			safestrncpy(global_db_hostname, value, sizeof(global_db_hostname));
		else
		if( strcmpi(key, "db_port") == 0 )
			global_db_port = (uint16)strtoul(value, NULL, 10);
		else
		if( strcmpi(key, "db_username") == 0 )
			safestrncpy(global_db_username, value, sizeof(global_db_username));
		else
		if( strcmpi(key, "db_password") == 0 )
			safestrncpy(global_db_password, value, sizeof(global_db_password));
		else
		if( strcmpi(key, "db_database") == 0 )
			safestrncpy(global_db_database, value, sizeof(global_db_database));
		else
		if( strcmpi(key, "codepage") == 0 )
			safestrncpy(global_codepage, value, sizeof(global_codepage));
		else
			return false;// not found
		return true;
	}

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
	if( strcmpi(key, "log_db_db") == 0 )
		safestrncpy(log_db_database, value, sizeof(log_db_database));
	else
	if( strcmpi(key, "log_codepage") == 0 )
		safestrncpy(log_codepage, value, sizeof(log_codepage));
	else
	if( strcmpi(key, "log_login_db") == 0 )
		safestrncpy(log_login_db, value, sizeof(log_login_db));
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
	const char* username;
	const char* password;
	const char* hostname;
	uint16      port;
	const char* database;
	const char* codepage;

	if( log_db_hostname[0] != '\0' )
	{// local settings
		username = log_db_username;
		password = log_db_password;
		hostname = log_db_hostname;
		port     = log_db_port;
		database = log_db_database;
		codepage = log_codepage;
	}
	else
	{// global settings
		username = global_db_username;
		password = global_db_password;
		hostname = global_db_hostname;
		port     = global_db_port;
		database = global_db_database;
		codepage = global_codepage;
	}

	sql_handle = Sql_Malloc();

	if( SQL_ERROR == Sql_Connect(sql_handle, username, password, hostname, port, database) )
	{
        ShowError("Couldn't connect with uname='%s',passwd='%s',host='%s',port='%d',database='%s'\n",
                        username, password, hostname, port, database);
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	if( codepage[0] != '\0' && SQL_ERROR == Sql_SetEncoding(sql_handle, codepage) )
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
	sql_handle = NULL;
	return true;
}
