//
//  discord_http.hpp
//  roCORD
//
//  Created by Norman Ziebal on 22.08.18.
//  Copyright © 2018 Norman Ziebal. All rights reserved.
//

#ifndef discord_http_hpp
#define discord_http_hpp

#include "discord_log.hpp"
#include "discord_request.hpp"
#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <chrono>
#include <curl/curl.h>
#include <future>
#include <iostream>
#include <stdio.h>
#include <string>

namespace rocord {
class http
{
public:
  /*
   * http object for api requests.
   * token: the token to authenticate against the discord api.
   * logger: a logger object.
   */
  http(std::string token, std::shared_ptr<log> logger); // TODO copy by value

  virtual ~http();

  /*
   * send is used for performing an api request to discord.
   * payload: contains the data to be send.
   * channel_id: contains the id of the channel the data is send to.
   */
  void send(const std::string& payload, const std::string& channel_id);

  /*
   * setDisplayName is used to change the bots name.
   * display_name: new name for the bot.
   * guild_id: identifier for the server the bots operates in.
   */
  void setDisplayName(const std::string& display_name,
                      const std::string& guild_id);

private:
  CURL* curl;
  std::string token; // TODO should be const
  std::shared_ptr<log> logger;

  // Rate limit variables
  int limit = -1;
  int remaining = -1;
  //  long long int reset;
  int64_t reset = 0;

  /*
   * send_loop is used by http_thr to perfrom the requests and check the queue.
   */
  void send_loop();

  std::atomic<bool> stop_send_loop;

  /*
   * Queue for requests to discord api.
   * This queue contains a request object which hold all data to perform the
   * request.
   */
  boost::lockfree::spsc_queue<http_request, boost::lockfree::capacity<1024>>
    spsc_queue;

  /*
   * libcurl, callback function for https respond. (data)
   */
  static size_t write_callback(char* ptr, size_t size, size_t nmemb,
                               void* userdata);

  /*
   * libcurl, callback function for https respond. (header)
   */
  static size_t header_callback(char* buffer, size_t size, size_t nitems,
                                void* userdata);

  /*
   * http_thr is used to send http request to the discord api.
   * It is checking the spsc_queue once every 100ms.
   */
  std::thread http_thr;
  /*
   *
   */
  void request(http_request& r);
};
}
#endif /* discord_http_hpp */
