// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _BATTLEGROUND_HPP_
#define _BATTLEGROUND_HPP_

#include "../common/cbasetypes.h"
#include "../common/mmo.h" // struct party

#define MAX_BG_MEMBERS 30

struct battleground_member_data {
	unsigned short x, y;
	struct map_session_data *sd;
	unsigned afk : 1;
};

struct battleground_data {
	unsigned int bg_id;
	unsigned char count;
	struct battleground_member_data members[MAX_BG_MEMBERS];
	// BG Cementery
	unsigned short mapindex, x, y;
	// Logout Event
	char logout_event[EVENT_NAME_LENGTH];
	char die_event[EVENT_NAME_LENGTH];
};

void do_init_battleground(void);
void do_final_battleground(void);

struct battleground_data* bg_team_search(int bg_id);
int bg_send_dot_remove(struct map_session_data *sd);
int bg_team_get_id(struct block_list *bl);
struct map_session_data* bg_getavailablesd(struct battleground_data *bg);

int bg_create(unsigned short mapindex, short rx, short ry, const char *ev, const char *dev);
int bg_team_join(int bg_id, struct map_session_data *sd);
int bg_team_delete(int bg_id);
int bg_team_leave(struct map_session_data *sd, int flag);
int bg_team_warp(int bg_id, unsigned short mapindex, short x, short y);
int bg_member_respawn(struct map_session_data *sd);
int bg_send_message(struct map_session_data *sd, const char *mes, int len);

#endif /* _BATTLEGROUND_HPP_ */
