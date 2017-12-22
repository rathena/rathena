// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _DUEL_HPP_
#define _DUEL_HPP_

#include "../common/cbasetypes.h"

struct duel {
	int members_count;
	int invites_count;
	int max_players_limit;
};

//#define MAX_DUEL 1024 //max number of duels on server
//extern struct duel duel_list[MAX_DUEL]; //list of current duel
//extern int duel_count; //current number of duel on server

duel& duel_GetDuelId( size_t did );
bool duel_exist( size_t did );
size_t duel_Getcount();

//Duel functions // [LuzZza]
size_t duel_create(struct map_session_data* sd, const unsigned int maxpl);
bool duel_invite(const size_t did, struct map_session_data* sd, struct map_session_data* target_sd);
bool duel_accept(const size_t did, struct map_session_data* sd);
bool duel_reject(const size_t did, struct map_session_data* sd);
bool duel_leave(const size_t did, struct map_session_data* sd);
void duel_showinfo(const size_t did, struct map_session_data* sd);
bool duel_checktime(struct map_session_data* sd);
bool duel_check_player_limit( struct duel& pDuel );

void do_init_duel(void);
void do_final_duel(void);


#endif /* _DUEL_HPP_ */
