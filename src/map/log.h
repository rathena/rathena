// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOG_H_
#define _LOG_H_

//#include "map.h"
struct map_session_data;
struct mob_data;
struct item;

//New logs
int log_pick_pc(struct map_session_data *sd, const char *type, int nameid, int amount, struct item *itm);
int log_pick_mob(struct mob_data *md, const char *type, int nameid, int amount, struct item *itm);
int log_zeny(struct map_session_data *sd, char *type, struct map_session_data *src_sd, int amount);

int log_npc(struct map_session_data *sd, const char *message);
int log_chat(const char* type, int type_id, int src_charid, int src_accid, const char* map, int x, int y, const char* dst_charname, const char* message);
int log_atcommand(struct map_session_data *sd, const char *message);

//Old, but useful logs
int log_branch(struct map_session_data *sd);
int log_mvpdrop(struct map_session_data *sd, int monster_id, int *log_mvp);

int log_config_read(char *cfgName);

typedef enum log_what {
	LOG_ALL                 = 0xFFF,
	LOG_TRADES              = 0x002,
	LOG_VENDING             = 0x004,
	LOG_PLAYER_ITEMS        = 0x008, // dropped/picked
	LOG_MONSTER_ITEMS       = 0x010, // dropped/looted
	LOG_NPC_TRANSACTIONS    = 0x020, // npc shops?
	LOG_SCRIPT_TRANSACTIONS = 0x040,
	LOG_STOLEN_ITEMS        = 0x080, // stolen from mobs
	LOG_USED_ITEMS          = 0x100, // used by player
	LOG_MVP_PRIZE           = 0x200,
	LOG_COMMAND_ITEMS       = 0x400  // created/deleted through @/# commands
} log_what;

extern struct Log_Config {
	enum log_what enable_logs;
	int filter;
	bool sql_logs;
	int rare_items_log,refine_items_log,price_items_log,amount_items_log; //for filter
	int branch, drop, mvpdrop, zeny, gm, npc, chat;
	char log_branch[64], log_pick[64], log_zeny[64], log_mvpdrop[64], log_gm[64], log_npc[64], log_chat[64];
	char log_branch_db[32], log_pick_db[32], log_zeny_db[32], log_mvpdrop_db[32], log_gm_db[32], log_npc_db[32], log_chat_db[32];
} log_config;

#endif /* _LOG_H_ */
