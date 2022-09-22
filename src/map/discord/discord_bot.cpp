//
//  discord_bot.cpp
//  roCORD
//
//  Created by Norman Ziebal on 21.08.18.
//  Copyright © 2018 Norman Ziebal. All rights reserved.
//

#include "discord_bot.hpp"
#include "discord_core.hpp"
#include "discord_http.hpp"
#include "discord_websocket.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace nlohmann;

// TODO remove global vars
std::unique_ptr<rocord::core> dcore;
std::shared_ptr<rocord::log> logger =
  std::shared_ptr<rocord::log>(new rocord::log());

#ifdef TESTING
void
discord_handle()
{
  dcore->handle_events();
  // handle logs
}
#else
/*
 * Entry point to hand control to bot.
 */
TIMER_FUNC(discord_handle)
{
  if (!dcore) {
    logger->print("core is not running!", rocord::log_type::error);
    return -1;
  }

  dcore->handle_events();
  // handle logs
  // add_timer(gettick() + 100, discord_handle, 0, 0);
  return 0;
}
#endif

/*
 * Entrypoint to send a message to discord.
 */
void
discord_send(const char* msg, const char* channel, const char* name)
{
  if (!dcore) {
    logger->print("core is not running!", rocord::log_type::error);
    return;
  }

  if (!msg || !channel || !name) {
    logger->print("discord_send arguments contained nullptr!",
                  rocord::log_type::error);
    return;
  }

  std::string msg_s = msg;
  std::string channel_s = channel;
  std::string name_s = name;
  dcore->to_discord(msg_s, channel_s, &name_s);
}

/*
 * Entrypoint for script command.
 */
int
discord_script(const char* msg, const char* channel)
{
  if (!dcore) {
    logger->print("core is not running!", rocord::log_type::error);
    return -1;
  }

  std::string msg_s = msg;
  std::string channel_s = channel;
  return dcore->to_discord(msg_s, channel_s, nullptr);
}

/*
 * Entrypoint to announce mobdrops.
 */
void
discord_announce_drop(const char* msg)
{
  if (!dcore) {
    logger->print("core is not running!", rocord::log_type::error);
    return;
  }

  std::string channel = "drop_announce";
  std::string msg_s = msg;
  dcore->to_discord(msg_s, channel, nullptr);
}

/*
 * Restarts a specific part of the bot.
 */
int
discord_restart(const std::string& type)
{
  if (!dcore) {
    logger->print("core is not running!", rocord::log_type::error);
    return -1;
  }

  if (type == "soft") {
    dcore->restart_websocket();
  } else {
    logger->print("Restart has failed!", rocord::log_type::warning);
    return -1;
  }
  return 0;
}

/*
 * Inits the core functionality of the bot. With the given config.
 * A config file looks like following:
 *{
 *      "version": 1,                   # Required
 *      "debug": 0,
 *      "log_level": 255 (*)
 *      "display_name": "roCORD",
 *      "token": "<token>",             # Required
 *      "presence": "by Normynator",
 *      "channels" : {
 *         "general": "1234567890"      # Required
 *      }
 *}
 *
 * (*) the log level is computed this way:
 * debug:		1
 * info:		2
 * status:	4
 * warning: 8
 * error:		16
 *
 * now you can combine any log levels like this:
 *	error, warning and status => 16 + 8 + 4 = 28
 *	so log_level would be 28.
 */
int
discord_init()
{
  // ShowStatus("Loading roCORD by norm\n");
  logger->welcome();
#ifdef TESTING
  std::ifstream ifs("../config/config.json"); // TODO: fix hardcoded path!
#else
  std::ifstream ifs("conf/discord/config.json");
#endif
  json data;
  int debug;
  std::shared_ptr<std::vector<std::pair<std::string, std::string>>>
    channel_mapping;
  std::string display_name, token, presence;
  if (ifs.fail()) {
    // ShowError("[roCORD] Failed to open config.json!\n");
    logger->print("Failed to open config.json!", rocord::log_type::error);
    return -1;
  }

  try {
    data = json::parse(ifs);

    if (data.find("log_level") != data.end())
      logger->set_level(data.at("log_level"));
    else
      logger->print("Log level is not defined. Using default value!",
                    rocord::log_type::warning);

    if (data.find("token") != data.end())
      token = data.at("token");
    else {
      // ShowError("[roCORD] Token is not defined! Aborting!\n");
      logger->print("Token is not defined! Aborting!", rocord::log_type::error);
      return -1;
    }

    if (data.find("display_name") != data.end())
      display_name = data.at("display_name");
    else {
      // ShowInfo("[roCORD] No display_name defined using alternative!\n");
      logger->print("No display_name defined using alternative!",
                    rocord::log_type::warning);
      display_name = "roCORD";
    }

    if (data.find("presence") != data.end())
      presence = data.at("presence");
    else {
      // ShowInfo("[roCORD] No presence defined using alternative!\n");
      logger->print("No presence defined using alternative!",
                    rocord::log_type::warning);
      presence = "by Normynator";
    }

    if (data.find("debug") != data.end())
      debug = data.at("debug");
    else {
      debug = 0;
    }

    channel_mapping = std::make_shared<
      std::vector<std::pair<std::string, std::string>>>(); // TODO leak?
    for (auto it = data.at("channels").begin(); it != data.at("channels").end();
         ++it) {
      channel_mapping->push_back(std::make_pair<std::string, std::string>(
        ((json)(it.key())), ((json)(it.value()))));
    }

    if (channel_mapping->empty()) {
      // ShowError("[roCORD] No channel mapping found! Aborting!\n");
      logger->print("No channel mapping found! Aborting!",
                    rocord::log_type::error);
      return -1;
    }
  } catch (json::parse_error& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  std::unique_ptr<rocord::websocket> dwss(new rocord::websocket(
    token, "wss://gateway.discord.gg/?v=6&encoding=json", logger)); // TODO use
  // factory pattern
  std::unique_ptr<rocord::http> dhttps(new rocord::http(token, logger));

  /* TODO:
   *  Validate channel mapping !
   *  Check if the given channels do exist.
   *  Maybe validate somewhere else, since we dont know the Discord Channels
   * yet!
   */
  dcore = std::unique_ptr<rocord::core>(
    new rocord::core(display_name, token, presence, debug, channel_mapping,
                     std::move(dwss), std::move(dhttps), logger));
  dcore->connect();
#ifndef TESTING
  add_timer_func_list(discord_handle, "discord_handle");
  add_timer_interval(gettick() + 100, discord_handle, 0, 0, 100);
#endif
  return 0;
}
