// Logging functions by Azndragon & Codemaster
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/strlib.h"
#include "../common/nullpo.h"
#include "itemdb.h"
#include "map.h"
#include "log.h"

struct Log_Config log_config;

char timestring[255];
time_t curtime;

//FILTER OPTIONS
//0 = Don't log
//1 = Log any item
//Bits: ||
//2 - Healing items (0)
//3 - Etc Items(3) + Arrows (10)
//4 - Usable Items(2)
//5 - Weapon(4)
//6 - Shields,Armor,Headgears,Accessories,etc(5)
//7 - Cards(6)
//8 - Pet Accessories(8) + Eggs(7) (well, monsters don't drop 'em but we'll use the same system for ALL logs)
//9 - Log expensive items ( >= price_log)
//10 - Log big amount of items ( >= amount_log)
//11 - Log refined items (if their refine >= refine_log )
//12 - Log rare items (if their drop chance <= rare_log )

//check if this item should be logger according the settings
int should_log_item(int filter, int nameid) {
	struct item_data *item_data;
	if (nameid<512 || (item_data= itemdb_search(nameid)) == NULL) return 0;
	if ( (filter&1) || // Filter = 1, we log any item
		(filter&2 && item_data->type == 0 ) ||	//healing items
		(filter&4 && (item_data->type == 3 || item_data->type == 10) ) ||	//etc+arrows
		(filter&8 && item_data->type == 2 ) ||	//usable
		(filter&16 && item_data->type == 4 ) ||	//weapon
		(filter&32 && item_data->type == 5 ) ||	//armor
		(filter&64 && item_data->type == 6 ) ||	//cards
		(filter&128 && (item_data->type == 7 || item_data->type == 8) ) ||	//eggs+pet access
		(filter&256 && item_data->value_buy >= log_config.price_items_log ) ||
		(filter&512 && item_data->refine >= log_config.refine_items_log )
	) return item_data->nameid;

	return 0;
}

int log_branch(struct map_session_data *sd)
{
#ifndef TXT_ONLY
		char t_name[100];
#endif
	FILE *logfp;

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`branch_date`, `account_id`, `char_id`, `char_name`, `map`) VALUES (NOW(), '%d', '%d', '%s', '%s')",
			log_config.log_branch_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), sd->mapname);
		if(mysql_query(&mmysql_handle, tmp_sql))
			printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_branch,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d:%d]\t%s%s", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, sd->mapname, RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}

int log_drop(struct map_session_data *sd, int monster_id, int *log_drop)
{
	FILE *logfp;
	int i,flag = 0;

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
	for (i = 0; i<10; i++) { //Should we log these items? [Lupus]
		flag += should_log_item(log_config.drop,log_drop[i]);
	}
	if (flag==0) return 0; //we skip logging this items set - they doesn't met our logging conditions [Lupus]

#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`drop_date`, `kill_char_id`, `monster_id`, `item1`, `item2`, `item3`, `item4`, `item5`, `item6`, `item7`, `item8`, `item9`, `itemCard`, `map`) VALUES (NOW(), '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s') ", log_config.log_drop_db, sd->status.char_id, monster_id, log_drop[0], log_drop[1], log_drop[2], log_drop[3], log_drop[4], log_drop[5], log_drop[6], log_drop[7], log_drop[8], log_drop[9], sd->mapname);
		if(mysql_query(&mmysql_handle, tmp_sql))
			printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_drop,"a+")) != NULL) {
			

			time_t curtime;
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d:%d]\t%d\t%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, monster_id, log_drop[0], log_drop[1], log_drop[2], log_drop[3], log_drop[4], log_drop[5], log_drop[6], log_drop[7], log_drop[8], log_drop[9], RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 1; //Logged
}

