#ifdef HAVE_TS_SCRIPTING

#include "dialog_context.hpp"

#include <sstream>

#include "arg_helpers.hpp"
#include "dialog_session.hpp"
#include "host_misc.hpp"
#include "host_player.hpp"
#include "host_world.hpp"
#include "js_object_reader.hpp"
#include "player_binding.hpp"
#include "npc_registry.hpp"
#include "script_host.hpp"
#include "../clif.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../pc.hpp"
#include "../../common/random.hpp"
#include "../../common/showmsg.hpp"
#include "../../common/timer.hpp"
// Auto-generated header that defines surface_stubs::install_all().
// Walks the api.d.ts interface tree and installs stub object
// templates for every method we haven't hand-implemented. Lives at
// build/generated/scripting/ — added to the include path by
// src/map/CMakeLists.txt.
#include "surface_stubs.generated.hpp"

namespace rathena::scripting {

namespace {

// Unwrap the JS receiver (the `ctx` object) back to its owning
// DialogContext. The wrapper carries the C++ pointer in internal
// field 0.
DialogContext* unwrap(v8::Local<v8::Object> obj) {
    if (obj.IsEmpty() || obj->InternalFieldCount() < 1) return nullptr;
    auto ext = obj->GetInternalField(0).As<v8::External>();
    return static_cast<DialogContext*>(ext->Value(v8::kExternalPointerTypeTagDefault));
}

// Build a JS string array → "opt1:opt2:opt3" the way rAthena's
// clif_scriptmenu expects (colon-separated).
std::string join_options(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                         v8::Local<v8::Value> value) {
    if (value.IsEmpty() || !value->IsArray()) return {};
    auto arr = value.As<v8::Array>();
    std::ostringstream out;
    for (uint32_t i = 0; i < arr->Length(); ++i) {
        if (i > 0) out << ':';
        v8::Local<v8::Value> elem;
        if (!arr->Get(ctx, i).ToLocal(&elem)) continue;
        out << js::to_std_string(iso, elem);
    }
    return out.str();
}

// Build an already-resolved promise (used by ctx.mes — non-suspending).
v8::Local<v8::Promise> resolved_promise(v8::Isolate* iso, v8::Local<v8::Context> ctx) {
    auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    (void)resolver->Resolve(ctx, v8::Undefined(iso));
    return resolver->GetPromise();
}

// Build a pending promise + arm the session's resolver. The caller is
// expected to send the matching clif_script* packet right after, and
// the resolver fires from ScriptHost::dispatch_npc_resume.
v8::Local<v8::Promise> arm_pending(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                   DialogSession& session, PendingKind kind) {
    auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    session.pending_kind = kind;
    session.pending_resolver.Reset(iso, resolver);
    return resolver->GetPromise();
}

} // namespace

DialogContext::DialogContext(map_session_data& sd, npc_data& nd, DialogSession& session)
    : sd_(sd), nd_(nd), session_(session) {}

DialogContext::~DialogContext() = default;

// ---- callback shims --------------------------------------------------------

void DialogContext::mes_callback(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* iso = info.GetIsolate();
    auto ctx = iso->GetCurrentContext();
    auto* self = unwrap(info.This());
    if (!self) {
        iso->ThrowError("ctx.mes called on a detached context");
        return;
    }
    auto text = info.Length() > 0 ? js::to_std_string(iso, info[0]) : std::string{};
    auto p = self->mes(iso, text);
    info.GetReturnValue().Set(p);
}

void DialogContext::next_callback(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* iso = info.GetIsolate();
    auto* self = unwrap(info.This());
    if (!self) {
        iso->ThrowError("ctx.next called on a detached context");
        return;
    }
    info.GetReturnValue().Set(self->next(iso));
}

void DialogContext::select_callback(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* iso = info.GetIsolate();
    auto* self = unwrap(info.This());
    if (!self) {
        iso->ThrowError("ctx.select called on a detached context");
        return;
    }
    v8::Local<v8::Value> opts = info.Length() > 0 ? info[0] : v8::Local<v8::Value>{};
    info.GetReturnValue().Set(self->select(iso, opts));
}

void DialogContext::close_callback(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* iso = info.GetIsolate();
    auto* self = unwrap(info.This());
    if (!self) {
        iso->ThrowError("ctx.close called on a detached context");
        return;
    }
    info.GetReturnValue().Set(self->close(iso));
}

// ---- C++ implementations ---------------------------------------------------

v8::Local<v8::Promise> DialogContext::mes(v8::Isolate* iso, const std::string& text) {
    clif_scriptmes(sd_, static_cast<uint32>(nd_.bl.id), text.c_str());
    // Non-suspending: client just appends the line. JS continues
    // immediately. Match ClearScript's Task.CompletedTask shape.
    return resolved_promise(iso, iso->GetCurrentContext());
}

v8::Local<v8::Promise> DialogContext::next(v8::Isolate* iso) {
    clif_scriptnext(sd_, static_cast<uint32>(nd_.bl.id));
    return arm_pending(iso, iso->GetCurrentContext(), session_, PendingKind::Next);
}

v8::Local<v8::Promise> DialogContext::select(v8::Isolate* iso, v8::Local<v8::Value> options) {
    auto ctx = iso->GetCurrentContext();
    auto joined = join_options(iso, ctx, options);

    // The client sends a 1..N index back; clif_parse_NpcSelectMenu
    // bounds-checks against sd->npc_menu and GM_kicks the player on
    // mismatch. We have to pre-populate it the way the legacy
    // `buildin_menu` does, or the player gets disconnected on the
    // first click after the menu appears.
    int option_count = 0;
    if (!options.IsEmpty() && options->IsArray()) {
        option_count = static_cast<int>(options.As<v8::Array>()->Length());
    }
    sd_.npc_menu = option_count;
    sd_.state.menu_or_input = 1;

    clif_scriptmenu(sd_, static_cast<uint32>(nd_.bl.id), joined.c_str());
    return arm_pending(iso, ctx, session_, PendingKind::Menu);
}

v8::Local<v8::Promise> DialogContext::close(v8::Isolate* iso) {
    clif_scriptclose(sd_, static_cast<uint32>(nd_.bl.id));
    return arm_pending(iso, iso->GetCurrentContext(), session_, PendingKind::Close);
}

v8::Local<v8::Promise> DialogContext::input(v8::Isolate* iso) {
    sd_.state.menu_or_input = 1;
    clif_scriptinput(sd_, static_cast<uint32>(nd_.bl.id));
    return arm_pending(iso, iso->GetCurrentContext(), session_, PendingKind::Input);
}

v8::Local<v8::Promise> DialogContext::inputString(v8::Isolate* iso) {
    sd_.state.menu_or_input = 1;
    clif_scriptinputstr(sd_, static_cast<uint32>(nd_.bl.id));
    return arm_pending(iso, iso->GetCurrentContext(), session_, PendingKind::InputStr);
}

namespace { TIMER_FUNC(ts_sleep_timer_cb_fwd); } // defined further down

v8::Local<v8::Promise> DialogContext::sleep(v8::Isolate* iso, int ms) {
    auto promise = arm_pending(iso, iso->GetCurrentContext(), session_, PendingKind::Sleep);
    add_timer(gettick() + ms, ts_sleep_timer_cb_fwd, sd_.bl.id, 0);
    return promise;
}

v8::Local<v8::Promise> DialogContext::resolved(v8::Isolate* iso) {
    return resolved_promise(iso, iso->GetCurrentContext());
}

void DialogContext::end_session() {
    // The session is owned by ScriptHostImpl::sessions_. Clearing the
    // sd's npc_id is enough — the next click will start fresh, and
    // the leftover dialog window is the player's to dismiss. We don't
    // touch session.pending_resolver: if a promise was armed when the
    // script called end(), it'll dangle but the session destructor
    // releases the Global so V8 reclaims it on the next GC.
    sd_.npc_id = 0;
}

// ---- JS wrapper construction ----------------------------------------------

v8::Local<v8::Object> DialogContext::to_js(v8::Isolate* iso, v8::Local<v8::Context> ctx) {
    auto tmpl = v8::ObjectTemplate::New(iso);
    tmpl->SetInternalFieldCount(1);

    // Install the auto-generated stub tree FIRST so hand-impl
    // bind() calls below can override anything that slipped through
    // (they shouldn't, since the codegen skips entries listed in
    // bindings.yaml: api_surface.hand_impl).
    surface_stubs::install_all(iso, tmpl);

    auto bind = [&](const char* name, v8::FunctionCallback cb) {
        tmpl->Set(iso, name, v8::FunctionTemplate::New(iso, cb));
    };
    bind("mes",    &DialogContext::mes_callback);
    bind("next",   &DialogContext::next_callback);
    bind("select", &DialogContext::select_callback);
    bind("menu",   &DialogContext::select_callback);  // alias
    bind("close",  &DialogContext::close_callback);

    auto obj = tmpl->NewInstance(ctx).ToLocalChecked();
    obj->SetInternalField(0, v8::External::New(iso, this, v8::kExternalPointerTypeTagDefault));

    // ====================================================================
    // Override the auto-generated stubs on every sub-object with a real
    // host class. Lifetime of the hosts is tied to DialogContext (which
    // DialogSession owns on the heap) — the External pointers stay
    // valid for the duration of the dialog.
    // ====================================================================

    auto fetch = [&](const char* name) -> v8::Local<v8::Object> {
        auto key = v8::String::NewFromUtf8(iso, name).ToLocalChecked();
        auto v = obj->Get(ctx, key).ToLocalChecked();
        return v->IsObject() ? v.As<v8::Object>() : v8::Local<v8::Object>{};
    };

    // ctx.npc
    if (auto npc_obj = fetch("npc"); !npc_obj.IsEmpty()) {
        if (!npc_host_) npc_host_ = std::make_unique<NpcInfoHost>(nd_);
        npc_host_->install_on_object(iso, ctx, npc_obj);
    }

    // ctx.player + the player sub-objects (quest, achievement, …)
    if (auto player_obj = fetch("player"); !player_obj.IsEmpty()) {
        if (!player_host_) player_host_ = std::make_unique<PlayerHost>(sd_);
        player_host_->install_on_object(iso, ctx, player_obj);
        populate_player_object(iso, ctx, player_obj, sd_);

        auto fetch_in = [&](v8::Local<v8::Object> parent, const char* name) {
            auto key = v8::String::NewFromUtf8(iso, name).ToLocalChecked();
            auto v = parent->Get(ctx, key).ToLocalChecked();
            return v->IsObject() ? v.As<v8::Object>() : v8::Local<v8::Object>{};
        };
        if (auto o = fetch_in(player_obj, "quest"); !o.IsEmpty()) {
            if (!quest_host_) quest_host_ = std::make_unique<QuestHost>(sd_);
            quest_host_->install_on_object(iso, ctx, o);
        }
        if (auto o = fetch_in(player_obj, "achievement"); !o.IsEmpty()) {
            if (!achievement_host_) achievement_host_ = std::make_unique<AchievementHost>(sd_);
            achievement_host_->install_on_object(iso, ctx, o);
        }
        if (auto o = fetch_in(player_obj, "storage"); !o.IsEmpty()) {
            if (!storage_host_) storage_host_ = std::make_unique<StorageHost>(sd_);
            storage_host_->install_on_object(iso, ctx, o);
        }
        if (auto o = fetch_in(player_obj, "cart"); !o.IsEmpty()) {
            if (!cart_host_) cart_host_ = std::make_unique<CartHost>(sd_);
            cart_host_->install_on_object(iso, ctx, o);
        }
        if (auto o = fetch_in(player_obj, "mail"); !o.IsEmpty()) {
            if (!mail_host_) mail_host_ = std::make_unique<MailHost>(sd_);
            mail_host_->install_on_object(iso, ctx, o);
        }
        if (auto o = fetch_in(player_obj, "pet"); !o.IsEmpty()) {
            if (!pet_host_) pet_host_ = std::make_unique<PetHost>(sd_);
            pet_host_->install_on_object(iso, ctx, o);
        }
        if (auto o = fetch_in(player_obj, "hom"); !o.IsEmpty()) {
            if (!hom_host_) hom_host_ = std::make_unique<HomHost>(sd_);
            hom_host_->install_on_object(iso, ctx, o);
        }
        if (auto o = fetch_in(player_obj, "merc"); !o.IsEmpty()) {
            if (!merc_host_) merc_host_ = std::make_unique<MercHost>(sd_);
            merc_host_->install_on_object(iso, ctx, o);
        }
    }

    // ctx.world / ctx.party / ctx.guild / ctx.instance / ctx.bg / ctx.channel
    if (auto o = fetch("world"); !o.IsEmpty()) {
        if (!world_host_) world_host_ = std::make_unique<WorldHost>(&sd_);
        world_host_->install_on_object(iso, ctx, o);
    }
    if (auto o = fetch("party"); !o.IsEmpty()) {
        if (!party_host_) party_host_ = std::make_unique<PartyHost>(&sd_);
        party_host_->install_on_object(iso, ctx, o);
    }
    if (auto o = fetch("guild"); !o.IsEmpty()) {
        if (!guild_host_) guild_host_ = std::make_unique<GuildHost>(&sd_);
        guild_host_->install_on_object(iso, ctx, o);
    }
    if (auto o = fetch("instance"); !o.IsEmpty()) {
        if (!instance_host_) instance_host_ = std::make_unique<InstanceHost>(&sd_);
        instance_host_->install_on_object(iso, ctx, o);
    }
    if (auto o = fetch("bg"); !o.IsEmpty()) {
        if (!battleground_host_) battleground_host_ = std::make_unique<BattlegroundHost>(&sd_);
        battleground_host_->install_on_object(iso, ctx, o);
    }
    if (auto o = fetch("channel"); !o.IsEmpty()) {
        if (!channel_host_) channel_host_ = std::make_unique<ChannelHost>(&sd_);
        channel_host_->install_on_object(iso, ctx, o);
    }

    // Top-level NpcContext flow utils. rand / randRange have real
    // bodies; the rest are type-correct placeholders waiting on
    // input-dialog / timer / event plumbing.
    bind_flow_utils(iso, ctx, obj);

    return obj;
}

// ---- top-level NpcContext flow methods ------------------------------------
//
// These live on `ctx` directly (not on a sub-object) so they live
// here rather than in a separate host class.

namespace {
DialogContext* unwrap_dialog(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto ext = v8::Local<v8::External>::Cast(info.Data());
    return static_cast<DialogContext*>(ext->Value(v8::kExternalPointerTypeTagDefault));
}

void rand_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    using namespace args;
    int max = int_arg(info, 0, 1);
    if (max <= 0) max = 1;
    ret_int(info, rnd_value<int>(0, max - 1));
}
void randRange_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    using namespace args;
    int lo = int_arg(info, 0);
    int hi = int_arg(info, 1, lo);
    if (hi < lo) std::swap(lo, hi);
    ret_int(info, rnd_value<int>(lo, hi));
}
void input_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = unwrap_dialog(info);
    if (!self) { args::ret_int(info, 0); return; }
    info.GetReturnValue().Set(self->input(info.GetIsolate()));
}
void inputString_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = unwrap_dialog(info);
    if (!self) { args::ret_str(info, ""); return; }
    info.GetReturnValue().Set(self->inputString(info.GetIsolate()));
}
void doevent_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = unwrap_dialog(info);
    if (!self) return;
    auto ev = args::str_arg(info, 0);
    if (ev.empty()) return;
    // Prefer the TS-side registry: that's where bundle-loaded scripts
    // register OnLabel handlers. If we don't have one, fall through to
    // rAthena's classic npc_event so legacy NPCs still work.
    if (!ScriptHost::instance().dispatch_event(ev, &self->sd()))
        npc_event(&self->sd(), ev.c_str(), 0);
}

