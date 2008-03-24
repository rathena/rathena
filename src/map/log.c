// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/strlib.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "battle.h"
#include "itemdb.h"
#include "log.h"
#include "map.h"
#include "mob.h"
#include "pc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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
int should_log_item(int filter, int nameid, int amount)
{
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
	if(!log_config.enable_logs)
		return 0;

	nullpo_retr(0, sd);

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		SqlStmt* stmt;
		stmt = SqlStmt_Malloc(logmysql_handle);
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "INSERT DELAYED INTO `%s` (`branch_date`, `account_id`, `char_id`, `char_name`, `map`) VALUES (NOW(), '%d', '%d', ?, '%s')", log_config.log_branch_db, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex) )
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, sd->status.name, strnlen(sd->status.name, NAME_LENGTH))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return 0;
		}
		SqlStmt_Free(stmt);
	}
	else
#endif
	{
		FILE* logfp;
		if((logfp = fopen(log_config.log_branch, "a+")) == NULL)
			return 0;
		time(&curtime);
		strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp,"%s - %s[%d:%d]\t%s\n", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex));
		fclose(logfp);
	}

	return 1;
}


int log_pick_pc(struct map_session_data *sd, const char *type, int nameid, int amount, struct item *itm)
{
	nullpo_retr(0, sd);

	if (!should_log_item(log_config.filter, nameid, amount))
		return 0; //we skip logging this item set - it doesn't meet our logging conditions [Lupus]

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		if( itm == NULL ) { //We log common item
			if (SQL_ERROR == Sql_Query(logmysql_handle, "INSERT DELAYED INTO `%s` (`time`, `char_id`, `type`, `nameid`, `amount`, `map`) VALUES (NOW(), '%d', '%s', '%d', '%d', '%s')",
				log_config.log_pick_db, sd->status.char_id, type, nameid, amount, mapindex_id2name(sd->mapindex)) )
			{
				Sql_ShowDebug(logmysql_handle);
				return 0;
			}
		} else { //We log Extended item
			if (SQL_ERROR == Sql_Query(logmysql_handle, "INSERT DELAYED INTO `%s` (`time`, `char_id`, `type`, `nameid`, `amount`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`) VALUES (NOW(), '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s')",
				log_config.log_pick_db, sd->status.char_id, type, itm->nameid, amount, itm->refine, itm->card[0], itm->card[1], itm->card[2], itm->card[3], mapindex_id2name(sd->mapindex)) )
			{
				Sql_ShowDebug(logmysql_handle);
				return 0;
			}
		}
	}
	else
#endif
	{
		FILE* logfp;

		if((logfp = fopen(log_config.log_pick, "a+")) == NULL)
			return 0;
		time(&curtime);
		strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));

		if( itm == NULL ) { //We log common item
			fprintf(logfp,"%s - %d\t%s\t%d,%d,%s\n", timestring, sd->status.char_id, type, nameid, amount, mapindex_id2name(sd->mapindex));
		} else { //We log Extended item
			fprintf(logfp,"%s - %d\t%s\t%d,%d,%d,%d,%d,%d,%d,%s\n", timestring, sd->status.char_id, type, itm->nameid, amount, itm->refine, itm->card[0], itm->card[1], itm->card[2], itm->card[3], mapindex_id2name(sd->mapindex));
		}
		fclose(logfp);
	}
	
	return 1; //Logged
}

//Mob picked item
int log_pick_mob(struct mob_data *md, const char *type, int nameid, int amount, struct item *itm)
{
	char* mapname;

	nullpo_retr(0, md);

	if (!should_log_item(log_config.filter, nameid, amount))
		return 0; //we skip logging this item set - it doesn't meet our logging conditions [Lupus]

	//either PLAYER or MOB (here we get map name and objects ID)
	mapname = map[md->bl.m].name;
	if(mapname==NULL)
		mapname="";

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		if( itm == NULL ) { //We log common item
			if (SQL_ERROR == Sql_Query(logmysql_handle, "INSERT DELAYED INTO `%s` (`time`, `char_id`, `type`, `nameid`, `amount`, `map`) VALUES (NOW(), '%d', '%s', '%d', '%d', '%s')",
				log_config.log_pick_db, md->class_, type, nameid, amount, mapname) )
			{
				Sql_ShowDebug(logmysql_handle);
				return 0;
			}
		} else { //We log Extended item
			if (SQL_ERROR == Sql_Query(logmysql_handle, "INSERT DELAYED INTO `%s` (`time`, `char_id`, `type`, `nameid`, `amount`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`) VALUES (NOW(), '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s')",
				log_config.log_pick_db, md->class_, type, itm->nameid, amount, itm->refine, itm->card[0], itm->card[1], itm->card[2], itm->card[3], mapname) )
			{
				Sql_ShowDebug(logmysql_handle);
				return 0;
			}
		}
	}
	else
