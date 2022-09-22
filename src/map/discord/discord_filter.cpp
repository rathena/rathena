// some header
//
//

#include "discord_filter.hpp"

namespace rocord {
filter::filter(username_sensibility user_mode, nickname_sensibility nick_mode,
               word_sensibility word_mode, core_interface& icore)
  : user_mode(user_mode)
  , nick_mode(nick_mode)
  , word_mode(word_mode)
  , icore(icore)
{
}

int
filter::add_to_namelist(const std::string& name)
{
  // update list
  // save to file
  if (name == "")
    return -1;
  return 0;
}

int
filter::add_to_wordlist(const std::string& word)
{
  // update list
  // save to file
  if (word == "")
    return -1;
  return 0;
}

void
filter::reload_lists()
{
  // reload namelist form file
  // reload wordlist from file
}

void
filter::chmod_username(username_sensibility mode)
{
  user_mode = mode;
}

void
filter::chmod_nickname(nickname_sensibility mode)
{
  nick_mode = mode;
}

void
filter::chmod_word(word_sensibility mode)
{
  word_mode = mode;
}

int
filter::check_name(member& memb)
{
  if (true /* TODO: condition needed */) { // check if name is in list
    switch (user_mode) {
      case username_sensibility::ban:
        icore.ban_member(memb, "Reason: bad username", 7);
        break;
      case username_sensibility::force_nick:
      // call change nick in core & change name var
      case username_sensibility::nothing:
      default:
        return 0;
    }
    return -1;
  }
  return 0;
}

int
filter::check_message(std::string& str)
{
  if (false /* TODO: condition needed */) { // check if word is in list
    switch (word_mode) {
      case word_sensibility::ban:
        // call ban in core
        break;
      case word_sensibility::remove:
        // call delete in core
        break;
      case word_sensibility::censor:
      // filter string
      case word_sensibility::nothing:
      default:
        return 0;
    }
    return -1;
  }
  return 0;
}

filter::~filter()
{
}
}
