//
//  discord_websocket.cpp
//  roCORD
//
//  Created by Norman Ziebal on 15.08.18.
//  Copyright © 2018 Norman Ziebal. All rights reserved.
//

#include "discord_websocket.hpp"
#include "discord_core.hpp"
#include "discord_error.hpp"
#include "discord_member.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <mutex>
#include <sstream>

using namespace nlohmann; // TODO remove
typedef std::chrono::duration<int, std::micro> dur_type;

template <typename Verifier>
class verbose_verification
{
public:
  verbose_verification(Verifier verifier)
    : verifier_(verifier)
  {
  }

  bool operator()(bool preverified, boost::asio::ssl::verify_context& ctx)
  {
    // TODO ctx unused
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    bool verified = verifier_(preverified, ctx);
    /*std::stringstream ss;
    ss << "Verifying: " << subject_name << "\n"
       << "Verified: " << verified;
    */
    return verified; // TODO handle result
  }

private:
  Verifier verifier_;
};

template <typename Verifier>
verbose_verification<Verifier>
make_verbose_verification(Verifier verifier)
{
  return verbose_verification<Verifier>(verifier);
}

namespace rocord {

websocket::websocket(std::string token, std::string uri,
                     std::shared_ptr<log> logger)
  : token(token)
  , uri(uri)
  , logger(logger)
{
}

websocketpp::lib::shared_ptr<boost::asio::ssl::context> websocket::on_tls_init(
  websocketpp::connection_hdl)
{

  try {
    websocketpp::lib::shared_ptr<boost::asio::ssl::context> ctx =
      websocketpp::lib::make_shared<boost::asio::ssl::context>(
        boost::asio::ssl::context::sslv23);

    ctx->set_options(boost::asio::ssl::context::default_workarounds |
                     boost::asio::ssl::context::no_sslv2 |
                     boost::asio::ssl::context::no_sslv3 |
                     boost::asio::ssl::context::single_dh_use);
#ifdef TESTING
    ctx->load_verify_file("../config/cacert.pem"); // TODO catch exception
#else
    ctx->load_verify_file("conf/discord/cacert.pem");
#endif
    ctx->set_verify_mode(boost::asio::ssl::verify_peer);
    ctx->set_verify_callback(make_verbose_verification(
      boost::asio::ssl::rfc2818_verification("gateway.discord.gg")));

    c_socket_init = std::chrono::high_resolution_clock::now();
    std::cout << "TLS init done!" << std::endl;
    return ctx;
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return nullptr;
  }
}

std::chrono::time_point<std::chrono::system_clock>
websocket::getStartTime()
{
  return this->start_time;
}

void
websocket::do_shutdown()
{
  if (this->heartbeat_active) {
    this->heartbeat_active = false;
    logger->print("Waiting for heartbeat thread to finish!", log_type::debug,
                  true);
    this->heartbeat_thr.join();
    logger->print("Heartbeat thread successfully closed!", log_type::debug,
                  true);
  }
  this->started = false; // use type, state = ON, OFF, SHUTDOWN
  auto event_ptr = std::bind(&core::handle_close, std::placeholders::_1);
  std::lock_guard<std::mutex> lock(m);
  event_queue.push(event_ptr);
}

void
websocket::on_open(websocketpp::connection_hdl hdl)
{
  logger->print("Websocket successfully opened!", log_type::debug, true);
  this->start_time = std::chrono::system_clock::now();
  this->socket_open = true;
  c.unlock();
}

void
websocket::on_fail(websocketpp::connection_hdl hdl)
{
  websocketpp::client<websocketpp::config::asio_tls_client>::connection_ptr
    con = client.get_con_from_hdl(hdl);

  std::cout << "Fail handler" << std::endl;
  std::cout << con->get_state() << std::endl;
  std::cout << con->get_local_close_code() << std::endl;
  std::cout << con->get_local_close_reason() << std::endl;
  std::cout << con->get_remote_close_code() << std::endl;
  std::cout << con->get_remote_close_reason() << std::endl;
  std::cout << con->get_ec() << " - " << con->get_ec().message() << std::endl;

  this->do_shutdown();
  if (!this->socket_open) // Instead of opening the socket it failed, on_open
    c.unlock();           // wont be called.
}

void
websocket::on_close(websocketpp::connection_hdl hdl)
{
  c_close = std::chrono::high_resolution_clock::now();

  std::cout
    << "Socket Init: "
    << std::chrono::duration_cast<dur_type>(c_socket_init - c_start).count()
    << std::endl;
  std::cout
    << "TLS Init: "
    << std::chrono::duration_cast<dur_type>(c_tls_init - c_start).count()
    << std::endl;
  std::cout << "Close: "
            << std::chrono::duration_cast<dur_type>(c_close - c_start).count()
            << std::endl;

  this->do_shutdown();
}

void
websocket::on_socket_init(websocketpp::connection_hdl hdl)
{
  c_socket_init = std::chrono::high_resolution_clock::now();
}

void
websocket::on_message(
  websocketpp::client<websocketpp::config::asio_tls_client>* client,
  websocketpp::connection_hdl hdl,
  websocketpp::config::asio_tls_client::message_type::ptr msg)
{

  json s, t, d;
  int op = -1;
  int heartbeat_interval = -1;
  std::function<void(core*)> event_ptr;
  websocketpp::lib::error_code errorCode;
  json payload;

  try {
    payload = json::parse(msg->get_payload());
    logger->print(payload.dump(), log_type::debug, true);

    s = payload.at("s");

    if (s.dump() != "null") {
      this->sequence_number = (int)s;
    }

    op = (int)payload.at("op");
    switch (op) {
      case 0:
        t = payload.at("t");
        if (t == "READY") {
          // std::cout << payload.at("d").dump() << std::endl;
          std::string guild = payload.at("d").at("guilds")[0].at("id");
          event_ptr =
            std::bind(&core::handle_ready, std::placeholders::_1, guild);
        } else if (t == "GUILD_CREATE") {
          // TODO
          event_ptr =
            std::bind(&core::handle_guild_create, std::placeholders::_1);
        } else if (t == "MESSAGE_CREATE") {
          d = payload.at("d");
          if (d.value("webhook_id", -1) == -1) {
            std::string content = d.at("content");
            std::string channel_id = d.at("channel_id");
            if (d.at("content") == "!info") {
              event_ptr = std::bind(&core::handle_cmd_info,
                                    std::placeholders::_1, channel_id);
            } else if (d.at("content") == "!uptime") {
              event_ptr = std::bind(&core::handle_cmd_uptime,
                                    std::placeholders::_1, channel_id);
            } else {
              json user_tmp = d.at("author");
              std::string avatar = "";
              bool bot = false;
              if (user_tmp.find("bot") != user_tmp.end())
                bot = user_tmp.at("bot");
              if (!user_tmp.at("avatar").is_null())
                avatar = user_tmp.at("avatar");
              std::unique_ptr<user> usr(new user(1, // user_tmp.at("id"),
                                                 user_tmp.at("username"),
                                                 user_tmp.at("discriminator"),
                                                 avatar, bot));
              std::string nick;
              if (d.at("member").find("nick") != d.at("member").end() &&
                  !d.at("member").at("nick").is_null())
                nick = d.at("member").at("nick");
              std::vector<uint64_t> roles; // TODO: fill
              std::shared_ptr<member> membr(
                new member(std::move(usr), nick, roles));
              event_ptr =
                std::bind(&core::handle_message_create, std::placeholders::_1,
                          membr, content, channel_id);
            }
          }
        } else
          std::cout << "Unhandled t value: " << t << std::endl;
        break;
      case 10:
        heartbeat_interval = payload.at("d").at("heartbeat_interval");
        event_ptr = std::bind(&core::handle_hello, std::placeholders::_1,
                              heartbeat_interval);
        break;
      case 11:
        std::cout << "Heartbeat ACK: " << std::string(payload.dump())
                  << std::endl;
        break;
      default:
        std::cout << "Unhandled OP code! " << op << std::endl;
    }
  } catch (nlohmann::json::exception const& e) {
    std::stringstream ss;
    ss << "Parsing failed because: ";
    ss << e.what();
    logger->print(ss.str(), log_type::warning);
  }

  std::lock_guard<std::mutex> lock(m);
  event_queue.push(event_ptr);
}

std::function<void(core*)>
websocket::get_next_event()
{
  if (!m.try_lock())
    return nullptr;

  if (event_queue.empty()) {
    m.unlock();
    return nullptr;
  }

  std::function<void(core*)> ret = event_queue.front();
  event_queue.pop();
  m.unlock();
  return ret;
}

void
websocket::run()
{
  // c.lock();
  if (shutdown)
    return;
  // std::cout << "thread was started" << std::endl;
  this->logger->print("Websocket thread was started!", log_type::debug, true);

  client.init_asio();

  client.set_access_channels(
    websocketpp::log::alevel::none); // Simple debugging is set in on_message

  client.set_message_handler(websocketpp::lib::bind(
    &websocket::on_message, this, &client, websocketpp::lib::placeholders::_1,
    websocketpp::lib::placeholders::_2));
  client.set_tls_init_handler(websocketpp::lib::bind(
    &websocket::on_tls_init, this, websocketpp::lib::placeholders::_1));
  client.set_socket_init_handler(websocketpp::lib::bind(
    &websocket::on_socket_init, this, websocketpp::lib::placeholders::_1));
  client.set_close_handler(websocketpp::lib::bind(
    &websocket::on_close, this, websocketpp::lib::placeholders::_1));
  client.set_fail_handler(websocketpp::lib::bind(
    &websocket::on_fail, this, websocketpp::lib::placeholders::_1));
  client.set_open_handler(websocketpp::lib::bind(
    &websocket::on_open, this, websocketpp::lib::placeholders::_1));

  websocketpp::lib::error_code errorCode;
  connection = client.get_connection(this->uri, errorCode);
  if (errorCode) {
    std::stringstream ss;

    ss << "Could not create an connection because " << errorCode.message();
    logger->print(ss.str(), log_type::error, true);
    c.unlock();
    return;
  }

  client.connect(connection);
  c_start = std::chrono::high_resolution_clock::now();
  this->started = true;
  client.run();
}

void
websocket::send_identify(const std::string& token, const std::string& presence)
{
  websocketpp::lib::error_code errorCode;
  json identify = { { "op", 2 },
                    { "d",
                      {
                        { "token", token },
                        { "properties",
                          {
#ifdef __linux__
                            { "$os", "linux" },
#elif _WIN32
                            { "$os", "windows" },
#else
                            { "$os", "osx" },
#endif
                            { "$browser", "roCORD" },
                            { "$device", "roCORD" },
                            { "$referrer", "" },
                            { "$referring_domain", "" } } },
                        { "compress", false },
                        { "large_threshold", 250 },
                        { "shard", { 0, 1 } },
                        { "presence",
                          {
                            { "game",
                              {
                                { "name", presence }, { "type", 0 },
                              } },
                            { "status", "dnd" },
                            { "since", NULL },
                            { "afk", false },
                          } },
                      } } };
  client.send(this->connection->get_handle(), identify.dump(),
              websocketpp::frame::opcode::text, errorCode);
  if (errorCode) {
    std::cerr << errorCode.message() << std::endl;
  }
}

void
websocket::start_heartbeat(int interval)
{
  this->interval = interval;
  this->heartbeat_active = true;
  this->heartbeat_thr = std::thread(&websocket::do_heartbeat, this);
}

void
websocket::do_heartbeat()
{
  websocketpp::lib::error_code errorCode;
  json heartb;
  while (this->heartbeat_active) {
    heartb = { { "op", 1 }, { "d", this->sequence_number } };
    std::string payload = heartb.dump();
    this->client.send(this->connection->get_handle(), payload,
                      websocketpp::frame::opcode::text, errorCode);
    if (errorCode) {
      std::stringstream ss;
      ss << "Heartbeat failed because " << errorCode.message();
      logger->print(ss.str(), log_type::error, true);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{ this->interval });
  }
  logger->print("Heartbeat stopped!", log_type::debug, true);
}

void
websocket::start()
{
  c.lock();
  this->socket_thr = std::thread(&websocket::run, this);
}

websocket::~websocket()
{

  c.lock();
  // std::cout << "Websocket is shutting down!" << std::endl;
  logger->print("Websocket is shutting down!", log_type::debug);
  if (this->started) { // TODO started and shutdown are useless because
                       // the locks sync it already.
    auto test = this->connection->get_handle();
    try {
      this->client.close(test, websocketpp::close::status::going_away,
                         "Connection closed by client.");
    } catch (websocketpp::exception const& e) {
      //      std::cout << e.what() << std::endl;
      logger->print(e.what(), log_type::debug);
    }
    // std::cout << "DWSS Connection closing!" << std::endl;
    logger->print("DWSS Connection closing!", log_type::debug);
  }
  this->shutdown = true;
  c.unlock();
  if (socket_thr.joinable())
    this->socket_thr.join();
}
}
