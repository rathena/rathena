#ifdef HAVE_TS_SCRIPTING

#include "script_host.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include <libplatform/libplatform.h>
#include <v8.h>

#include "dialog_context.hpp"
#include "dialog_session.hpp"
#include "js_object_reader.hpp"
#include "npc_registry.hpp"
#include "npc_spawner.hpp"
#include "registrar_bindings.hpp"
#include "../clif.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../pc.hpp"
#include "../../common/showmsg.hpp"
#include "../../common/timer.hpp"

namespace rathena::scripting {

// --- ScriptHostImpl ---------------------------------------------------------

class ScriptHostImpl {
public:
    void init();
    void load_entry_point(const std::string& entry_path);
    bool dispatch_npc_click(map_session_data& sd, npc_data& nd);
    bool dispatch_npc_resume(map_session_data& sd, int npc_id, bool closing);
    void shutdown();

private:
    static std::unique_ptr<v8::Platform> platform_;
    static std::unique_ptr<v8::ArrayBuffer::Allocator> allocator_;

    v8::Isolate* isolate_{nullptr};
    v8::Global<v8::Context> context_;
    DialogSessionStore sessions_;
    bool initialized_{false};

    void run_one_handler(v8::Local<v8::Context> ctx, v8::Local<v8::Function> fn,
                         v8::Local<v8::Value> arg);
    void log_exception(v8::Isolate* iso, v8::TryCatch& tc, const char* where);
};

std::unique_ptr<v8::Platform> ScriptHostImpl::platform_{};
std::unique_ptr<v8::ArrayBuffer::Allocator> ScriptHostImpl::allocator_{};

void ScriptHostImpl::init() {
    if (initialized_) return;

    if (!platform_) {
        v8::V8::InitializeICUDefaultLocation("rathena");
        v8::V8::InitializeExternalStartupData("rathena");
        platform_ = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform_.get());
        v8::V8::Initialize();
    }

    v8::Isolate::CreateParams params;
    allocator_.reset(v8::ArrayBuffer::Allocator::NewDefaultAllocator());
    params.array_buffer_allocator = allocator_.get();
    isolate_ = v8::Isolate::New(params);
    // Drain microtasks ourselves after every Resolve() — gives us
    // deterministic ordering for the dialog resume path.
    isolate_->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);

    // Surface unhandled promise rejections. Async TS handlers that
    // throw don't propagate through our outer TryCatch — V8 attaches
    // the rejection to the returned Promise instead, and silently
    // swallows it unless we install this callback. Without it, a typo
    // or bad property access inside an onClick just kills the dialog
    // with no log.
    isolate_->SetPromiseRejectCallback([](v8::PromiseRejectMessage msg) {
        if (msg.GetEvent() != v8::kPromiseRejectWithNoHandler) return;
        auto* iso = v8::Isolate::GetCurrent();
        v8::HandleScope scope(iso);
        v8::String::Utf8Value reason(iso, msg.GetValue());
        const char* reason_str = *reason ? *reason : "(no reason)";
        ShowError("[ts-scripting] unhandled promise rejection: %s\n", reason_str);
    });

    {
        v8::Isolate::Scope iso_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        auto context = v8::Context::New(isolate_);
        context_.Reset(isolate_, context);

        v8::Context::Scope ctx_scope(context);
        bind_registrars(isolate_, context, global_npc_registry());
    }

    initialized_ = true;
    ShowStatus("[ts-scripting] V8 engine ready.\n");
}

void ScriptHostImpl::load_entry_point(const std::string& entry_path) {
    if (!initialized_) {
        ShowError("[ts-scripting] load_entry_point() called before init().\n");
        return;
    }

    std::ifstream in(entry_path, std::ios::in | std::ios::binary);
    if (!in) {
        ShowWarning(
            "[ts-scripting] bundle not found at '%s'. "
            "Run `npm run build` in npc-ts/. "
            "Map server will start with zero TS-registered NPCs.\n",
            entry_path.c_str());
        return;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    auto source = ss.str();

    v8::Isolate::Scope iso_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    auto context = context_.Get(isolate_);
    v8::Context::Scope ctx_scope(context);

    global_npc_registry().clear();

    v8::TryCatch try_catch(isolate_);
    auto js_src = v8::String::NewFromUtf8(isolate_, source.c_str(),
                                          v8::NewStringType::kNormal,
                                          static_cast<int>(source.size())).ToLocalChecked();
    auto js_name = v8::String::NewFromUtf8(isolate_, entry_path.c_str()).ToLocalChecked();
    v8::ScriptOrigin origin(js_name);

    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, js_src, &origin).ToLocal(&script)) {
        log_exception(isolate_, try_catch, "compile");
        return;
    }
    v8::Local<v8::Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        log_exception(isolate_, try_catch, "evaluate");
        return;
    }

    ShowStatus("[ts-scripting] loaded bundle '%s' (%zu NPCs registered).\n",
               entry_path.c_str(), global_npc_registry().count());
}

