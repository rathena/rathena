// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOG_H_
#define _LOG_H_

#include "map.h"

#ifndef TXT_ONLY

extern char db_server_logdb[32];

#endif //NOT TXT_ONLY

//New logs
int log_pick(struct map_session_data *sd, char *type, int mob_id, int nameid, int amount, struct item *itm);
int log_zeny(struct map_session_data *sd, char *type, struct map_session_data *src_sd, int amount);

int log_npc(struct map_session_data *sd, const char *message);
int log_chat(char *type, int type_id, int src_charid, int src_accid, char *map, int x, int y, char *dst_charname, char *message);
int log_atcommand(struct map_session_data *sd, const char *message);

//Old, but useful logs
int log_branch(struct map_session_data *sd);
int log_mvpdrop(struct map_session_data *sd, int monster_id, int *log_mvp);

int log_config_read(char *cfgName);

int should_log_item(int filter, int nameid, int amount); //log filter check

extern struct Log_Config {
	int enable_logs, filter;
	int sql_logs;
	int rare_items_log,refine_items_log,price_items_log,amount_items_log; //for filter
	int branch, drop, mvpdrop, zeny, gm, npc, chat;
	char log_branch[32], log_pick[32], log_zeny[32], log_mvpdrop[32], log_gm[32], log_npc[32], log_chat[32];
	char log_branch_db[32], log_pick_db[32], log_zeny_db[32], log_mvpdrop_db[32], log_gm_db[32], log_npc_db[32], log_chat_db[32];
	int uptime;
	char log_uptime[32];
} log_config;

#endif
