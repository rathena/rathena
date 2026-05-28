// Map-server-owned V8 host. Mirrors the design of
// mmo-csharp/Map.Server/Scripting/ScriptHost.cs — single Isolate +
// Context for the lifetime of the map server, populated at boot by
// evaluating a single bundled JS entry point.
//
// Public surface is tiny on purpose: the map server boots the host,
// dispatches NPC click + dialog resume into it, and shuts it down.
// All V8-specific plumbing (promise resolvers, ctx object, registrar
// installation) lives inside the .cpp.

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <memory>
#include <string>

// Forward decls. We avoid including v8.h here so consumers of this
// header (map.cpp, npc.cpp) don't drag the V8 SDK into their build.
struct map_session_data;
struct npc_data;

namespace rathena::scripting {

class ScriptHostImpl;

class ScriptHost {
public:
    static ScriptHost& instance();

    // Initialize the V8 platform + create the isolate + create the
    // context + bind registrars. Idempotent. Call once at map boot.
    void init();

    // Evaluate `entry_path` (a JS bundle). All registerNpc() calls in
    // the bundle's module-evaluation phase populate the NPC registry.
    // Safe to call multiple times — replaces previous registrations.
    void load_entry_point(const std::string& entry_path);

    // Click dispatch. Returns true if a TS handler ran (the legacy
    // run_script() should be skipped). False = no TS handler for this
    // NPC, fall through to the legacy engine.
    bool dispatch_npc_click(map_session_data& sd, npc_data& nd);

    // Resume dispatch (after a menu / next / close packet arrives).
    // Returns true if the player is in a TS dialog and the resume
    // was handled. False = not a TS dialog, fall through.
    bool dispatch_npc_resume(map_session_data& sd, int npc_id, bool closing);

    // Tear down. Disposes the isolate + platform. Call once at shutdown.
    void shutdown();

    ScriptHost(const ScriptHost&) = delete;
    ScriptHost& operator=(const ScriptHost&) = delete;

private:
    ScriptHost();
    ~ScriptHost();
    std::unique_ptr<ScriptHostImpl> impl_;
};

} // namespace rathena::scripting

// C-friendly hooks for map.cpp / npc.cpp so they don't need to know
// about the C++ class. Map them straight to ScriptHost::instance().
extern "C" {
    void script_host_init();
    void script_host_load_entry(const char* entry_path);
    // Walks the registered NPCs and creates them in the world.
    // Call AFTER do_init_npc() so npcname_db / map mapid tables are ready.
    void script_host_spawn_npcs();
    void script_host_shutdown();
    bool script_host_dispatch_npc_click(map_session_data* sd, npc_data* nd);
    bool script_host_dispatch_npc_resume(map_session_data* sd, int npc_id, bool closing);
}

#endif // HAVE_TS_SCRIPTING
