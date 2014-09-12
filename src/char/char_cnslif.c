// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/socket.h"
#include "../common/showmsg.h"
#include "../common/timer.h"
#include "../common/ers.h"
#include "../common/cli.h"
#include "char.h"
#include "char_cnslif.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*======================================================
 * Login-Server help option info
 *------------------------------------------------------*/
void display_helpscreen(bool do_exit)
{
	ShowInfo("Usage: %s [options]\n", SERVER_NAME);
	ShowInfo("\n");
	ShowInfo("Options:\n");
	ShowInfo("  -?, -h [--help]\t\tDisplays this help screen.\n");
	ShowInfo("  -v [--version]\t\tDisplays the server's version.\n");
	ShowInfo("  --run-once\t\t\tCloses server after loading (testing).\n");
	ShowInfo("  --char-config <file>\t\tAlternative char-server configuration.\n");
	ShowInfo("  --lan-config <file>\t\tAlternative lag configuration.\n");
	ShowInfo("  --inter-config <file>\t\tAlternative inter-server configuration.\n");
	ShowInfo("  --msg-config <file>\t\tAlternative message configuration.\n");
	if( do_exit )
		exit(EXIT_SUCCESS);
}

/**
 * Timered function to check if the console has a new event to be read.
 * @param tid: timer id
 * @param tick: tick of execution
 * @param id: user account id
 * @param data: unused
 * @return 0
 */
int cnslif_console_timer(int tid, unsigned int tick, int id, intptr_t data) {
	char buf[MAX_CONSOLE_IN]; //max cmd atm is 63+63+63+3+3

	memset(buf,0,MAX_CONSOLE_IN); //clear out buf

	if(cli_hasevent()){
		if(fgets(buf, MAX_CONSOLE_IN, stdin)==NULL)
			return -1;
		else if(strlen(buf)>MIN_CONSOLE_IN)
			cnslif_parse(buf);
	}
	return 0;
}

// Console Command Parser [Wizputer]
int cnslif_parse(const char* buf)
{
	char type[64];
	char command[64];
	int n=0;

	if( ( n = sscanf(buf, "%63[^:]:%63[^\n]", type, command) ) < 2 ){
		if((n = sscanf(buf, "%63[^\n]", type))<1) return -1; //nothing to do no arg
	}
	if( n != 2 ){ //end string
		ShowNotice("Type: '%s'\n",type);
		command[0] = '\0';
	}
	else
		ShowNotice("Type of command: '%s' || Command: '%s'\n",type,command);

	if( n == 2 && strcmpi("server", type) == 0 ){
		if( strcmpi("shutdown", command) == 0 || strcmpi("exit", command) == 0 || strcmpi("quit", command) == 0 ){
			runflag = 0;
		}
		else if( strcmpi("alive", command) == 0 || strcmpi("status", command) == 0 )
			ShowInfo(CL_CYAN"Console: "CL_BOLD"I'm Alive."CL_RESET"\n");
		else if( strcmpi("reloadconf", command) == 0 ) {
			ShowInfo("Reloading config file \"%s\"\n", CHAR_CONF_NAME);
			char_config_read(CHAR_CONF_NAME, false);
		}
	}
	else if( strcmpi("ers_report", type) == 0 ){
		ers_report();
	}
	else if( strcmpi("help", type) == 0 ){
		ShowInfo("Available commands:\n");
		ShowInfo("\t server:shutdown => Stops the server.\n");
		ShowInfo("\t server:alive => Checks if the server is running.\n");
		ShowInfo("\t server:reloadconf => Reload config file: \"%s\"\n", CHAR_CONF_NAME);
		ShowInfo("\t ers_report => Displays database usage.\n");
	}

	return 0;
}

void do_init_chcnslif(void){
	if( charserv_config.console ){ //start listening
		add_timer_func_list(cnslif_console_timer, "cnslif_console_timer");
		add_timer_interval(gettick()+1000, cnslif_console_timer, 0, 0, 1000); //start in 1s each 1sec
	}
}