#endif
	{
		FILE *logfp;

		if((logfp=fopen(log_config.log_pick,"a+")) == NULL)
			return 0;
		time(&curtime);
		strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));

		if( itm == NULL ) { //We log common item
			fprintf(logfp,"%s - %d\t%s\t%d,%d,%s\n", timestring, md->class_, type, nameid, amount, mapname);
		} else { //We log Extended item
			fprintf(logfp,"%s - %d\t%s\t%d,%d,%d,%d,%d,%d,%d,%s\n", timestring, md->class_, type, itm->nameid, amount, itm->refine, itm->card[0], itm->card[1], itm->card[2], itm->card[3], mapname);
		}
		fclose(logfp);
	}
	
	return 1; //Logged
}

int log_zeny(struct map_session_data *sd, char *type, struct map_session_data *src_sd, int amount)
{
	if(!log_config.enable_logs || (log_config.zeny != 1 && abs(amount) < log_config.zeny))
		return 0;

	nullpo_retr(0, sd);

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		if (SQL_ERROR == Sql_Query(logmysql_handle, "INSERT DELAYED INTO `%s` (`time`, `char_id`, `src_id`, `type`, `amount`, `map`) VALUES (NOW(), '%d', '%d', '%s', '%d', '%s')",
			 log_config.log_zeny_db, sd->status.char_id, src_sd->status.char_id, type, amount, mapindex_id2name(sd->mapindex)) )
		{
			Sql_ShowDebug(logmysql_handle);
			return 0;
		}
	}
	else
#endif
	{
		FILE* logfp;
		if((logfp=fopen(log_config.log_zeny,"a+")) == NULL)
			return 0;
		time(&curtime);
		strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp, "%s - %s[%d]\t%s[%d]\t%d\t\n", timestring, src_sd->status.name, src_sd->status.account_id, sd->status.name, sd->status.account_id, amount);
		fclose(logfp);
	}

	return 1;
}

int log_mvpdrop(struct map_session_data *sd, int monster_id, int *log_mvp)
{
	if(!log_config.enable_logs)
		return 0;

	nullpo_retr(0, sd);

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		if (SQL_ERROR == Sql_Query(logmysql_handle, "INSERT DELAYED INTO `%s` (`mvp_date`, `kill_char_id`, `monster_id`, `prize`, `mvpexp`, `map`) VALUES (NOW(), '%d', '%d', '%d', '%d', '%s') ",
			log_config.log_mvpdrop_db, sd->status.char_id, monster_id, log_mvp[0], log_mvp[1], mapindex_id2name(sd->mapindex)) )
		{
			Sql_ShowDebug(logmysql_handle);
			return 0;
		}
	}
	else
#endif
	{
		FILE* logfp;
		if((logfp=fopen(log_config.log_mvpdrop,"a+")) == NULL)
			return 0;
		time(&curtime);
		strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp,"%s - %s[%d:%d]\t%d\t%d,%d\n", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, monster_id, log_mvp[0], log_mvp[1]);
		fclose(logfp);
	}

	return 1;
}


int log_atcommand(struct map_session_data* sd, const char* message)
{
	if(!log_config.enable_logs)
		return 0;

	nullpo_retr(0, sd);

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		SqlStmt* stmt;

		stmt = SqlStmt_Malloc(logmysql_handle);
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "INSERT DELAYED INTO `%s` (`atcommand_date`, `account_id`, `char_id`, `char_name`, `map`, `command`) VALUES (NOW(), '%d', '%d', ?, '%s', ?)", log_config.log_gm_db, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex) )
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, sd->status.name, strnlen(sd->status.name, NAME_LENGTH))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (char*)message, safestrnlen(message, 255))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return 0;
		}
		SqlStmt_Free(stmt);
	}
	else
#endif
	{
		FILE* logfp;
		if((logfp = fopen(log_config.log_gm, "a+")) == NULL)
			return 0;
		time(&curtime);
		strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp, "%s - %s[%d]: %s\n", timestring, sd->status.name, sd->status.account_id, message);
		fclose(logfp);
	}
	
	return 1;
}

