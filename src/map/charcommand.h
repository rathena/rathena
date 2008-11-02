// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARCOMMAND_H_
#define _CHARCOMMAND_H_

//#include "map.h"
struct map_session_data;

extern char charcommand_symbol;
typedef int (*CharCommandFunc)(const int fd, struct map_session_data* sd, const char* command, const char* message);

bool is_charcommand(const int fd, struct map_session_data* sd, const char* message);
bool is_charcommand_sub(const int fd, struct map_session_data* sd, const char* str, int gmlvl);
int get_charcommand_level(const CharCommandFunc func);

int charcommand_config_read(const char* cfgName);

#endif /* _CHARCOMMAND_H_ */
