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
void atcommand_db_load_groups(int* group_ids);

bool atcommand_exists(const char* name);

const char* msg_txt(int msg_number);
int msg_config_read(const char* cfgName);
void do_final_msg(void);

#define MAX_ATCMD_BINDINGS 100

// @commands (script based)
typedef struct Atcmd_Binding {
	char command[50];
	char npc_event[50];
	int level;
	int level2;
} Atcmd_Binding;

struct Atcmd_Binding atcmd_binding[MAX_ATCMD_BINDINGS];
struct Atcmd_Binding* get_atcommandbind_byname(const char* name);

#endif /* _ATCOMMAND_H_ */