TIMER_FUNC(ts_sleep_timer_cb_fwd) {
    (void)tick; (void)data;
    ScriptHost::instance().dispatch_sleep_resume(static_cast<int>(id));
    return 0;
}
void sleep_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = unwrap_dialog(info);
    if (!self) return;
    int ms = args::int_arg(info, 0, 0);
    if (ms <= 0) {
        info.GetReturnValue().Set(self->resolved(info.GetIsolate()));
        return;
    }
    info.GetReturnValue().Set(self->sleep(info.GetIsolate(), ms));
}

void end_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = unwrap_dialog(info);
    if (!self) return;
    // `end` terminates the script without sending a close packet — the
    // dialog window stays open client-side until the user dismisses it
    // manually, mirroring legacy `end`. We just teardown the session.
    self->end_session();
}

void clear_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = unwrap_dialog(info);
    if (!self) return;
    clif_scriptclear(self->sd(), static_cast<int32>(self->nd().bl.id));
}

TIMER_FUNC(ts_npc_timer_cb) {
    (void)tick;
    auto* event_target = reinterpret_cast<std::string*>(data);
    if (!event_target) return 0;
    map_session_data* sd = map_id2sd(static_cast<int>(id));
    ScriptHost::instance().dispatch_event(*event_target, sd);
    delete event_target;
    return 0;
}
void addTimer_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = unwrap_dialog(info);
    if (!self) return;
    int ms = args::int_arg(info, 0, 0);
    auto target = args::str_arg(info, 1);
    if (ms < 0 || target.empty()) return;
    // The string is leaked into the timer arg; we delete it in the cb.
    auto* heap_target = new std::string(target);
    int tid = add_timer(gettick() + ms, ts_npc_timer_cb,
                       self->sd().bl.id,
                       reinterpret_cast<intptr_t>(heap_target));
    args::ret_int(info, tid);
}
void delTimer_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    // Caller passes the tid returned by addTimer. Best-effort cancel —
    // returns 1 if the timer was alive, 0 if it had already fired.
    int tid = args::int_arg(info, 0);
    if (tid <= 0) { args::ret_int(info, 0); return; }
    delete_timer(tid, ts_npc_timer_cb);
    args::ret_int(info, 1);
}
void addPlayerTimer_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    // npc_addtimer-style: the timer is keyed on the player. For now we
    // share the same scheduler as addTimer — the difference becomes
    // material once we expose npc.cpp's timerevent_list helpers.
    addTimer_cb(info);
}

