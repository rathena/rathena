// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAT_H_
#define _CHAT_H_

//#include "map.h"
struct map_session_data;
struct chat_data;

int chat_createpcchat(struct map_session_data* sd, const char* title, const char* pass, int limit, bool pub);
int chat_joinchat(struct map_session_data* sd, int chatid, const char* pass);
int chat_leavechat(struct map_session_data* sd, bool kicked);
int chat_changechatowner(struct map_session_data* sd, const char* nextownername);
int chat_changechatstatus(struct map_session_data* sd, const char* title, const char* pass, int limit, bool pub);
int chat_kickchat(struct map_session_data* sd, const char* kickusername);

int chat_createnpcchat(struct npc_data* nd, const char* title, int limit, bool pub, int trigger, const char* ev);
int chat_deletenpcchat(struct npc_data* nd);
int chat_enableevent(struct chat_data* cd);
int chat_disableevent(struct chat_data* cd);
int chat_npckickall(struct chat_data* cd);

#endif /* _CHAT_H_ */
