// Reusable accessors for pulling fields off a v8::Local<v8::Object>.
// Mirror of mmo-csharp's JsObjectReader.cs — every Require* throws a
// ScriptRegistrationException-equivalent (here: std::runtime_error)
// when the field is missing or wrong type; every Optional* returns a
// fallback.

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <stdexcept>
#include <string>
#include <v8.h>

namespace rathena::scripting {

class ScriptRegistrationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

namespace js {

// All Require/Optional helpers expect an Isolate that already has an
// active HandleScope + an entered Context. They throw on type mismatch.

std::string require_string(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                           v8::Local<v8::Object> obj, const char* field,
                           const std::string& context);

std::string optional_string(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                            v8::Local<v8::Object> obj, const char* field,
                            const std::string& fallback = {});

int require_int(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                v8::Local<v8::Object> obj, const char* field,
                const std::string& context);

int optional_int(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                 v8::Local<v8::Object> obj, const char* field,
                 int fallback);

// Get a sub-object field (returns IsEmpty()/undefined if missing).
v8::MaybeLocal<v8::Object> optional_object(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                                           v8::Local<v8::Object> obj, const char* field);

// Pull a callable off the object. Returns IsEmpty() if missing.
// Throws if present but not callable.
v8::MaybeLocal<v8::Function> optional_function(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                                               v8::Local<v8::Object> obj, const char* field,
                                               const std::string& context);

bool is_undefined_or_null(v8::Local<v8::Value> v);

// Pretty-print the v8 type name for error messages.
const char* describe_type(v8::Local<v8::Value> v);

// Convert a JS UTF-8 string into a std::string. Caller owns the result.
std::string to_std_string(v8::Isolate* isolate, v8::Local<v8::Value> v);

} // namespace js
} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
