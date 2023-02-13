// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef DUEL_HPP
#define DUEL_HPP

#include <common/cbasetypes.hpp>

class map_session_data;
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
size_t duel_create(map_session_data* sd, const unsigned int maxpl);
bool duel_invite(const size_t did, map_session_data* sd, map_session_data* target_sd);
bool duel_accept(const size_t did, map_session_data* sd);
bool duel_reject(const size_t did, map_session_data* sd);
bool duel_leave(const size_t did, map_session_data* sd);
void duel_showinfo(const size_t did, map_session_data* sd);
bool duel_checktime(map_session_data* sd);
bool duel_check_player_limit( struct duel& pDuel );

void do_init_duel(void);
void do_final_duel(void);

#endif /* DUEL_HPP */
