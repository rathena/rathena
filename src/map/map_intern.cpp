#include "map.hpp"

#include <stdlib.h>
#include <math.h>

#include "../common/cbasetypes.hpp"
#include "../common/cli.hpp"
#include "../common/core.hpp"
#include "../common/ers.hpp"
#include "../common/grfio.hpp"
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp" // WFIFO*()
#include "../common/strlib.hpp"
#include "../common/timer.hpp"
#include "../common/utilities.hpp"
#include "../common/utils.hpp"

#include "achievement.hpp"
#include "atcommand.hpp"
#include "battle.hpp"
#include "battleground.hpp"
#include "cashshop.hpp"
#include "channel.hpp"
#include "chat.hpp"
#include "chrif.hpp"
#include "clan.hpp"
#include "clif.hpp"
#include "duel.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "log.hpp"
#include "mapreg.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pet.hpp"
#include "quest.hpp"
#include "storage.hpp"
#include "trade.hpp"

using namespace rathena;

struct map_session_data * map_obj.id2sd(int id){
	if (id <= 0) return NULL;
	return (struct map_session_data*)idb_get(pc_db,id);
}
