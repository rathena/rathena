#!/usr/bin/env python3
"""
Binding code generator for the TS scripting host.

Reads tools/scripting/bindings.yaml, parses the named C++ structs via
libclang to discover their fields, and emits two artifacts:

    src/map/scripting/player_binding.generated.cpp
    npc-ts/types/player.generated.d.ts

Run manually with:

    python3 tools/scripting/gen_bindings.py

…or let CMake invoke it as a build-time dependency (see the custom
target in src/map/CMakeLists.txt).

Renames upstream (e.g. rAthena renames `status_point` → `status_pts`)
will surface as a missing field error at generation time, *before*
the C++ compile starts. That's the whole point: replace silent
type-erased setter calls with an AST-checked descriptor table.
"""

from __future__ import annotations

import os
import shutil
import sys
import textwrap
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

try:
    import yaml
except ImportError:
    sys.exit("error: PyYAML is required. Install with: pip install pyyaml")

try:
    import clang.cindex as cl
except ImportError:
    sys.exit("error: libclang Python bindings missing. "
             "Install with: pip install libclang clang")

# ---- libclang setup --------------------------------------------------------

# Homebrew + common Linux paths. Adjust if libclang lives elsewhere on
# your system — or set CLANG_LIBRARY_FILE in the environment.
_LIBCLANG_CANDIDATES = [
    os.environ.get("CLANG_LIBRARY_FILE"),
    "/opt/homebrew/opt/llvm/lib/libclang.dylib",
    "/usr/local/opt/llvm/lib/libclang.dylib",
    "/usr/lib/x86_64-linux-gnu/libclang-18.so.1",
    "/usr/lib/x86_64-linux-gnu/libclang.so",
    "/usr/lib/llvm/lib/libclang.so",
]
_LIBCLANG = next((p for p in _LIBCLANG_CANDIDATES if p and Path(p).exists()), None)
if not _LIBCLANG:
    sys.exit("error: libclang shared library not found. "
             "Set CLANG_LIBRARY_FILE env var to its path.")
cl.Config.set_library_file(_LIBCLANG)

REPO_ROOT = Path(__file__).resolve().parents[2]
MANIFEST_PATH = REPO_ROOT / "tools" / "scripting" / "bindings.yaml"

# Defaults — overridable via --cpp-out / --dts-out so CMake can drop
# the artifacts into its binary dir + a gitignored TS folder.
DEFAULT_CPP_OUT = REPO_ROOT / "build" / "generated" / "scripting" / "player_binding.generated.cpp"
DEFAULT_DTS_OUT = REPO_ROOT / "npc-ts" / "generated" / "player.generated.d.ts"
DEFAULT_BUILDIN_CPP_OUT = REPO_ROOT / "build" / "generated" / "scripting" / "buildin_stubs.generated.hpp"
DEFAULT_BUILDIN_DTS_OUT = REPO_ROOT / "npc-ts" / "generated" / "buildins.generated.d.ts"

# ---- types -----------------------------------------------------------------

# These are the rAthena C type names we know how to convert to JS.
# Adding a new type here is purely additive — extends what struct
# fields we recognize as "primitive enough to expose".
_INTEGRAL_TYPES = {
    "int8", "uint8", "int16", "uint16", "int32", "uint32",
    "int64", "uint64",
    "int", "unsigned int", "short", "unsigned short",
    "long", "unsigned long", "long long", "unsigned long long",
    "char", "unsigned char", "signed char",
    "bool",
    "size_t", "ssize_t",
    "t_exp", "t_itemid", "t_tick",
    "uchar",
}


@dataclass
class FieldSpec:
    cpp_name: str                 # snake_case as on the struct
    js_name: str                  # camelCase exported to JS
    cpp_type: str                 # 'int32', 'char[24]', etc.
    is_string: bool = False       # true ⇒ const char* style getter

    @property
    def ts_type(self) -> str:
        return "string" if self.is_string else "number"


@dataclass
class ManualField:
    js_name: str
    cpp_expr: str                 # spliced verbatim into the generated cpp
    ts_type: str                  # "string" | "number"


@dataclass
class StructBinding:
    struct: str                   # C++ struct name
    header: Path                  # absolute path to the header
    js_object: str                # JS object name ("player", "npc", …)
    source: str                   # C++ source expression (sd, sd.status, …)
    fields: list[FieldSpec] = field(default_factory=list)


def snake_to_camel(name: str) -> str:
    if "_" not in name:
        return name
    head, *rest = name.split("_")
    return head + "".join(part.capitalize() for part in rest)


