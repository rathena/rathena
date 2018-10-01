// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef QUEST_HPP
#define QUEST_HPP

#include "../common/cbasetypes.hpp"
#include "../common/strlib.hpp"

#include "map.hpp"

struct map_session_data;

struct quest_dropitem {
	uint16 nameid;
	uint16 count;
	uint16 rate;
	uint16 mob_id;
	//uint8 bound;
	//bool isAnnounced;
	//bool isGUID;
};

struct quest_objective {
	uint16 mob;
	uint16 count;
};

struct quest_db {
	// TODO: find out if signed or unsigned in client
	int id;
	unsigned int time;
	bool time_type;
	uint8 objectives_count;
	struct quest_objective *objectives;
	uint8 dropitem_count;
	struct quest_dropitem *dropitem;
	StringBuf name;
};

extern struct quest_db quest_dummy;	///< Dummy entry for invalid quest lookups

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
void quest_update_objective(TBL_PC * sd, int mob_id);
int quest_update_status(TBL_PC * sd, int quest_id, enum quest_state status);
int quest_check(TBL_PC * sd, int quest_id, enum quest_check_type type);

struct quest_db *quest_search(int quest_id);

void do_init_quest(void);
void do_final_quest(void);
void do_reload_quest(void);

#endif /* QUEST_HPP */
