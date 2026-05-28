// Shared helpers used by every host class. Keep these inline and tiny —
// they're inlined into every method callback (hundreds of them).
//
// Three families:
//   int_arg / str_arg / bool_arg / obj_arg / arr_arg  — unpack args
//   ret_int / ret_str / ret_bool / ret_obj            — pack return values
//   unwrap<Host>                                       — read `this` out of
//                                                       FunctionCallbackInfo::Data

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <string>
#include <v8.h>

#include "js_object_reader.hpp"

namespace rathena::scripting::args {

inline v8::Isolate* iso(const v8::FunctionCallbackInfo<v8::Value>& info) {
    return info.GetIsolate();
}

inline v8::Local<v8::Context> ctx(const v8::FunctionCallbackInfo<v8::Value>& info) {
    return info.GetIsolate()->GetCurrentContext();
}

// ---- arg unpackers --------------------------------------------------------

inline bool has(const v8::FunctionCallbackInfo<v8::Value>& info, int idx) {
    return info.Length() > idx
        && !info[idx].IsEmpty()
        && !info[idx]->IsUndefined()
        && !info[idx]->IsNull();
}

inline int int_arg(const v8::FunctionCallbackInfo<v8::Value>& info, int idx, int def = 0) {
    if (!has(info, idx)) return def;
    return info[idx]->Int32Value(ctx(info)).FromMaybe(def);
}

inline uint32_t uint_arg(const v8::FunctionCallbackInfo<v8::Value>& info, int idx, uint32_t def = 0) {
    if (!has(info, idx)) return def;
    return info[idx]->Uint32Value(ctx(info)).FromMaybe(def);
}

inline std::string str_arg(const v8::FunctionCallbackInfo<v8::Value>& info, int idx, const char* def = "") {
    if (!has(info, idx)) return def;
    return js::to_std_string(iso(info), info[idx]);
}

inline bool bool_arg(const v8::FunctionCallbackInfo<v8::Value>& info, int idx, bool def = false) {
    if (!has(info, idx)) return def;
    return info[idx]->BooleanValue(iso(info));
}

inline v8::Local<v8::Object> obj_arg(const v8::FunctionCallbackInfo<v8::Value>& info, int idx) {
    if (!has(info, idx) || !info[idx]->IsObject()) return {};
    return info[idx].As<v8::Object>();
}

inline v8::Local<v8::Array> arr_arg(const v8::FunctionCallbackInfo<v8::Value>& info, int idx) {
    if (!has(info, idx) || !info[idx]->IsArray()) return {};
    return info[idx].As<v8::Array>();
}

// ---- return packers -------------------------------------------------------

inline void ret_int(const v8::FunctionCallbackInfo<v8::Value>& info, int32_t v) {
    info.GetReturnValue().Set(v8::Integer::New(iso(info), v));
}

inline void ret_str(const v8::FunctionCallbackInfo<v8::Value>& info, const char* s) {
    info.GetReturnValue().Set(
        v8::String::NewFromUtf8(iso(info), s ? s : "").ToLocalChecked());
}

inline void ret_str(const v8::FunctionCallbackInfo<v8::Value>& info, const std::string& s) {
    info.GetReturnValue().Set(
        v8::String::NewFromUtf8(iso(info), s.c_str()).ToLocalChecked());
}

inline void ret_bool(const v8::FunctionCallbackInfo<v8::Value>& info, bool v) {
    info.GetReturnValue().Set(v8::Boolean::New(iso(info), v));
}

inline void ret_null(const v8::FunctionCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().SetNull();
}

inline void ret_undefined(const v8::FunctionCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().SetUndefined();
}

// ---- `this` unpacker ------------------------------------------------------

template <typename Host>
inline Host* unwrap(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto data = info.Data();
    if (data.IsEmpty() || !data->IsExternal()) return nullptr;
    return static_cast<Host*>(
        data.As<v8::External>()->Value(v8::kExternalPointerTypeTagDefault));
}

// ---- method installer -----------------------------------------------------
//
// Each host's install_on_object walks a static table of {name, callback}
// pairs and binds them onto a JS object. The Data slot carries the host
// pointer so callbacks can recover `this`.

template <typename Host>
inline void bind_method(v8::Isolate* iso, v8::Local<v8::Context> context,
                        v8::Local<v8::Object> obj, Host* self,
                        const char* name, v8::FunctionCallback cb) {
    auto data = v8::External::New(iso, self, v8::kExternalPointerTypeTagDefault);
    auto fn = v8::FunctionTemplate::New(iso, cb, data)
        ->GetFunction(context).ToLocalChecked();
    auto key = v8::String::NewFromUtf8(iso, name).ToLocalChecked();
    (void)obj->Set(context, key, fn);
}

} // namespace rathena::scripting::args

#endif // HAVE_TS_SCRIPTING
