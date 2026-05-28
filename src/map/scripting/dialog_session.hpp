// Per-player pending-promise state for an in-flight TS dialog.
//
// When `ctx.select([...])` is called from JS, we create a
// v8::Promise::Resolver, store it here keyed by the player's
// account_id, and send the menu packet. When the client's menu
// selection arrives (via npc_scriptcont -> ScriptHost::dispatch_npc_resume),
// we look up this entry, resolve the promise with the chosen index,
// and pump microtasks so the awaiting JS function continues.

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <memory>
#include <unordered_map>
#include <v8.h>

struct map_session_data;
struct npc_data;

namespace rathena::scripting {

class DialogContext;

enum class PendingKind {
    None,
    Next,   // ctx.next() — resume with `undefined`
    Menu,   // ctx.select(...) — resume with the chosen index (0-based)
    Close,  // ctx.close() — resume with `undefined`, then teardown
};

// One per in-flight TS dialog. Lives from onClick until the script
// either returns or calls ctx.close(). Owns the C++ DialogContext
// (which the JS-side `ctx` wrapper proxies into) and the pending
// promise resolver (one at a time — every dialog op is sequential
// from the script author's POV: `await ctx.mes(...); await ctx.select(...)`).
struct DialogSession {
    DialogSession();
    ~DialogSession();
    DialogSession(const DialogSession&) = delete;
    DialogSession& operator=(const DialogSession&) = delete;

    int account_id{0};
    int npc_id{0};
    map_session_data* sd{nullptr};
    npc_data* nd{nullptr};

    PendingKind pending_kind{PendingKind::None};
    v8::Global<v8::Promise::Resolver> pending_resolver;

    std::unique_ptr<DialogContext> ctx;
    v8::Global<v8::Object> ctx_js;
};

class DialogSessionStore {
public:
    DialogSession* get_or_create(int account_id);
    DialogSession* find(int account_id);
    void remove(int account_id);
    void clear();
    size_t count() const { return sessions_.size(); }

private:
    std::unordered_map<int, std::unique_ptr<DialogSession>> sessions_;
};

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
