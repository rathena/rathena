/**
 * @file loginlog.c
 * Module purpose is to register (log) events into a file or sql database.
 * Licensed under GNU GPL.
 *  For more information, see LICENCE in the main folder.
 * @author Athena Dev Teams rev < 15k
 * @author rAthena Dev Team
 */

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/socket.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include <stdlib.h> // exit

static Sql* sql_handle = NULL;

struct Loginlog_Config {
	// local sql settings
	uint16 log_db_port;
	StringBuf *log_db_hostname;
	StringBuf *log_db_username;
	StringBuf *log_db_password;
	StringBuf *log_db_database;
	StringBuf *log_codepage;
	StringBuf *log_login_table;

	bool enabled;
};
struct Loginlog_Config loginlog_config; /// LoginLog config

void loginlog_config_init(void) {
	loginlog_config.log_db_port = 3306;
	loginlog_config.log_db_hostname = StringBuf_FromStr("127.0.0.1");
	loginlog_config.log_db_username = StringBuf_FromStr("ragnarok");
	loginlog_config.log_db_password = StringBuf_FromStr("");
	loginlog_config.log_db_database = StringBuf_FromStr("ragnarok");
	loginlog_config.log_codepage    = StringBuf_FromStr("");
	loginlog_config.log_login_table = StringBuf_FromStr("loginlog");

	loginlog_config.enabled = false;;
}

static void loginlog_config_final(void) {
	StringBuf_Free(loginlog_config.log_db_hostname);
	StringBuf_Free(loginlog_config.log_db_username);
	StringBuf_Free(loginlog_config.log_db_password);
	StringBuf_Free(loginlog_config.log_db_database);
	StringBuf_Free(loginlog_config.log_codepage);
	StringBuf_Free(loginlog_config.log_login_table);
}

static bool loginlog_check_table(void) {
	ShowInfo("Start checking DB integrity (Login Log)\n");

	// IP ban List
	if( SQL_ERROR == Sql_Query(sql_handle,
		"SELECT `time`, `ip`, `user`, `rcode`, `log` "
		"FROM `%s`;", StringBuf_Value(loginlog_config.log_login_table)) )
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

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `ip` = '%s' AND `rcode` = '1' AND `time` > NOW() - INTERVAL %d MINUTE",
		StringBuf_Value(loginlog_config.log_login_table), ip2str(ip,NULL), minutes) )// how many times failed account? in one ip.
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
		StringBuf_Value(loginlog_config.log_login_table), ip2str(ip,NULL), esc_username, rcode, esc_message);

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
		StringBuf_PrintfClear(loginlog_config.log_db_hostname, value);
	else
	if( strcmpi(key, "log_db_port") == 0 )
		loginlog_config.log_db_port = (uint16)strtoul(value, NULL, 10);
	else
	if( strcmpi(key, "log_db_id") == 0 )
		StringBuf_PrintfClear(loginlog_config.log_db_username, value);
	else
	if( strcmpi(key, "log_db_pw") == 0 )
		StringBuf_PrintfClear(loginlog_config.log_db_password, value);
	else
	if( strcmpi(key, "log_db_db") == 0 )
		StringBuf_PrintfClear(loginlog_config.log_db_database, value);
	else
	if( strcmpi(key, "log_codepage") == 0 )
		StringBuf_PrintfClear(loginlog_config.log_codepage, value);
	else
	if( strcmpi(key, "log_login_table") == 0 )
		StringBuf_PrintfClear(loginlog_config.log_login_table, value);
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

	if( SQL_ERROR == Sql_Connect(sql_handle, StringBuf_Value(loginlog_config.log_db_username), StringBuf_Value(loginlog_config.log_db_password), StringBuf_Value(loginlog_config.log_db_hostname), loginlog_config.log_db_port, StringBuf_Value(loginlog_config.log_db_database)) )
	{
		ShowError("Couldn't connect with uname='%s',passwd='%s',host='%s',port='%d',database='%s'\n",
			StringBuf_Value(loginlog_config.log_db_username), StringBuf_Value(loginlog_config.log_db_password), StringBuf_Value(loginlog_config.log_db_hostname), loginlog_config.log_db_port, StringBuf_Value(loginlog_config.log_db_database));
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	if( StringBuf_Length(loginlog_config.log_codepage) && SQL_ERROR == Sql_SetEncoding(sql_handle, StringBuf_Value(loginlog_config.log_codepage)) )
		Sql_ShowDebug(sql_handle);

	if (!loginlog_check_table()) {
		ShowFatalError("login-server (loginlog) : A table is missing in sql-server, please fix it, see (sql-files/logs.sql for structure) \n");
		exit(EXIT_FAILURE);
	}

	loginlog_config.enabled = true;

	ShowStatus("Loginlog connection: Database '"CL_WHITE"%s"CL_RESET"' at '"CL_WHITE"%s"CL_RESET"'\n", StringBuf_Value(loginlog_config.log_db_database), StringBuf_Value(loginlog_config.log_db_username));
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
