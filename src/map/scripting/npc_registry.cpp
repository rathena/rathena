#ifdef HAVE_TS_SCRIPTING

#include "npc_registry.hpp"
#include "../../common/showmsg.hpp"

namespace rathena::scripting {

void NpcRegistry::add(std::unique_ptr<NpcRegistration> reg) {
    auto name = reg->name;
    auto [it, inserted] = by_name_.emplace(std::move(name), std::move(reg));
    if (!inserted) {
        ShowWarning("[ts-scripting] duplicate NPC name '%s'; the second registration wins.\n",
                    it->first.c_str());
        it->second = std::move(reg);
    }
}

const NpcRegistration* NpcRegistry::find_by_name(std::string_view name) const {
    auto it = by_name_.find(std::string{name});
    return it == by_name_.end() ? nullptr : it->second.get();
}

void NpcRegistry::clear() {
    by_name_.clear();
    events_.clear();
    functions_.clear();
}

void NpcRegistry::add_event_handler(const std::string& target,
                                    v8::Global<v8::Function> fn) {
    events_[target] = std::move(fn);
}

v8::Global<v8::Function>* NpcRegistry::find_event_handler(const std::string& target) {
    auto it = events_.find(target);
    return it == events_.end() ? nullptr : &it->second;
}

void NpcRegistry::add_function(const std::string& name,
                               v8::Global<v8::Function> fn) {
    functions_[name] = std::move(fn);
}

v8::Global<v8::Function>* NpcRegistry::find_function(const std::string& name) {
    auto it = functions_.find(name);
    return it == functions_.end() ? nullptr : &it->second;
}

NpcRegistry& global_npc_registry() {
    static NpcRegistry instance;
    return instance;
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
