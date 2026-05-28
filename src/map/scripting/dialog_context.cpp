#ifdef HAVE_TS_SCRIPTING

#include "dialog_context.hpp"

#include <sstream>

#include "dialog_session.hpp"
#include "js_object_reader.hpp"
#include "player_binding.hpp"
#include "../clif.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../pc.hpp"
#include "../../common/showmsg.hpp"
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

    // ctx.player was auto-instantiated by V8 from the ObjectTemplate
    // tree above (carries all the surface_stubs methods). Decorate it
    // with the live snapshot fields. Names that collide between the
    // stub and the snapshot are won by the snapshot (later set wins).
    auto player_key = v8::String::NewFromUtf8(iso, "player").ToLocalChecked();
    auto player_val = obj->Get(ctx, player_key).ToLocalChecked();
    if (player_val->IsObject()) {
        populate_player_object(iso, ctx, player_val.As<v8::Object>(), sd_);
    }

    return obj;
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
