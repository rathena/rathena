// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _PC_GROUPS_HPP_
#define _PC_GROUPS_HPP_

#include "../common/cbasetypes.h"

enum AtCommandType : uint8;

extern int pc_group_max;

bool pc_group_exists(int group_id);
bool pc_group_can_use_command(int group_id, const char *command, AtCommandType type);
bool pc_group_has_permission(int group_id, int permission);
bool pc_group_should_log_commands(int group_id);
const char* pc_group_id2name(int group_id);
int pc_group_id2level(int group_id);
void pc_group_pc_load(struct map_session_data *sd);

void do_init_pc_groups(void);
void do_final_pc_groups(void);
void pc_groups_reload(void);

enum e_pc_permission : uint32 {
	PC_PERM_NONE                = 0,
	PC_PERM_TRADE               = 0x00000001,
	PC_PERM_PARTY               = 0x00000002,
	PC_PERM_ALL_SKILL           = 0x00000004,
	PC_PERM_USE_ALL_EQUIPMENT   = 0x00000008,
	PC_PERM_SKILL_UNCONDITIONAL = 0x00000010,
	PC_PERM_JOIN_ALL_CHAT       = 0x00000020,
	PC_PERM_NO_CHAT_KICK        = 0x00000040,
	PC_PERM_HIDE_SESSION        = 0x00000080,
	PC_PERM_WHO_DISPLAY_AID     = 0x00000100,
	PC_PERM_RECEIVE_HACK_INFO   = 0x00000200,
	PC_PERM_WARP_ANYWHERE       = 0x00000400,
	PC_PERM_VIEW_HPMETER        = 0x00000800,
	PC_PERM_VIEW_EQUIPMENT      = 0x00001000,
	PC_PERM_USE_CHECK           = 0x00002000,
	PC_PERM_USE_CHANGEMAPTYPE   = 0x00004000,
	PC_PERM_USE_ALL_COMMANDS    = 0x00008000,
	PC_PERM_RECEIVE_REQUESTS    = 0x00010000,
	PC_PERM_SHOW_BOSS           = 0x00020000,
	PC_PERM_DISABLE_PVM         = 0x00040000,
	PC_PERM_DISABLE_PVP         = 0x00080000,
	PC_PERM_DISABLE_CMD_DEAD    = 0x00100000,
	PC_PERM_CHANNEL_ADMIN       = 0x00200000,
	PC_PERM_TRADE_BOUNDED       = 0x00400000,
	PC_PERM_ITEM_UNCONDITIONAL  = 0x00800000,
	PC_PERM_ENABLE_COMMAND      = 0x01000000,
	PC_PERM_BYPASS_STAT_ONCLONE = 0x02000000,
	PC_PERM_BYPASS_MAX_STAT     = 0x04000000,
	PC_PERM_CASHSHOP_SALE		= 0x08000000,
	//.. add other here
	PC_PERM_ALLPERMISSION       = 0xFFFFFFFF,
};

static const struct s_pcg_permission_name {
	const char *name;
	enum e_pc_permission permission;
} pc_g_permission_name[] = {
	{ "can_trade", PC_PERM_TRADE },
	{ "can_party", PC_PERM_PARTY },
	{ "all_skill", PC_PERM_ALL_SKILL },
	{ "all_equipment", PC_PERM_USE_ALL_EQUIPMENT },
	{ "skill_unconditional", PC_PERM_SKILL_UNCONDITIONAL },
	{ "join_chat", PC_PERM_JOIN_ALL_CHAT },
	{ "kick_chat", PC_PERM_NO_CHAT_KICK },
	{ "hide_session", PC_PERM_HIDE_SESSION },
	{ "who_display_aid", PC_PERM_WHO_DISPLAY_AID },
	{ "hack_info", PC_PERM_RECEIVE_HACK_INFO },
	{ "any_warp", PC_PERM_WARP_ANYWHERE },
	{ "view_hpmeter", PC_PERM_VIEW_HPMETER },
	{ "view_equipment", PC_PERM_VIEW_EQUIPMENT },
	{ "use_check", PC_PERM_USE_CHECK },
	{ "use_changemaptype", PC_PERM_USE_CHANGEMAPTYPE },
	{ "all_commands", PC_PERM_USE_ALL_COMMANDS },
	{ "receive_requests", PC_PERM_RECEIVE_REQUESTS },
	{ "show_bossmobs", PC_PERM_SHOW_BOSS },
	{ "disable_pvm", PC_PERM_DISABLE_PVM },
	{ "disable_pvp", PC_PERM_DISABLE_PVP },
	{ "disable_commands_when_dead", PC_PERM_DISABLE_CMD_DEAD },
	{ "channel_admin", PC_PERM_CHANNEL_ADMIN },
	{ "can_trade_bounded", PC_PERM_TRADE_BOUNDED },
	{ "item_unconditional", PC_PERM_ITEM_UNCONDITIONAL },
	{ "command_enable",PC_PERM_ENABLE_COMMAND },
	{ "bypass_stat_onclone",PC_PERM_BYPASS_STAT_ONCLONE },
	{ "bypass_max_stat",PC_PERM_BYPASS_MAX_STAT },
	{ "cashshop_sale", PC_PERM_CASHSHOP_SALE },
	{ "all_permission", PC_PERM_ALLPERMISSION },
};

#endif // _PC_GROUPS_HPP_
