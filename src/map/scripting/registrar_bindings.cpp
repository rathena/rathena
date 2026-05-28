#ifdef HAVE_TS_SCRIPTING

#include "registrar_bindings.hpp"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "js_object_reader.hpp"
#include "npc_registry.hpp"
#include "../../common/showmsg.hpp"

namespace rathena::scripting {

namespace {

// Snapshot of every hook function captured from a registerNpc input,
// kept as v8::Local while we build the registration tree. Each
// NpcRegistration (primary + duplicates) then constructs its own
// v8::Global from these locals, so duplicates share the same JS
// function instance without coordination at dispatch time.
struct HookSnapshot {
    v8::Local<v8::Function> on_click;
};

// Pull the hook closures off the input object into a HookSnapshot.
// Throws if any present hook isn't actually callable.
HookSnapshot extract_hooks(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                           v8::Local<v8::Object> obj, const std::string& trace) {
    HookSnapshot h;
    if (auto fn_maybe = js::optional_function(iso, ctx, obj, "onClick", trace);
        !fn_maybe.IsEmpty()) {
        h.on_click = fn_maybe.ToLocalChecked();
    }
    return h;
}

// Reset all Global<Function> members of a NpcRegistration from a
// HookSnapshot. Called once per registration (primary + each duplicate).
void install_hooks(v8::Isolate* iso, NpcRegistration& reg, const HookSnapshot& h) {
    if (!h.on_click.IsEmpty()) reg.on_click.Reset(iso, h.on_click);
}

// Inheritable defaults a duplicate pulls from its parent NPC.
struct DuplicateDefaults {
    const std::string& parent_name;
    int dir;
    int sprite;
    std::optional<std::pair<int, int>> trigger_area;
};

// Read a single duplicate entry. Duplicates carry their own name +
// position; sprite / dir / triggerArea fall back to the parent.
std::unique_ptr<NpcRegistration>
read_duplicate(v8::Isolate* iso, v8::Local<v8::Context> ctx,
               v8::Local<v8::Value> raw,
               const DuplicateDefaults& parent,
               const HookSnapshot& hooks,
               size_t index) {
    if (raw.IsEmpty() || !raw->IsObject() || raw->IsArray() || raw->IsFunction()) {
        throw ScriptRegistrationError(
            "registerNpc('" + parent.parent_name + "').duplicates[" + std::to_string(index) +
            "] must be an object; got " + js::describe_type(raw) + ".");
    }
    auto obj = raw.As<v8::Object>();
    std::string trace = "registerNpc('" + parent.parent_name + "').duplicates[" +
                        std::to_string(index) + "]";

    auto reg = std::make_unique<NpcRegistration>();
    reg->name = js::require_string(iso, ctx, obj, "name", trace);
    reg->map  = js::require_string(iso, ctx, obj, "map",  trace);
    reg->x    = js::require_int(iso, ctx, obj, "x", trace);
    reg->y    = js::require_int(iso, ctx, obj, "y", trace);
    // Optional overrides — inherit from parent when omitted.
    reg->dir    = js::optional_int(iso, ctx, obj, "dir",    parent.dir);
    reg->sprite = js::optional_int(iso, ctx, obj, "sprite", parent.sprite);
    reg->trigger_area = parent.trigger_area;

    install_hooks(iso, *reg, hooks);
    return reg;
}

// Read the primary NpcRegistration plus all its duplicates. Returns
// 1+N registrations in one shot — the caller adds each to the registry.
std::vector<std::unique_ptr<NpcRegistration>>
read_npc_with_duplicates(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                         v8::Local<v8::Value> raw) {
    if (raw.IsEmpty() || !raw->IsObject() || raw->IsArray() || raw->IsFunction()) {
        throw ScriptRegistrationError(
            std::string("registerNpc() requires an object literal; got ") +
            js::describe_type(raw) + ".");
    }
    auto obj = raw.As<v8::Object>();
    auto name = js::require_string(iso, ctx, obj, "name", "registerNpc");
    std::string trace = "registerNpc('" + name + "')";

    // Build the primary.
    auto primary = std::make_unique<NpcRegistration>();
    primary->name   = std::move(name);
    primary->map    = js::require_string(iso, ctx, obj, "map", trace);
    primary->x      = js::require_int(iso, ctx, obj, "x", trace);
    primary->y      = js::require_int(iso, ctx, obj, "y", trace);
    primary->dir    = js::optional_int(iso, ctx, obj, "dir", 0);
    primary->sprite = js::require_int(iso, ctx, obj, "sprite", trace);

    if (auto area_maybe = js::optional_object(iso, ctx, obj, "triggerArea");
        !area_maybe.IsEmpty()) {
        auto area = area_maybe.ToLocalChecked();
        primary->trigger_area = std::make_pair(
            js::require_int(iso, ctx, area, "xs", trace + ".triggerArea"),
            js::require_int(iso, ctx, area, "ys", trace + ".triggerArea"));
    }

    auto hooks = extract_hooks(iso, ctx, obj, trace);
    install_hooks(iso, *primary, hooks);

    std::vector<std::unique_ptr<NpcRegistration>> out;
    out.push_back(std::move(primary));

    // Process the optional `duplicates` array. Each entry produces an
    // additional NpcRegistration that shares the parent's hook closures
    // (each registration owns its own v8::Global wrapping the same
    // underlying v8::Function — sharing is at the JS-function level,
    // not the Global handle).
    auto dups_key = v8::String::NewFromUtf8(iso, "duplicates").ToLocalChecked();
    v8::Local<v8::Value> dups_val;
    if (obj->Get(ctx, dups_key).ToLocal(&dups_val)
        && !dups_val.IsEmpty()
        && !dups_val->IsUndefined()
        && !dups_val->IsNull()) {
        if (!dups_val->IsArray()) {
            throw ScriptRegistrationError(
                trace + ": 'duplicates' must be an array.");
        }
        auto dups = dups_val.As<v8::Array>();
        const DuplicateDefaults defaults{
            out[0]->name, out[0]->dir, out[0]->sprite, out[0]->trigger_area,
        };
        for (uint32_t i = 0; i < dups->Length(); ++i) {
            v8::Local<v8::Value> dup_val;
            if (!dups->Get(ctx, i).ToLocal(&dup_val)) continue;
            out.push_back(read_duplicate(iso, ctx, dup_val, defaults, hooks, i));
        }
    }

    return out;
}

void register_npc_callback(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* isolate = info.GetIsolate();
    auto ctx = isolate->GetCurrentContext();
    auto* registry = static_cast<NpcRegistry*>(
        info.Data().As<v8::External>()->Value(v8::kExternalPointerTypeTagDefault));

    try {
        for (int i = 0; i < info.Length(); ++i) {
            auto regs = read_npc_with_duplicates(isolate, ctx, info[i]);
            for (auto& reg : regs) registry->add(std::move(reg));
        }
    } catch (const std::exception& e) {
        isolate->ThrowException(v8::Exception::Error(
            v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked()));
    }
}

} // namespace

void bind_registrars(v8::Isolate* isolate, v8::Local<v8::Context> context,
                     NpcRegistry& registry) {
    auto external = v8::External::New(isolate, &registry, v8::kExternalPointerTypeTagDefault);

    auto tmpl = v8::FunctionTemplate::New(isolate, &register_npc_callback, external);
    auto fn = tmpl->GetFunction(context).ToLocalChecked();

    auto global = context->Global();
    auto key = v8::String::NewFromUtf8(isolate, "registerNpc").ToLocalChecked();
    (void)global->Set(context, key, fn);
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
