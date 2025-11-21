#ifndef DISCORD_HPP
#define DISCORD_HPP

#include <vector>
#include <string>

#include "mob.hpp"

struct DiscordChannelConfig {
	bool enabled;
	bool mention_everyone;
	char webhook_url[256];
};

struct Discord_Config {
	DiscordChannelConfig mvp;
	DiscordChannelConfig status;
	DiscordChannelConfig card;
};

extern Discord_Config discord_config;

void discord_set_defaults(void);
bool discord_handle_config(const char* key, const char* value);

void discord_notify_mvp_kill(mob_data* md, map_session_data* killer, const std::vector<s_mvp_damage_entry>& lootdmg, int64 total_damage, uint32 kill_duration_ms, t_itemid mvp_reward_item_id, t_exp mvp_reward_exp);
void discord_notify_mvp_respawn(mob_data* md);
void discord_notify_card_drop(map_session_data* sd, uint16 mob_id, t_itemid item_id);

void discord_notify_server_status(bool online, int32 player_count, bool force = false, bool wait_completion = false, bool force_all_offline = false);
void discord_on_user_count_change(int32 users);
void discord_handle_server_shutdown(void);

#endif /* DISCORD_HPP */
