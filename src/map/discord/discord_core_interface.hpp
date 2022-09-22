#ifndef discord_core_interface_hpp
#define discord_core_interface_hpp

#include "discord_member.hpp"
#include <string>

namespace rocord {
class core_interface
{
public:
  virtual void info() = 0;
  virtual int to_discord(std::string& msg, const std::string& channel,
                         std::string* name) = 0;
  virtual void set_display_name(const std::string& display_name) = 0;
  virtual void handle_events() = 0;
  virtual void connect() = 0;
  virtual void ban_member(member& memb, const std::string& reason,
                          int delete_message_days) = 0;
  virtual void change_nick(member& memb, const std::string& new_nick) = 0;
};
}
#endif /* discord_core_interface_hpp */
