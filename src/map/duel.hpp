// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#pragma once
#ifndef _DUEL_HPP_
#define _DUEL_HPP_

#include "../common/cbasetypes.h"

struct duel {
	int members_count;
	int invites_count;
	int max_players_limit;
};

duel& duel_get_duelid( size_t did );
bool duel_exist( size_t did );
size_t duel_counttotal();
size_t duel_countactives();

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
