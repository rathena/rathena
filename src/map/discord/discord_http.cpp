//
//  discord_http.cpp
//  roCORD
//
//  Created by Norman Ziebal on 22.08.18.
//  Copyright © 2018 Norman Ziebal. All rights reserved.
//

#include "discord_http.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <chrono>
#include <future>
#include <nlohmann/json.hpp>
#include <regex>

using namespace nlohmann;

namespace rocord {
http::http(std::string token, std::shared_ptr<log> logger)
  : logger(logger)
{
  this->token = token;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  this->stop_send_loop = false;
  this->http_thr = std::thread(&http::send_loop, this);
  this->curl = curl_easy_init();
}

http::~http()
{
  this->stop_send_loop = true;
  if (this->http_thr.joinable())
    this->http_thr.join();
  curl_easy_cleanup(this->curl);
  curl_global_cleanup();
  logger->print("Http is shutting down!", log_type::debug);
}

void
http::send_loop()
{
  while (1) {
    if (stop_send_loop)
      break;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto now = std::chrono::system_clock::now();
    // std::cout << this->reset << " : " <<
    // std::chrono::time_point_cast<std::chrono::seconds>(now).time_since_epoch().count()
    // << std::endl;
    if (this->remaining != 0 ||
        ((this->reset) < std::chrono::time_point_cast<std::chrono::seconds>(now)
                           .time_since_epoch()
                           .count())) {
      http_request req;
      bool succ = this->spsc_queue.pop(req);
      if (succ)
        request(req);
    }
  }
}

void
http::send(const std::string& payload, const std::string& channel_id)
{
  struct curl_slist* header = nullptr;
  std::string url, content;
  std::string type = "POST";

  url = "channels/";
  url.append(channel_id);
  url.append("/messages");

  content = "content=";
  content.append(payload);

  bool succ = this->spsc_queue.push(http_request(header, url, content, type));
  if (!succ)
    logger->print("Error while trying to push to spsc_queue", log_type::error);
}

void
http::setDisplayName(const std::string& display_name,
                     const std::string& guild_id)
{
  struct curl_slist* header = nullptr;
  std::string url, content;
  std::string type = "PATCH";

  if (guild_id.empty() || display_name.empty())
    return;

  header = curl_slist_append(header, "Content-Type:application/json");
  url = "guilds/";
  url.append(guild_id);
  url.append("/members/@me/nick");
  json json_content = { { "username", display_name } };
  content = json_content.dump();

  bool succ = this->spsc_queue.push(http_request(header, url, content, type));
  if (!succ)
    logger->print("Error while trying to push to spsc_queue", log_type::error);
}

size_t
http::header_callback(char* buffer, size_t size, size_t nitems, void* userdata)
{
  const std::string payload(buffer, nitems);
  std::regex rgx("(\\+|-)?[[:digit:]]+");
  std::smatch match;
  auto http_obj = static_cast<http*>(userdata);

  if (boost::starts_with(payload, "X-RateLimit-Remaining") &&
      std::regex_search(payload.begin(), payload.end(), match, rgx))
    http_obj->remaining = std::stoi(match.str(0));

  if (boost::starts_with(payload, "X-RateLimit-Limit") &&
      std::regex_search(payload.begin(), payload.end(), match, rgx))
    http_obj->limit = std::stoi(match.str(0));

  if (boost::starts_with(payload, "X-RateLimit-Reset") &&
      std::regex_search(payload.begin(), payload.end(), match, rgx))
    http_obj->reset = std::stoll(match.str(0));

  return nitems;
}

size_t
http::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  std::string payload(ptr, nmemb);
  auto http_obj = static_cast<http*>(userdata);
  http_obj->logger->print(payload, log_type::debug, true);
  return nmemb;
}

void
http::request(http_request& r)
{
  if (curl) {
    CURLcode res;
    char auth[256]; // TODO: fix me
    strcpy(auth, "Authorization:Bot ");
    strcat(auth, token.c_str());
    r.header = curl_slist_append(r.header, auth);
    r.header = curl_slist_append(
      r.header,
      "User-Agent:roCORD (https://github.com/Normynator/Ragnarok, v1)");
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, r.header);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, r.type.c_str());
    std::string api_url = "https://discordapp.com/api/v6/";
    api_url.append(r.url);
    curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str()); // TODO fixme

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, r.content.c_str());
#ifdef TESTING
    curl_easy_setopt(curl, CURLOPT_CAINFO, "../config/cacert.pem");
#else
    curl_easy_setopt(curl, CURLOPT_CAINFO, "conf/discord/cacert.pem");
#endif

#ifdef SKIP_PEER_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, http::header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
    // Perform the request, res will get the return code
    res = curl_easy_perform(curl); // TODO handle curl_easy_perfrom with:
    // https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
    // Check for errors
    if (res != CURLE_OK)
      logger->print("curl_easy_perform() failed: TODO print error!",
                    log_type::error, true);
    // fprintf(stderr, "curl_easy_perform() failed: %s\n",
    //        curl_easy_strerror(res));
    // always cleanup
    curl_slist_free_all(r.header);
    // curl_easy_cleanup(curl);
  }
}
}
