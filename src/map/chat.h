// $Id: chat.h,v 1.3 2004/09/25 05:32:18 MouseJstr Exp $
#ifndef _CHAT_H_
#define _CHAT_H_

#include "map.h"

int chat_createchat(struct map_session_data *,int,int,char*,char*,int);
int chat_joinchat(struct map_session_data *,int,char*);
int chat_leavechat(struct map_session_data* );
int chat_changechatowner(struct map_session_data *,char *);
int chat_changechatstatus(struct map_session_data *,int,int,char*,char*,int);
int chat_kickchat(struct map_session_data *,char *);

int chat_createnpcchat(struct npc_data *nd,int limit,int pub,int trigger,char* title,int titlelen,const char *ev);
int chat_deletenpcchat(struct npc_data *nd);
int chat_enableevent(struct chat_data *cd);
int chat_disableevent(struct chat_data *cd);
int chat_npckickall(struct chat_data *cd);

int do_final_chat(void);

#endif
