#include "discord_member.hpp"

namespace rocord {
member::member(std::unique_ptr<user> usr, std::string nick,
               std::vector<uint64_t> roles)
  : usr(std::move(usr))
  , nick(nick)
  , roles(roles)
{
}

bool
member::has_role(uint64_t role_id)
{
  for (auto it = roles.begin(); it != roles.end(); ++it) {
    if (*it == role_id)
      return true;
  }
  return false;
}

const std::string&
member::get_username() const
{
  return usr->get_username();
}

const std::string&
member::get_discriminator() const
{
  return usr->get_discriminator();
}

const std::string&
member::get_fullname() const
{
  return usr->get_fullname();
}

const std::string&
member::get_nick() const
{
  return nick;
}

member::~member()
{
}
}
