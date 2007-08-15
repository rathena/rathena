// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAT_H_
#define _CHAT_H_

//#include "map.h"
struct map_session_data;
struct chat_data;

int chat_createchat(struct map_session_data* sd, int limit, bool pub, char* pass, char* title, int titlelen);
int chat_joinchat(struct map_session_data* sd, int chatid, char* pass);
int chat_leavechat(struct map_session_data* sd);
int chat_changechatowner(struct map_session_data* sd, char* nextownername);
int chat_changechatstatus(struct map_session_data* sd, char* title, char* pass, int limit, bool pub);
int chat_kickchat(struct map_session_data* sd, char* kickusername);

int chat_createnpcchat(struct npc_data* nd, int limit, bool pub, int trigger, const char* title, const char* ev);
int chat_deletenpcchat(struct npc_data* nd);
int chat_enableevent(struct chat_data* cd);
int chat_disableevent(struct chat_data* cd);
int chat_npckickall(struct chat_data* cd);

#endif /* _CHAT_H_ */