bool ScriptHostImpl::dispatch_npc_click(map_session_data& sd, npc_data& nd) {
    if (!initialized_) return false;

    // Look up by exname — that's rAthena's unique-per-NPC identifier
    // (the part after `::` in the legacy syntax). `nd.name` is just the
    // display name and may not be unique across the world.
    const auto* reg = global_npc_registry().find_by_name(nd.exname);
    if (!reg || reg->on_click.IsEmpty()) return false;

    ShowStatus("[ts-scripting] click → '%s' (npc_id=%d) by aid=%d\n",
               nd.exname, nd.bl.id, sd.bl.id);

    v8::Isolate::Scope iso_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    auto context = context_.Get(isolate_);
    v8::Context::Scope ctx_scope(context);

    auto* session = sessions_.get_or_create(sd.bl.id);
    session->sd = &sd;
    session->nd = &nd;
    session->npc_id = nd.bl.id;
    session->ctx = std::make_unique<DialogContext>(sd, nd, *session);

    auto js_ctx = session->ctx->to_js(isolate_, context);
    session->ctx_js.Reset(isolate_, js_ctx);

    // Mark sd as in dialog so npc_scriptcont routes here.
    sd.npc_id = nd.bl.id;

#ifdef SECURE_NPCTIMEOUT
    // Without this, clif_parse_NpcSelectMenu (and Next/Close) silently
    // returns when npc_idle_timer is INVALID_TIMER — the menu pick
    // never reaches npc_scriptcont, no resume, dialog appears dead to
    // the player. Mirror what script.cpp:4366 does when the legacy
    // engine starts a dialog.
    if (sd.npc_idle_timer == INVALID_TIMER && !sd.state.ignoretimeout) {
        sd.npc_idle_timer = add_timer(
            gettick() + (SECURE_NPCTIMEOUT_INTERVAL * 1000),
            npc_secure_timeout_timer, sd.bl.id, 0);
    }
    sd.npc_idle_tick = gettick();
#endif

    auto fn = reg->on_click.Get(isolate_);
    run_one_handler(context, fn, js_ctx);
    return true;
}

bool ScriptHostImpl::dispatch_npc_resume(map_session_data& sd, int npc_id, bool closing) {
    if (!initialized_) return false;
    auto* session = sessions_.find(sd.bl.id);
    if (!session) {
        ShowStatus("[ts-scripting] resume: no session for aid=%d npc_id=%d (fall through to legacy)\n",
                   sd.bl.id, npc_id);
        return false;
    }
    if (session->pending_kind == PendingKind::None) {
        ShowStatus("[ts-scripting] resume: session for aid=%d has no armed promise — fall through\n",
                   sd.bl.id);
        return false;
    }
    if (session->npc_id != npc_id) {
        ShowStatus("[ts-scripting] resume: npc_id mismatch (session=%d, packet=%d)\n",
                   session->npc_id, npc_id);
        return false;
    }
    const char* kind_str = "?";
    switch (session->pending_kind) {
        case PendingKind::Next:  kind_str = "next"; break;
        case PendingKind::Menu:  kind_str = "menu"; break;
        case PendingKind::Close: kind_str = "close"; break;
        default: break;
    }
    ShowStatus("[ts-scripting] resume %s for aid=%d npc=%d closing=%d npc_menu=%d\n",
               kind_str, sd.bl.id, npc_id, (int)closing, (int)sd.npc_menu);

    v8::Isolate::Scope iso_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    auto context = context_.Get(isolate_);
    v8::Context::Scope ctx_scope(context);

    auto resolver = session->pending_resolver.Get(isolate_);
    session->pending_resolver.Reset();
    auto kind = session->pending_kind;
    session->pending_kind = PendingKind::None;

    v8::Local<v8::Value> resume_value = v8::Undefined(isolate_);
    if (kind == PendingKind::Menu) {
        // rAthena clamps npc_menu to 1..254 (255 = cancel); the TS
        // contract is 0-based with -1 for cancel. Translate here.
        int chosen = sd.npc_menu;
        int normalized = (chosen == 0xff) ? -1 : (chosen - 1);
        resume_value = v8::Integer::New(isolate_, normalized);
    }

    v8::TryCatch try_catch(isolate_);
    (void)resolver->Resolve(context, resume_value);
    // Microtasks-explicit: drain so the awaiting JS continuation runs
    // before we return to npc_scriptcont's epilogue.
    isolate_->PerformMicrotaskCheckpoint();
    if (try_catch.HasCaught()) {
        log_exception(isolate_, try_catch, "resume");
    }

    if (closing || kind == PendingKind::Close) {
        sessions_.remove(sd.bl.id);
        sd.npc_id = 0;
    }
    return true;
}

