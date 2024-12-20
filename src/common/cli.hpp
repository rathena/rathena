// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CLI_HPP
#define CLI_HPP

#include "cbasetypes.hpp"
#include "timer.hpp"

#define MAX_CONSOLE_IN 200 //max is map...
#define MIN_CONSOLE_IN 4 //min is help
//map
 extern const char* MAP_CONF_NAME;
 extern const char* INTER_CONF_NAME;
 extern const char* LOG_CONF_NAME;
 extern const char* BATTLE_CONF_FILENAME;
 extern const char* ATCOMMAND_CONF_FILENAME;
 extern const char* SCRIPT_CONF_NAME;
 extern const char* GRF_PATH_FILENAME;
//char
 extern const char* CHAR_CONF_NAME;
//login
 extern const char* LOGIN_CONF_NAME;
 extern const char *LOGIN_MSG_CONF_NAME;
//common
 extern const char* LAN_CONF_NAME; //char-login
 extern const char* MSG_CONF_NAME_EN; //all

extern void display_helpscreen(bool exit);
bool cli_hasevent();
void display_versionscreen(bool do_exit);
bool opt_has_next_value(const char* option, int32 i, int32 argc);
int32 cli_get_options(int32 argc, char ** argv);
TIMER_FUNC(parse_console_timer);
extern int32 parse_console(const char* buf); //particular for each serv

#endif /* CLI_HPP */
