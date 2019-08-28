// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ATCOMMAND_HPP
#define ATCOMMAND_HPP

#include "../common/cbasetypes.hpp"
#include "../common/mmo.hpp"

struct map_session_data;

//This is the distance at which @autoloot works,
//if the item drops farther from the player than this,
//it will not be autolooted. [Skotlex]
//Note: The range is unlimited unless this define is set.
//#define AUTOLOOT_DISTANCE AREA_SIZE

//global var
extern char atcommand_symbol;
extern char charcommand_symbol;
extern int atcmd_binding_count;

enum AtCommandType : uint8 {
	COMMAND_ATCOMMAND = 1,
	COMMAND_CHARCOMMAND = 2,
} ;

typedef int (*AtCommandFunc)(const int fd, struct map_session_data* sd, const char* command, const char* message);

bool is_atcommand(const int fd, struct map_session_data* sd, const char* message, int type);

void do_init_atcommand(void);
void do_final_atcommand(void);
void atcommand_db_load_groups(int* group_ids);

bool atcommand_exists(const char* name);

// @commands (script based)
struct atcmd_binding_data {
	char command[50];
	char npc_event[EVENT_NAME_LENGTH];
	int level;
	int level2;
};
extern struct atcmd_binding_data** atcmd_binding;
struct atcmd_binding_data* get_atcommandbind_byname(const char* name);

#endif /* ATCOMMAND_HPP */