void ScriptHostImpl::shutdown() {
    if (!initialized_) return;
    sessions_.clear();
    global_npc_registry().clear();
    context_.Reset();
    if (isolate_) {
        isolate_->Dispose();
        isolate_ = nullptr;
    }
    initialized_ = false;
    // We deliberately leave the v8::Platform alive across shutdown —
    // V8::Initialize is one-shot per process; reinitializing isn't
    // supported. The OS reclaims it at process exit.
}

void ScriptHostImpl::run_one_handler(v8::Local<v8::Context> ctx,
                                     v8::Local<v8::Function> fn,
                                     v8::Local<v8::Value> arg) {
    v8::TryCatch try_catch(isolate_);
    v8::Local<v8::Value> argv[] = { arg };
    v8::Local<v8::Value> result;
    if (!fn->Call(ctx, v8::Undefined(isolate_), 1, argv).ToLocal(&result)) {
        log_exception(isolate_, try_catch, "onClick invocation");
        return;
    }
    // Async fn returns a Promise; we don't await it on this side. Pump
    // microtasks once so any inline awaits (e.g. ctx.mes resolves
    // immediately) get a chance to advance to the next suspension point.
    isolate_->PerformMicrotaskCheckpoint();
}

void ScriptHostImpl::log_exception(v8::Isolate* iso, v8::TryCatch& tc, const char* where) {
    v8::HandleScope scope(iso);
    auto ctx = iso->GetCurrentContext();
    v8::String::Utf8Value msg(iso, tc.Exception());
    const char* msg_str = *msg ? *msg : "(no message)";

    v8::String::Utf8Value trace(iso,
        tc.StackTrace(ctx).FromMaybe(v8::Local<v8::Value>{}));
    const char* trace_str = *trace ? *trace : "";

    ShowError("[ts-scripting] %s failed: %s\n%s\n", where, msg_str, trace_str);
}

// --- ScriptHost (public facade) --------------------------------------------

ScriptHost::ScriptHost() : impl_(std::make_unique<ScriptHostImpl>()) {}
ScriptHost::~ScriptHost() = default;

ScriptHost& ScriptHost::instance() {
    static ScriptHost host;
    return host;
}

void ScriptHost::init() { impl_->init(); }
void ScriptHost::load_entry_point(const std::string& p) { impl_->load_entry_point(p); }
bool ScriptHost::dispatch_npc_click(map_session_data& sd, npc_data& nd) {
    return impl_->dispatch_npc_click(sd, nd);
}
bool ScriptHost::dispatch_npc_resume(map_session_data& sd, int npc_id, bool closing) {
    return impl_->dispatch_npc_resume(sd, npc_id, closing);
}
void ScriptHost::shutdown() { impl_->shutdown(); }

} // namespace rathena::scripting

// --- C-friendly hooks for map.cpp/npc.cpp ----------------------------------

extern "C" {

void script_host_init() {
    rathena::scripting::ScriptHost::instance().init();
}

void script_host_load_entry(const char* entry_path) {
    rathena::scripting::ScriptHost::instance().load_entry_point(entry_path ? entry_path : "");
}

void script_host_shutdown() {
    rathena::scripting::ScriptHost::instance().shutdown();
}

void script_host_spawn_npcs() {
    int n = rathena::scripting::spawn_registered_npcs(rathena::scripting::global_npc_registry());
    ShowStatus("[ts-scripting] spawned %d TS NPCs into the world.\n", n);
}

bool script_host_dispatch_npc_click(map_session_data* sd, npc_data* nd) {
    if (!sd || !nd) return false;
    return rathena::scripting::ScriptHost::instance().dispatch_npc_click(*sd, *nd);
}

bool script_host_dispatch_npc_resume(map_session_data* sd, int npc_id, bool closing) {
    if (!sd) return false;
    return rathena::scripting::ScriptHost::instance().dispatch_npc_resume(*sd, npc_id, closing);
}

} // extern "C"

#endif // HAVE_TS_SCRIPTING
