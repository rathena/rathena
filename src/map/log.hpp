// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef LOG_HPP
#define LOG_HPP

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

struct block_list;
class map_session_data;
struct mob_data;
struct item;

enum e_log_chat_type : uint8
{
	LOG_CHAT_GLOBAL      = 0x01,
	LOG_CHAT_WHISPER     = 0x02,
	LOG_CHAT_PARTY       = 0x04,
	LOG_CHAT_GUILD       = 0x08,
	LOG_CHAT_MAINCHAT    = 0x10,
	LOG_CHAT_CLAN        = 0x20,
	// all
	LOG_CHAT_ALL         = 0xFF,
};

enum e_log_pick_type : uint32
{
	LOG_TYPE_NONE             = 0x00000000,
	LOG_TYPE_TRADE            = 0x00000001,
	LOG_TYPE_VENDING          = 0x00000002,
	LOG_TYPE_PICKDROP_PLAYER  = 0x00000004,
	LOG_TYPE_PICKDROP_MONSTER = 0x00000008,
	LOG_TYPE_NPC              = 0x00000010,
	LOG_TYPE_SCRIPT           = 0x00000020,
	LOG_TYPE_STEAL            = 0x00000040,
	LOG_TYPE_CONSUME          = 0x00000080,
	LOG_TYPE_PRODUCE          = 0x00000100,
	LOG_TYPE_MVP              = 0x00000200,
	LOG_TYPE_COMMAND          = 0x00000400,
	LOG_TYPE_STORAGE          = 0x00000800,
	LOG_TYPE_GSTORAGE         = 0x00001000,
	LOG_TYPE_MAIL             = 0x00002000,
	LOG_TYPE_AUCTION          = 0x00004000,
	LOG_TYPE_BUYING_STORE     = 0x00008000,
	LOG_TYPE_OTHER            = 0x00010000,
	LOG_TYPE_CASH             = 0x00020000,
	LOG_TYPE_BANK             = 0x00040000,
	LOG_TYPE_BOUND_REMOVAL    = 0x00080000,
	LOG_TYPE_ROULETTE         = 0x00100000,
	LOG_TYPE_MERGE_ITEM       = 0x00200000,
	LOG_TYPE_QUEST            = 0x00400000,
	LOG_TYPE_PRIVATE_AIRSHIP  = 0x00800000,
	LOG_TYPE_BARTER           = 0x01000000,
	LOG_TYPE_LAPHINE          = 0x02000000,
	LOG_TYPE_ENCHANTGRADE     = 0x04000000,
	LOG_TYPE_REFORM           = 0x08000000,
	LOG_TYPE_ENCHANT          = 0x10000000,
	LOG_TYPE_PACKAGE          = 0x20000000,
	// combinations
	LOG_TYPE_LOOT             = LOG_TYPE_PICKDROP_MONSTER|LOG_TYPE_CONSUME,
	// all
	LOG_TYPE_ALL              = 0xFFFFFFFF,
};

enum e_log_cash_type : uint8
{
	LOG_CASH_TYPE_CASH = 0x1,
	LOG_CASH_TYPE_KAFRA = 0x2
};

enum e_log_feeding_type : uint8 
{
	LOG_FEED_HOMUNCULUS = 0x1,
	LOG_FEED_PET        = 0x2,
};

/// new logs
void log_pick_pc(map_session_data* sd, e_log_pick_type type, int32 amount, struct item* itm);
void log_pick_mob(struct mob_data* md, e_log_pick_type type, int32 amount, struct item* itm);
void log_zeny(const map_session_data &target_sd, e_log_pick_type type, uint32 src_id, int32 amount);
void log_cash( map_session_data* sd, e_log_pick_type type, e_log_cash_type cash_type, int32 amount );
void log_npc( struct npc_data* nd, const char* message );
void log_npc(map_session_data* sd, const char *message);
void log_chat(e_log_chat_type type, int32 type_id, int32 src_charid, int32 src_accid, const char* map, int32 x, int32 y, const char* dst_charname, const char* message);
void log_atcommand(map_session_data* sd, const char* message);
void log_feeding(map_session_data *sd, e_log_feeding_type type, t_itemid nameid);

/// old, but useful logs
void log_branch(map_session_data* sd);
void log_mvpdrop(map_session_data* sd, int32 monster_id, t_itemid nameid, t_exp exp);

int32 log_config_read(const char* cfgName);

extern struct Log_Config
{
	e_log_pick_type enable_logs;
	int32 filter;
	bool sql_logs;
	bool log_chat_woe_disable;
	bool cash;
	int32 rare_items_log,refine_items_log,price_items_log,amount_items_log; //for filter
	int32 branch, mvpdrop, zeny, commands, npc, chat;
	unsigned feeding : 2;
	char log_branch[64], log_pick[64], log_zeny[64], log_mvpdrop[64], log_gm[64], log_npc[64], log_chat[64], log_cash[64];
	char log_feeding[64];
} log_config;

#endif /* LOG_HPP */
