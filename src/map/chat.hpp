// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAT_HPP
#define CHAT_HPP

#include "map.hpp" // block_list, CHATROOM_TITLE_SIZE

class map_session_data;

#define MAX_CHAT_USERS 20

struct chat_data : public block_list {
	char title[CHATROOM_TITLE_SIZE]; // room title 
	char pass[CHATROOM_PASS_SIZE];   // password
	bool pub;                        // private/public flag
	uint8 users;                     // current user count
	uint8 limit;                     // join limit
	uint8 trigger;                   // number of users needed to trigger event
	uint32 zeny;						 // required zeny to join
	uint32 minLvl;					 // minimum base level to join
	uint32 maxLvl;					 // maximum base level allowed to join
	map_session_data* usersd[MAX_CHAT_USERS];
	block_list* owner;
	char npc_event[EVENT_NAME_LENGTH];
	DBMap* kick_list;				//DBMap of users who were kicked from this chat
};

int32 chat_createpcchat(map_session_data* sd, const char* title, const char* pass, int32 limit, bool pub);
int32 chat_joinchat(map_session_data* sd, int32 chatid, const char* pass);
int32 chat_leavechat(map_session_data* sd, bool kicked);
int32 chat_changechatowner(map_session_data* sd, const char* nextownername);
int32 chat_changechatstatus(map_session_data* sd, const char* title, const char* pass, int32 limit, bool pub);
int32 chat_kickchat(map_session_data* sd, const char* kickusername);

int32 chat_createnpcchat(npc_data* nd, const char* title, int32 limit, bool pub, int32 trigger, const char* ev, int32 zeny, int32 minLvl, int32 maxLvl);
int32 chat_deletenpcchat(npc_data* nd);
int32 chat_enableevent(chat_data* cd);
int32 chat_disableevent(chat_data* cd);
int32 chat_npckickall(chat_data* cd);

int32 chat_npckickchat(chat_data* cd, const char* kickusername);

#endif /* CHAT_HPP */
