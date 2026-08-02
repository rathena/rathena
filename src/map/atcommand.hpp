// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ATCOMMAND_HPP
#define ATCOMMAND_HPP

#include <string>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

class map_session_data;

//global var
extern char atcommand_symbol;
extern char charcommand_symbol;
extern int32 atcmd_binding_count;

enum AtCommandType : uint8 {
	COMMAND_ATCOMMAND = 1,
	COMMAND_CHARCOMMAND = 2,
} ;

typedef int32 (*AtCommandFunc)(const int32 fd, map_session_data* sd, const char* command, const char* message);

bool is_atcommand(const int32 fd, map_session_data* sd, const char* message, int32 type);

void do_init_atcommand(void);
void do_final_atcommand(void);
void atcommand_db_load_groups();

bool atcommand_exists(const char* name);
const char* atcommand_alias_lookup( const std::string& cmd );

// @commands (script based)
struct atcmd_binding_data {
	char command[50];
	char npc_event[EVENT_NAME_LENGTH];
	int32 level;
	int32 level2;
};
extern struct atcmd_binding_data** atcmd_binding;
struct atcmd_binding_data* get_atcommandbind_byname(const char* name);

#endif /* ATCOMMAND_HPP */
