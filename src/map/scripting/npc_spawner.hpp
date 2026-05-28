// Walks the NpcRegistry after the bundle has been evaluated and
// actually creates each NPC in the running world via rAthena's
// npc_create_npc + map_addnpc + clif_spawn machinery.
//
// Mirrors mmo-csharp's NpcSpawnService (Map.Server/Scripting/
// NpcSpawnService.cs). Runs once per boot, after do_init_npc() so
// npcname_db / ev_db / map mapid tables are ready.

#pragma once

#ifdef HAVE_TS_SCRIPTING

namespace rathena::scripting {

class NpcRegistry;

// Spawn every entry in `registry` into the world. Each entry becomes
// an NPCTYPE_SCRIPT NPC with `u.scr.script = nullptr` — the click path
// short-circuits into the TS host via ScriptHost::dispatch_npc_click
// before any legacy bytecode runs, so a null script is correct.
//
// Returns the number of NPCs successfully spawned.
int spawn_registered_npcs(NpcRegistry& registry);

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
