### Fixes

- `csubstr::operator==(std::nullptr_t)` now returns true if either `.str==nullptr` or `.len==0`.
- Fix `csubstr::last_not_of()`: optional positional parameter was ignored [PR #62](https://github.com/biojppm/c4core/pull/62).
- `atof()`, `atod()`, `atox()`, `substr::is_real()`, `substr::first_real_span()`: accept `infinity`, `inf` and `nan` as valid reals [PR #60](https://github.com/biojppm/c4core/pull/60).
- Add missing export symbols [PR #56](https://github.com/biojppm/c4core/pull/56), [PR #57](https://github.com/biojppm/c4core/pull/57).
- `c4/substr_fwd.hpp`: fix compilation failure in Xcode 12 and earlier, where the forward declaration for `std::allocator` is inside the `inline namespace __1`, unlike later versions [PR #61](https://github.com/biojppm/c4core/pull/61), reported in [rapidyaml#185](https://github.com/biojppm/rapidyaml/issues/185).
- `c4/error.hpp`: fix compilation failure in debug mode in Xcode 12 and earlier: `__clang_major__` does not mean the same as in the common clang, and as a result the warning `-Wgnu-inline-cpp-without-extern` does not exist there.

### New features

- Added macro `C4_IF_CONSTEXPR` resolving to `if constexpr (...)` if the c++ standard is at least c++17.

### Thanks

- @aviktorov
