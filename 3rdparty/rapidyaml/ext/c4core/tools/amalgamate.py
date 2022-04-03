import re
from os.path import abspath, dirname
import sys
import subprocess

projdir = abspath(dirname(dirname(__file__)))
sys.path.insert(0, f"{projdir}/cmake")
import amalgamate_utils as am


def amalgamate_fastfloat():
    fastfloatdir = f"{projdir}/src/c4/ext/fast_float"
    subprocess.run([
        sys.executable,
        f"{fastfloatdir}/script/amalgamate.py",
        "--license", "MIT",
        "--output", f"{fastfloatdir}/../fast_float_all.h"
    ], cwd=fastfloatdir).check_returncode()


def amalgamate_c4core(filename: str,
                      with_stl: bool=True,
                      with_fastfloat: bool=True):
    if with_fastfloat:
        amalgamate_fastfloat()
    repo = "https://github.com/biojppm/c4core"
    defmacro = "C4CORE_SINGLE_HDR_DEFINE_NOW"
    exports_def_code = f"""// shared library: export when defining
#if defined(C4CORE_SHARED) && defined({defmacro}) && !defined(C4CORE_EXPORTS)
#define C4CORE_EXPORTS
#endif
"""
    required_gcc4_8_include = """// these includes are needed to work around conditional
// includes in the gcc4.8 shim
#include <cstdint>
#include <type_traits>
#include <cstring>
"""
    srcblocks = [
        am.cmttext(f"""
c4core - C++ utilities

{repo}

DO NOT EDIT. This file is generated automatically.
This is an amalgamated single-header version of the library.

INSTRUCTIONS:
  - Include at will in any header of your project
  - In one (and only one) of your project source files,
    #define {defmacro} and then include this header.
    This will enable the function and class definitions in
    the header file.
  - To compile into a shared library, just define the
    preprocessor symbol C4CORE_SHARED . This will take
    care of symbol export/import.
"""),
        am.cmtfile("LICENSE.txt"),
        am.injcode(exports_def_code),
        "src/c4/export.hpp",
        "src/c4/preprocessor.hpp",
        "src/c4/platform.hpp",
        "src/c4/cpu.hpp",
        "src/c4/compiler.hpp",
        am.injcode(required_gcc4_8_include),
        "cmake/compat/c4/gcc-4.8.hpp",
        "src/c4/language.hpp",
        "src/c4/types.hpp",
        "src/c4/config.hpp",
        am.hdrfile("src/c4/ext/debugbreak/debugbreak.h", "c4/ext/debugbreak/debugbreak.h", "DEBUG_BREAK_H"),
        "src/c4/error.hpp",
        "src/c4/memory_util.hpp",
        "src/c4/memory_resource.hpp",
        "src/c4/ctor_dtor.hpp",
        "src/c4/allocator.hpp",
        "src/c4/char_traits.hpp",
        "src/c4/hash.hpp",
        "src/c4/szconv.hpp",
        "src/c4/blob.hpp",
        "src/c4/substr_fwd.hpp",
        "src/c4/substr.hpp",
        am.onlyif(with_fastfloat, am.injfile("src/c4/ext/fast_float_all.h", "c4/ext/fast_float_all.h")),
        am.onlyif(with_fastfloat, "src/c4/ext/fast_float.hpp"),
        "src/c4/std/vector_fwd.hpp",
        "src/c4/std/string_fwd.hpp",
        "src/c4/std/std_fwd.hpp",
        "src/c4/charconv.hpp",
        "src/c4/utf.hpp",
        "src/c4/format.hpp",
        "src/c4/dump.hpp",
        "src/c4/enum.hpp",
        "src/c4/bitmask.hpp",
        "src/c4/span.hpp",
        "src/c4/type_name.hpp",
        "src/c4/base64.hpp",
        am.onlyif(with_stl, am.ignfile("src/c4/std/std.hpp")), # this is an umbrella include
        am.onlyif(with_stl, "src/c4/std/string.hpp"),
        am.onlyif(with_stl, "src/c4/std/vector.hpp"),
        am.onlyif(with_stl, "src/c4/std/tuple.hpp"),
        "src/c4/ext/rng/rng.hpp",
        "src/c4/ext/sg14/inplace_function.h",
        am.ignfile("src/c4/common.hpp"),
        am.ignfile("src/c4/c4_push.hpp"),
        am.ignfile("src/c4/c4_pop.hpp"),
        am.ignfile("src/c4/restrict.hpp"),
        am.ignfile("src/c4/unrestrict.hpp"),
        "src/c4/language.cpp",
        "src/c4/format.cpp",
        "src/c4/memory_util.cpp",
        "src/c4/char_traits.cpp",
        "src/c4/memory_resource.cpp",
        "src/c4/utf.cpp",
        "src/c4/base64.cpp",
        am.injcode("#define C4_WINDOWS_POP_HPP_"),
        "src/c4/windows_push.hpp",
        "src/c4/windows.hpp",
        "src/c4/windows_pop.hpp", # do NOT include this before windows.hpp
        "src/c4/error.cpp",
    ]
    result = am.catfiles(srcblocks,
                         projdir,
                         # comment out lines with these patterns:
                         include_regexes=[
                             re.compile(r'^\s*#\s*include "(c4/.*)".*$'),
                             re.compile(r'^\s*#\s*include <(c4/.*)>.*$'),
                         ],
                         definition_macro=defmacro,
                         repo=repo,
                         result_incguard="_C4CORE_SINGLE_HEADER_AMALGAMATED_HPP_")
    result_with_only_first_includes = am.include_only_first(result)
    am.file_put_contents(filename, result_with_only_first_includes)


def mkparser():
    return am.mkparser(fastfloat=(True, "enable fastfloat library"),
                       stl=(True, "enable stl interop"))


if __name__ == "__main__":
    args = mkparser().parse_args()
    amalgamate_c4core(filename=args.output,
                      with_fastfloat=args.fastfloat,
                      with_stl=args.stl)