def classify_field(cursor: cl.Cursor) -> Optional[FieldSpec]:
    """
    Decide whether a struct field is exposable, and how. Returns
    None for non-primitive / unsupported fields.
    """
    t = cursor.type
    canonical = t.get_canonical().spelling.strip()
    typename = t.spelling.strip()
    kind = t.kind

    # char[N] => string
    if kind == cl.TypeKind.CONSTANTARRAY:
        elem = t.element_type.spelling.strip()
        if elem in ("char", "const char"):
            return FieldSpec(cursor.spelling, snake_to_camel(cursor.spelling),
                             typename, is_string=True)
        return None

    # Strip typedef veneer (uint32 → unsigned int).
    if canonical in _INTEGRAL_TYPES or typename in _INTEGRAL_TYPES:
        return FieldSpec(cursor.spelling, snake_to_camel(cursor.spelling),
                         typename, is_string=False)

    # Aliased through known shorthand (rAthena typedefs uint32 etc.).
    if any(typename.startswith(p + " ") or typename == p for p in
           ("int8", "uint8", "int16", "uint16", "int32", "uint32",
            "int64", "uint64")):
        return FieldSpec(cursor.spelling, snake_to_camel(cursor.spelling),
                         typename, is_string=False)

    return None


def find_struct(tu: cl.TranslationUnit, name: str) -> Optional[cl.Cursor]:
    """DFS the TU looking for a struct definition with `name`."""
    def walk(node):
        if (node.kind in (cl.CursorKind.STRUCT_DECL, cl.CursorKind.CLASS_DECL)
                and node.spelling == name
                and node.is_definition()):
            return node
        for child in node.get_children():
            hit = walk(child)
            if hit is not None:
                return hit
        return None
    return walk(tu.cursor)


def parse_struct(spec: dict, includes: list[str]) -> StructBinding:
    header = (REPO_ROOT / spec["header"]).resolve()
    if not header.exists():
        sys.exit(f"error: header not found: {header}")

    args = ["-x", "c++", "-std=c++17"]
    for inc in includes:
        args.append(f"-I{inc}")
    # PACKETVER affects which fields exist on mmo_charstatus.
    # Match a reasonable modern client; same default used elsewhere
    # in the repo.
    args.extend(["-DPACKETVER=20211103"])

    idx = cl.Index.create()
    tu = idx.parse(str(header), args=args, options=cl.TranslationUnit.PARSE_SKIP_FUNCTION_BODIES)

    fatal = [d for d in tu.diagnostics if d.severity >= cl.Diagnostic.Error]
    if fatal:
        for d in fatal[:5]:
            print(f"  clang: {d.severity}: {d.spelling} [{d.location}]", file=sys.stderr)
        # Don't bail — clang errors on parts of the header we don't
        # care about are common when only the struct shape is needed.

    cursor = find_struct(tu, spec["struct"])
    if cursor is None:
        sys.exit(f"error: struct '{spec['struct']}' not found in {header}")

    rename = spec.get("rename") or {}
    exclude = set(spec.get("exclude") or [])
    include_only = set(spec.get("include_only") or [])

    binding = StructBinding(
        struct=spec["struct"],
        header=header,
        js_object=spec["js_object"],
        source=spec["source"],
    )

    for child in cursor.get_children():
        if child.kind != cl.CursorKind.FIELD_DECL:
            continue
        name = child.spelling
        if include_only and name not in include_only:
            continue
        if name in exclude:
            continue
        fs = classify_field(child)
        if fs is None:
            continue
        if name in rename:
            fs.js_name = rename[name]
        binding.fields.append(fs)

    return binding


# ---- code emission ---------------------------------------------------------

GENERATED_HEADER = textwrap.dedent("""\
    // === AUTO-GENERATED — DO NOT EDIT ===
    //
    // Generated by tools/scripting/gen_bindings.py from
    // tools/scripting/bindings.yaml. Edit the manifest, then run:
    //
    //     python3 tools/scripting/gen_bindings.py
    //
    // The generator uses libclang to walk the actual struct
    // definitions in the rAthena headers — every field name and
    // type below is verified against the C++ AST at generation
    // time.

""")


