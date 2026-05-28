// The `ctx` object handed to a TS onClick closure. Mirror of
// mmo-csharp's DialogContext.cs — `mes` / `next` / `select` / `close`
// return JS Promises that the script awaits; the resume happens when
// the player responds via npc_scriptcont.
//
// For the POC the only members exposed are the four dialog ops above.
// In the full mmo-csharp port `ctx` also carries `npc` / `player` /
// `world` / `party` / `guild` / ... — those land in later commits.

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <memory>
#include <v8.h>

struct map_session_data;
struct npc_data;

namespace rathena::scripting {

struct DialogSession;
class PlayerHost;

class DialogContext {
public:
    DialogContext(map_session_data& sd, npc_data& nd, DialogSession& session);
    ~DialogContext();

    // Returns the JS Promise + arms the session's resolver. Caller is
    // expected to send the corresponding clif_script* packet right
    // after; resolution happens from ScriptHost::dispatch_npc_resume.
    v8::Local<v8::Promise> mes(v8::Isolate* iso, const std::string& text);
    v8::Local<v8::Promise> next(v8::Isolate* iso);
    v8::Local<v8::Promise> select(v8::Isolate* iso, v8::Local<v8::Value> options);
    v8::Local<v8::Promise> close(v8::Isolate* iso);

    // Build a freshly-allocated JS object that proxies into this C++
    // DialogContext. Used by ScriptHost when invoking onClick.
    v8::Local<v8::Object> to_js(v8::Isolate* iso, v8::Local<v8::Context> ctx);

private:
    map_session_data& sd_;
    npc_data& nd_;
    DialogSession& session_;
    std::unique_ptr<PlayerHost> player_host_;

    static void mes_callback(const v8::FunctionCallbackInfo<v8::Value>& info);
    static void next_callback(const v8::FunctionCallbackInfo<v8::Value>& info);
    static void select_callback(const v8::FunctionCallbackInfo<v8::Value>& info);
    static void close_callback(const v8::FunctionCallbackInfo<v8::Value>& info);
};

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
