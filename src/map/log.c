// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// Logging functions by Azndragon & Codemaster
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/strlib.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "itemdb.h"
#include "map.h"
#include "log.h"

#ifndef SQL_DEBUG

#define mysql_query(_x, _y) mysql_real_query(_x, _y, strlen(_y)) //supports ' in names and runs faster [Kevin]

#else 

#define mysql_query(_x, _y) debug_mysql_query(__FILE__, __LINE__, _x, _y)

#endif

struct Log_Config log_config;

char timestring[255];
time_t curtime;

//FILTER OPTIONS
//0 = Don't log
//1 = Log any item
//Bits: ||
//2 - Healing items (0)
//3 - Etc Items(3) + Arrows (10)
//4 - Usable Items(2) + Scrolls,Lures(11)
//5 - Weapon(4)
//6 - Shields,Armor,Headgears,Accessories,etc(5)
//7 - Cards(6)
//8 - Pet Accessories(8) + Eggs(7) (well, monsters don't drop 'em but we'll use the same system for ALL logs)
//9 - Log expensive items ( >= price_log)
//10 - Log big amount of items ( >= amount_log)
//11 - Log refined items (if their refine >= refine_log )
//12 - Log rare items (if their drop chance <= rare_log )

//check if this item should be logged according the settings
int should_log_item(int filter, int nameid, int amount) {
	struct item_data *item_data;
	if ((item_data= itemdb_exists(nameid)) == NULL) return 0;
	if ((filter&1) || // Filter = 1, we log any item
		(filter&2 && item_data->type == IT_HEALING ) ||
		(filter&4 && (item_data->type == IT_ETC || item_data->type == IT_AMMO) ) ||
		(filter&8 && item_data->type == IT_USABLE ) ||
		(filter&16 && item_data->type == IT_WEAPON ) ||
		(filter&32 && item_data->type == IT_ARMOR ) ||
		(filter&64 && item_data->type == IT_CARD ) ||
		(filter&128 && (item_data->type == IT_PETEGG || item_data->type == IT_PETARMOR) ) ||
		(filter&256 && item_data->value_buy >= log_config.price_items_log ) ||		//expensive items
		(filter&512 && abs(amount) >= log_config.amount_items_log ) ||			//big amount of items
		(filter&2048 && ((item_data->maxchance <= log_config.rare_items_log) || item_data->nameid == 714) ) //Rare items or Emperium
	) return item_data->nameid;

	return 0;
}