int log_mvpdrop(struct map_session_data *sd, int monster_id, int *log_mvp)
{
	FILE *logfp;

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`mvp_date`, `kill_char_id`, `monster_id`, `prize`, `mvpexp`, `map`) VALUES (NOW(), '%d', '%d', '%d', '%d', '%s') ", log_config.log_mvpdrop_db, sd->status.char_id, monster_id, log_mvp[0], log_mvp[1], sd->mapname);
		if(mysql_query(&mmysql_handle, tmp_sql))
			printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_mvpdrop,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d:%d]\t%d\t%d,%d%s", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, monster_id, log_mvp[0], log_mvp[1], RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}

int log_present(struct map_session_data *sd, int source_type, int nameid)
{
	FILE *logfp;
#ifndef TXT_ONLY
		char t_name[100];
#endif

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
	if(!should_log_item(log_config.present,nameid)) return 0;	//filter [Lupus]
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`present_date`, `src_id`, `account_id`, `char_id`, `char_name`, `nameid`, `map`) VALUES (NOW(), '%d', '%d', '%d', '%s', '%d', '%s') ",
			log_config.log_present_db, source_type, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), nameid, sd->mapname);
		if(mysql_query(&mmysql_handle, tmp_sql))
			printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_present,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d:%d]\t%d\t%d%s", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, source_type, nameid, RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}

int log_produce(struct map_session_data *sd, int nameid, int slot1, int slot2, int slot3, int success)
{
	FILE *logfp;
#ifndef TXT_ONLY
		char t_name[100];
#endif

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
	if(!should_log_item(log_config.produce,nameid)) return 0;	//filter [Lupus]
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`produce_date`, `account_id`, `char_id`, `char_name`, `nameid`, `slot1`, `slot2`, `slot3`, `map`, `success`) VALUES (NOW(), '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%s', '%d') ",
			log_config.log_produce_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), nameid, slot1, slot2, slot3, sd->mapname, success);
		if(mysql_query(&mmysql_handle, tmp_sql))
			printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_produce,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d:%d]\t%d\t%d,%d,%d\t%d%s", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, nameid, slot1, slot2, slot3, success, RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}

int log_refine(struct map_session_data *sd, int n, int success)
{
	FILE *logfp;
	int log_card[4];
	int item_level;
	int i;
#ifndef TXT_ONLY
		char t_name[100];
#endif

	if(log_config.enable_logs <= 0)
		return 0;

	nullpo_retr(0, sd);

	if(success == 0)
		item_level = 0;
	else
		item_level = sd->status.inventory[n].refine + 1;
	if(!should_log_item(log_config.refine,sd->status.inventory[n].nameid)) return 0;	//filter [Lupus]
	for(i=0;i<4;i++)
		log_card[i] = sd->status.inventory[n].card[i];

#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`refine_date`, `account_id`, `char_id`, `char_name`, `nameid`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`, `success`, `item_level`) VALUES (NOW(), '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%s', '%d', '%d')",
			log_config.log_refine_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), sd->status.inventory[n].nameid, sd->status.inventory[n].refine, log_card[0], log_card[1], log_card[2], log_card[3], sd->mapname, success, item_level);
		if(mysql_query(&mmysql_handle, tmp_sql))
			printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_refine,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d:%d]\t%d,%d\t%d%d%d%d\t%d,%d%s", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, sd->status.inventory[n].nameid, sd->status.inventory[n].refine, log_card[0], log_card[1], log_card[2], log_card[3], success, item_level, RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}

