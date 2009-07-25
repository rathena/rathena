// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _QUEST_H_
#define _QUEST_H_

typedef enum quest_check_type { HAVEQUEST, PLAYTIME, HUNTING } quest_check_type;

struct s_quest_db {
	int id;
	unsigned int time;
	int mob[MAX_QUEST_OBJECTIVES];
	int count[MAX_QUEST_OBJECTIVES];
	//char name[NAME_LENGTH];
};
struct s_quest_db quest_db[MAX_QUEST_DB];

int quest_pc_login(TBL_PC * sd);

int quest_add(TBL_PC * sd, int quest_id);
int quest_add_ack(int char_id, int quest_id, int success);

int quest_delete(TBL_PC * sd, int quest_id);
int quest_delete_ack(int char_id, int quest_id, int success);

void quest_update_objective(TBL_PC * sd, int mob);
int quest_update_status(TBL_PC * sd, int quest_id, int status);
int quest_save(TBL_PC * sd);
int quest_save_ack(int char_id, int quest_id, int success);

int quest_check_quest(TBL_PC * sd, int quest_id, quest_check_type type);

int quest_search_db(int quest_id);

void do_init_quest();

#endif
