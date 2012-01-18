// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/sql.h" // SQL_INNODB
#include "../common/strlib.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "battle.h"
#include "itemdb.h"
#include "log.h"
#include "map.h"
#include "mob.h"
#include "pc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// filters for item logging
typedef enum e_log_filter
{
	LOG_FILTER_NONE     = 0x000,
	LOG_FILTER_ALL      = 0x001,
	// bits
	LOG_FILTER_HEALING  = 0x002,  // Healing items (0)
	LOG_FILTER_ETC_AMMO = 0x004,  // Etc Items(3) + Arrows (10)
	LOG_FILTER_USABLE   = 0x008,  // Usable Items(2) + Scrolls, Lures(11) + Usable Cash Items(18)
	LOG_FILTER_WEAPON   = 0x010,  // Weapons(4)
	LOG_FILTER_ARMOR    = 0x020,  // Shields, Armors, Headgears, Accessories, Garments and Shoes(5)
	LOG_FILTER_CARD     = 0x040,  // Cards(6)
	LOG_FILTER_PETITEM  = 0x080,  // Pet Accessories(8) + Eggs(7) (well, monsters don't drop 'em but we'll use the same system for ALL logs)
	LOG_FILTER_PRICE    = 0x100,  // Log expensive items ( >= price_log )
	LOG_FILTER_AMOUNT   = 0x200,  // Log large amount of items ( >= amount_log )
	LOG_FILTER_REFINE   = 0x400,  // Log refined items ( refine >= refine_log ) [not implemented]
	LOG_FILTER_CHANCE   = 0x800,  // Log rare items and Emperium ( drop chance <= rare_log )
}
e_log_filter;


struct Log_Config log_config;


#ifdef SQL_INNODB
// database is using an InnoDB engine so do not use DELAYED
#define LOG_QUERY "INSERT"
#else
// database is using a MyISAM engine so use DELAYED
#define LOG_QUERY "INSERT DELAYED"
#endif


/// obtain log type character for item/zeny logs
static char log_picktype2char(e_log_pick_type type)
{
	switch( type )
	{
		case LOG_TYPE_TRADE:            return 'T';  // (T)rade
		case LOG_TYPE_VENDING:          return 'V';  // (V)ending
		case LOG_TYPE_PICKDROP_PLAYER:  return 'P';  // (P)player
		case LOG_TYPE_PICKDROP_MONSTER: return 'M';  // (M)onster
		case LOG_TYPE_NPC:              return 'S';  // NPC (S)hop
		case LOG_TYPE_SCRIPT:           return 'N';  // (N)PC Script
		//case LOG_TYPE_STEAL:            return 'D';  // Steal/Snatcher
		case LOG_TYPE_CONSUME:          return 'C';  // (C)onsumed
		case LOG_TYPE_PRODUCE:          return 'O';  // Pr(O)duced/Ingredients
		//case LOG_TYPE_MVP:              return 'U';  // MVP Rewards
		case LOG_TYPE_COMMAND:          return 'A';  // (A)dmin command
		case LOG_TYPE_STORAGE:          return 'R';  // Sto(R)age
		case LOG_TYPE_GSTORAGE:         return 'G';  // (G)uild storage
		case LOG_TYPE_MAIL:             return 'E';  // (E)mail attachment
		case LOG_TYPE_AUCTION:          return 'I';  // Auct(I)on
		case LOG_TYPE_BUYING_STORE:     return 'B';  // (B)uying Store
		case LOG_TYPE_LOOT:             return 'L';  // (L)oot (consumed monster pick/drop)
		case LOG_TYPE_OTHER:			return 'X';  // Other
	}

	// should not get here, fallback
	ShowDebug("log_picktype2char: Unknown pick type %d.\n", type);
	return 'X';
}


