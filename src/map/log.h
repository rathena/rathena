#ifndef _LOG_H_
#define _LOG_H_

#include "map.h"

#ifndef TXT_ONLY

extern char db_server_logdb[32];

#endif //NOT TXT_ONLY

int log_branch(struct map_session_data *sd);
int log_drop(struct map_session_data *sd, int monster_id, int *log_drop);
int log_mvpdrop(struct map_session_data *sd, int monster_id, int *log_mvp);
int log_present(struct map_session_data *sd, int source_type, int nameid);
int log_produce(struct map_session_data *sd, int nameid, int slot1, int slot2, int slot3, int success);
int log_refine(struct map_session_data *sd, int n, int success);
int log_trade(struct map_session_data *sd,struct map_session_data *target_sd,int n,int amount);
int log_vend(struct map_session_data *sd,struct map_session_data *vsd,int n,int amount,int zeny);
int log_zeny(struct map_session_data *sd, struct map_session_data *target_sd,int amount);
int log_atcommand(struct map_session_data *sd, const char *message);

int log_config_read(char *cfgName);

extern struct Log_Config {
	int enable_logs;
	int sql_logs;
	int branch, drop, mvpdrop, present, produce, refine, trade, vend, zeny, gm;
	char log_branch[32], log_drop[32], log_mvpdrop[32], log_present[32], log_produce[32], log_refine[32], log_trade[32], log_vend[32], log_gm[32];
	char log_branch_db[32], log_drop_db[32], log_mvpdrop_db[32], log_present_db[32], log_produce_db[32], log_refine_db[32], log_trade_db[32], log_vend_db[32], log_gm_db[32];
} log_config;

#endif
