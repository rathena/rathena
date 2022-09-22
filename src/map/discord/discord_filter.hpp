#ifndef discord_filter_hpp
#define discord_filter_hpp

#include "discord_core_interface.hpp"
#include "discord_member.hpp"
#include <string>

namespace rocord {
enum class nickname_sensibility
{
  ban,
  remove,
  rename,
  nothing
};

enum class username_sensibility
{
  ban,
  force_nick,
  nothing
};

enum class word_sensibility
{
  ban,
  censor,
  remove,
  nothing
};

class filter
{
public:
  filter(username_sensibility user_mode, nickname_sensibility nick_mode,
         word_sensibility word_mode, core_interface& icore
         /*, some lists */);
  virtual ~filter();
  int add_to_namelist(const std::string& name);
  int add_to_wordlist(const std::string& word);
  int check_name(member& memb);
  int check_message(std::string& str);
  void reload_lists();
  void chmod_username(username_sensibility mode);
  void chmod_nickname(nickname_sensibility mode);
  void chmod_word(word_sensibility mode);

private:
  username_sensibility user_mode;
  nickname_sensibility nick_mode;
  word_sensibility word_mode;
  std::vector<std::string> list;
  core_interface& icore;
};
}
#endif /* discord_filter_hpp */