int log_tostorage(struct map_session_data *sd,int n, int guild) 
{
  FILE *logfp;

  if(log_config.enable_logs <= 0 || log_config.storage == 0 || log_config.log_storage[0] == '\0')
    return 0;

  nullpo_retr(0, sd);
  if(sd->status.inventory[n].nameid==0 || sd->inventory_data[n] == NULL)
    return 1;

  if(sd->status.inventory[n].amount < 0)
    return 1;

  if((logfp=fopen(log_config.log_trade,"a+")) != NULL) {
    time(&curtime);
    strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
    fprintf(logfp,"%s - to %s: %s[%d:%d]\t%d\t%d\t%d\t%d,%d,%d,%d%s", timestring, guild ? "guild_storage": "storage", sd->status.name, sd->status.account_id, sd->status.char_id, 
      sd->status.inventory[n].nameid,
      sd->status.inventory[n].amount,
      sd->status.inventory[n].refine,
      sd->status.inventory[n].card[0],
      sd->status.inventory[n].card[1],
      sd->status.inventory[n].card[2],
      sd->status.inventory[n].card[3], RETCODE);
    fclose(logfp);
  }
  return 0;
}

int log_fromstorage(struct map_session_data *sd,int n, int guild) 
{
  FILE *logfp;

  if(log_config.enable_logs <= 0 || log_config.storage == 0 || log_config.log_storage[0] == '\0')
    return 0;

  nullpo_retr(0, sd);

  if(sd->status.inventory[n].nameid==0 || sd->inventory_data[n] == NULL)
    return 1;

  if(sd->status.inventory[n].amount < 0)
    return 1;

  if((logfp=fopen(log_config.log_trade,"a+")) != NULL) {
    time(&curtime);
    strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
    fprintf(logfp,"%s - from %s: %s[%d:%d]\t%d\t%d\t%d\t%d,%d,%d,%d%s", timestring, guild ? "guild_storage": "storage", sd->status.name, sd->status.account_id, sd->status.char_id, 
      sd->status.inventory[n].nameid,
      sd->status.inventory[n].amount,
      sd->status.inventory[n].refine,
      sd->status.inventory[n].card[0],
      sd->status.inventory[n].card[1],
      sd->status.inventory[n].card[2],
      sd->status.inventory[n].card[3], RETCODE);
    fclose(logfp);
  }
  return 0;
}

int log_trade(struct map_session_data *sd, struct map_session_data *target_sd, int n,int amount)
{
	FILE *logfp;
	int log_nameid, log_amount, log_refine, log_card[4];
	int i;
#ifndef TXT_ONLY
		char t_name[100],t_name2[100];
#endif

	if(log_config.enable_logs <= 0)
		return 0;

	nullpo_retr(0, sd);

	if(sd->status.inventory[n].nameid==0 || amount <= 0 || sd->status.inventory[n].amount<amount || sd->inventory_data[n] == NULL)
		return 1;

	if(sd->status.inventory[n].amount < 0)
		return 1;
	if(!should_log_item(log_config.trade,sd->status.inventory[n].nameid)) return 0;	//filter [Lupus]
	log_nameid = sd->status.inventory[n].nameid;
	log_amount = sd->status.inventory[n].amount;
	log_refine = sd->status.inventory[n].refine;

	for(i=0;i<4;i++)
		log_card[i] = sd->status.inventory[n].card[i];

#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`trade_date`, `src_account_id`, `src_char_id`, `src_char_name`, `des_account_id`, `des_char_id`, `des_char_name`, `nameid`, `amount`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`) VALUES (NOW(), '%d', '%d', '%s', '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s')",
			log_config.log_trade_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), target_sd->status.account_id, target_sd->status.char_id, jstrescapecpy(t_name2, target_sd->status.name), log_nameid, log_amount, log_refine, log_card[0], log_card[1], log_card[2], log_card[3], sd->mapname);
		if(mysql_query(&mmysql_handle, tmp_sql))
			printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_trade,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d:%d]\t%s[%d:%d]\t%d\t%d\t%d\t%d,%d,%d,%d%s", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, target_sd->status.name, target_sd->status.account_id, target_sd->status.char_id, log_nameid, log_amount, log_refine, log_card[0], log_card[1], log_card[2], log_card[3], RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}

