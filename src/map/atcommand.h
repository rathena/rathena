// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ATCOMMAND_H_
#define _ATCOMMAND_H_

struct map_session_data;

//This is the distance at which @autoloot works,
//if the item drops farther from the player than this,
//it will not be autolooted. [Skotlex]
//Note: The range is unlimited unless this define is set.
//#define AUTOLOOT_DISTANCE AREA_SIZE

extern char atcommand_symbol;
extern char charcommand_symbol;

typedef enum {
	COMMAND_ATCOMMAND = 1,
	COMMAND_CHARCOMMAND = 2,
} AtCommandType;

typedef int (*AtCommandFunc)(const int fd, struct map_session_data* sd, const char* command, const char* message);

bool is_atcommand(const int fd, struct map_session_data* sd, const char* message, int type);

void do_init_atcommand(void);
void do_final_atcommand(void);

bool atcommand_exists(const char* name);

const char* msg_txt(int msg_number);
int msg_config_read(const char* cfgName);
void do_final_msg(void);

#endif /* _ATCOMMAND_H_ */
