// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _QUEST_H_
#define _QUEST_H_

#define MAX_QUEST_DB (62238 + 1) // Highest quest ID + 1

struct quest_db {
	int id;
	unsigned int time;
	int mob[MAX_QUEST_OBJECTIVES];
	int count[MAX_QUEST_OBJECTIVES];
	int num_objectives;
	//char name[NAME_LENGTH];
};

struct quest_db *quest_db_data[MAX_QUEST_DB];	///< Quest database
struct quest_db quest_dummy;					///< Dummy entry for invalid quest lookups

// Questlog check types
enum quest_check_type {
	HAVEQUEST, ///< Query the state of the given quest
	PLAYTIME,  ///< Check if the given quest has been completed or has yet to expire
	HUNTING,   ///< Check if the given hunting quest's requirements have been met
};

int quest_pc_login(TBL_PC *sd);

int quest_add(TBL_PC * sd, int quest_id);
int quest_delete(TBL_PC * sd, int quest_id);
int quest_change(TBL_PC * sd, int qid1, int qid2);
int quest_update_objective_sub(struct block_list *bl, va_list ap);
void quest_update_objective(TBL_PC * sd, int mob);
int quest_update_status(TBL_PC * sd, int quest_id, enum quest_state status);
int quest_check(TBL_PC * sd, int quest_id, enum quest_check_type type);
void quest_clear(void);

struct quest_db *quest_db(int quest_id);

void do_init_quest(void);
void do_final_quest(void);
void do_reload_quest(void);

#endif
