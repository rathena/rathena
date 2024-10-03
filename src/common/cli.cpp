// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cli.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef WIN32
	#include <conio.h>
#else
	#include <poll.h>
#endif

#include "cbasetypes.hpp"
#include "core.hpp"
#include "showmsg.hpp"
#include "timer.hpp"

using namespace rathena::server_core;

//map confs
const char* MAP_CONF_NAME = "conf/map_athena.conf";
const char* INTER_CONF_NAME = "conf/inter_athena.conf";
const char* LOG_CONF_NAME = "conf/log_athena.conf";
const char* BATTLE_CONF_FILENAME = "conf/battle_athena.conf";
const char* SCRIPT_CONF_NAME = "conf/script_athena.conf";
const char* GRF_PATH_FILENAME = "conf/grf-files.txt";
//char confs
const char* CHAR_CONF_NAME = "conf/char_athena.conf";
//login confs
const char* LOGIN_CONF_NAME = "conf/login_athena.conf";
const char *LOGIN_MSG_CONF_NAME = "conf/msg_conf/login_msg.conf";
//common conf (used by multiple serv)
const char* LAN_CONF_NAME = "conf/subnet_athena.conf"; //char-login
const char* MSG_CONF_NAME_EN = "conf/msg_conf/char_msg.conf"; //all

/**
 * Function to check if the specified option has an argument following it.
 * @param option: actual args string
 * @param i: index of current args
 * @param argc: arguments count
 * @return
 *   false : no other args found, and throw a warning
 *   true : something following us
 */
bool opt_has_next_value(const char* option, int i, int argc){
	if (i >= argc - 1) {
		ShowWarning("Missing value for option '%s'.\n", option);
		return false;
	}

	return true;
}

/**
 * Display some information about the emulator, such as:
 *   svn version
 *   website/forum address
 *   irc hangout
 * @param do_exit: terminate execution ?
 */
void display_versionscreen(bool do_exit)
{
	const char* svn = get_svn_revision();
	if( svn[0] != UNKNOWN_VERSION )
		ShowInfo("rAthena SVN Revision: '" CL_WHITE "%s" CL_RESET "'\n", svn);
	else {
		const char* git = get_git_hash();
		if( git[0] != UNKNOWN_VERSION )
			ShowInfo("rAthena Git Hash: '" CL_WHITE "%s" CL_RESET "'\n", git);
	}
	ShowInfo(CL_GREEN "Website/Forum:" CL_RESET "\thttp://rathena.org/\n");
	ShowInfo("Open " CL_WHITE "README.md" CL_RESET " for more information.\n");
	if (do_exit)
		exit(EXIT_SUCCESS);
}

/**
 * Read the option specified in the command line
 * and assign the confs used by the different servers.
 * @TODO remove and place into csnlif of different serv
 * @param argc: arguments count (from main)
 * @param argv: arguments values (from main)
 * @return true or exit on failure
 */
int cli_get_options(int argc, char ** argv) {
	int i = 0;
	for (i = 1; i < argc; i++) {
		const char* arg = argv[i];

		// to temporarily support mapgenerator options
		if (!arg)
			continue;

		if (arg[0] != '-' && (arg[0] != '/' || arg[1] == '-')) {// -, -- and /
			ShowError("Unknown option '%s'.\n", argv[i]);
			exit(EXIT_FAILURE);
		}
		else if ((++arg)[0] == '-') {// long option
			arg++;

			if (strcmp(arg, "help") == 0) {
				display_helpscreen(true);
			}
			else if (strcmp(arg, "version") == 0) {
				display_versionscreen(true);
			}
			else if (strcmp(arg, "msg-config") == 0) {
				if (opt_has_next_value(arg, i, argc))
					MSG_CONF_NAME_EN = argv[++i];
			}
			else if (strcmp(arg, "run-once") == 0) { // close the map-server as soon as its done.. for testing [Celest]
				global_core->set_run_once( true );
			}else if( global_core->get_type() == e_core_type::LOGIN || global_core->get_type() == e_core_type::CHARACTER ){
				if (strcmp(arg, "lan-config") == 0) {
					if (opt_has_next_value(arg, i, argc))
						LAN_CONF_NAME = argv[++i];
				}else if( global_core->get_type() == e_core_type::LOGIN ){
					if (strcmp(arg, "login-config") == 0) {
						if (opt_has_next_value(arg, i, argc))
							LOGIN_CONF_NAME = argv[++i];
					}
					else {
						ShowError("Unknown option '%s'.\n", argv[i]);
						exit(EXIT_FAILURE);
					}
				}else if( global_core->get_type() == e_core_type::CHARACTER ){
					if (strcmp(arg, "char-config") == 0) {
						if (opt_has_next_value(arg, i, argc))
							CHAR_CONF_NAME = argv[++i];
					}
					else if (strcmp(arg, "inter-config") == 0) {
						if (opt_has_next_value(arg, i, argc))
							INTER_CONF_NAME = argv[++i];
					}
					else {
						ShowError("Unknown option '%s'.\n", argv[i]);
						exit(EXIT_FAILURE);
					}
				}
			}else if( global_core->get_type() == e_core_type::MAP ){
				if (strcmp(arg, "map-config") == 0) {
					if (opt_has_next_value(arg, i, argc))
						MAP_CONF_NAME = argv[++i];
				}
				else if (strcmp(arg, "battle-config") == 0) {
					if (opt_has_next_value(arg, i, argc))
						BATTLE_CONF_FILENAME = argv[++i];
				}
				else if (strcmp(arg, "script-config") == 0) {
					if (opt_has_next_value(arg, i, argc))
						SCRIPT_CONF_NAME = argv[++i];
				}
				else if (strcmp(arg, "grf-path-file") == 0) {
					if (opt_has_next_value(arg, i, argc))
						GRF_PATH_FILENAME = argv[++i];
				}
				else if (strcmp(arg, "inter-config") == 0) {
					if (opt_has_next_value(arg, i, argc))
						INTER_CONF_NAME = argv[++i];
				}
				else if (strcmp(arg, "log-config") == 0) {
					if (opt_has_next_value(arg, i, argc))
						LOG_CONF_NAME = argv[++i];
				}
				else {
					ShowError("Unknown option '%s'.\n", argv[i]);
					exit(EXIT_FAILURE);
				}
			}
		}
		else {
			switch (arg[0]) {// short option
				case '?':
				case 'h':
					display_helpscreen(true);
					break;
				case 'v':
					display_versionscreen(true);
					break;
				default:
					ShowError("Unknown option '%s'.\n", argv[i]);
					exit(EXIT_FAILURE);
				}
		}
	}
	return 1;
}

/**
 * Detect if the console has some input to be read.
 * @return true if event, else false
 */
bool cli_hasevent(){
#ifdef WIN32
	return (_kbhit()!=0);
#else
	struct pollfd fds;
	fds.fd = 0; /* this is STDIN */
	fds.events = POLLIN;
	return (poll(&fds, 1, 0)>0);
#endif
}

/**
 * Timered function to check if the console has a new event to be read.
 * @param tid: timer id
 * @param tick: tick of execution
 * @param id: user account id
 * @param data: unused
 * @return 0
 */
TIMER_FUNC(parse_console_timer){
	char buf[MAX_CONSOLE_IN]; //max cmd atm is 63+63+63+3+3

	memset(buf,0,MAX_CONSOLE_IN); //clear out buf

	if(cli_hasevent()){
		if(fgets(buf, MAX_CONSOLE_IN, stdin)==nullptr)
			return -1;
		else if(strlen(buf)>MIN_CONSOLE_IN)
			parse_console(buf);
	}
	return 0;
}

