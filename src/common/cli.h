/**
 * @file cli.h
 * Module purpose is to handle the console (cli=console line input) while the servers launch and run.
 *  This contains functions common to all servers, but then dispatches them to a specific parser on each server.
 * Licensed under GNU GPL.
 *  For more information, see LICENCE in the main folder.
 * @author rAthena Dev Team
 */

#ifndef CLI_H
#define	CLI_H

#ifdef	__cplusplus
extern "C" {
#endif

#define MAX_CONSOLE_IN 200 //max is map...
#define MIN_CONSOLE_IN 4 //min is help
//map
 extern char* MAP_CONF_NAME;
 extern char* INTER_CONF_NAME;
 extern char* LOG_CONF_NAME;
 extern char* BATTLE_CONF_FILENAME;
 extern char* ATCOMMAND_CONF_FILENAME;
 extern char* SCRIPT_CONF_NAME;
 extern char* GRF_PATH_FILENAME;
//char
 extern char* CHAR_CONF_NAME;
 extern char* SQL_CONF_NAME;
//login
 extern char* LOGIN_CONF_NAME;
//common
 extern char* LAN_CONF_NAME; //char-login
 extern char* MSG_CONF_NAME_EN; //all

extern void display_helpscreen(bool exit);
bool cli_hasevent();
void display_versionscreen(bool do_exit);
bool opt_has_next_value(const char* option, int i, int argc);
int cli_get_options(int argc, char ** argv);
int parse_console_timer(int tid, unsigned int tick, int id, intptr_t data);
extern int parse_console(const char* buf); //particular for each serv

#ifdef	__cplusplus
}
#endif

#endif	/* CLI_H */

