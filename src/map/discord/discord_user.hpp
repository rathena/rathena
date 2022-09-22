// Test header
#ifndef discord_user_hpp
#define discord_user_hpp

#include <cstdint>
#include <string>

namespace rocord {
class user
{
public:
  user(uint64_t id, std::string username, std::string discriminator,
       std::string avatar, bool bot);
  virtual ~user();
  uint64_t get_id() const;
  const std::string& get_username() const;
  const std::string& get_discriminator() const;
  const std::string& get_fullname() const;
  bool is_bot() const;

private:
  uint64_t id;
  std::string username;
  std::string discriminator;
  std::string avatar;
  std::string full_name;
  bool bot;
};
}
#endif /* discord_user_hpp */
