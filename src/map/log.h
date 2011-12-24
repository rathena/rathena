// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOG_H_
#define _LOG_H_

//#include "map.h"
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
	LOG_TYPE_TRADE            = 0x0001,
	LOG_TYPE_VENDING          = 0x0002,
	LOG_TYPE_PICKDROP_PLAYER  = 0x0004,
	LOG_TYPE_PICKDROP_MONSTER = 0x0008,
	LOG_TYPE_NPC              = 0x0010,
	LOG_TYPE_SCRIPT           = 0x0020,
	//LOG_TYPE_STEAL            = 0x0040,
	LOG_TYPE_CONSUME          = 0x0080,
	//LOG_TYPE_PRODUCE          = 0x0100,
	//LOG_TYPE_MVP              = 0x0200,
	LOG_TYPE_COMMAND          = 0x0400,
	LOG_TYPE_STORAGE          = 0x0800,
	LOG_TYPE_GSTORAGE         = 0x1000,
	LOG_TYPE_MAIL             = 0x2000,
	//LOG_TYPE_AUCTION          = 0x4000,
	LOG_TYPE_BUYING_STORE     = 0x8000,
	// combinations
	LOG_TYPE_LOOT             = LOG_TYPE_PICKDROP_MONSTER|LOG_TYPE_CONSUME,
	// all
	LOG_TYPE_ALL              = 0xFFFF,
}
e_log_pick_type;


//New logs
void log_pick_pc(struct map_session_data *sd, e_log_pick_type type, int nameid, int amount, struct item *itm);
void log_pick_mob(struct mob_data *md, e_log_pick_type type, int nameid, int amount, struct item *itm);
void log_pick(struct block_list* bl, e_log_pick_type type, int nameid, int amount, struct item* itm);
void log_zeny(struct map_session_data *sd, e_log_pick_type type, struct map_session_data *src_sd, int amount);

void log_npc(struct map_session_data *sd, const char *message);
void log_chat(e_log_chat_type type, int type_id, int src_charid, int src_accid, const char* map, int x, int y, const char* dst_charname, const char* message);
void log_atcommand(struct map_session_data *sd, int cmdlvl, const char *message);

//Old, but useful logs
void log_branch(struct map_session_data *sd);
void log_mvpdrop(struct map_session_data *sd, int monster_id, int *log_mvp);

int log_config_read(char *cfgName);

extern struct Log_Config
{
	enum e_log_pick_type enable_logs;
	int filter;
	bool sql_logs;
	bool log_chat_woe_disable;
	int rare_items_log,refine_items_log,price_items_log,amount_items_log; //for filter
	int branch, drop, mvpdrop, zeny, gm, npc, chat;
	char log_branch[64], log_pick[64], log_zeny[64], log_mvpdrop[64], log_gm[64], log_npc[64], log_chat[64];
	char log_branch_db[32], log_pick_db[32], log_zeny_db[32], log_mvpdrop_db[32], log_gm_db[32], log_npc_db[32], log_chat_db[32];
}
log_config;

#endif /* _LOG_H_ */