int log_branch(struct map_session_data *sd)
{
#ifndef TXT_ONLY
		char t_name[NAME_LENGTH*2];
#endif
	FILE *logfp;

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`branch_date`, `account_id`, `char_id`, `char_name`, `map`) VALUES (NOW(), '%d', '%d', '%s', '%s')",
			log_config.log_branch_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), mapindex_id2name(sd->mapindex));
		if(mysql_query(&logmysql_handle, tmp_sql))
		{
			ShowSQL("DB error - %s\n",mysql_error(&logmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
	} else {
#endif
		if((logfp=fopen(log_config.log_branch,"a+")) != NULL) {
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%d:%d]\t%s%s", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex), RETCODE);
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
}


int log_pick(struct map_session_data *sd, char *type, int mob_id, int nameid, int amount, struct item *itm)
{
	FILE *logfp;
	char *mapname;
	int obj_id;

	nullpo_retr(0, sd);
	//Should we log this item? [Lupus]
	if (!should_log_item(log_config.filter,nameid, amount))
		return 0; //we skip logging this items set - they doesn't met our logging conditions [Lupus]

	//either PLAYER or MOB (here we get map name and objects ID)
	if(mob_id) {
		struct mob_data *md = (struct mob_data*)sd;
		obj_id = mob_id;
		mapname = map[md->bl.m].name;
	} else {
		obj_id = sd->char_id;
		mapname = (char*)mapindex_id2name(sd->mapindex);
	}
	if(mapname==NULL)
		mapname="";

#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		if (itm==NULL) {
		//We log common item
			sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`time`, `char_id`, `type`, `nameid`, `amount`, `map`) VALUES (NOW(), '%d', '%s', '%d', '%d', '%s')",
			 log_config.log_pick_db, obj_id, type, nameid, amount, mapname);
		} else {
		//We log Extended item
			sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`time`, `char_id`, `type`, `nameid`, `amount`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`) VALUES (NOW(), '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s')",
			 log_config.log_pick_db, obj_id, type, itm->nameid, amount, itm->refine, itm->card[0], itm->card[1], itm->card[2], itm->card[3], mapname);
		}

		if(mysql_query(&logmysql_handle, tmp_sql))
		{
			ShowSQL("DB error - %s\n",mysql_error(&logmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
	} else {
#endif
		if((logfp=fopen(log_config.log_pick,"a+")) != NULL) {
			time_t curtime;
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));

			if (itm==NULL) {
			//We log common item
				fprintf(logfp,"%s - %d\t%s\t%d,%d,%s%s",
					timestring, obj_id, type, nameid, amount, mapname, RETCODE);

			} else {
			//We log Extended item
				fprintf(logfp,"%s - %d\t%s\t%d,%d,%d,%d,%d,%d,%d,%s%s",
					timestring, obj_id, type, itm->nameid, amount, itm->refine, itm->card[0], itm->card[1], itm->card[2], itm->card[3], mapname, RETCODE);
			}
			fclose(logfp);
		}
#ifndef TXT_ONLY
	}
#endif
	return 1; //Logged
}

int log_zeny(struct map_session_data *sd, char *type, struct map_session_data *src_sd, int amount)
{
//	FILE *logfp;
	if(log_config.enable_logs <= 0 || (log_config.zeny!=1 && abs(amount)<log_config.zeny))
		return 0;

	nullpo_retr(0, sd);
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`time`, `char_id`, `src_id`, `type`, `amount`, `map`) VALUES (NOW(), '%d', '%d', '%s', '%d', '%s')",
			 log_config.log_zeny_db, sd->char_id, src_sd->char_id, type, amount, mapindex_id2name(sd->mapindex));
		if(mysql_query(&logmysql_handle, tmp_sql))
		{
			ShowSQL("DB error - %s\n",mysql_error(&logmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
	} else {
#endif
//		if((logfp=fopen(log_config.log_zeny,"a+")) != NULL) {
//			time(&curtime);
//			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
//			fprintf(logfp,"%s - %s[%d]\t%s[%d]\t%d\t%s", timestring, sd->status.name, sd->status.account_id, target_sd->status.name, target_sd->status.account_id, sd->deal.zeny, RETCODE);
//			fclose(logfp);
//		}
#ifndef TXT_ONLY
	}
#endif
	return 0;
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
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`mvp_date`, `kill_char_id`, `monster_id`, `prize`, `mvpexp`, `map`) VALUES (NOW(), '%d', '%d', '%d', '%d', '%s') ", log_config.log_mvpdrop_db, sd->status.char_id, monster_id, log_mvp[0], log_mvp[1], mapindex_id2name(sd->mapindex));
		if(mysql_query(&logmysql_handle, tmp_sql))
		{
			ShowSQL("DB error - %s\n",mysql_error(&logmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
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


int log_atcommand(struct map_session_data *sd, const char *message)
{
	FILE *logfp;
#ifndef TXT_ONLY
		char t_name[NAME_LENGTH*2];
		char t_msg[MESSAGE_SIZE*2+1]; //These are the contents of an @ call, so there shouldn't be overflow danger here?
#endif

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`atcommand_date`, `account_id`, `char_id`, `char_name`, `map`, `command`) VALUES(NOW(), '%d', '%d', '%s', '%s', '%s') ",
			log_config.log_gm_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), mapindex_id2name(sd->mapindex), jstrescapecpy(t_msg, (char *)message));
		if(mysql_query(&logmysql_handle, tmp_sql))
		{
			ShowSQL("DB error - %s\n",mysql_error(&logmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
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
		char t_name[NAME_LENGTH*2];
		char t_msg[255+1]; //it's 255 chars MAX. 
	#endif

	if(log_config.enable_logs <= 0)
		return 0;
	nullpo_retr(0, sd);
#ifndef TXT_ONLY
	if(log_config.sql_logs > 0)
	{
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`npc_date`, `account_id`, `char_id`, `char_name`, `map`, `mes`) VALUES(NOW(), '%d', '%d', '%s', '%s', '%s') ",
			log_config.log_npc_db, sd->status.account_id, sd->status.char_id, jstrescapecpy(t_name, sd->status.name), mapindex_id2name(sd->mapindex), jstrescapecpy(t_msg, (char *)message));
		if(mysql_query(&logmysql_handle, tmp_sql))
		{
			ShowSQL("DB error - %s\n",mysql_error(&logmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
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

//ChatLogging
// Log CHAT (currently only: Party, Guild, Whisper)
// LOGGING FILTERS [Lupus]
//=============================================================
//0 = Don't log at all
//1 = Log any chat messages
//Advanced Filter Bits: ||
//2 - Log Whisper messages
//3 - Log Party messages
//4 - Log Guild messages
//5 - Log Common messages (not implemented)
//6 - Don't log when WOE is on
//Example:
//log_chat: 1	= logs ANY messages
//log_chat: 6	= logs both Whisper & Party messages
//log_chat: 8	= logs only Guild messages
//log_chat: 18	= logs only Whisper, when WOE is off

int log_chat(char *type, int type_id, int src_charid, int src_accid, char *map, int x, int y, char *dst_charname, char *message){
#ifndef TXT_ONLY
	char t_charname[NAME_LENGTH*2];
	char t_msg[MESSAGE_SIZE*2+1]; //Chat line fully escaped, with an extra space just in case.
#else
	FILE *logfp;
#endif
	
	//Check ON/OFF
	if(log_config.chat <= 0)
		return 0; //Deactivated

#ifndef TXT_ONLY
	if(log_config.sql_logs > 0){
		sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`time`, `type`, `type_id`, `src_charid`, `src_accountid`, `src_map`, `src_map_x`, `src_map_y`, `dst_charname`, `message`) VALUES (NOW(), '%s', '%d', '%d', '%d', '%s', '%d', '%d', '%s', '%s')", 
		 	log_config.log_chat_db, type, type_id, src_charid, src_accid, map, x, y, jstrescapecpy(t_charname, dst_charname), jstrescapecpy(t_msg, message));
	
		if(mysql_query(&logmysql_handle, tmp_sql)){
			ShowSQL("DB error - %s\n",mysql_error(&logmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			return -1;	
		}else{
			return 0;
		}
	}			
#endif

#ifdef TXT_ONLY
	if((logfp = fopen(log_config.log_chat, "a+")) != NULL){
		time(&curtime);
		strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		//DATE - type,type_id,src_charid,src_accountid,src_map,src_x,src_y,dst_charname,message
		fprintf(logfp, "%s - %s,%d,%d,%d,%s,%d,%d,%s,%s%s", 
			timestring, type, type_id, src_charid, src_accid, map, x, y, dst_charname, message, RETCODE);
		fclose(logfp);
		return 0;
	}else{
		return -1;
	}
#endif
return -1;
}


void log_set_defaults(void)
{
	memset(&log_config, 0, sizeof(log_config));

	//LOG FILTER Default values
	log_config.refine_items_log = 5; //log refined items, with refine >= +7
	log_config.rare_items_log = 100; //log rare items. drop chance <= 1%
	log_config.price_items_log = 1000; //1000z
	log_config.amount_items_log = 100;	
}

int log_config_read(char *cfgName)
{
	static int count = 0;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	if ((count++) == 0)
		log_set_defaults();		

	if((fp = fopen(cfgName, "r")) == NULL)
	{
		ShowError("Log configuration file not found at: %s\n", cfgName);
		return 1;
	}	

	while(fgets(line, sizeof(line) -1, fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;

		if(sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2)
		{
			if(strcmpi(w1,"enable_logs") == 0) {
				log_config.enable_logs = (atoi(w2));
				if (log_config.enable_logs&1) //Log everything.
					log_config.enable_logs=0xFFFFFFFF;
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
			} else if(strcmpi(w1,"log_filter") == 0) {
				log_config.filter = (atoi(w2));
			} else if(strcmpi(w1,"log_zeny") == 0) {
				log_config.zeny = (atoi(w2));
			} else if(strcmpi(w1,"log_gm") == 0) {
				log_config.gm = (atoi(w2));
			} else if(strcmpi(w1,"log_npc") == 0) {
				log_config.npc = (atoi(w2));
			} else if(strcmpi(w1, "log_chat") == 0) {
				log_config.chat = (atoi(w2));
			} else if(strcmpi(w1,"log_mvpdrop") == 0) {
				log_config.mvpdrop = (atoi(w2));
			}

#ifndef TXT_ONLY
			else if(strcmpi(w1, "log_branch_db") == 0) {
				strcpy(log_config.log_branch_db, w2);
				if(log_config.branch == 1)
					ShowNotice("Logging Dead Branch Usage to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_pick_db") == 0) {
				strcpy(log_config.log_pick_db, w2);
				if(log_config.filter)
					ShowNotice("Logging Item Picks to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_zeny_db") == 0) {
				strcpy(log_config.log_zeny_db, w2);
				if(log_config.zeny == 1)
					ShowNotice("Logging Zeny to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_mvpdrop_db") == 0) {
				strcpy(log_config.log_mvpdrop_db, w2);
				if(log_config.mvpdrop == 1)
					ShowNotice("Logging MVP Drops to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_gm_db") == 0) {
				strcpy(log_config.log_gm_db, w2);
				if(log_config.gm > 0)
					ShowNotice("Logging GM Level %d Commands to table `%s`\n", log_config.gm, w2);
			} else if(strcmpi(w1, "log_npc_db") == 0) {
				strcpy(log_config.log_npc_db, w2);
				if(log_config.npc > 0)
					ShowNotice("Logging NPC 'logmes' to table `%s`\n", w2);
			} else if(strcmpi(w1, "log_chat_db") == 0) {
				strcpy(log_config.log_chat_db, w2);
				if(log_config.chat > 0)
					ShowNotice("Logging CHAT to table `%s`\n", w2);
			}
#endif

			else if(strcmpi(w1, "log_branch_file") == 0) {
				strcpy(log_config.log_branch, w2);
				if(log_config.branch > 0 && log_config.sql_logs < 1)
					ShowNotice("Logging Dead Branch Usage to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_pick_file") == 0) {
				strcpy(log_config.log_pick, w2);
				if(log_config.filter > 0 && log_config.sql_logs < 1)
					ShowNotice("Logging Item Picks to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_zeny_file") == 0) {
				strcpy(log_config.log_zeny, w2);
				if(log_config.zeny > 0 && log_config.sql_logs < 1)
					ShowNotice("Logging Zeny to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_mvpdrop_file") == 0) {
				strcpy(log_config.log_mvpdrop, w2);
				if(log_config.mvpdrop > 0 && log_config.sql_logs < 1)
					ShowNotice("Logging MVP Drops to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_gm_file") == 0) {
				strcpy(log_config.log_gm, w2);
				if(log_config.gm > 0 && log_config.sql_logs < 1)
					ShowNotice("Logging GM Level %d Commands to file `%s`.txt\n", log_config.gm, w2);
			} else if(strcmpi(w1, "log_npc_file") == 0) {
				strcpy(log_config.log_npc, w2);
				if(log_config.npc > 0 && log_config.sql_logs < 1)
					ShowNotice("Logging NPC 'logmes' to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_chat_file") == 0) {
				strcpy(log_config.log_chat, w2);
				if(log_config.chat > 0 && log_config.sql_logs < 1)					
					ShowNotice("Logging CHAT to file `%s`.txt\n", w2);
			//support the import command, just like any other config
			} else if(strcmpi(w1,"import") == 0) {
				log_config_read(w2);
			}
		}
	}

	fclose(fp);
	return 0;
}