/// obtain log type character for chat logs
static char log_chattype2char(e_log_chat_type type)
{
	switch( type )
	{
		case LOG_CHAT_GLOBAL:   return 'O';  // Gl(O)bal
		case LOG_CHAT_WHISPER:  return 'W';  // (W)hisper
		case LOG_CHAT_PARTY:    return 'P';  // (P)arty
		case LOG_CHAT_GUILD:    return 'G';  // (G)uild
		case LOG_CHAT_MAINCHAT: return 'M';  // (M)ain chat
	}

	// should not get here, fallback
	ShowDebug("log_chattype2char: Unknown chat type %d.\n", type);
	return 'O';
}


/// check if this item should be logged according the settings
static bool should_log_item(int nameid, int amount, int refine)
{
	int filter = log_config.filter;
	struct item_data* id;

	if( ( id = itemdb_exists(nameid) ) == NULL )
		return false;

	if( ( filter&LOG_FILTER_ALL ) ||
		( filter&LOG_FILTER_HEALING && id->type == IT_HEALING ) ||
		( filter&LOG_FILTER_ETC_AMMO && ( id->type == IT_ETC || id->type == IT_AMMO ) ) ||
		( filter&LOG_FILTER_USABLE && ( id->type == IT_USABLE || id->type == IT_CASH ) ) ||
		( filter&LOG_FILTER_WEAPON && id->type == IT_WEAPON ) ||
		( filter&LOG_FILTER_ARMOR && id->type == IT_ARMOR ) ||
		( filter&LOG_FILTER_CARD && id->type == IT_CARD ) ||
		( filter&LOG_FILTER_PETITEM && ( id->type == IT_PETEGG || id->type == IT_PETARMOR ) ) ||
		( filter&LOG_FILTER_PRICE && id->value_buy >= log_config.price_items_log ) ||
		( filter&LOG_FILTER_AMOUNT && abs(amount) >= log_config.amount_items_log ) ||
		( filter&LOG_FILTER_REFINE && refine >= log_config.refine_items_log ) ||
		( filter&LOG_FILTER_CHANCE && ( ( id->maxchance != -1 && id->maxchance <= log_config.rare_items_log ) || id->nameid == ITEMID_EMPERIUM ) )
	)
		return true;

	return false;
}


/// logs items, that summon monsters
void log_branch(struct map_session_data* sd)
{
	nullpo_retv(sd);

	if( !log_config.branch )
		return;

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		SqlStmt* stmt;
		stmt = SqlStmt_Malloc(logmysql_handle);
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, LOG_QUERY " INTO `%s` (`branch_date`, `account_id`, `char_id`, `char_name`, `map`) VALUES (NOW(), '%d', '%d', ?, '%s')", log_config.log_branch, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex) )
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, sd->status.name, strnlen(sd->status.name, NAME_LENGTH))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return;
		}
		SqlStmt_Free(stmt);
	}
	else
#endif
	{
		char timestring[255];
		time_t curtime;
		FILE* logfp;

		if( ( logfp = fopen(log_config.log_branch, "a") ) == NULL )
			return;
		time(&curtime);
		strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp,"%s - %s[%d:%d]\t%s\n", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex));
		fclose(logfp);
	}
}

/// logs item transactions (generic)
void log_pick(int id, int m, e_log_pick_type type, int amount, struct item* itm)
{
	nullpo_retv(itm);
	if( ( log_config.enable_logs&type ) == 0 )
	{// disabled
		return;
	}

	if( !should_log_item(itm->nameid, amount, itm->refine) )
		return; //we skip logging this item set - it doesn't meet our logging conditions [Lupus]

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		if( SQL_ERROR == Sql_Query(logmysql_handle, LOG_QUERY " INTO `%s` (`time`, `char_id`, `type`, `nameid`, `amount`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`) VALUES (NOW(), '%d', '%c', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s')",
			log_config.log_pick, id, log_picktype2char(type), itm->nameid, amount, itm->refine, itm->card[0], itm->card[1], itm->card[2], itm->card[3], map[m].name?map[m].name:"") )
		{
			Sql_ShowDebug(logmysql_handle);
			return;
		}
	}
	else
