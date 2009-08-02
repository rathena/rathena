// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _QUEST_H_
#define _QUEST_H_

typedef enum quest_check_type { HAVEQUEST, PLAYTIME, HUNTING } quest_check_type;

int quest_pc_login(TBL_PC * sd);

int quest_add(TBL_PC * sd, int quest_id);
int quest_delete(TBL_PC * sd, int quest_id);
int quest_change(TBL_PC * sd, int qid1, int qid2);
void quest_update_objective(TBL_PC * sd, int mob);
int quest_update_status(TBL_PC * sd, int quest_id, quest_state status);
int quest_check(TBL_PC * sd, int quest_id, quest_check_type type);

void do_init_quest();

#endif
