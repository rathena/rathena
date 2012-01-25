// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOG_H_
#define _LOG_H_

//#include "map.h"
struct block_list;
struct map_session_data;
struct mob_data;
struct item;


typedef enum e_log_chat_type
{
	LOG_CHAT_GLOBAL      = 0x01,
	LOG_CHAT_WHISPER     = 0x02,
	LOG_CHAT_PARTY       = 0x04,
	LOG_CHAT_GUILD       = 0x08,
	LOG_CHAT_MAINCHAT    = 0x10,
	// all
	LOG_CHAT_ALL         = 0xFF,
}
e_log_chat_type;


typedef enum e_log_pick_type
{
	LOG_TYPE_NONE             = 0,
	LOG_TYPE_TRADE            = 0x00001,
	LOG_TYPE_VENDING          = 0x00002,
	LOG_TYPE_PICKDROP_PLAYER  = 0x00004,
	LOG_TYPE_PICKDROP_MONSTER = 0x00008,
	LOG_TYPE_NPC              = 0x00010,
	LOG_TYPE_SCRIPT           = 0x00020,
	LOG_TYPE_STEAL            = 0x00040,
	LOG_TYPE_CONSUME          = 0x00080,
	LOG_TYPE_PRODUCE          = 0x00100,
	LOG_TYPE_MVP              = 0x00200,
	LOG_TYPE_COMMAND          = 0x00400,
	LOG_TYPE_STORAGE          = 0x00800,
	LOG_TYPE_GSTORAGE         = 0x01000,
	LOG_TYPE_MAIL             = 0x02000,
	LOG_TYPE_AUCTION          = 0x04000,
	LOG_TYPE_BUYING_STORE     = 0x08000,
	LOG_TYPE_OTHER            = 0x10000,
	// combinations
	LOG_TYPE_LOOT             = LOG_TYPE_PICKDROP_MONSTER|LOG_TYPE_CONSUME,
	// all
	LOG_TYPE_ALL              = 0xFFFFF,
}
e_log_pick_type;


/// new logs
void log_pick_pc(struct map_session_data* sd, e_log_pick_type type, int amount, struct item* itm);
void log_pick_mob(struct mob_data* md, e_log_pick_type type, int amount, struct item* itm);
void log_zeny(struct map_session_data* sd, e_log_pick_type type, struct map_session_data* src_sd, int amount);

void log_npc(struct map_session_data* sd, const char *message);
void log_chat(e_log_chat_type type, int type_id, int src_charid, int src_accid, const char* map, int x, int y, const char* dst_charname, const char* message);
void log_atcommand(struct map_session_data* sd, int cmdlvl, const char* message);

/// old, but useful logs
void log_branch(struct map_session_data* sd);
void log_mvpdrop(struct map_session_data* sd, int monster_id, int* log_mvp);

int log_config_read(const char* cfgName);

extern struct Log_Config
{
	e_log_pick_type enable_logs;
	int filter;
	bool sql_logs;
	bool log_chat_woe_disable;
	int rare_items_log,refine_items_log,price_items_log,amount_items_log; //for filter
	int branch, mvpdrop, zeny, gm, npc, chat;
	char log_branch[64], log_pick[64], log_zeny[64], log_mvpdrop[64], log_gm[64], log_npc[64], log_chat[64];
}
log_config;

#endif /* _LOG_H_ */