#endif
	{
		char timestring[255];
		time_t curtime;
		FILE* logfp;

		if( ( logfp = fopen(log_config.log_pick, "a") ) == NULL )
			return;
		time(&curtime);
		strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp,"%s - %d\t%c\t%d,%d,%d,%d,%d,%d,%d,%s\n", timestring, id, log_picktype2char(type), itm->nameid, amount, itm->refine, itm->card[0], itm->card[1], itm->card[2], itm->card[3], map[m].name?map[m].name:"");
		fclose(logfp);
	}
}

/// logs item transactions (players)
void log_pick_pc(struct map_session_data* sd, e_log_pick_type type, int amount, struct item* itm)
{
	nullpo_retv(sd);
	log_pick(sd->status.char_id, sd->bl.m, type, amount, itm);
}


/// logs item transactions (monsters)
void log_pick_mob(struct mob_data* md, e_log_pick_type type, int amount, struct item* itm)
{
	nullpo_retv(md);
	log_pick(md->class_, md->bl.m, type, amount, itm);
}

/// logs zeny transactions
void log_zeny(struct map_session_data* sd, e_log_pick_type type, struct map_session_data* src_sd, int amount)
{
	nullpo_retv(sd);

	if( !log_config.zeny || ( log_config.zeny != 1 && abs(amount) < log_config.zeny ) )
		return;

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		if( SQL_ERROR == Sql_Query(logmysql_handle, LOG_QUERY " INTO `%s` (`time`, `char_id`, `src_id`, `type`, `amount`, `map`) VALUES (NOW(), '%d', '%d', '%c', '%d', '%s')",
			log_config.log_zeny, sd->status.char_id, src_sd->status.char_id, log_picktype2char(type), amount, mapindex_id2name(sd->mapindex)) )
		{
			Sql_ShowDebug(logmysql_handle);
			return;
		}
	}
	else
#endif
	{
		char timestring[255];
		time_t curtime;
		FILE* logfp;

		if( ( logfp = fopen(log_config.log_zeny, "a") ) == NULL )
			return;
		time(&curtime);
		strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp, "%s - %s[%d]\t%s[%d]\t%d\t\n", timestring, src_sd->status.name, src_sd->status.account_id, sd->status.name, sd->status.account_id, amount);
		fclose(logfp);
	}
}


/// logs MVP monster rewards
void log_mvpdrop(struct map_session_data* sd, int monster_id, int* log_mvp)
{
	nullpo_retv(sd);

	if( !log_config.mvpdrop )
		return;

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		if( SQL_ERROR == Sql_Query(logmysql_handle, LOG_QUERY " INTO `%s` (`mvp_date`, `kill_char_id`, `monster_id`, `prize`, `mvpexp`, `map`) VALUES (NOW(), '%d', '%d', '%d', '%d', '%s') ",
			log_config.log_mvpdrop, sd->status.char_id, monster_id, log_mvp[0], log_mvp[1], mapindex_id2name(sd->mapindex)) )
		{
			Sql_ShowDebug(logmysql_handle);
			return;
		}
	}
	else
#endif
	{
		char timestring[255];
		time_t curtime;
		FILE* logfp;

		if( ( logfp = fopen(log_config.log_mvpdrop,"a") ) == NULL )
			return;
		time(&curtime);
		strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp,"%s - %s[%d:%d]\t%d\t%d,%d\n", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, monster_id, log_mvp[0], log_mvp[1]);
		fclose(logfp);
	}
}


/// logs used GM commands
void log_atcommand(struct map_session_data* sd, int cmdlvl, const char* message)
{
	nullpo_retv(sd);

	if( cmdlvl < log_config.gm )
		return;

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		SqlStmt* stmt;

		stmt = SqlStmt_Malloc(logmysql_handle);
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, LOG_QUERY " INTO `%s` (`atcommand_date`, `account_id`, `char_id`, `char_name`, `map`, `command`) VALUES (NOW(), '%d', '%d', ?, '%s', ?)", log_config.log_gm, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex) )
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, sd->status.name, strnlen(sd->status.name, NAME_LENGTH))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (char*)message, safestrnlen(message, 255))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return;
		}
		SqlStmt_Free(stmt);
	}
	else
