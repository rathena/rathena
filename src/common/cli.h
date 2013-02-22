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
 extern char* MSG_CONF_NAME; //all

extern void display_helpscreen(bool exit);
int cli_get_options(int argc, char ** argv);

#ifdef	__cplusplus
}
#endif

#endif	/* CLI_H */

