//
//  discord_core.cpp
//  roCORD
//
//  Created by Norman Ziebal on 21.08.18.
//  Copyright © 2018 Norman Ziebal. All rights reserved.
//

#include "discord_core.hpp"
#include <boost/locale.hpp>
#include <future>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#ifndef TESTING
#include "../channel.hpp"
#include "../clif.hpp"
#endif
#include "discord_error.hpp"

namespace rocord {

core::core(std::string display_name_, std::string token_, std::string presence_,
           int debug_,
           std::shared_ptr<std::vector<std::pair<std::string, std::string>>>
             channel_mapping_,
           std::unique_ptr<websocket> dwss_, std::unique_ptr<http> dhttps_,
           std::shared_ptr<log> logger_)
  : display_name(display_name_)
  , token(token_)
  , presence(presence_)
  , debug(debug_)
  , channel_mapping(channel_mapping_)
  , dwss(std::move(dwss_))
  , dhttps(std::move(dhttps_))
  , logger(logger_)
{
  this->start_time = std::chrono::system_clock::now();
  this->info();
}

core::~core()
{
  this->state = OFF;
  logger->print("Core is shutting down now!", log_type::info);
}

/* Public
 * Connects to the discord api.
 */
void
core::connect()
{
  // TODO: check if already connected
  this->dwss->start();
}

/*
 * Public
 * Sends a message from rAthena SRC to Discord
 * @param name, can be nullptr if message not from player
 */
int
core::to_discord(std::string& msg, const std::string& channel,
                 std::string* name)
{
  /*
     if (!name)
   *name = ""; // if webhooks are used, this should be the bot name;
   */
  if (this->get_state() == OFF) {
    // ShowError("Bot is not in ON State!");
    logger->print("Bot is not in ON State!", log_type::warning);
    return -1;
  }

  std::string channel_id;
  for (auto it = channel_mapping->begin(); it != channel_mapping->end(); it++) {
    if (it->first == channel) {
      channel_id = it->second;
      break;
    }
  }

  if (channel_id.empty()) {
    // ShowWarning("Discord channel has no mapping!");
    return -2; // no mapping
  }
#ifdef _WIN32
  if (name)
    msg.erase(0, 3); // TODO: what happens when executing script cmd on linux,
// same as osx?
#endif
  convert_latin1(msg);
  std::stringstream ss;
  if (name)
    // TODO: convert_latin1(name) requires template
    ss << "<" << *name << "> " << msg; // << " | " << channel_id;
  else
    ss << msg;
  this->dhttps->send(ss.str(), channel_id);
  return 0;
}

/*
 * Public
 * Return current connection state
 */
State
core::get_state()
{
  return this->state;
}

void
core::restart_websocket()
{
  this->state = OFF;
  handle_close();
}
/*
 * Public
 * A public method to change the display name via discord_bot adapter.
 */
void
core::set_display_name(const std::string& display_name)
{
  if (this->display_name == display_name)
    return;
  this->dhttps->setDisplayName(display_name, this->guild_id);
  this->display_name = display_name;
}

/*
 *  Public
 *  Bans a user from discord
 */
void
core::ban_member(member& memb, const std::string& reason,
                 int delete_message_days)
{

  //  this.https->send_ban_event(memb->user.id, reason, delete_message_days);
}

void
core::change_nick(member& memb, const std::string& new_nick)
{
  // TODO: implement
}
/*
 * Public
 * Handles events from discord API.
 */
void
core::handle_events()
{
  std::function<void(core*)> event = this->dwss->get_next_event();
  if (event)
    event(this);
  logger->handle_print();
}

/*
 * Public
 * Displays an info about the loaded config!
 */
void
core::info()
{
  std::stringstream ss;
  std::string msg;

  ss << "Core loaded with config:\n";
  ss << " - Bot name: " << this->display_name << "\n";
  ss << " - Token: " << this->token << "\n";
  ss << " - Presence: " << this->presence << "\n";
  ss << " - Channel mapping: \n";
  for (auto it = this->channel_mapping->begin();
       it != this->channel_mapping->end(); it++) {
    ss << "\t\t" << it->first << " <-> " << it->second << std::endl; // TODO
  }
  // ShowInfo(ss.str().c_str());
  logger->print(ss.str(), log_type::status);
}

/*
 * Private
 * Handles the Ready Event from Discord API.
 */
void
core::handle_ready(const std::string& guild_id)
{
  // ShowInfo("Discord: Ready Event!");
  logger->print("API: Ready event!", log_type::info);
  this->guild_id = guild_id; // TODO
  this->dhttps->setDisplayName(this->display_name,
                               this->guild_id); // init set of display_name
  std::string payload = " * We launched into outer space * "; // DEBUG VALUE
  this->dhttps->send(payload, channel_mapping->begin()->second);
  this->state = ON;
}

/*
 * Private
 * Handles a message from Discord server.
 */
//   core::handle:message_create(message msg)
void
core::handle_message_create(std::shared_ptr<member> membr, std::string& content,
                            const std::string& d_channel)
{
  const std::string author = membr->get_username();
  const std::string nick = membr->get_nick();
  // ShowInfo("Discord: Message Event!");
  logger->print("API: Message Event!", log_type::info);
  if (author == this->display_name)
    return;

  if (content.length() > 150)
    return;

  std::string channel = "#";
  for (auto it = channel_mapping->begin(); it != channel_mapping->end(); it++) {
    if (it->second == d_channel) {
      channel.append(it->first);
      break;
    }
  }

  if (channel.empty()) {
    // ShowWarning("Discord channel has no mapping!");
    logger->print("Channel has no mapping!", log_type::info);
    return;
  }

// discord_core::convert_utf8(content);
#ifndef TESTING
  Channel* r_channel = channel_name2channel((char*)channel.c_str(), NULL, 0);
  if (!r_channel) {
    // ShowError("[roCORD] Channel was not found!");
    logger->print("Channel was not found!", log_type::error);
    return;
  }

  std::string msg = r_channel->alias;
#else
  std::string msg = channel;
#endif
  convert_utf8(content);

  // Checking if #channel or @main was used, remove it for now!
  content = std::regex_replace(content, std::regex("<@![0-9]*>"), "");
  content = std::regex_replace(content, std::regex("<#[0-9]*>"), "");

  // Remove whitespace at the end and beginning.
  content = std::regex_replace(content, std::regex("^\\s|\\s$"), "");
  if (content.length() < 1)
    return;
  msg.append("<");
  if (!nick.empty() && check_ISO8859_1(nick))
    msg.append(nick);
  else if (check_ISO8859_1(author))
    msg.append(author);
  else
    return;
  msg.append(">: ");
  msg.append(content);

#ifdef TESTING
  // ShowInfo(msg.c_str());
  logger->print(msg, log_type::info);
#else
  clif_channel_msg(r_channel, msg.c_str(), r_channel->color);
#endif
}

/*
 * Private
 * Handles the content of the Discord server.
 */
void
core::handle_guild_create()
{
  // ShowInfo("Discord: GuildCreate Event");
  // std::cout << "GuildCreate has to be handled!" << std::endl;
  logger->print("API: GuildCreate Event", log_type::info);
  logger->print("GuildCreate has to be handled!", log_type::debug);
}

/*
 * Private
 * Handles the init Event from Discord API.
 */
void
core::handle_hello(int heartbeat_interval)
{
  //  ShowInfo("Discord: Hello Event!");
  logger->print("API: Hello Event!", log_type::debug);
  this->dwss->start_heartbeat(heartbeat_interval / 2); // TODO configable
  this->dwss->send_identify(this->token, this->presence);
  this->state = CONNECTING;
}

void
core::handle_close()
{
  // this->dhttps->send("Debugging: WebSocket was closed! Trying to restart!",
  //                   channel_mapping->begin()->second);
  // ShowError("WebSocket was closed! Trying to restart!");
  logger->print("Websocket was closed! Trying to restart!", log_type::status);
  this->dwss.reset(new websocket(
    this->token, "wss://gateway.discord.gg/?v=6&encoding=json", this->logger));
  connect();
}

/*
 * Private
 * Gives information about the bot back to discord.
*/
void
core::handle_cmd_info(const std::string& channel_id)
{
  this->dhttps->send("Bot created by norm.\nAvailable commands:\n- !info: "
                     "shows this info text\n- !uptime: shows how long "
                     "bot/socket/server are running without a restart\n",
                     channel_id);
}

/*
 * Private
 */
void
core::handle_cmd_uptime(const std::string& channel_id)
{
  std::chrono::time_point<std::chrono::system_clock> bot, socket;
  int bot_uptime_h = std::chrono::duration_cast<std::chrono::hours>(
                       std::chrono::system_clock::now() - this->start_time)
                       .count();
  int socket_uptime_h =
    std::chrono::duration_cast<std::chrono::hours>(
      std::chrono::system_clock::now() - this->dwss->getStartTime())
      .count();
  int socket_uptime_m =
    std::chrono::duration_cast<std::chrono::minutes>(
      std::chrono::system_clock::now() - this->dwss->getStartTime())
      .count();
  int bot_uptime_m = std::chrono::duration_cast<std::chrono::minutes>(
                       std::chrono::system_clock::now() - this->start_time)
                       .count();

  socket_uptime_m -= socket_uptime_h * 60;
  bot_uptime_m -= bot_uptime_h * 60;
  std::ostringstream ss;
  ss << "Uptime:\n--> Bot: " << bot_uptime_h << "h " << bot_uptime_m
     << "m\n--> Socket: " << socket_uptime_h << "h " << socket_uptime_m
     << "m\n--> Server: not available";
  this->dhttps->send(ss.str(), channel_id);
}

/*
 * Private
 */
bool
core::check_ISO8859_1(const std::string& content)
{
  try {
    boost::locale::conv::from_utf(content, "ISO-8859-1",
                                  boost::locale::conv::method_type::stop);
  } catch (const boost::locale::conv::conversion_error& e) {
    return false;
  }
  return true;
}

/*
 * Private
 */
void
core::convert_utf8(std::string& content)
{
  content = boost::locale::conv::from_utf(content, "ISO-8859-1");
}

/*
 * Private
 */
void
core::convert_latin1(std::string& content)
{
  std::string latin1 = "ISO-8859-1";
  content = boost::locale::conv::to_utf<char>(content, "ISO-8859-1");
}
}