#endif
	{
		char timestring[255];
		time_t curtime;
		FILE* logfp;

		if( ( logfp = fopen(log_config.log_gm, "a") ) == NULL )
			return;
		time(&curtime);
		strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp, "%s - %s[%d]: %s\n", timestring, sd->status.name, sd->status.account_id, message);
		fclose(logfp);
	}
}


/// logs messages passed to script command 'logmes'
void log_npc(struct map_session_data* sd, const char* message)
{
	nullpo_retv(sd);

	if( !log_config.npc )
		return;

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		SqlStmt* stmt;
		stmt = SqlStmt_Malloc(logmysql_handle);
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, LOG_QUERY " INTO `%s` (`npc_date`, `account_id`, `char_id`, `char_name`, `map`, `mes`) VALUES (NOW(), '%d', '%d', ?, '%s', ?)", log_config.log_npc, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex) )
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, sd->status.name, strnlen(sd->status.name, NAME_LENGTH))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (char*)message, safestrnlen(message, 255))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return;
		}
		SqlStmt_Free(stmt);
	}
	else
#endif
	{
		char timestring[255];
		time_t curtime;
		FILE* logfp;

		if( ( logfp = fopen(log_config.log_npc, "a") ) == NULL )
			return;
		time(&curtime);
		strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp, "%s - %s[%d]: %s\n", timestring, sd->status.name, sd->status.account_id, message);
		fclose(logfp);
	}
}


/// logs chat
void log_chat(e_log_chat_type type, int type_id, int src_charid, int src_accid, const char* map, int x, int y, const char* dst_charname, const char* message)
{
	if( ( log_config.chat&type ) == 0 )
	{// disabled
		return;
	}

	if( log_config.log_chat_woe_disable && ( agit_flag || agit2_flag ) )
	{// no chat logging during woe
		return;
	}

#ifndef TXT_ONLY
	if( log_config.sql_logs )
	{
		SqlStmt* stmt;

		stmt = SqlStmt_Malloc(logmysql_handle);
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, LOG_QUERY " INTO `%s` (`time`, `type`, `type_id`, `src_charid`, `src_accountid`, `src_map`, `src_map_x`, `src_map_y`, `dst_charname`, `message`) VALUES (NOW(), '%c', '%d', '%d', '%d', '%s', '%d', '%d', ?, ?)", log_config.log_chat, log_chattype2char(type), type_id, src_charid, src_accid, map, x, y)
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, (char*)dst_charname, safestrnlen(dst_charname, NAME_LENGTH))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (char*)message, safestrnlen(message, CHAT_SIZE_MAX))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return;
		}
		SqlStmt_Free(stmt);
	}
	else
#endif
	{
		char timestring[255];
		time_t curtime;
		FILE* logfp;

		if( ( logfp = fopen(log_config.log_chat, "a") ) == NULL )
			return;
		time(&curtime);
		strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp, "%s - %c,%d,%d,%d,%s,%d,%d,%s,%s\n", timestring, log_chattype2char(type), type_id, src_charid, src_accid, map, x, y, dst_charname, message);
		fclose(logfp);
	}
}


void log_set_defaults(void)
{
	memset(&log_config, 0, sizeof(log_config));

	//LOG FILTER Default values
	log_config.refine_items_log = 5;    // log refined items, with refine >= +5
	log_config.rare_items_log   = 100;  // log rare items. drop chance <= 1%
	log_config.price_items_log  = 1000; // 1000z
	log_config.amount_items_log = 100;
}


