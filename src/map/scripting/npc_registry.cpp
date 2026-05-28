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
}

NpcRegistry& global_npc_registry() {
    static NpcRegistry instance;
    return instance;
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
