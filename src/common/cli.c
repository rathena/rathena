#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef WIN32
	#include <conio.h>
#else
	#include <sys/poll.h>
#endif

#include "cbasetypes.h"
#include "showmsg.h"
#include "core.h"
#include "cli.h"

//map confs
char* MAP_CONF_NAME;
char* INTER_CONF_NAME;
char* LOG_CONF_NAME;
char* BATTLE_CONF_FILENAME;
char* ATCOMMAND_CONF_FILENAME;
char* SCRIPT_CONF_NAME;
char* GRF_PATH_FILENAME;
//char confs
char* CHAR_CONF_NAME;
char* SQL_CONF_NAME;
//login confs
char* LOGIN_CONF_NAME;
//common conf (used by multiple serv)
char* LAN_CONF_NAME; //char-login
char* MSG_CONF_NAME_EN; //all

/*
 * Function to check if specified option have an argument follow it
 * return
 *   false : no other args found, and throw a warning
 *   true : something following us
 */
bool opt_has_next_value(const char* option, int i, int argc)
{
    if (i >= argc - 1) {
	ShowWarning("Missing value for option '%s'.\n", option);
	return false;
    }

    return true;
}

/*
 * Display some info about emulator such as
 *   svn version
 *   website/forum adresse
 *   irc hangout
 */
void display_versionscreen(bool do_exit)
{
    ShowInfo(CL_WHITE"rAthena SVN version: %s" CL_RESET"\n", get_svn_revision());
    ShowInfo(CL_GREEN"Website/Forum:"CL_RESET"\thttp://rathena.org/\n");
    ShowInfo(CL_GREEN"IRC Channel:"CL_RESET"\tirc://irc.rizon.net/#rathena\n");
    ShowInfo("Open "CL_WHITE"readme.txt"CL_RESET" for more information.\n");
    if (do_exit)
	exit(EXIT_SUCCESS);
}

/*
 * Read the option specify in command line
 * and assign the confs used by the different server
 * exit on failure or return true
 */
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
			MSG_CONF_NAME_EN = argv[++i];
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

int cli_hasevent(){
#ifdef WIN32
	return _kbhit();
#else
	struct pollfd fds;
	fds.fd = 0; /* this is STDIN */
	fds.events = POLLIN;
	return poll(&fds, 1, 0);
#endif
}

int parse_console_timer(int tid, unsigned int tick, int id, intptr_t data) {
	char buf[MAX_CONSOLE_IN]; //max cmd atm is 63+63+63+3+3

	memset(buf,0,MAX_CONSOLE_IN); //clear out buf

	if(cli_hasevent()){
		if(fgets(buf, MAX_CONSOLE_IN, stdin)==NULL)
			return -1;
		else if(strlen(buf)>MIN_CONSOLE_IN)
			parse_console(buf);
	}
	return 0;
}

