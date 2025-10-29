// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "logincnslif.hpp"

#include <cstdlib>
#include <cstring>

#include <common/cli.hpp>
#include <common/md5calc.hpp>
#include <common/mmo.hpp> //cbasetype + NAME_LENGTH
#include <common/showmsg.hpp> //show notice
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "login.hpp"

/**
 * Login-server console help: starting option info.
 *  Do not rename function used as extern.
 * @param do_exit: terminate program execution ?
 */
void display_helpscreen(bool do_exit) {
	ShowInfo("Usage: %s [options]\n", SERVER_NAME);
	ShowInfo("\n");
	ShowInfo("Options:\n");
	ShowInfo("  -?, -h [--help]\t\tDisplays this help screen.\n");
	ShowInfo("  -v [--version]\t\tDisplays the server's version.\n");
	ShowInfo("  --run-once\t\t\tCloses server after loading (testing).\n");
	ShowInfo("  --login-config <file>\t\tAlternative login-server configuration.\n");
	ShowInfo("  --lan-config <file>\t\tAlternative lan configuration.\n");
	ShowInfo("  --msg-config <file>\t\tAlternative message configuration.\n");
	if( do_exit )
		exit(EXIT_SUCCESS);
}

/**
 * Console Command Parser
 * Transmited from command cli.cpp
 * note common name for all serv do not rename (extern in cli)
 * @author [Wizputer]
 * @param buf: buffer to parse, (from console)
 * @return 1=success
 */
int32 cnslif_parse(const char* buf){
	char type[64];
	char command[64];
	int32 n=0;

	if( ( n = sscanf(buf, "%127[^:]:%255[^\n\r]", type, command) ) < 2 ){
		if((n = sscanf(buf, "%63[^\n]", type))<1) return -1; //nothing to do no arg
	}
	if( n != 2 ){ //end string
		ShowNotice("Type: '%s'\n",type);
		command[0] = '\0';
	}
	else
		ShowNotice("Type of command: '%s' || Command: '%s'\n",type,command);

	if( n == 2 ){
		if(strcmpi("server", type) == 0 ){
			if( strcmpi("shutdown", command) == 0 || strcmpi("exit", command) == 0 || strcmpi("quit", command) == 0 ){
				global_core->signal_shutdown();
			}
			else if( strcmpi("alive", command) == 0 || strcmpi("status", command) == 0 )
				ShowInfo(CL_CYAN "Console: " CL_BOLD "I'm Alive." CL_RESET"\n");
			else if( strcmpi("reloadconf", command) == 0 ) {
				ShowInfo("Reloading config file \"%s\"\n", LOGIN_CONF_NAME);
				login_config_read(LOGIN_CONF_NAME, false);
			}
		}
		if( strcmpi("create",type) == 0 )
		{
			char username[NAME_LENGTH], password[NAME_LENGTH], md5password[32+1], sex; //23+1 plaintext 32+1 md5
			bool md5 = 0;
			if( sscanf(command, "%23s %23s %c", username, password, &sex) < 3 || strnlen(username, sizeof(username)) < 4 || strnlen(password, sizeof(password)) < 1 ){
				ShowWarning("Console: Invalid parameters for '%s'. Usage: %s <username> <password> <sex:F/M>\n", type, type);
				return 0;
			}
			if( login_config.use_md5_passwds ){
				MD5_String(password,md5password);
				md5 = 1;
			}
			if( login_mmo_auth_new(username,(md5?md5password:password), TOUPPER(sex), "0.0.0.0") != -1 ){
				ShowError("Console: Account creation failed.\n");
				return 0;
			}
			ShowStatus("Console: Account '%s' created successfully.\n", username);
		}
	}
	else if( strcmpi("help", type) == 0 ){
		ShowInfo("Available commands:\n");
		ShowInfo("\t server:shutdown => Stops the server.\n");
		ShowInfo("\t server:alive => Checks if the server is running.\n");
		ShowInfo("\t server:reloadconf => Reload config file: \"%s\"\n", LOGIN_CONF_NAME);
		ShowInfo("\t create:<username> <password> <sex:M|F> => Creates a new account.\n");
	}
	return 1;
}

/**
 * Initialize the module.
 * Launched at login-serv start, create db or other long scope variable here.
 */
void do_init_logincnslif(void){
	if( login_config.console ) {
		add_timer_func_list(parse_console_timer, "parse_console_timer");
		add_timer_interval(gettick()+1000, parse_console_timer, 0, 0, 1000); //start in 1s each 1sec
	}
}

/**
 * Handler to cleanup module, called when login-server stops.
 */
void do_final_logincnslif(void){
	return;
}
