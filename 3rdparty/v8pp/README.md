# v8pp

[v8pp](https://github.com/pmed/v8pp) is a header-only C++ helper library
for binding C++ types and functions into V8. It's the C++ equivalent of
what ClearScript does on .NET.

This directory is intentionally **empty** in the repo. v8pp is brought in
out-of-tree so we don't vendor ~150 KiB of headers that drift on every
upstream release. Pick one of the two install paths below; the CMake find
module (`3rdparty/cmake/FindV8.cmake` + `Findv8pp.cmake`) tries both.

## Install path A — git submodule (recommended)

    git submodule add https://github.com/pmed/v8pp.git 3rdparty/v8pp/src
    git submodule update --init --recursive

CMake picks up `3rdparty/v8pp/src/v8pp/*.hpp` automatically.

## Install path B — system install via Homebrew (macOS) / apt

    brew install v8       # macOS
    sudo apt install libv8-dev   # Debian/Ubuntu (older v8, may not work)

Then point CMake at v8pp headers manually:

    cmake -DV8PP_INCLUDE_DIR=/path/to/v8pp/include ..

## Why v8pp and not raw V8?

Raw V8 binding looks like:

```cpp
v8::Local<v8::FunctionTemplate> tmpl =
    v8::FunctionTemplate::New(isolate, &MyClass::SomeMethodCallback);
proto->Set(isolate, "someMethod", tmpl);
```

…with each callback unpacking `FunctionCallbackInfo` by hand. v8pp folds
all of that into:

```cpp
v8pp::class_<MyClass> cls(isolate);
cls.function("someMethod", &MyClass::SomeMethod);
```

which is much closer to ClearScript's auto-binding semantics on the .NET
side. It also handles `Task<T>` → `Promise<T>` style return conversion
when paired with our `dialog_promise.hpp` helper.
