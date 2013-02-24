#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cbasetypes.h"
#include "showmsg.h"
#include "core.h"
#include "cli.h"

char* MAP_CONF_NAME;
char* INTER_CONF_NAME;
char* LOG_CONF_NAME;
char* BATTLE_CONF_FILENAME;
char* ATCOMMAND_CONF_FILENAME;
char* SCRIPT_CONF_NAME;
char* GRF_PATH_FILENAME;
//char
char* CHAR_CONF_NAME;
char* SQL_CONF_NAME;
//login
char* LOGIN_CONF_NAME;
//common
char* LAN_CONF_NAME; //char-login
char* MSG_CONF_NAME; //all

bool opt_has_next_value(const char* option, int i, int argc)
{
    if (i >= argc - 1) {
	ShowWarning("Missing value for option '%s'.\n", option);
	return false;
    }

    return true;
}

/*======================================================
 * Servers Version Screen [MC Cameri]
 *------------------------------------------------------*/
void display_versionscreen(bool do_exit)
{
    ShowInfo(CL_WHITE"rAthena SVN version: %s" CL_RESET"\n", get_svn_revision());
    ShowInfo(CL_GREEN"Website/Forum:"CL_RESET"\thttp://rathena.org/\n");
    ShowInfo(CL_GREEN"IRC Channel:"CL_RESET"\tirc://irc.rathena.net/#rathena\n");
    ShowInfo("Open "CL_WHITE"readme.txt"CL_RESET" for more information.\n");
    if (do_exit)
	exit(EXIT_SUCCESS);
}

int cli_get_options(int argc, char ** argv)
{
    int i = 0;
    for (i = 1; i < argc; i++) {
	const char* arg = argv[i];

	if (arg[0] != '-' && (arg[0] != '/' || arg[1] == '-')) {// -, -- and /
	    ShowError("Unknown option '%s'.\n", argv[i]);
	    exit(EXIT_FAILURE);
	} else if ((++arg)[0] == '-') {// long option
	    arg++;

	    if (strcmp(arg, "help") == 0) {
		display_helpscreen(true);
	    } else if (strcmp(arg, "version") == 0) {
		display_versionscreen(true);
	    } else if (strcmp(arg, "msg-config") == 0) {
		    if (opt_has_next_value(arg, i, argc))
			MSG_CONF_NAME = argv[++i];
	    } else if (strcmp(arg, "run-once") == 0) // close the map-server as soon as its done.. for testing [Celest]
	    {
		runflag = CORE_ST_STOP;
	    } else if (SERVER_TYPE & (ATHENA_SERVER_LOGIN | ATHENA_SERVER_CHAR)) { //login or char
		if (strcmp(arg, "lan-config") == 0) {
		    if (opt_has_next_value(arg, i, argc))
			LAN_CONF_NAME = argv[++i];
		} else if (SERVER_TYPE == ATHENA_SERVER_LOGIN) { //login
		    if (strcmp(arg, "login-config") == 0) {
			if (opt_has_next_value(arg, i, argc))
			    LOGIN_CONF_NAME = argv[++i];
		    } else {
			ShowError("Unknown option '%s'.\n", argv[i]);
			exit(EXIT_FAILURE);
		    }
		} else if (SERVER_TYPE == ATHENA_SERVER_CHAR) { //char
		    if (strcmp(arg, "char-config") == 0) {
			if (opt_has_next_value(arg, i, argc))
			    CHAR_CONF_NAME = argv[++i];
		    } else if (strcmp(arg, "inter-config") == 0) {
			if (opt_has_next_value(arg, i, argc))
			    INTER_CONF_NAME = argv[++i];
		    } else {
			ShowError("Unknown option '%s'.\n", argv[i]);
			exit(EXIT_FAILURE);
		    }
		}
	    } else if (SERVER_TYPE == ATHENA_SERVER_MAP) { //map
		if (strcmp(arg, "map-config") == 0) {
		    if (opt_has_next_value(arg, i, argc))
			MAP_CONF_NAME = argv[++i];
		} else if (strcmp(arg, "battle-config") == 0) {
		    if (opt_has_next_value(arg, i, argc))
			BATTLE_CONF_FILENAME = argv[++i];
		} else if (strcmp(arg, "atcommand-config") == 0) {
		    if (opt_has_next_value(arg, i, argc))
			ATCOMMAND_CONF_FILENAME = argv[++i];
		} else if (strcmp(arg, "script-config") == 0) {
		    if (opt_has_next_value(arg, i, argc))
			SCRIPT_CONF_NAME = argv[++i];
		} else if (strcmp(arg, "grf-path-file") == 0) {
		    if (opt_has_next_value(arg, i, argc))
			GRF_PATH_FILENAME = argv[++i];
		} else if (strcmp(arg, "inter-config") == 0) {
		    if (opt_has_next_value(arg, i, argc))
			INTER_CONF_NAME = argv[++i];
		} else if (strcmp(arg, "log-config") == 0) {
		    if (opt_has_next_value(arg, i, argc))
			LOG_CONF_NAME = argv[++i];
		}
		else {
		    ShowError("Unknown option '%s'.\n", argv[i]);
		    exit(EXIT_FAILURE);
		}
	    }
	} else switch (arg[0]) {// short option
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
    return 1;
}