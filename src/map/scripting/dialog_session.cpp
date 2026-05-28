#ifdef HAVE_TS_SCRIPTING

#include "dialog_session.hpp"

#include "dialog_context.hpp"  // complete type required for unique_ptr<DialogContext> dtor

namespace rathena::scripting {

DialogSession::DialogSession() = default;
DialogSession::~DialogSession() = default;

DialogSession* DialogSessionStore::get_or_create(int account_id) {
    auto& slot = sessions_[account_id];
    if (!slot) {
        slot = std::make_unique<DialogSession>();
        slot->account_id = account_id;
    }
    return slot.get();
}

DialogSession* DialogSessionStore::find(int account_id) {
    auto it = sessions_.find(account_id);
    return it == sessions_.end() ? nullptr : it->second.get();
}

void DialogSessionStore::remove(int account_id) {
    sessions_.erase(account_id);
}

void DialogSessionStore::clear() {
    sessions_.clear();
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