def render_cpp(bindings: list[StructBinding],
               manual: dict[str, list[ManualField]]) -> str:
    grouped: dict[str, list[tuple[StructBinding, FieldSpec]]] = {}
    for b in bindings:
        for f in b.fields:
            grouped.setdefault(b.js_object, []).append((b, f))

    out = [GENERATED_HEADER, "#ifdef HAVE_TS_SCRIPTING\n",
           '#include "player_binding.hpp"', "",
           "#include <type_traits>", "",
           # Map dir + scripting dir are both on the include path —
           # see src/map/CMakeLists.txt.
           '#include "map.hpp"',
           '#include "pc.hpp"', "",
           "namespace rathena::scripting {", "", "namespace {", "",
           "inline v8::Local<v8::Value> v(v8::Isolate* iso, const char* s) {",
           "    return v8::String::NewFromUtf8(iso, s ? s : \"\").ToLocalChecked();",
           "}", "",
           "template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>",
           "inline v8::Local<v8::Value> v(v8::Isolate* iso, T x) {",
           "    return v8::Integer::New(iso, static_cast<int32_t>(x));",
           "}", "",
           "using Getter = v8::Local<v8::Value>(*)(v8::Isolate*, map_session_data&);",
           "struct Field { const char* js_name; Getter get; };", "",
           "#define F(jsname, expr) \\",
           "    { jsname, [](v8::Isolate* i, map_session_data& sd) -> v8::Local<v8::Value> { return v(i, expr); } }",
           ""]

    for js_object, rows in grouped.items():
        out.append(f"static const Field k{js_object.capitalize()}Fields[] = {{")
        # Auto-generated rows.
        last_struct = None
        for binding, f in rows:
            if last_struct != binding.struct:
                out.append(f"    // ---- {binding.struct} ({binding.source}) ----")
                last_struct = binding.struct
            expr = f"{binding.source}.{f.cpp_name}"
            out.append(f'    F("{f.js_name}", {expr}),')
        # Hand-written manuals.
        if js_object in manual:
            out.append(f"    // ---- manual entries (bindings.yaml: manual.{js_object}) ----")
            for m in manual[js_object]:
                out.append(f'    F("{m.js_name}", {m.cpp_expr}),')
        out.append("};")
        out.append("")

    out.extend([
        "#undef F", "",
        "} // namespace", "",
        "// Populate an EXISTING JS object with the snapshot fields. We",
        "// don't create the object here because the v8::ObjectTemplate",
        "// in surface_stubs has already auto-instantiated ctx.player",
        "// with the stub methods baked in — we just decorate that object",
        "// with the live data.",
        "void populate_player_object(v8::Isolate* iso,",
        "                            v8::Local<v8::Context> ctx,",
        "                            v8::Local<v8::Object> obj,",
        "                            map_session_data& sd) {",
        "    for (const auto& f : kPlayerFields) {",
        "        auto key = v8::String::NewFromUtf8(iso, f.js_name).ToLocalChecked();",
        "        (void)obj->Set(ctx, key, f.get(iso, sd));",
        "    }",
        "}",
        "",
        "} // namespace rathena::scripting",
        "",
        "#endif // HAVE_TS_SCRIPTING",
    ])
    return "\n".join(out) + "\n"


# ============================================================================
# api.d.ts surface parser  +  C++ stub installer codegen
# ============================================================================
#
# The hand-written npc-ts/types/api.d.ts is the source of truth for the
# host API surface. We walk it with a regex parser (the .d.ts is regular
# enough that a real TS parser is overkill for the POC), build a tree of
# interfaces reachable from `NpcContext`, and emit a C++ header that
# installs a nested object structure of stubs on the ctx ObjectTemplate.
#
# Stubs are no-ops that log the call. Hand-impls listed in
# bindings.yaml: api_surface.hand_impl are skipped, so the codegen
# never shadows them.

import re

# Match `export interface FooBar { ... }` blocks. We use a balanced-brace
# heuristic: capture until the matching `^}` at column 0 (so nested
# `{ x: number; y: number }` literals inside the body don't confuse us).
_INTERFACE_RE = re.compile(
    r"^export\s+interface\s+(\w+)(?:\s+extends\s+[^{]+)?\s*\{\s*\n(.*?)\n\}\s*$",
    re.MULTILINE | re.DOTALL,
)

# Strip multi-line `/** ... */` and `// ...` comments before parsing
# members. Done line-by-line so we keep accurate offsets.
_COMMENT_BLOCK_RE = re.compile(r"/\*\*?.*?\*/", re.DOTALL)
_LINE_COMMENT_RE = re.compile(r"//[^\n]*")


