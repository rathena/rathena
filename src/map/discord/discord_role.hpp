// Test header

#ifndef discord_role_hpp
#define discord_role_hpp

namespace rocord {
class role
{
public:
  role(uint64_t id, std::string name, int permissions);
  uint64_t get_id();
  std::string get_name();
  int get_permissions();

private:
  uint64_t id;
  std::string name;
  int permissions
};
}
#endif /* discord_member_hpp*/