int log_vend(struct map_session_data *sd,struct map_session_data *vsd,int n,int amount, int zeny)
{
	FILE *logfp;
	int log_nameid, log_amount, log_refine, log_card[4];
	int i;
#ifndef TXT_ONLY
		char t_name[100],t_name2[100];
#endif

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);

	if(sd->status.inventory[n].nameid==0 || amount <= 0 || sd->status.inventory[n].amount<amount || sd->inventory_data[n] == NULL)
		return 1;
	if(sd->status.inventory[n].amount< 0)
		return 1;
	if(!should_log_item(log_config.vend,sd->status.inventory[n].nameid)) return 0;	//filter [Lupus]
	log_nameid = sd->status.inventory[n].nameid;
	log_amount = sd->status.inventory[n].amount;
	log_refine = sd->status.inventory[n].refine;
	for(i=0;i<4;i++)
		log_card[i] = sd->status.inventory[n].card[i];

#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
			sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`vend_date`, `vend_account_id`, `vend_char_id`, `vend_char_name`, `buy_account_id`, `buy_char_id`, `buy_char_name`, `nameid`, `amount`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`, `zeny`) VALUES (NOW(), '%d', '%d', '%s', '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s', '%d')",
				log_config.log_vend_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), vsd->status.account_id, vsd->status.char_id, jstrescapecpy(t_name2, vsd->status.name), log_nameid, log_amount, log_refine, log_card[0], log_card[1], log_card[2], log_card[3], sd->mapname, zeny);
			if(mysql_query(&mmysql_handle, tmp_sql))
				printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_vend,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d:%d]\t%s[%d:%d]\t%d\t%d\t%d\t%d,%d,%d,%d\t%d%s", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, vsd->status.name, vsd->status.account_id, vsd->status.char_id, log_nameid, log_amount, log_refine, log_card[0], log_card[1], log_card[2], log_card[3], zeny, RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}

int log_zeny(struct map_session_data *sd, struct map_session_data *target_sd,int amount)
{
	FILE *logfp;
#ifndef TXT_ONLY
		char t_name[100],t_name2[100];
#endif

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql,"INSERT DELAYED INTO `%s` (`trade_date`, `src_account_id`, `src_char_id`, `src_char_name`, `des_account_id`, `des_char_id`, `des_char_name`, `map`, `zeny`) VALUES (NOW(), '%d', '%d', '%s', '%d', '%d', '%s', '%s', '%d')",
			log_config.log_trade_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), target_sd->status.account_id, target_sd->status.char_id, jstrescapecpy(t_name2, target_sd->status.name), sd->mapname, sd->deal_zeny);
		if(mysql_query(&mmysql_handle, tmp_sql))
			printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_trade,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d]\t%s[%d]\t%d\t%s", timestring, sd->status.name, sd->status.account_id, target_sd->status.name, target_sd->status.account_id, sd->deal_zeny, RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}

int log_atcommand(struct map_session_data *sd, const char *message)
{
	FILE *logfp;
#ifndef TXT_ONLY
		char t_name[100];
#endif

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`atcommand_date`, `account_id`, `char_id`, `char_name`, `map`, `command`) VALUES(NOW(), '%d', '%d', '%s', '%s', '%s') ",
			log_config.log_gm_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), sd->mapname, message);
		if(mysql_query(&mmysql_handle, tmp_sql))
			printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_gm,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d]: %s%s",timestring,sd->status.name,sd->status.account_id,message,RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}

int log_npc(struct map_session_data *sd, const char *message)
{	//[Lupus]
	FILE *logfp;
	#ifndef TXT_ONLY
		char t_name[100];
	#endif

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`npc_date`, `account_id`, `char_id`, `char_name`, `map`, `mes`) VALUES(NOW(), '%d', '%d', '%s', '%s', '%s') ",
			log_config.log_npc_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), sd->mapname, message);
		if(mysql_query(&mmysql_handle, tmp_sql))
			printf("DB server Error - %s\n",mysql_error(&mmysql_handle));
	} else {
#endif
		if((logfp=fopen(log_config.log_npc,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d]: %s%s",timestring,sd->status.name,sd->status.account_id,message,RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}

int log_config_read(char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

        memset(&log_config, 0, sizeof(log_config));

	if((fp = fopen(cfgName, "r")) == NULL)
	{
		printf("Log configuration file not found at: %s\n", cfgName);
		return 1;
	}
	
	//LOG FILTER Default values
	log_config.refine_items_log = 7; //log refined items, with refine >= +7
	log_config.rare_items_log = 100; //log rare items. drop chance <= 1%
	log_config.price_items_log = 1000; //1000z
	log_config.amount_items_log = 100;

	while(fgets(line, sizeof(line) -1, fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;

		if(sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2)
		{
			if(strcmpi(w1,"enable_logs") == 0) {
				log_config.enable_logs = (atoi(w2));
			} else if(strcmpi(w1,"sql_logs") == 0) {
				log_config.sql_logs = (atoi(w2));
//start of common filter settings
			} else if(strcmpi(w1,"rare_items_log") == 0) {
				log_config.rare_items_log = (atoi(w2));
			} else if(strcmpi(w1,"refine_items_log") == 0) {
				log_config.refine_items_log = (atoi(w2));
			} else if(strcmpi(w1,"price_items_log") == 0) {
				log_config.price_items_log = (atoi(w2));
			} else if(strcmpi(w1,"amount_items_log") == 0) {
				log_config.amount_items_log = (atoi(w2));
//end of common filter settings
			} else if(strcmpi(w1,"log_branch") == 0) {
				log_config.branch = (atoi(w2));
			} else if(strcmpi(w1,"log_drop") == 0) {
				log_config.drop = (atoi(w2));
			} else if(strcmpi(w1,"log_mvpdrop") == 0) {
				log_config.mvpdrop = (atoi(w2));
			} else if(strcmpi(w1,"log_present") == 0) {
				log_config.present = (atoi(w2));
			} else if(strcmpi(w1,"log_produce") == 0) {
				log_config.produce = (atoi(w2));
			} else if(strcmpi(w1,"log_refine") == 0) {
				log_config.refine = (atoi(w2));
			} else if(strcmpi(w1,"log_trade") == 0) {
				log_config.trade = (atoi(w2));
			} else if(strcmpi(w1,"log_storage") == 0) {
				log_config.storage = (atoi(w2));
			} else if(strcmpi(w1,"log_vend") == 0) {
				log_config.vend = (atoi(w2));
			} else if(strcmpi(w1,"log_zeny") == 0) {
				if(log_config.trade != 1)
					log_config.zeny = 0;
				else
					log_config.zeny = (atoi(w2));
			} else if(strcmpi(w1,"log_gm") == 0) {				
				log_config.gm = (atoi(w2));
			} else if(strcmpi(w1,"log_npc") == 0) {
				log_config.npc = (atoi(w2));
			}

#ifndef TXT_ONLY
			else if(strcmpi(w1, "log_branch_db") == 0) {
				strcpy(log_config.log_branch_db, w2);
				if(log_config.branch == 1)
					printf("Logging Dead Branch Usage to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_drop_db") == 0) {
				strcpy(log_config.log_drop_db, w2);
				if(log_config.drop == 1)
					printf("Logging Item Drops to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_mvpdrop_db") == 0) {
				strcpy(log_config.log_mvpdrop_db, w2);
				if(log_config.mvpdrop == 1)
					printf("Logging MVP Drops to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_present_db") == 0) {
				strcpy(log_config.log_present_db, w2);
				if(log_config.present == 1)
					printf("Logging Present Usage & Results to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_produce_db") == 0) {
				strcpy(log_config.log_produce_db, w2);
				if(log_config.produce == 1)
					printf("Logging Producing to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_refine_db") == 0) {
				strcpy(log_config.log_refine_db, w2);
				if(log_config.refine == 1)
					printf("Logging Refining to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_trade_db") == 0) {
				strcpy(log_config.log_trade_db, w2);
				if(log_config.trade == 1)
				{
					printf("Logging Item Trades");
					if(log_config.zeny == 1)
						printf("and Zeny Trades");
					printf(" to table `%s`\n", w2);
				}
//			} else if(strcmpi(w1, "log_storage_db") == 0) {
//				strcpy(log_config.log_storage_db, w2);
//				if(log_config.storage == 1)
//				{
//					printf("Logging Item Storages");
//					printf(" to table `%s`\n", w2);
//				}
			} else if(strcmpi(w1, "log_vend_db") == 0) {
				strcpy(log_config.log_vend_db, w2);
				if(log_config.vend == 1)
					printf("Logging Vending to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_gm_db") == 0) {
				strcpy(log_config.log_gm_db, w2);
				if(log_config.gm > 0)
					printf("Logging GM Level %d Commands to table `%s`\n", log_config.gm, w2);
			} else if(strcmpi(w1, "log_npc_db") == 0) {
				strcpy(log_config.log_npc_db, w2);
				if(log_config.npc > 0)
					printf("Logging NPC 'logmes' to table `%s`\n", w2);
			}
#endif

			else if(strcmpi(w1, "log_branch_file") == 0) {
				strcpy(log_config.log_branch, w2);
				if(log_config.branch > 0 && log_config.sql_logs < 1)
					printf("Logging Dead Branch Usage to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_drop_file") == 0) {
				strcpy(log_config.log_drop, w2);
				if(log_config.drop > 0 && log_config.sql_logs < 1)
					printf("Logging Item Drops to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_mvpdrop_file") == 0) {
				strcpy(log_config.log_mvpdrop, w2);
				if(log_config.mvpdrop > 0 && log_config.sql_logs < 1)
					printf("Logging MVP Drops to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_present_file") == 0) {
				strcpy(log_config.log_present, w2);
				if(log_config.present > 0 && log_config.sql_logs < 1)
					printf("Logging Present Usage & Results to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_produce_file") == 0) {
				strcpy(log_config.log_produce, w2);
				if(log_config.produce > 0 && log_config.sql_logs < 1)
					printf("Logging Producing to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_refine_file") == 0) {
				strcpy(log_config.log_refine, w2);
				if(log_config.refine > 0 && log_config.sql_logs < 1)
					printf("Logging Refining to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_trade_file") == 0) {
				strcpy(log_config.log_trade, w2);
				if(log_config.trade > 0 && log_config.sql_logs < 1)
				{
					printf("Logging Item Trades");
					if(log_config.zeny > 0)
						printf("and Zeny Trades");
					printf(" to file `%s`.txt\n", w2);
				}
			} else if(strcmpi(w1, "log_storage_file") == 0) {
				strcpy(log_config.log_storage, w2);
				if(log_config.storage > 0 && log_config.sql_logs < 1)
				{
					printf("Logging Item Storages");
					printf(" to file `%s`.txt\n", w2);
				}
			} else if(strcmpi(w1, "log_vend_file") == 0) {
				strcpy(log_config.log_vend, w2);
				if(log_config.vend > 0  && log_config.sql_logs < 1)
					printf("Logging Vending to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_gm_file") == 0) {
				strcpy(log_config.log_gm, w2);
				if(log_config.gm > 0 && log_config.sql_logs < 1)
					printf("Logging GM Level %d Commands to file `%s`.txt\n", log_config.gm, w2);
			} else if(strcmpi(w1, "log_npc_file") == 0) {
				strcpy(log_config.log_npc, w2);
				if(log_config.npc > 0 && log_config.sql_logs < 1)
					printf("Logging NPC 'logmes' to file `%s`.txt\n", w2);
			//support the import command, just like any other config
			} else if(strcmpi(w1,"import") == 0) {
				log_config_read(w2);
			}
		}
	}

	fclose(fp);
	return 0;
}
