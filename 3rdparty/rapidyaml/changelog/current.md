### Breaking changes

As part of the [new feature to track source locations](https://github.com/biojppm/rapidyaml/pull/168), opportunity was taken to address a number of pre-existing API issues. These changes consisted of:

- Deprecate `c4::yml::parse()` and `c4::yml::Parser::parse()` overloads; all these functions will be shortly removed. Until removal, any call from client code will trigger a compiler warning.
- Add `parse()` alternatives, either `parse_in_place()` or `parse_in_arena()`:
  - `parse_in_place()` receives only `substr` buffers, ie mutable YAML source buffers. Trying to pass a `csubstr` buffer to `parse_in_place()` will cause a compile error:
    ```c++
    substr readwrite = /*...*/;
    Tree tree = parse_in_place(readwrite); // OK
    
    csubstr readonly = /*...*/;
    Tree tree = parse_in_place(readonly); // compile error
    ```
  - `parse_in_arena()` receives only `csubstr` buffers, ie immutable YAML source buffers. Prior to parsing, the buffer is copied to the tree's arena, then the copy is parsed in place. Because `parse_in_arena()` is meant for immutable buffers, overloads receiving a `substr` YAML buffer are now declared and marked deprecated, and intentionally left undefined, such that calling `parse_in_arena()` with a `substr` will cause a linker error.
    ```c++
    substr readwrite = /*...*/;
    Tree tree = parse_in_arena(readwrite); // compile warning+linker error
    ```
    This is to prevent an accidental extra copy of the mutable source buffer to the tree's arena: `substr` is implicitly convertible to `csubstr`. If you really intend to parse an originally mutable buffer in the tree's arena, convert it first to immutable by assigning the `substr` to a `csubstr` prior to calling `parse_in_arena()`:
    ```c++
    substr readwrite = /*...*/;
    csubstr readonly = readwrite; // ok
    Tree tree = parse_in_arena(readonly); // ok
    ```
    This problem is not raised by `parse_in_place()` because `csubstr` is not implicitly convertible to `substr`. 
- In the python API, `ryml.parse()` was removed and not just deprecated; the `parse_in_arena()` and `parse_in_place()` now replace this.
- `Callbacks`: changed behavior in `Parser` and `Tree`:
  - When a tree is copy-constructed or move-constructed to another, the receiving tree will start with the callbacks of the original.
  - When a tree is copy-assigned or move-assigned to another, the receiving tree will now change its callbacks to the original.
  - When a parser creates a new tree, the tree will now use a copy of the parser's callbacks object.
  - When an existing tree is given directly to the parser, both the tree and the parser now retain their own callback objects; any allocation or error during parsing will go through the respective callback object.


### New features

- `Parser`:
  - add `source()` and `filename()` to get the latest buffer and filename to be parsed
  - add `callbacks()` to get the parser's callbacks
- Add tracking of source code locations. This is useful for reporting semantic errors after the parsing phase (ie where the YAML is syntatically valid and parsing is successful, but the tree contents are semantically invalid). The locations can be obtained lazily from the parser when the first location is queried:
  ```c++
  // To obtain locations, use of the parser is needed:
  ryml::Parser parser;
  ryml::Tree tree = parser.parse_in_arena("source.yml", R"({
  aa: contents,
  foo: [one, [two, three]]
  })");
  // After parsing, on the first call to obtain a location,
  // the parser will cache a lookup structure to accelerate
  // tracking the location of a node, with complexity
  // O(numchars(srcbuffer)). Then it will do the lookup, with
  // complexity O(log(numlines(srcbuffer))).
  ryml::Location loc = parser.location(tree.rootref());
  assert(parser.location_contents(loc).begins_with("{"));
  // note the location members are zero-based:
  assert(loc.offset == 0u);
  assert(loc.line == 0u);
  assert(loc.col == 0u);
  // On the next call to location(), the accelerator is reused
  // and only the lookup is done.
  loc = parser.location(tree["aa"]);
  assert(parser.location_contents(loc).begins_with("aa"));
  assert(loc.offset == 2u);
  assert(loc.line == 1u);
  assert(loc.col == 0u);
  // KEYSEQ in flow style: points at the key
  loc = parser.location(tree["foo"]);
  assert(parser.location_contents(loc).begins_with("foo"));
  assert(loc.offset == 16u);
  assert(loc.line == 2u);
  assert(loc.col == 0u);
  loc = parser.location(tree["foo"][0]);
  assert(parser.location_contents(loc).begins_with("one"));
  assert(loc.line == 2u);
  assert(loc.col == 6u);
  // SEQ in flow style: location points at the opening '[' (there's no key)
  loc = parser.location(tree["foo"][1]);
  assert(parser.location_contents(loc).begins_with("["));
  assert(loc.line == 2u);
  assert(loc.col == 11u);
  loc = parser.location(tree["foo"][1][0]);
  assert(parser.location_contents(loc).begins_with("two"));
  assert(loc.line == 2u);
  assert(loc.col == 12u);
  loc = parser.location(tree["foo"][1][1]);
  assert(parser.location_contents(loc).begins_with("three"));
  assert(loc.line == 2u);
  assert(loc.col == 17u);
  // NOTE: reusing the parser with a new YAML source buffer
  // will invalidate the accelerator.
  ```
  See more details in the [quickstart sample](https://github.com/biojppm/rapidyaml/blob/bfb073265abf8c58bbeeeed7fb43270e9205c71c/samples/quickstart.cpp#L3759). Thanks to @cschreib for submitting a working example proving how simple it could be to achieve this.


### Fixes

- Accept `infinity`,`inf` and `nan` as special float values (but not mixed case: eg `InFiNiTy` or `Inf` or `NaN` are not accepted) ([PR #186](https://github.com/biojppm/rapidyaml/pull/186)).
- Accept special float values with upper or mixed case: `.Inf`, `.INF`, `.NaN`, `.NAN`. Previously, only low-case `.inf` and `.nan` were accepted ([PR #186](https://github.com/biojppm/rapidyaml/pull/186)).
- Accept `null` with upper or mixed case: `Null` or `NULL`. Previously, only low-case `null` was accepted ([PR #186](https://github.com/biojppm/rapidyaml/pull/186)).
- Fix [#182](https://github.com/biojppm/rapidyaml/issues/182): add missing export of DLL symbols, and document requirements for compiling shared library from the amalgamated header. [PR #183](https://github.com/biojppm/rapidyaml/pull/183), also [PR c4core#56](https://github.com/biojppm/c4core/pull/56) and [PR c4core#57](https://github.com/biojppm/c4core/pull/57).
- Fix [#185](https://github.com/biojppm/rapidyaml/issues/185): compilation failures in earlier Xcode versions ([PR #187](https://github.com/biojppm/rapidyaml/pull/187) and [PR c4core#61](https://github.com/biojppm/c4core/pull/61)):
  - `c4/substr_fwd.hpp`: (failure in Xcode 12 and earlier) forward declaration for `std::allocator` is inside the `inline namespace __1`, unlike later versions.
  - `c4/error.hpp`: (failure in debug mode in Xcode 11 and earlier) `__clang_major__` does not mean the same as in the common clang, and as a result the warning `-Wgnu-inline-cpp-without-extern` does not exist there.


### Improvements

- Rewrite filtering of scalars to improve parsing performance ([PR #188](https://github.com/biojppm/rapidyaml/pull/188)). Previously the scalar strings were parsed in place, which resulted in quadratic filtering complexity. This did not matter for small scalars fitting the cache (which is the more frequent case), but had a cost as the scalars grew larger. To achieve linearity, the code was changed so that the strings are now filtered to a temporary scratch space in the parser, and copied back to the output buffer after filtering, if any change occurred. The improvements were large for the folded scalars; the table below shows the benchmark results of throughput (MB/s) for several files containing large scalars of a single type:
  | scalar type	| before |	after |	improvement |
  |:------------|-------:|-------:|---------:|
  | block folded   | 276	| 561	| 103% |
  | block literal  | 331	| 611	| 85% |
  | single quoted  | 247	| 267	| 8% |
  | double quoted  | 212	| 230	| 8% |
  | plain (unquoted) | 173	| 186	| 8% |
  
  The cost for small scalars is negligible, with benchmark improvement in the interval of -2% to 5%, so well within the margin of benchmark variability in a regular OS.
- `Callbacks`: add `operator==()` and `operator!=()` ([PR #168](https://github.com/biojppm/rapidyaml/pull/168)).
- `Tree`: on error or assert prefer the error callback stored into the tree's current `Callbacks`, rather than the global `Callbacks` ([PR #168](https://github.com/biojppm/rapidyaml/pull/168)).
- `detail::stack<>`: improve behavior when assigning from objects `Callbacks`, test all rule-of-5 scenarios ([PR #168](https://github.com/biojppm/rapidyaml/pull/168)).


### Thanks

- @cschreib
- @aviktorov
