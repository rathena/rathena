// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _PC_GROUPS_H_
#define _PC_GROUPS_H_

#include "atcommand.h" // AtCommandType

extern int pc_group_max;

bool pc_group_exists(int group_id);
bool pc_group_can_use_command(int group_id, const char *command, AtCommandType type);
bool pc_group_has_permission(int group_id, int permission);
bool pc_group_should_log_commands(int group_id);
const char* pc_group_id2name(int group_id);
int pc_group_id2level(int group_id);
void pc_group_pc_load(struct map_session_data *);

void do_init_pc_groups(void);
void do_final_pc_groups(void);
void pc_groups_reload(void);

#endif // _PC_GROUPS_H_