@dataclass
class TsMember:
    name: str
    kind: str         # "method" | "property"
    raw: str          # the original member line (for documentation)
    type_text: str    # for properties: the TS type. for methods: the return type.


@dataclass
class TsInterface:
    name: str
    members: list[TsMember]


def _strip_comments(text: str) -> str:
    text = _COMMENT_BLOCK_RE.sub("", text)
    text = _LINE_COMMENT_RE.sub("", text)
    return text


def _split_members(body: str) -> list[str]:
    """Split an interface body into member declarations. We can't just
    split on `;` because members can contain inline object literals,
    type parameters, and unions with `;` inside them. Track nesting
    depth across (), [], {}, <> and only split at top-level semicolons.
    """
    out: list[str] = []
    buf: list[str] = []
    depth = 0
    for ch in body:
        if ch in "([{<":
            depth += 1
        elif ch in ")]}>":
            depth = max(0, depth - 1)
        if ch == ";" and depth == 0:
            out.append("".join(buf))
            buf.clear()
            continue
        buf.append(ch)
    tail = "".join(buf).strip()
    if tail:
        out.append(tail)
    return out


def parse_dts(path: Path) -> dict[str, TsInterface]:
    """Find every `export interface X { ... }` block and parse its
    member list. Returns a dict keyed by interface name.

    `path` may be a single .d.ts file or a directory — when a
    directory, every *.d.ts inside (non-recursive) is parsed and
    merged. Useful for the themed-split apidef layout
    (registrations.d.ts + npc.d.ts + player.d.ts + …).
    """
    if path.is_dir():
        text = "\n".join(
            sorted_path.read_text()
            for sorted_path in sorted(path.glob("*.d.ts"))
        )
    else:
        text = path.read_text()
    interfaces: dict[str, TsInterface] = {}
    for m in _INTERFACE_RE.finditer(text):
        name = m.group(1)
        body = _strip_comments(m.group(2))
        members: list[TsMember] = []
        # Brace/paren/bracket/angle-aware split on top-level semicolons.
        for raw_member in _split_members(body):
            stripped = raw_member.strip()
            if not stripped:
                continue
            # Skip index signatures and `readonly` markers.
            if stripped.startswith("["):
                continue
            # Strip readonly modifier — irrelevant for runtime stubs.
            if stripped.startswith("readonly "):
                stripped = stripped[len("readonly "):]
            # Method: `name(args): RetType` (optional `?` after name)
            mm = re.match(
                r"(\w+)\??\s*\(([^)]*(?:\([^)]*\)[^)]*)*)\)\s*:\s*(.+)",
                stripped,
                re.DOTALL,
            )
            if mm:
                members.append(TsMember(
                    name=mm.group(1),
                    kind="method",
                    raw=stripped,
                    type_text=mm.group(3).strip(),
                ))
                continue
            # Property: `name: Type` (optional `?`)
            pm = re.match(r"(\w+)\??\s*:\s*(.+)", stripped, re.DOTALL)
            if pm:
                members.append(TsMember(
                    name=pm.group(1),
                    kind="property",
                    raw=stripped,
                    type_text=pm.group(2).strip(),
                ))
        interfaces[name] = TsInterface(name=name, members=members)
    return interfaces


def collect_reachable(root: str, interfaces: dict[str, TsInterface]) -> list[TsInterface]:
    """Walk from `root` through every property whose type names another
    interface in the dict. Returns the reachable interfaces in BFS
    order (root first), with duplicates removed."""
    seen: dict[str, TsInterface] = {}
    queue = [root]
    while queue:
        name = queue.pop(0)
        if name in seen or name not in interfaces:
            continue
        iface = interfaces[name]
        seen[name] = iface
        for m in iface.members:
            if m.kind != "property":
                continue
            # Pull bare identifier names out of the type. Strips `| null`,
            # `Record<K,V>`, `Foo[]`, etc. so we follow "Foo" but not e.g.
            # "Record<string, number>".
            for tok in re.findall(r"\b([A-Z]\w*)", m.type_text):
                if tok in interfaces and tok not in seen:
                    queue.append(tok)
    return list(seen.values())


