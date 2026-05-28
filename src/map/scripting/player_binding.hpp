// Builds a JS object that mirrors mmo-csharp's PlayerContext surface
// (Map.Server/Scripting/Dialog/PlayerContext.*). Snapshotted at dialog
// start — for live values, swap the plain Set() calls below for
// SetNativeDataProperty getters.
//
// Exposed to TS as `ctx.player`.

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <v8.h>

struct map_session_data;

namespace rathena::scripting {

// Populate an existing JS object with the player's identity / stats /
// vitals / location snapshot. The object itself is created by V8 when
// it instantiates the ctx.player ObjectTemplate (which carries the
// stub methods); we just decorate it with live data.
void populate_player_object(v8::Isolate* iso,
                            v8::Local<v8::Context> ctx,
                            v8::Local<v8::Object> obj,
                            map_session_data& sd);

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
