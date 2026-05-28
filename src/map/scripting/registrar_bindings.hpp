// Installs the host-side `registerNpc()` global on a fresh V8 context.
// Mirror of mmo-csharp's RegistrarBindings.cs, slimmed down to just
// registerNpc for the POC.

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <v8.h>

namespace rathena::scripting {

class NpcRegistry;

// Bind `globalThis.registerNpc(...registrations)` into `context`.
// All registrations land in `registry`.
//
// The function accepts varargs; each argument must be an object with
// at minimum { name, map, x, y, sprite } and may carry an `onClick`
// async function. Missing fields throw a JS exception that surfaces
// in the bundle's evaluation TryCatch and aborts boot.
void bind_registrars(v8::Isolate* isolate, v8::Local<v8::Context> context,
                     NpcRegistry& registry);

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
