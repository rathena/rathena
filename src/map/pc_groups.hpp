// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PC_GROUPS_HPP
#define PC_GROUPS_HPP

#include <bitset>
#include <map>
#include <vector>

#include "../common/cbasetypes.hpp"
#include "../common/database.hpp"

// Forward declaration from atcommands.hpp
enum AtCommandType : uint8;

void pc_group_pc_load(struct map_session_data *sd);
void do_init_pc_groups(void);
void do_final_pc_groups(void);
void pc_groups_reload(void);

enum e_pc_permission : uint32 {
	PC_PERM_TRADE = 0,
	PC_PERM_PARTY,
	PC_PERM_ALL_SKILL,
	PC_PERM_USE_ALL_EQUIPMENT,
	PC_PERM_SKILL_UNCONDITIONAL,
	PC_PERM_JOIN_ALL_CHAT,
	PC_PERM_NO_CHAT_KICK,
	PC_PERM_HIDE_SESSION,
	PC_PERM_WHO_DISPLAY_AID,
	PC_PERM_RECEIVE_HACK_INFO,
	PC_PERM_WARP_ANYWHERE,
	PC_PERM_VIEW_HPMETER,
	PC_PERM_VIEW_EQUIPMENT,
	PC_PERM_USE_CHECK,
	PC_PERM_USE_CHANGEMAPTYPE,
	PC_PERM_USE_ALL_COMMANDS,
	PC_PERM_RECEIVE_REQUESTS,
	PC_PERM_SHOW_BOSS,
	PC_PERM_DISABLE_PVM,
	PC_PERM_DISABLE_PVP,
	PC_PERM_DISABLE_CMD_DEAD,
	PC_PERM_CHANNEL_ADMIN,
	PC_PERM_TRADE_BOUNDED,
	PC_PERM_ITEM_UNCONDITIONAL,
	PC_PERM_ENABLE_COMMAND,
	PC_PERM_BYPASS_STAT_ONCLONE,
	PC_PERM_BYPASS_MAX_STAT,
	PC_PERM_ATTENDANCE,
	//.. add other here
	PC_PERM_MAX,
};

static const struct s_pcg_permission_name {
	const char *name;
	enum e_pc_permission permission;
} pc_g_permission_name[PC_PERM_MAX] = {
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
	{ "attendance",PC_PERM_ATTENDANCE },
};

struct s_player_group{
	uint32 id;
	std::string name;
	uint32 level;
	bool log_commands;
	std::vector<std::string> commands;
	std::vector<std::string> char_commands;
	std::bitset<PC_PERM_MAX> permissions;
	uint32 index;

public:
	bool can_use_command( const std::string& command, AtCommandType type );
	bool has_permission( e_pc_permission permission );
	bool should_log_commands();
};

class PlayerGroupDatabase : public TypesafeYamlDatabase<uint32, s_player_group>{
private:
	std::map<uint32, std::vector<std::string>> inheritance;
	bool parseCommands( const ryml::NodeRef& node, std::vector<std::string>& commands );

public:
	PlayerGroupDatabase() : TypesafeYamlDatabase( "PLAYER_GROUP_DB", 1 ){

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode( const ryml::NodeRef& node ) override;
	void loadingFinished() override;
};

extern PlayerGroupDatabase player_group_db;

#endif /* PC_GROUPS_HPP */