def render_surface_hpp(reachable: list[TsInterface],
                       hand_impl: set[str]) -> str:
    """Emit a header (included from dialog_context.cpp) that exposes:

        void surface_stubs::install_all(iso, ctx_tmpl);

    `install_all` recursively builds the nested-object structure
    matching the .d.ts: for each property whose type is another
    interface, a fresh ObjectTemplate is attached; for each method,
    the shared stub_callback is bound.
    """
    out = [GENERATED_HEADER, "#pragma once",
           "#ifdef HAVE_TS_SCRIPTING\n",
           "// Included by src/map/scripting/dialog_context.cpp.",
           "// Walks the interface tree rooted at NpcContext (per",
           "// npc-ts/types/api.d.ts) and installs a no-op stub for",
           "// every method we haven't hand-implemented.",
           "",
           "namespace rathena::scripting::surface_stubs {",
           "",
           "// Shared stub: logs the qualified name + arg count and",
           "// returns undefined. The qualified name lives in the",
           "// FunctionTemplate's Data slot as a JS string.",
           "inline void stub_callback(const v8::FunctionCallbackInfo<v8::Value>& info) {",
           "    auto* iso = info.GetIsolate();",
           "    v8::String::Utf8Value name(iso, info.Data());",
           "    ShowDebug(\"[ts-scripting] stub: %s called with %d args\\n\",",
           "              *name ? *name : \"?\", info.Length());",
           "    info.GetReturnValue().SetUndefined();",
           "}",
           "",
           "inline void bind_stub(v8::Isolate* iso,",
           "                      v8::Local<v8::ObjectTemplate> tmpl,",
           "                      const char* method,",
           "                      const char* qualified) {",
           "    auto data = v8::String::NewFromUtf8(iso, qualified).ToLocalChecked();",
           "    tmpl->Set(iso, method,",
           "        v8::FunctionTemplate::New(iso, &stub_callback, data));",
           "}",
           ""]

    # Generate one install_<Iface> per reachable interface.
    for iface in reachable:
        out.append(f"// {iface.name} — from npc-ts/types/api.d.ts")
        out.append(f"inline void install_{iface.name}(")
        out.append("    v8::Isolate* iso, v8::Local<v8::ObjectTemplate> tmpl);")
    out.append("")

    for iface in reachable:
        out.append(f"inline void install_{iface.name}(")
        out.append("        v8::Isolate* iso, v8::Local<v8::ObjectTemplate> tmpl) {")
        for m in iface.members:
            qualified = f"{iface.name}.{m.name}"
            if qualified in hand_impl:
                out.append(f'    // skip {qualified} — hand-implemented')
                continue
            if m.kind == "method":
                out.append(f'    bind_stub(iso, tmpl, "{m.name}", "{qualified}");')
            elif m.kind == "property":
                # If the property's type is another reachable interface,
                # build a nested ObjectTemplate.
                sub_iface = _first_known_iface(m.type_text, reachable)
                if sub_iface:
                    out.append(f'    {{')
                    out.append(f'        auto sub = v8::ObjectTemplate::New(iso);')
                    out.append(f'        install_{sub_iface}(iso, sub);')
                    out.append(f'        tmpl->Set(iso, "{m.name}", sub);')
                    out.append(f'    }}')
                else:
                    # Scalar / record property — runtime sets this
                    # per-instance (e.g. ctx.player.name from snapshot).
                    out.append(f'    // {m.name}: {m.type_text}')
                    out.append(f'    //   (scalar / Record / array — set per-instance at runtime)')
        out.append("}")
        out.append("")

    out.extend([
        "// Top-level entry — DialogContext::to_js calls this on the",
        "// ctx ObjectTemplate after binding its own hand-impl methods.",
        "inline void install_all(v8::Isolate* iso,",
        "                        v8::Local<v8::ObjectTemplate> ctx_tmpl) {",
        f"    install_NpcContext(iso, ctx_tmpl);",
        "}",
        "",
        "} // namespace rathena::scripting::surface_stubs",
        "",
        "#endif // HAVE_TS_SCRIPTING",
        "",
    ])
    return "\n".join(out)


def _first_known_iface(type_text: str,
                       reachable: list[TsInterface]) -> Optional[str]:
    """If the type string references a known interface, return its
    name. Used to decide 'this property is a sub-object, recurse'."""
    known = {i.name for i in reachable}
    for tok in re.findall(r"\b([A-Z]\w*)", type_text):
        if tok in known:
            return tok
    return None


def render_surface_dts_proxy(_: object) -> str:
    """No separate .d.ts generated for the surface — api.d.ts IS the
    source of truth. We just emit an empty stub file so the CMake
    output set stays valid when the manifest changes shape."""
    return "// Intentionally empty. The source of truth lives in\n" \
           "// npc-ts/types/api.d.ts; this file is a placeholder.\n"


