// In-memory list of TS-registered NPCs. Populated at bundle load time
// by the registerNpc() registrar (see registrar_bindings.cpp). Looked
// up by NPC name on click dispatch.
//
// Mirror of mmo-csharp's INpcRegistry, slimmed down to just the
// onClick hook for this POC.

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <v8.h>

namespace rathena::scripting {

struct NpcRegistration {
    std::string name;          // unique display name, used for click lookup
    std::string map;
    int x{0};
    int y{0};
    int dir{0};
    int sprite{0};
    std::optional<std::pair<int, int>> trigger_area; // xs, ys

    // The async onClick(ctx) closure. Stored as a Global so it survives
    // across handle scopes between registration and dispatch.
    v8::Global<v8::Function> on_click;
};

class NpcRegistry {
public:
    void add(std::unique_ptr<NpcRegistration> reg);
    const NpcRegistration* find_by_name(std::string_view name) const;
    size_t count() const { return by_name_.size(); }
    void clear();

    template <typename F>
    void for_each(F&& fn) const {
        for (auto& [_, reg] : by_name_) fn(*reg);
    }

    // Event-label registry. Keyed by "NpcName::OnLabel" — the same
    // shape rAthena's doevent / addtimer use. Populated by the TS
    // bundle at load time and looked up by ScriptHost::dispatch_event /
    // sleep timer / addtimer fires.
    void add_event_handler(const std::string& target,
                           v8::Global<v8::Function> fn);
    v8::Global<v8::Function>* find_event_handler(const std::string& target);

    // User-function registry — bare names without "::" (legacy
    // `function<Name>` declarations). Populated by registerFunction()
    // and looked up by ctx.callfunc(name, ...).
    void add_function(const std::string& name,
                      v8::Global<v8::Function> fn);
    v8::Global<v8::Function>* find_function(const std::string& name);

private:
    std::unordered_map<std::string, std::unique_ptr<NpcRegistration>> by_name_;
    std::unordered_map<std::string, v8::Global<v8::Function>> events_;
    std::unordered_map<std::string, v8::Global<v8::Function>> functions_;
};

NpcRegistry& global_npc_registry();

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
