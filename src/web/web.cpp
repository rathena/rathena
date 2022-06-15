// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "web.hpp"

#include <chrono>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <thread>

#include "../common/cli.hpp"
#include "../common/core.hpp"
#include "../common/malloc.hpp"
#include "../common/md5calc.hpp"
#include "../common/mmo.hpp"
#include "../common/msg_conf.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp" //ip2str
#include "../common/sql.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"
#include "../common/utilities.hpp"
#include "../common/utils.hpp"
#include "../config/core.hpp"

#include "charconfig_controller.hpp"
#include "emblem_controller.hpp"
#include "http.hpp"
#include "userconfig_controller.hpp"
#include "merchantstore_controller.hpp"


using namespace rathena;

#define WEB_MAX_MSG 30				/// Max number predefined in msg_conf
static char* msg_table[WEB_MAX_MSG];	/// Web Server messages_conf

struct Web_Config web_config {};
struct Inter_Config inter_config {};
std::shared_ptr<httplib::Server> http_server;

int login_server_port = 3306;
char login_server_ip[64] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "";
char login_server_db[32] = "ragnarok";

int  char_server_port = 3306;
char char_server_ip[64] = "127.0.0.1";
char char_server_id[32] = "ragnarok";
char char_server_pw[32] = "";
char char_server_db[32] = "ragnarok";

int web_server_port = 3306;
char web_server_ip[64] = "127.0.0.1";
char web_server_id[32] = "ragnarok";
char web_server_pw[32] = "";
char web_server_db[32] = "ragnarok";

char default_codepage[32] = "";

Sql * login_handle = NULL;
Sql * char_handle = NULL;
Sql * web_handle = NULL;

char login_table[32] = "login";
char guild_emblems_table[32] = "guild_emblems";
char user_configs_table[32] = "user_configs";
char char_configs_table[32] = "char_configs";
char merchant_configs_table[32] = "merchant_configs";
char guild_db_table[32] = "guild";
char char_db_table[32] = "char";

int parse_console(const char * buf) {
	return 1;
}

std::thread svr_thr;

/// Msg_conf tayloring
int web_msg_config_read(char *cfgName){
	return _msg_config_read(cfgName,WEB_MAX_MSG,msg_table);
}
const char* web_msg_txt(int msg_number){
	return _msg_txt(msg_number,WEB_MAX_MSG,msg_table);
}
void web_do_final_msg(void){
	_do_final_msg(WEB_MAX_MSG,msg_table);
}
/// Set and read Configurations

/**
 * Reading main configuration file.
 * @param cfgName: Name of the configuration (could be fullpath)
 * @param normal: Config read normally when server started
 * @return True:success, Fals:failure (file not found|readable)
 */
bool web_config_read(const char* cfgName, bool normal) {
	char line[1024], w1[32], w2[1024];
	FILE* fp = fopen(cfgName, "r");
	if (fp == NULL) {
		ShowError("Configuration file (%s) not found.\n", cfgName);
		return false;
	}
	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		
		if (sscanf(line, "%31[^:]: %1023[^\r\n]", w1, w2) < 2)
			continue;
		
		// Config that loaded only when server started, not by reloading config file
		if (normal) {
			if (!strcmpi(w1, "bind_ip")) {
				web_config.web_ip = w2;
			} else if (!strcmpi(w1, "web_port"))
				web_config.web_port = (uint16)atoi(w2);
		}

		if (!strcmpi(w1, "timestamp_format"))
			safestrncpy(timestamp_format, w2, 20);
		else if (!strcmpi(w1, "db_path"))
			safestrncpy(db_path, w2, ARRAYLENGTH(db_path));
		else if (!strcmpi(w1, "stdout_with_ansisequence"))
			stdout_with_ansisequence = config_switch(w2);
		else if (!strcmpi(w1, "console_silent")) {
			msg_silent = atoi(w2);
			if (msg_silent) /* only bother if we have actually this enabled */
				ShowInfo("Console Silent Setting: %d\n", msg_silent);
		}
		else if (!strcmpi(w1, "console_msg_log"))
			console_msg_log = atoi(w2);
		else if (!strcmpi(w1, "console_log_filepath"))
			safestrncpy(console_log_filepath, w2, sizeof(console_log_filepath));
		else if (!strcmpi(w1, "print_req_res"))
			web_config.print_req_res = config_switch(w2);
		else if (!strcmpi(w1, "import"))
			web_config_read(w2, normal);
		else if (!strcmpi(w1, "allow_gifs"))
			web_config.allow_gifs = config_switch(w2) == 1;
	}
	fclose(fp);
	ShowInfo("Finished reading %s.\n", cfgName);
	return true;
}

