// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAT_HPP
#define CHAT_HPP

#include "map.hpp" // struct block_list, CHATROOM_TITLE_SIZE
#include <array>

inline constexpr int MAX_CHAT_USERS = 20;
namespace chats{

class ChatData {
public:
	block_list bl;            // data for this map object
	char title[CHATROOM_TITLE_SIZE]; // room title 
	char pass[CHATROOM_PASS_SIZE];   // password
	bool pub;                        // private/public flag
	uint32 trigger;                   // number of users needed to trigger event
	uint32 users;                     // current user count
	uint32 limit;                     // join limit
	uint32 zeny;						 // required zeny to join
	uint32 min_level;					 // minimum base level to join
	uint32 max_level;					 // maximum base level allowed to join
	std::array<map_session_data*,MAX_CHAT_USERS> usersd;
	block_list* owner;
	char npc_event[EVENT_NAME_LENGTH];
	DBMap* kick_list;				//DBMap of users who were kicked from this chat

	void TriggerEvent();
	void EnableEvent();
	void DisableEvent();
	void NpcKickAll();
	void NpcKickChat(const char* kick_user_name);

	static ChatData* CreateChat(block_list* bl, const char* title, const char* pass, int limit, bool pub, int trigger, const char* ev, int zeny, int min_level, int max_level);

};
void JoinChat(map_session_data* sd, int chatid, const char* pass);
void CreatePcChat(map_session_data* sd, const char* title, const char* pass, int limit, bool pub);
void LeaveChat(map_session_data* sd, bool kicked);
void ChangeChatOwner(const map_session_data* sd, const char* nextownername);
void ChangeChatStatus(const map_session_data* sd, const char* title, const char* pass, int limit, bool pub);
void KickPcFromChat(map_session_data* sd, const char* kickusername);
void CreateNpcChat(npc_data* nd, const char* title, int limit, bool pub, int trigger, const char* ev, int zeny, int minLvl, int maxLvl);
void DeleteNpcChat(npc_data* nd);

}
#endif /* CHAT_HPP */
