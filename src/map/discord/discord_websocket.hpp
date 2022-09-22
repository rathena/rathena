//
//  discord_websocket.hpp
//  roCORD
//
//  Created by Norman Ziebal on 15.08.18.
//  Copyright © 2018 Norman Ziebal. All rights reserved.
//

#ifndef discord_websocket_hpp
#define discord_websocket_hpp

#include "discord_log.hpp"
#include <chrono>
#include <functional>
#include <nlohmann/json.hpp>
#include <queue>
#include <stdio.h>
#include <string>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

namespace rocord {
class core;

class websocket
{
public:
  websocket(std::string token, std::string uri,
            std::shared_ptr<log> logger); // TODO copy by value
  virtual ~websocket();
  void final();
  std::function<void(core*)> get_next_event();
  void start();
  void send_identify(const std::string& token, const std::string& presence);
  void start_heartbeat(int interval);
  std::chrono::time_point<std::chrono::system_clock> getStartTime();

private:
  void on_message(
    websocketpp::client<websocketpp::config::asio_tls_client>* client,
    websocketpp::connection_hdl hdl,
    websocketpp::config::asio_tls_client::message_type::ptr msg);
  void on_close(websocketpp::connection_hdl hdl);
  void on_fail(websocketpp::connection_hdl hdl);
  void on_open(websocketpp::connection_hdl hdl);
  void on_socket_init(websocketpp::connection_hdl);
  void do_shutdown();
  websocketpp::lib::shared_ptr<boost::asio::ssl::context> on_tls_init(
    websocketpp::connection_hdl);
  void run();
  std::chrono::high_resolution_clock::time_point c_start;
  std::chrono::high_resolution_clock::time_point c_socket_init;
  std::chrono::high_resolution_clock::time_point c_tls_init;
  std::chrono::high_resolution_clock::time_point c_close;
  std::chrono::time_point<std::chrono::system_clock> start_time;
  int sequence_number;
  std::string token;
  std::string uri;
  bool heartbeat_active = false;
  bool shutdown = false;
  bool started = false;
  bool socket_open = false;
  void do_heartbeat();
  int interval;
  std::thread heartbeat_thr;
  std::thread socket_thr;
  std::shared_ptr<log> logger;
  websocketpp::client<websocketpp::config::asio_tls_client> client;
  std::queue<std::function<void(core*)>> event_queue;
  websocketpp::client<websocketpp::config::asio_tls_client>::connection_ptr
    connection;
  std::mutex m;
  std::mutex c;
};
}
#endif /* discord_websocket_hpp */
