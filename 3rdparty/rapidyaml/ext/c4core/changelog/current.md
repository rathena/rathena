### Breaking changes

- fix [#63](https://github.com/biojppm/c4core/issues/63): remove `c4/time.hpp` and `c4/time.cpp` which prevented compilation in bare-metal mode ([PR #64](https://github.com/biojppm/c4core/issues/64)).

### New features

- Added decoding of UTF codepoints: `c4::decode_code_point()` ([PR #65](https://github.com/biojppm/c4core/issues/65)).
- Experimental feature: add formatted-dumping facilities: using semantics like `c4::cat()`, `c4::catsep()` and `c4::format()`, where the subject is not a string buffer but a dump callback accepting strings. This still requires a string buffer for serialization of non-string types, but the buffer's required size is now limited to the max serialized size of non-string arguments, in contrast to the requirement in `c4::cat()` et al which is the total serialized size of every argument. This enables very efficient and generic printf-like semantics with reuse of a single small buffer, and allows direct-printing to terminal or file ([PR #67](https://github.com/biojppm/c4core/issues/67)). This feature is still experimental and a minor amount of changes to the API is possible.
- Added macro `C4_IF_CONSTEXPR` resolving to `if constexpr (...)` if the c++ standard is at least c++17.
- `csubstr`: add `count(csubstr)` overload.
- Add support for RISC-V architectures ([PR #69](https://github.com/biojppm/c4core/issues/69)).
- Add support for bare-metal compilation ([PR #64](https://github.com/biojppm/c4core/issues/64)).
- gcc >= 4.8 support using polyfills for missing templates and features ([PR #74](https://github.com/biojppm/c4core/pull/74) and [PR #68](https://github.com/biojppm/c4core/pull/68)).

### Fixes

- `csubstr::operator==(std::nullptr_t)` now returns true if either `.str==nullptr` or `.len==0`.
- Fix: `bool operator==(const char (&s)[N], csubstr)`  and `operator==(const char (&s)[N], substr)`. The template declaration for these functions had an extra `const` which prevented these functions to participate in overload resolution, which in some cases resulted in calls resolving to `operator==(std::string const&, csubstr)` if that header was visible ([PR #64](https://github.com/biojppm/c4core/issues/64)).
- Fix `csubstr::last_not_of()`: optional positional parameter was ignored [PR #62](https://github.com/biojppm/c4core/pull/62).
- `atof()`, `atod()`, `atox()`, `substr::is_real()`, `substr::first_real_span()`: accept `infinity`, `inf` and `nan` as valid reals [PR #60](https://github.com/biojppm/c4core/pull/60).
- Add missing export symbols [PR #56](https://github.com/biojppm/c4core/pull/56), [PR #57](https://github.com/biojppm/c4core/pull/57).
- `c4/substr_fwd.hpp`: fix compilation failure in Xcode 12 and earlier, where the forward declaration for `std::allocator` is inside the `inline namespace __1`, unlike later versions [PR #61](https://github.com/biojppm/c4core/pull/61), reported in [rapidyaml#185](https://github.com/biojppm/rapidyaml/issues/185).
- `c4/error.hpp`: fix compilation failure in debug mode in Xcode 12 and earlier: `__clang_major__` does not mean the same as in the common clang, and as a result the warning `-Wgnu-inline-cpp-without-extern` does not exist there.


### Thanks

- @danngreen
- @Xeonacid
- @aviktorov
- @fargies
