#ifdef HAVE_TS_SCRIPTING

#include "npc_spawner.hpp"

#include "npc_registry.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../../common/cbasetypes.hpp"
#include "../../common/showmsg.hpp"
#include "../../common/strlib.hpp"

namespace rathena::scripting {

int spawn_registered_npcs(NpcRegistry& registry) {
    int spawned = 0;
    // We can't iterate the registry directly (its internal map isn't
    // public). For the POC we know there's exactly one lookup-by-name
    // surface; expose a tiny accessor inline.
    registry.for_each([&](const NpcRegistration& reg) {
        int16 mapid = map_mapname2mapid(reg.map.c_str());
        if (mapid < 0) {
            ShowWarning("[ts-scripting] cannot spawn NPC '%s': map '%s' not loaded.\n",
                        reg.name.c_str(), reg.map.c_str());
            return;
        }

        npc_data* nd = npc_create_npc(mapid, static_cast<int16>(reg.x), static_cast<int16>(reg.y));
        if (!nd) {
            ShowWarning("[ts-scripting] npc_create_npc returned null for '%s'\n", reg.name.c_str());
            return;
        }
        safestrncpy(nd->exname, reg.name.c_str(), sizeof(nd->exname));
        safestrncpy(nd->name,   reg.name.c_str(), sizeof(nd->name));
        nd->class_ = reg.sprite;

        if (reg.trigger_area) {
            nd->u.scr.xs = static_cast<int16>(reg.trigger_area->first);
            nd->u.scr.ys = static_cast<int16>(reg.trigger_area->second);
        } else {
            nd->u.scr.xs = -1;
            nd->u.scr.ys = -1;
        }

        if (npc_install_script_npc(nd, mapid, static_cast<int16>(reg.dir))) {
            ++spawned;
            ShowStatus("[ts-scripting] spawned NPC '%s' at %s (%d,%d) sprite=%d.\n",
                       reg.name.c_str(), reg.map.c_str(), reg.x, reg.y, reg.sprite);
        }
    });
    return spawned;
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
