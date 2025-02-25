#ifndef MAP_HPP
#define MAP_HPP

#include <memory>
#include "../common/cbasetypes.hpp"
#include "p2p_host.hpp"
#include "p2p_map.hpp"

struct map_session_data;

namespace rathena {

// Forward declarations
class P2PMapServer;
class P2PHostManager;

} // namespace rathena

// Function declarations
int do_init_p2p(void);
int do_final_p2p(void);
bool map_p2p_init(void);
void map_p2p_final(void);
bool map_p2p_register_player(struct map_session_data* sd);
void map_p2p_unregister_player(struct map_session_data* sd);
void map_p2p_player_enter(struct map_session_data* sd, const char* mapname);
void map_p2p_player_leave(struct map_session_data* sd, const char* mapname);

#endif // MAP_HPP
