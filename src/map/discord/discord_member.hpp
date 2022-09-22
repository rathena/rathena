// Test header
#ifndef discord_member_hpp
#define discord_member_hpp

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "discord_user.hpp"

namespace rocord {
class member
{
public:
  member(std::unique_ptr<user> usr, std::string nick,
         std::vector<uint64_t> roles);
  virtual ~member();

  const std::string& get_username() const;
  const std::string& get_discriminator() const;
  const std::string& get_fullname() const;
  const std::string& get_nick() const;
  bool has_role(uint64_t role_id);

  bool operator==(const member& m) const
  {
    return (m.get_username() == this->get_username()) &&
           (m.get_discriminator() == this->get_discriminator()) &&
           (m.get_fullname() == this->get_fullname()) &&
           (m.get_nick() == this->get_nick());
    // TODO user compare
  }

private:
  std::shared_ptr<user> usr;
  std::string nick;
  std::vector<uint64_t> roles;
};
}
#endif /* discord_member_hpp*/