/*==========================================
 * read config file
 *------------------------------------------*/
int inter_config_read(const char* cfgName)
{
	char line[1024];
	FILE* fp;

	fp = fopen(cfgName, "r");
	if(fp == NULL) {
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}

	while(fgets(line, sizeof(line), fp)) {
		char w1[24], w2[1024];

		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%23[^:]: %1023[^\r\n]", w1, w2) != 2)
			continue;

		if (!strcmpi(w1, "emblem_transparency_limit"))
			inter_config.emblem_woe_change = atoi(w2);
		else if (!strcmpi(w1, "emblem_transparency_limit"))
			inter_config.emblem_woe_change = config_switch(w2) == 1;
		else if(!strcmpi(w1,"login_server_ip"))
			safestrncpy(login_server_ip,w2,sizeof(login_server_ip));
		else if(!strcmpi(w1,"login_server_port"))
			login_server_port = atoi(w2);
		else if(!strcmpi(w1,"login_server_id"))
			safestrncpy(login_server_id,w2,sizeof(login_server_id));
		else if(!strcmpi(w1,"login_server_pw"))
			safestrncpy(login_server_pw,w2,sizeof(login_server_pw));
		else if(!strcmpi(w1,"login_server_db"))
			safestrncpy(login_server_db,w2,sizeof(login_server_db));
		else if(!strcmpi(w1,"char_server_ip"))
			safestrncpy(char_server_ip,w2,sizeof(char_server_ip));
		else if(!strcmpi(w1,"char_server_port"))
			char_server_port = atoi(w2);
		else if(!strcmpi(w1,"char_server_id"))
			safestrncpy(char_server_id,w2,sizeof(char_server_id));
		else if(!strcmpi(w1,"char_server_pw"))
			safestrncpy(char_server_pw,w2,sizeof(char_server_pw));
		else if(!strcmpi(w1,"char_server_db"))
			safestrncpy(char_server_db,w2,sizeof(char_server_db));
		else if(!strcmpi(w1,"web_server_ip"))
			safestrncpy(web_server_ip,w2,sizeof(web_server_ip));
		else if(!strcmpi(w1,"web_server_port"))
			web_server_port = atoi(w2);
		else if(!strcmpi(w1,"web_server_id"))
			safestrncpy(web_server_id,w2,sizeof(web_server_id));
		else if(!strcmpi(w1,"web_server_pw"))
			safestrncpy(web_server_pw,w2,sizeof(web_server_pw));
		else if(!strcmpi(w1,"web_server_db"))
			safestrncpy(web_server_db,w2,sizeof(web_server_db));
		else if(!strcmpi(w1,"default_codepage"))
			safestrncpy(default_codepage,w2,sizeof(default_codepage));
		else if (!strcmpi(w1, "user_configs"))
			safestrncpy(user_configs_table, w2, sizeof(user_configs_table));
		else if (!strcmpi(w1, "char_configs"))
			safestrncpy(char_configs_table, w2, sizeof(char_configs_table));
		else if (!strcmpi(w1, "merchant_configs"))
			safestrncpy(merchant_configs_table, w2, sizeof(merchant_configs_table));
		else if (!strcmpi(w1, "guild_emblems"))
			safestrncpy(guild_emblems_table, w2, sizeof(guild_emblems_table));
		else if (!strcmpi(w1, "login_server_account_db"))
			safestrncpy(login_table, w2, sizeof(login_table));
		else if (!strcmpi(w1, "guild_db"))
			safestrncpy(guild_db_table, w2, sizeof(guild_db_table));
		else if (!strcmpi(w1, "char_db"))
			safestrncpy(char_db_table, w2, sizeof(char_db_table));
		else if(!strcmpi(w1,"import"))
			inter_config_read(w2);
	}
	fclose(fp);

	ShowInfo ("Done reading %s.\n", cfgName);

	return 0;
}


