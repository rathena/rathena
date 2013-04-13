/*
 * File:   cli.h
 * Author: lighta
 *
 * Created on February 21, 2013, 6:15 PM
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
int cli_get_options(int argc, char ** argv);
int parse_console_timer(int tid, unsigned int tick, int id, intptr_t data);
extern int parse_console(const char* buf); //particular for each serv

#ifdef	__cplusplus
}
#endif

#endif	/* CLI_H */

