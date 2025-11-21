#ifndef DISCORD_HPP
#define DISCORD_HPP

#include <common/mmo.hpp>  // For int64, t_itemid, t_exp

// Forward declarations
struct mob_data;

// Discord webhook integration functions

void discord_set_mvp_webhook(const char* url);
void discord_set_card_webhook(const char* url);
void discord_set_server_webhook(const char* url);
void discord_set_server_label(const char* label);
const char* discord_get_server_label();

bool discord_handle_config(const char* key, const char* value);

void discord_clear_mvp_messages_on_start();
void discord_clear_card_messages_on_start();

void discord_notify_mvp_kill(
	const char* mvp_name,
	const char* map_name,
	const char* killer_name,
	int kill_duration_ms,
	const char* top1_name, int64 top1_damage,
	const char* top2_name, int64 top2_damage,
	const char* top3_name, int64 top3_damage,
	t_itemid mvp_reward_item_id,
	t_exp mvp_reward_exp
);
void discord_notify_mvp_respawn(struct mob_data* md);
void discord_notify_card_drop(const char* card_name, const char* mob_name, const char* map_name, const char* player_name);

void discord_notify_server_online();
void discord_notify_server_offline();

#endif /* DISCORD_HPP */
