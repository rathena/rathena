#ifndef P2P_COMMANDS_HPP
#define P2P_COMMANDS_HPP

#include "../common/cbasetypes.hpp"
#include "atcommand.hpp"

// GM command declarations
ACMD_FUNC_DEF(p2pstatus);
ACMD_FUNC_DEF(p2plist);
ACMD_FUNC_DEF(p2pvalidate);
ACMD_FUNC_DEF(p2pmetrics);

// Initialize P2P commands
void p2p_commands_init(void);

#endif // P2P_COMMANDS_HPP