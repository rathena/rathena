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

#define MAX_MSG 1500
#define msg_txt(sd, msg_number) atcommand_msgsd((sd), (msg_number))

//global var
extern char atcommand_symbol;
extern char charcommand_symbol;
extern int atcmd_binding_count;

/**
 * msg_table[lang_id][msg_id]
 **/
extern char*** msg_table;
extern uint8 max_message_table;

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

// @commands (script based)
struct atcmd_binding_data {
	char command[50];
	char npc_event[50];
	int level;
	int level2;
};
struct atcmd_binding_data** atcmd_binding;
struct atcmd_binding_data* get_atcommandbind_byname(const char* name);

const char* atcommand_msg(int msg_number);
const char* atcommand_msgsd(struct map_session_data *sd, int msg_number);
void atcommand_expand_message_table(void);
void atcommand_msg_set(uint8 lang_id, uint16 num, char *ptr);

bool msg_config_read(const char *cfg_name, bool allow_override);
void do_final_msg(void);

#endif /* _ATCOMMAND_H_ */