void flow_void_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    (void)info;  // callfunc fallback (the registry path handles it).
}
void flow_undefined_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().SetUndefined();
}

void callfunc_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    // Look up a TS function by name and invoke with the remaining args.
    auto* self = unwrap_dialog(info);
    if (!self || info.Length() < 1) { info.GetReturnValue().SetUndefined(); return; }
    auto name = args::str_arg(info, 0);
    auto& reg = global_npc_registry();
    // Plain function name (e.g. "F_GetWeekDay") → user-function table;
    // colon-qualified name (e.g. "MyNpc::OnTalk") → event-label table.
    auto* handler = reg.find_function(name);
    if (!handler) handler = reg.find_event_handler(name);
    if (!handler) { info.GetReturnValue().SetUndefined(); return; }
    auto iso = info.GetIsolate();
    auto ctx = iso->GetCurrentContext();
    auto fn = handler->Get(iso);
    std::vector<v8::Local<v8::Value>> argv;
    argv.reserve(info.Length() - 1);
    for (int i = 1; i < info.Length(); ++i) argv.push_back(info[i]);
    v8::TryCatch tc(iso);
    v8::Local<v8::Value> result;
    if (fn->Call(ctx, ctx->Global(),
                 static_cast<int>(argv.size()),
                 argv.empty() ? nullptr : argv.data()).ToLocal(&result)) {
        info.GetReturnValue().Set(result);
    } else {
        info.GetReturnValue().SetUndefined();
    }
}
} // namespace

void DialogContext::bind_flow_utils(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                    v8::Local<v8::Object> obj) {
    auto bind = [&](const char* name, v8::FunctionCallback cb) {
        auto data = v8::External::New(iso, this, v8::kExternalPointerTypeTagDefault);
        auto fn = v8::FunctionTemplate::New(iso, cb, data)
            ->GetFunction(ctx).ToLocalChecked();
        (void)obj->Set(ctx, v8::String::NewFromUtf8(iso, name).ToLocalChecked(), fn);
    };
    bind("rand",            &rand_cb);
    bind("randRange",       &randRange_cb);
    bind("input",           &input_cb);
    bind("inputString",     &inputString_cb);
    bind("doevent",         &doevent_cb);
    bind("sleep",           &sleep_cb);
    bind("addTimer",        &addTimer_cb);
    bind("delTimer",        &delTimer_cb);
    bind("addPlayerTimer",  &addPlayerTimer_cb);
    bind("callfunc",        &callfunc_cb);
    bind("end",             &end_cb);
    bind("clear",           &clear_cb);
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
