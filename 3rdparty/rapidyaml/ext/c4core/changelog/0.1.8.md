
### New features

- Add amalgamation into a single header file ([PR #48](https://github.com/biojppm/c4core/pull/48)):
  - The amalgamated header will be available together with the deliverables from each release.
  - To generate the amalgamated header:
    ```
    $ python tools/amalgamate.py c4core_all.hpp
    ```
  - To use the amalgamated header:
    - Include at will in any header of your project.
    - In one - and only one - of your project source files, `#define C4CORE_SINGLE_HDR_DEFINE_NOW` and then `#include <c4core_all.hpp>`. This will enable the function and class definitions in the header file. For example, here's a sample program:
      ```c++
      #include <iostream>
      #define C4CORE_SINGLE_HDR_DEFINE_NOW // do this before the include
      #include <c4core_all.hpp>
      int main()
      {
          for(c4::csubstr s : c4::csubstr("a/b/c/d").split('/'))
              std::cout << s << "\n";
      }
      ```
- Add `csubstr::is_unsigned_integer()` and `csubstr::is_real()` ([PR #49](https://github.com/biojppm/c4core/pull/49)).
- CMake: add alias target c4core::c4core, guaranteeing that the same code can be used with `add_subdirectory()` and `find_package()`. (see [rapidyaml #173](https://github.com/biojppm/rapidyaml/issues/173))
- Add support for compilation with emscripten (WebAssembly+javascript) ([PR #52](https://github.com/biojppm/c4core/pull/52)).


### Fixes

- Fix edge cases with empty strings in `span::first()`, `span::last()` and `span::range()`  ([PR #49](https://github.com/biojppm/c4core/pull/49)).
- Accept octal numbers in `substr::first_real_span()` and `substr::is_real()` ([PR #49](https://github.com/biojppm/c4core/pull/49)).
- `substr`: fix coverage misses in number query methods ([PR #49](https://github.com/biojppm/c4core/pull/49)).
- Use single-header version of fast_float ([PR #49](https://github.com/biojppm/c4core/pull/47)).
- Suppress warnings triggered from fast_float in clang (`-Wfortify-source`) ([PR #49](https://github.com/biojppm/c4core/pull/47)).
- Add missing `inline` in [src/c4/ext/rng/rng.hpp](src/c4/ext/rng/rng.hpp) ([PR #49](https://github.com/biojppm/c4core/pull/47)).
- Fix compilation of [src/c4/ext/rng/inplace_function.h](src/c4/ext/inplace_function.h) in C++11 ([PR #49](https://github.com/biojppm/c4core/pull/47)).
- Change order of headers, notably in `windows_push.hpp` ([PR #47](https://github.com/biojppm/c4core/pull/47)).
- In `c4/charconv.hpp`: do not use C4_ASSERT in `to_c_fmt()`, which is `constexpr`.
- Fix [#53](https://github.com/biojppm/c4core/issues/53): cmake install targets were missing call to `export()` ([PR #55](https://github.com/biojppm/c4core/pull/55)).
- Fix linking of subprojects with libc++: flags should be forwarded through `CMAKE_***_FLAGS` instead of being set explicitly per-target ([PR #54](https://github.com/biojppm/c4core/pull/54)).


### Thanks

- @cschreib