void web_set_defaults() {
	web_config.web_ip = "0.0.0.0";
	web_config.web_port = 3000;
	safestrncpy(web_config.webconf_name, "conf/web_athena.conf", sizeof(web_config.webconf_name));
	safestrncpy(web_config.msgconf_name, "conf/msg_conf/web_msg.conf", sizeof(web_config.msgconf_name));
	web_config.print_req_res = false;

	inter_config.emblem_transparency_limit = 100;
	inter_config.emblem_woe_change = true;
}


/// Constructor destructor and signal handlers

int web_sql_init(void) {
	// login db connection
	login_handle = Sql_Malloc();
	ShowInfo("Connecting to the Login DB server.....\n");

	if (SQL_ERROR == Sql_Connect(login_handle, login_server_id, login_server_pw, login_server_ip, login_server_port, login_server_db)) {
		ShowError("Couldn't connect with uname='%s',passwd='%s',host='%s',port='%d',database='%s'\n",
			login_server_id, login_server_pw, login_server_ip, login_server_port, login_server_db);
		Sql_ShowDebug(login_handle);
		Sql_Free(login_handle);
		exit(EXIT_FAILURE);
	}
	ShowStatus("Connect success! (Login Server Connection)\n");

	if (strlen(default_codepage) > 0) {
		if (SQL_ERROR == Sql_SetEncoding(login_handle, default_codepage))
			Sql_ShowDebug(login_handle);
	}

	char_handle = Sql_Malloc();
	ShowInfo("Connecting to the Char DB server.....\n");

	if (SQL_ERROR == Sql_Connect(char_handle, char_server_id, char_server_pw, char_server_ip, char_server_port, char_server_db)) {
		ShowError("Couldn't connect with uname='%s',passwd='%s',host='%s',port='%d',database='%s'\n",
			char_server_id, char_server_pw, char_server_ip, char_server_port, char_server_db);
		Sql_ShowDebug(char_handle);
		Sql_Free(char_handle);
		exit(EXIT_FAILURE);
	}
	ShowStatus("Connect success! (Char Server Connection)\n");

	if (strlen(default_codepage) > 0) {
		if (SQL_ERROR == Sql_SetEncoding(char_handle, default_codepage))
			Sql_ShowDebug(char_handle);
	}

	web_handle = Sql_Malloc();
	ShowInfo("Connecting to the Web DB server.....\n");

	if (SQL_ERROR == Sql_Connect(web_handle, web_server_id, web_server_pw, web_server_ip, web_server_port, web_server_db)) {
		ShowError("Couldn't connect with uname='%s',passwd='%s',host='%s',port='%d',database='%s'\n",
			web_server_id, web_server_pw, web_server_ip, web_server_port, web_server_db);
		Sql_ShowDebug(web_handle);
		Sql_Free(web_handle);
		exit(EXIT_FAILURE);
	}
	ShowStatus("Connect success! (Web Server Connection)\n");

	if (strlen(default_codepage) > 0) {
		if (SQL_ERROR == Sql_SetEncoding(web_handle, default_codepage))
			Sql_ShowDebug(web_handle);
	}


	return 0;
}

int web_sql_close(void)
{
	ShowStatus("Close Login DB Connection....\n");
	Sql_Free(login_handle);
	login_handle = NULL;
	ShowStatus("Close Char DB Connection....\n");
	Sql_Free(char_handle);
	char_handle = NULL;
	ShowStatus("Close Web DB Connection....\n");
	Sql_Free(web_handle);
	web_handle = NULL;

	return 0;
}

/**
 * Login-serv destructor
 *  dealloc..., function called at exit of the login-serv
 */
void do_final(void) {
	ShowStatus("Terminating...\n");
#ifdef WEB_SERVER_ENABLE
	http_server->stop();
	svr_thr.join();
	web_sql_close();
#endif
	do_final_msg();
	ShowStatus("Finished.\n");
}

/**
 * Signal handler
 *  This function attempts to properly close the server when an interrupt signal is received.
 *  current signal catch : SIGTERM, SIGINT
 */