int log_config_read(const char* cfgName)
{
	static int count = 0;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	if( count++ == 0 )
		log_set_defaults();

	if( ( fp = fopen(cfgName, "r") ) == NULL )
	{
		ShowError("Log configuration file not found at: %s\n", cfgName);
		return 1;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		if( line[0] == '/' && line[1] == '/' )
			continue;

		if( sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2 )
		{
			if( strcmpi(w1, "enable_logs") == 0 )
				log_config.enable_logs = (e_log_pick_type)config_switch(w2);
			else if( strcmpi(w1, "sql_logs") == 0 )
			{
				log_config.sql_logs = (bool)config_switch(w2);
#ifdef TXT_ONLY
				if( log_config.sql_logs )
				{
					ShowWarning("log_config_read: SQL logging is not supported on this server.\n");
					log_config.sql_logs = false;
				}
#endif
			}
//start of common filter settings
			else if( strcmpi(w1, "rare_items_log") == 0 )
				log_config.rare_items_log = atoi(w2);
			else if( strcmpi(w1, "refine_items_log") == 0 )
				log_config.refine_items_log = atoi(w2);
			else if( strcmpi(w1, "price_items_log") == 0 )
				log_config.price_items_log = atoi(w2);
			else if( strcmpi(w1, "amount_items_log") == 0 )
				log_config.amount_items_log = atoi(w2);
//end of common filter settings
			else if( strcmpi(w1, "log_branch") == 0 )
				log_config.branch = config_switch(w2);
			else if( strcmpi(w1, "log_filter") == 0 )
				log_config.filter = config_switch(w2);
			else if( strcmpi(w1, "log_zeny") == 0 )
				log_config.zeny = config_switch(w2);
			else if( strcmpi(w1, "log_gm") == 0 )
				log_config.gm = config_switch(w2);
			else if( strcmpi(w1, "log_npc") == 0 )
				log_config.npc = config_switch(w2);
			else if( strcmpi(w1, "log_chat") == 0 )
				log_config.chat = config_switch(w2);
			else if( strcmpi(w1, "log_mvpdrop") == 0 )
				log_config.mvpdrop = config_switch(w2);
			else if( strcmpi(w1, "log_chat_woe_disable") == 0 )
				log_config.log_chat_woe_disable = (bool)config_switch(w2);
			else if( strcmpi(w1, "log_branch_db") == 0 )
				safestrncpy(log_config.log_branch, w2, sizeof(log_config.log_branch));
			else if( strcmpi(w1, "log_pick_db") == 0 )
				safestrncpy(log_config.log_pick, w2, sizeof(log_config.log_pick));
			else if( strcmpi(w1, "log_zeny_db") == 0 )
				safestrncpy(log_config.log_zeny, w2, sizeof(log_config.log_zeny));
			else if( strcmpi(w1, "log_mvpdrop_db") == 0 )
				safestrncpy(log_config.log_mvpdrop, w2, sizeof(log_config.log_mvpdrop));
			else if( strcmpi(w1, "log_gm_db") == 0 )
				safestrncpy(log_config.log_gm, w2, sizeof(log_config.log_gm));
			else if( strcmpi(w1, "log_npc_db") == 0 )
				safestrncpy(log_config.log_npc, w2, sizeof(log_config.log_npc));
			else if( strcmpi(w1, "log_chat_db") == 0 )
				safestrncpy(log_config.log_chat, w2, sizeof(log_config.log_chat));
			//support the import command, just like any other config
			else if( strcmpi(w1,"import") == 0 )
				log_config_read(w2);
		}
	}

	fclose(fp);

	if( --count == 0 )
	{// report final logging state
		const char* target = log_config.sql_logs ? "table" : "file";

		if( log_config.enable_logs && log_config.filter )
		{
			ShowInfo("Logging item transactions to %s '%s'.\n", target, log_config.log_pick);
		}
		if( log_config.branch )
		{
			ShowInfo("Logging monster summon item usage to %s '%s'.\n", target, log_config.log_pick);
		}
		if( log_config.chat )
		{
			ShowInfo("Logging chat to %s '%s'.\n", target, log_config.log_chat);
		}
		if( log_config.gm )
		{
			ShowInfo("Logging gm commands to %s '%s'.\n", target, log_config.log_gm);
		}
		if( log_config.mvpdrop )
		{
			ShowInfo("Logging MVP monster rewards to %s '%s'.\n", target, log_config.log_mvpdrop);
		}
		if( log_config.npc )
		{
			ShowInfo("Logging 'logmes' messages to %s '%s'.\n", target, log_config.log_npc);
		}
		if( log_config.zeny )
		{
			ShowInfo("Logging Zeny transactions to %s '%s'.\n", target, log_config.log_zeny);
		}
	}

	return 0;
}
