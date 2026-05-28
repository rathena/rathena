#ifdef HAVE_TS_SCRIPTING

#include "js_object_reader.hpp"

namespace rathena::scripting::js {

bool is_undefined_or_null(v8::Local<v8::Value> v) {
    return v.IsEmpty() || v->IsUndefined() || v->IsNull();
}

const char* describe_type(v8::Local<v8::Value> v) {
    if (v.IsEmpty() || v->IsUndefined()) return "undefined";
    if (v->IsNull()) return "null";
    if (v->IsString()) return "string";
    if (v->IsBoolean()) return "boolean";
    if (v->IsFunction()) return "function";
    if (v->IsArray()) return "array";
    if (v->IsNumber()) return "number";
    if (v->IsObject()) return "object";
    return "unknown";
}

std::string to_std_string(v8::Isolate* isolate, v8::Local<v8::Value> v) {
    if (v.IsEmpty()) return {};
    v8::String::Utf8Value u(isolate, v);
    return std::string(*u ? *u : "", u.length());
}

static v8::Local<v8::Value> get_field(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                                      v8::Local<v8::Object> obj, const char* field) {
    auto key = v8::String::NewFromUtf8(isolate, field).ToLocalChecked();
    v8::Local<v8::Value> out;
    if (!obj->Get(ctx, key).ToLocal(&out)) {
        return v8::Local<v8::Value>{};
    }
    return out;
}

std::string require_string(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                           v8::Local<v8::Object> obj, const char* field,
                           const std::string& context) {
    auto v = get_field(isolate, ctx, obj, field);
    if (is_undefined_or_null(v)) {
        throw ScriptRegistrationError(context + ": required field '" + field + "' is missing.");
    }
    if (!v->IsString()) {
        throw ScriptRegistrationError(context + ": field '" + field +
                                      "' must be a string; got " + describe_type(v) + ".");
    }
    return to_std_string(isolate, v);
}

std::string optional_string(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                            v8::Local<v8::Object> obj, const char* field,
                            const std::string& fallback) {
    auto v = get_field(isolate, ctx, obj, field);
    if (is_undefined_or_null(v)) return fallback;
    if (!v->IsString()) {
        throw ScriptRegistrationError(std::string("field '") + field +
                                      "' must be a string when set; got " + describe_type(v) + ".");
    }
    return to_std_string(isolate, v);
}

int require_int(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                v8::Local<v8::Object> obj, const char* field,
                const std::string& context) {
    auto v = get_field(isolate, ctx, obj, field);
    if (is_undefined_or_null(v)) {
        throw ScriptRegistrationError(context + ": required field '" + field + "' is missing.");
    }
    if (!v->IsNumber()) {
        throw ScriptRegistrationError(context + ": field '" + field +
                                      "' must be a number; got " + describe_type(v) + ".");
    }
    return static_cast<int>(v->Int32Value(ctx).FromMaybe(0));
}

int optional_int(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                 v8::Local<v8::Object> obj, const char* field,
                 int fallback) {
    auto v = get_field(isolate, ctx, obj, field);
    if (is_undefined_or_null(v)) return fallback;
    if (!v->IsNumber()) {
        throw ScriptRegistrationError(std::string("field '") + field +
                                      "' must be a number when set; got " + describe_type(v) + ".");
    }
    return static_cast<int>(v->Int32Value(ctx).FromMaybe(0));
}

v8::MaybeLocal<v8::Object> optional_object(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                                           v8::Local<v8::Object> obj, const char* field) {
    auto v = get_field(isolate, ctx, obj, field);
    if (is_undefined_or_null(v)) return v8::MaybeLocal<v8::Object>{};
    if (!v->IsObject()) {
        throw ScriptRegistrationError(std::string("field '") + field +
                                      "' must be an object when set; got " + describe_type(v) + ".");
    }
    return v.As<v8::Object>();
}

v8::MaybeLocal<v8::Function> optional_function(v8::Isolate* isolate, v8::Local<v8::Context> ctx,
                                               v8::Local<v8::Object> obj, const char* field,
                                               const std::string& context) {
    auto v = get_field(isolate, ctx, obj, field);
    if (is_undefined_or_null(v)) return v8::MaybeLocal<v8::Function>{};
    if (!v->IsFunction()) {
        throw ScriptRegistrationError(
            context + ": hook '" + field + "' must be a function; got " +
            describe_type(v) + ". Did you write `async onClick(ctx) { ... }`?");
    }
    return v.As<v8::Function>();
}

} // namespace rathena::scripting::js

#endif // HAVE_TS_SCRIPTING