void do_shutdown(void) {
	if( runflag != WEBSERVER_ST_SHUTDOWN ) {
		runflag = WEBSERVER_ST_SHUTDOWN;
		ShowStatus("Shutting down...\n");
		runflag = CORE_ST_STOP;
	}
}

/**
 * Signal handler
 *  Function called when the server has received a crash signal.
 *  current signal catch : SIGSEGV, SIGFPE
 */
void do_abort(void) {
#ifdef WEB_SERVER_ENABLE
	http_server->stop();
	svr_thr.join();
#endif
}

/*======================================================
 * Map-Server help options screen
 *------------------------------------------------------*/
void display_helpscreen(bool do_exit)
{
	ShowInfo("Usage: %s\n", SERVER_NAME);
	if( do_exit )
		exit(EXIT_SUCCESS);
}


// Is this still used ??
void set_server_type(void) {
	SERVER_TYPE = ATHENA_SERVER_WEB;
}


// called just before sending repsonse
void logger(const Request & req, const Response & res) {
	// make this a config
	if (web_config.print_req_res) {
		ShowDebug("Incoming Headers are:\n");
		for (const auto & header : req.headers) {
			ShowDebug("\t%s: %s\n", header.first.c_str(), header.second.c_str());
		}
		ShowDebug("Incoming Pages are:\n");
		for (const auto & file : req.files) {
			ShowDebug("\t%s: %s\n", file.first.c_str(), file.second.content.c_str());
		}
		ShowDebug("Outgoing Headers are:\n");
		for (const auto & header : res.headers) {
			ShowDebug("\t%s: %s\n", header.first.c_str(), header.second.c_str());
		}
		ShowDebug("Response status is: %d\n", res.status);
		// since the body may be binary, might not print entire body (has null character).
		ShowDebug("Body is:\n%s\n", res.body.c_str());
	}
	ShowInfo("%s [%s %s] %d\n", req.remote_addr.c_str(), req.method.c_str(), req.path.c_str(), res.status);
}


int do_init(int argc, char** argv) {
	runflag = WEBSERVER_ST_STARTING;
#ifndef WEB_SERVER_ENABLE
	ShowStatus("The web-server is " CL_GREEN "idling" CL_RESET " (PACKETVER too old to use).\n\n");
	return 0;
#else
	INTER_CONF_NAME="conf/inter_athena.conf";

	safestrncpy(console_log_filepath, "./log/web-msg_log.log", sizeof(console_log_filepath));

	// read web-server configuration
	web_set_defaults();
	web_config_read(web_config.webconf_name, true);
	msg_config_read(web_config.msgconf_name);

	inter_config_read(INTER_CONF_NAME);
	// end config

	web_sql_init();

	ShowStatus("Starting server...\n");

	http_server = std::make_shared<httplib::Server>();
	// set up routes
	http_server->Post("/emblem/download", emblem_download);
	http_server->Post("/emblem/upload", emblem_upload);
	http_server->Post("/userconfig/load", userconfig_load);
	http_server->Post("/userconfig/save", userconfig_save);
	http_server->Post("/charconfig/load", charconfig_load);
	http_server->Post("/charconfig/save", charconfig_save);
	http_server->Post("/MerchantStore/load", merchantstore_load);
	http_server->Post("/MerchantStore/save", merchantstore_save);

	// set up logger
	http_server->set_logger(logger);
	shutdown_callback = do_shutdown;

	runflag = WEBSERVER_ST_RUNNING;

	svr_thr = std::thread([] {
		http_server->listen(web_config.web_ip.c_str(), web_config.web_port);
	});

	for (int i = 0; i < 10; i++) {
		if (http_server->is_running() || runflag != WEBSERVER_ST_RUNNING)
			break;
		ShowDebug("Web server not running, sleeping 1 second.\n");
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	if (!http_server->is_running()) {
		ShowError("Web server hasn't started, stopping.\n");
		runflag = CORE_ST_STOP;
		return 0;
	}

	ShowStatus("The web-server is " CL_GREEN "ready" CL_RESET " (Server is listening on the port %u).\n\n", web_config.web_port);
	return 0;
#endif
}
