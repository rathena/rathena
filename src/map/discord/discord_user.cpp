#include "discord_user.hpp"
#include <sstream>

namespace rocord {
user::user(uint64_t id, std::string username, std::string discriminator,
           std::string avatar, bool bot)
  : id(id)
  , username(username)
  , discriminator(discriminator)
  , avatar(avatar)
  , bot(bot)
{
  std::stringstream ss;
  ss << username << "#" << discriminator;
  full_name = ss.str();
}

uint64_t
user::get_id() const
{
  return id;
}

const std::string&
user::get_username() const
{
  return username;
}

const std::string&
user::get_discriminator() const
{
  return discriminator;
}

const std::string&
user::get_fullname() const
{
  return full_name;
}

bool
user::is_bot() const
{
  return bot;
}

user::~user()
{
}
}