def render_dts(bindings: list[StructBinding],
               manual: dict[str, list[ManualField]]) -> str:
    grouped: dict[str, list[tuple[StructBinding, FieldSpec]]] = {}
    for b in bindings:
        for f in b.fields:
            grouped.setdefault(b.js_object, []).append((b, f))

    out = [
        "// === AUTO-GENERATED — DO NOT EDIT ===",
        "//",
        "// Generated by tools/scripting/gen_bindings.py from",
        "// tools/scripting/bindings.yaml. Mirrors the C++ field set",
        "// produced by player_binding.generated.cpp so the TS",
        "// typechecker and the runtime agree on what's available.",
        "",
    ]
    for js_object, rows in grouped.items():
        out.append(f"export interface {js_object.capitalize()}Info {{")
        for binding, f in rows:
            out.append(f"    /** {binding.struct}.{f.cpp_name} ({f.cpp_type}) */")
            out.append(f"    {f.js_name}: {f.ts_type};")
        if js_object in manual:
            for m in manual[js_object]:
                out.append(f"    /** (manual) */")
                out.append(f"    {m.js_name}: {m.ts_type};")
        out.append("}")
        out.append("")
    return "\n".join(out)


# ---- main ------------------------------------------------------------------

def main():
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cpp-out", type=Path, default=DEFAULT_CPP_OUT)
    parser.add_argument("--dts-out", type=Path, default=DEFAULT_DTS_OUT)
    parser.add_argument("--buildin-cpp-out", type=Path, default=DEFAULT_BUILDIN_CPP_OUT)
    parser.add_argument("--buildin-dts-out", type=Path, default=DEFAULT_BUILDIN_DTS_OUT)
    args = parser.parse_args()

    with open(MANIFEST_PATH) as f:
        manifest = yaml.safe_load(f)

    includes = [
        str(REPO_ROOT / "src"),
        str(REPO_ROOT / "src" / "common"),
        str(REPO_ROOT / "src" / "map"),
        str(REPO_ROOT / "3rdparty" / "rapidyaml" / "src"),
    ]

    # ---- struct field bindings ----
    bindings = [parse_struct(s, includes) for s in manifest["structures"]]
    manual = {
        key: [ManualField(js_name=m["js"], cpp_expr=m["cpp"], ts_type=m["ts_type"])
              for m in entries]
        for key, entries in (manifest.get("manual") or {}).items()
    }

    args.cpp_out.parent.mkdir(parents=True, exist_ok=True)
    args.dts_out.parent.mkdir(parents=True, exist_ok=True)
    args.cpp_out.write_text(render_cpp(bindings, manual))
    args.dts_out.write_text(render_dts(bindings, manual))

    total_fields = sum(len(b.fields) for b in bindings) + sum(len(v) for v in manual.values())
    print(f"gen_bindings: wrote {total_fields} field bindings across "
          f"{len(set(b.js_object for b in bindings))} JS object(s)")
    print(f"  → {args.cpp_out}")
    print(f"  → {args.dts_out}")

    # ---- host surface stubs (api.d.ts → C++ stubs) ----
    api_cfg = manifest.get("api_surface") or {}
    if api_cfg:
        dts_path = REPO_ROOT / api_cfg["source"]
        root = api_cfg.get("root_interface", "NpcContext")
        hand_impl = set(api_cfg.get("hand_impl") or [])

        interfaces = parse_dts(dts_path)
        if root not in interfaces:
            sys.exit(f"error: root interface '{root}' not found in {dts_path}")
        reachable = collect_reachable(root, interfaces)

        args.buildin_cpp_out.parent.mkdir(parents=True, exist_ok=True)
        args.buildin_cpp_out.write_text(render_surface_hpp(reachable, hand_impl))
        # No companion .d.ts — api.d.ts IS the contract. Touch the
        # placeholder so CMake's custom_command output set is satisfied.
        args.buildin_dts_out.parent.mkdir(parents=True, exist_ok=True)
        args.buildin_dts_out.write_text(render_surface_dts_proxy(None))

        n_methods = sum(
            sum(1 for m in i.members
                if m.kind == "method" and f"{i.name}.{m.name}" not in hand_impl)
            for i in reachable
        )
        print(f"gen_bindings: walked {len(reachable)} interfaces from "
              f"`{root}`, emitted {n_methods} stub methods "
              f"(skipped {len(hand_impl)} hand-impls)")
        print(f"  → {args.buildin_cpp_out}")


if __name__ == "__main__":
    main()
