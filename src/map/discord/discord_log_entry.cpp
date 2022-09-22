#include "discord_log_entry.hpp"

namespace rocord {

log_entry::log_entry(std::string msg, log_type ltype_)
  : message(msg)
  , ltype(ltype_)
{
}

log_type
log_entry::get_type()
{
  return this->ltype;
}

std::string
log_entry::get_message()
{
  return this->message;
}

log_entry::~log_entry()
{
}
} // namespace rocord