int log_npc(struct map_session_data* sd, const char* message)
{
	if(!log_config.enable_logs)
		return 0;

	nullpo_retr(0, sd);

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		SqlStmt* stmt;
		stmt = SqlStmt_Malloc(logmysql_handle);
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "INSERT DELAYED INTO `%s` (`npc_date`, `account_id`, `char_id`, `char_name`, `map`, `mes`) VALUES (NOW(), '%d', '%d', ?, '%s', ?)", log_config.log_npc_db, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex) )
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, sd->status.name, strnlen(sd->status.name, NAME_LENGTH))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (char*)message, safestrnlen(message, 255))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return 0;
		}
		SqlStmt_Free(stmt);
	}
	else
#endif
	{
		FILE* logfp;
		if((logfp = fopen(log_config.log_npc, "a+")) == NULL)
			return 0;
		time(&curtime);
		strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp, "%s - %s[%d]: %s\n", timestring, sd->status.name, sd->status.account_id, message);
		fclose(logfp);
	}

	return 1;
}

int log_chat(const char* type, int type_id, int src_charid, int src_accid, const char* map, int x, int y, const char* dst_charname, const char* message)
{
	// Log CHAT (Global, Whisper, Party, Guild, Main chat)
	// LOGGING FILTERS [Lupus]
	// =============================================================
	// 0 = Don't log at all
	// 1 = Log EVERYTHING!
	// Advanced Filter Bits: ||
	// 02 - Log Global messages
	// 04 - Log Whisper messages
	// 08 - Log Party messages
	// 16 - Log Guild messages
	// 32 - Log Main chat messages
	// 64 - Don't log anything when WOE is on

	//Check ON/OFF
	if(log_config.chat <= 0)
		return 0; //Deactivated

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		SqlStmt* stmt;
		
		stmt = SqlStmt_Malloc(logmysql_handle);
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "INSERT DELAYED INTO `%s` (`time`, `type`, `type_id`, `src_charid`, `src_accountid`, `src_map`, `src_map_x`, `src_map_y`, `dst_charname`, `message`) VALUES (NOW(), '%s', '%d', '%d', '%d', '%s', '%d', '%d', ?, ?)", log_config.log_chat_db, type, type_id, src_charid, src_accid, map, x, y)
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, (char*)dst_charname, safestrnlen(dst_charname, NAME_LENGTH))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (char*)message, safestrnlen(message, CHAT_SIZE_MAX))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return 0;
		}
		SqlStmt_Free(stmt);
	}
	else
#endif
	{
		FILE* logfp;
		if((logfp = fopen(log_config.log_chat, "a+")) == NULL)
			return 0;
		time(&curtime);
		strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp, "%s - %s,%d,%d,%d,%s,%d,%d,%s,%s\n", timestring, type, type_id, src_charid, src_accid, map, x, y, dst_charname, message);
		fclose(logfp);
	}

	return 1;
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

	while(fgets(line, sizeof(line), fp))
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
				log_config.sql_logs = (bool)atoi(w2);
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
				if(log_config.branch > 0 && !log_config.sql_logs)
					ShowNotice("Logging Dead Branch Usage to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_pick_file") == 0) {
				strcpy(log_config.log_pick, w2);
				if(log_config.filter > 0 && !log_config.sql_logs)
					ShowNotice("Logging Item Picks to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_zeny_file") == 0) {
				strcpy(log_config.log_zeny, w2);
				if(log_config.zeny > 0 && !log_config.sql_logs)
					ShowNotice("Logging Zeny to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_mvpdrop_file") == 0) {
				strcpy(log_config.log_mvpdrop, w2);
				if(log_config.mvpdrop > 0 && !log_config.sql_logs)
					ShowNotice("Logging MVP Drops to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_gm_file") == 0) {
				strcpy(log_config.log_gm, w2);
				if(log_config.gm > 0 && !log_config.sql_logs)
					ShowNotice("Logging GM Level %d Commands to file `%s`.txt\n", log_config.gm, w2);
			} else if(strcmpi(w1, "log_npc_file") == 0) {
				strcpy(log_config.log_npc, w2);
				if(log_config.npc > 0 && !log_config.sql_logs)
					ShowNotice("Logging NPC 'logmes' to file `%s`.txt\n", w2);
			} else if(strcmpi(w1, "log_chat_file") == 0) {
				strcpy(log_config.log_chat, w2);
				if(log_config.chat > 0 && !log_config.sql_logs)					
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
