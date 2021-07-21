# Rapid YAML
[![MIT Licensed](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/biojppm/rapidyaml/blob/master/LICENSE.txt)
[![release](https://img.shields.io/github/v/release/biojppm/rapidyaml?color=g&include_prereleases&label=release%20&sort=semver)](https://github.com/biojppm/rapidyaml/releases)
[![Docs](https://img.shields.io/badge/docs-docsforge-blue)](https://rapidyaml.docsforge.com/)
[![Gitter](https://badges.gitter.im/rapidyaml/community.svg)](https://gitter.im/rapidyaml/community)

[![ci](https://github.com/biojppm/rapidyaml/workflows/ci/badge.svg?branch=master)](https://github.com/biojppm/rapidyaml/actions=workflow%3Aci)
[![Coveralls](https://coveralls.io/repos/github/biojppm/rapidyaml/badge.svg?branch=master)](https://coveralls.io/github/biojppm/rapidyaml)
[![Codecov](https://codecov.io/gh/biojppm/rapidyaml/branch/master/graph/badge.svg?branch=master)](https://codecov.io/gh/biojppm/rapidyaml)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/biojppm/rapidyaml.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/biojppm/rapidyaml/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/biojppm/rapidyaml.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/biojppm/rapidyaml/context:cpp)


Or ryml, for short. ryml is a library to parse and emit YAML, and do it fast.

ryml parses both read-only and in-situ source buffers; the resulting data
nodes hold only views to sub-ranges of the source buffer. No string copies or
duplications are done, and no virtual functions are used. The data tree is a
flat index-based structure stored in a single array. Serialization happens
only at your direct request, after parsing / before emitting. Internally the
data tree representation has no knowledge of types (but of course, every node
can have a YAML type tag). It is easy and fast to read, write and iterate
through the data tree.

ryml can use custom global and per-tree memory allocators, and is
exception-agnostic. Errors are reported via a custom error handler callback.
A default error handler implementation using `std::abort()` is provided, but
you can opt out, or provide your exception-throwing callback.

ryml has respect for your compilation times and therefore it is NOT
header-only. It uses standard cmake build files, so it is easy to compile and
install.

ryml has no dependencies, not even on the STL (although it does use the
libc). It provides optional headers that let you serialize/deserialize
STL strings and containers (or show you how to do it).

ryml is written in C++11, and is known to compile with:
* Visual Studio 2015 and later
* clang++ 3.9 and later
* g++ 5 and later

ryml is [extensively unit-tested in Linux, Windows and
MacOS](https://github.com/biojppm/rapidyaml/actions). The
tests run in the standard x64, x86 and arm architectures,
and include analysing ryml with:
  * valgrind
  * clang-tidy
  * clang sanitizers:
    * memory
    * address
    * undefined behavior
    * thread
  * [LGTM.com](https://lgtm.com/projects/g/biojppm/rapidyaml)

ryml is also partially available in Python, with more languages to follow (see
below).

See also [the changelog](./changelog) and [the roadmap](./ROADMAP.md).

Note that ryml uses submodules. Take care to use the `--recursive` flag
when cloning the repo, to ensure ryml's submodules are checked out as well:
```bash
git clone --recursive https://github.com/biojppm/rapidyaml
```
If you omit `--recursive`, after cloning you
will have to do `git submodule init` and `git submodule update` 
to ensure ryml's submodules are checked out.

------

## Table of contents

* [Is it rapid?](#is-it-rapid)
   * [Comparison with yaml-cpp](#comparison-with-yaml-cpp)
   * [Performance reading JSON](#performance-reading-json)
   * [Performance emitting](#performance-emitting)
* [Quick start](#quick-start)
* [Using ryml in your project](#using-ryml-in-your-project)
   * [Usage samples](#usage-samples)
   * [cmake build settings for ryml](#cmake-build-settings-for-ryml)
* [Other languages](#other-languages)
   * [Python](#python)
* [YAML standard conformance](#yaml-standard-conformance)
   * [Test suite status](#test-suite-status)
* [Known limitations](#known-limitations)
* [Alternative libraries](#alternative-libraries)
* [License](#license)


------

## Is it rapid?

You bet! On a i7-6800K CPU @3.40GHz:
 * ryml parses YAML at about ~150MB/s on Linux and ~100MB/s on Windows (vs2017). 
 * **ryml parses JSON at about ~450MB/s on Linux**, faster than sajson (didn't
   try yet on Windows).
 * compared against the other existing YAML libraries for C/C++:
   * ryml is in general between 2 and 3 times faster than [libyaml](https://github.com/yaml/libyaml)
   * ryml is in general between 20 and 70 times faster than
     [yaml-cpp](https://github.com/jbeder/yaml-cpp), and in some cases as
     much as 100x and [even
     200x](https://github.com/biojppm/c4core/pull/16#issuecomment-700972614) faster.

[Here's the benchmark](./bm/bm_parse.cpp). Using different
approaches within ryml (in-situ/read-only vs. with/without reuse), a YAML /
JSON buffer is repeatedly parsed, and compared against other libraries.

### Comparison with yaml-cpp

The first result set is for Windows, and is using a [appveyor.yml config
file](./bm/cases/appveyor.yml). A comparison of these results is
summarized on the table below:

| Read rates (MB/s)            | ryml   | yamlcpp | compared     |
|------------------------------|--------|---------|--------------|
| appveyor / vs2017 / Release  | 101.5  | 5.3     |  20x / 5.2%  |
| appveyor / vs2017 / Debug    |   6.4  | 0.0844  |  76x / 1.3%  |


The next set of results is taken in Linux, comparing g++ 8.2 and clang++ 7.0.1 in
parsing a YAML buffer from a [travis.yml config
file](./bm/cases/travis.yml) or a JSON buffer from a [compile_commands.json
file](./bm/cases/compile_commands.json). You
can [see the full results here](./bm/results/parse.linux.i7_6800K.md).
Summarizing:

| Read rates (MB/s)           | ryml   | yamlcpp | compared   |
|-----------------------------|--------|---------|------------|
| json   / clang++ / Release  | 453.5  | 15.1    |  30x / 3%  |
| json   /     g++ / Release  | 430.5  | 16.3    |  26x / 4%  |
| json   / clang++ / Debug    |  61.9  | 1.63    |  38x / 3%  |
| json   /     g++ / Debug    |  72.6  | 1.53    |  47x / 2%  |
| travis / clang++ / Release  | 131.6  | 8.08    |  16x / 6%  |
| travis /     g++ / Release  | 176.4  | 8.23    |  21x / 5%  |
| travis / clang++ / Debug    |  10.2  | 1.08    |   9x / 1%  |
| travis /     g++ / Debug    |  12.5  | 1.01    |  12x / 8%  |

The 450MB/s read rate for JSON puts ryml squarely in the same ballpark
as [RapidJSON](https://github.com/Tencent/rapidjson) and other fast json
readers
([data from here](https://lemire.me/blog/2018/05/03/how-fast-can-you-parse-json/)).
Even parsing full YAML is at ~150MB/s, which is still in that performance
ballpark, albeit at its lower end. This is something to be proud of, as the
YAML specification is much more complex than JSON: [23449 vs 1969 words](https://www.arp242.net/yaml-config.html#its-pretty-complex).


### Performance reading JSON

So how does ryml compare against other JSON readers? Well, it's one of the
fastest! 

The benchmark is the [same as above](./bm/parse.cpp), and it is reading
the [compile_commands.json](./bm/cases/compile_commands.json), The `_ro`
suffix notes parsing a read-only buffer (so buffer copies are performed),
while the `_rw` suffix means that the source buffer can be parsed in
situ. The `_reuse` means the data tree and/or parser are reused on each
benchmark repeat.

Here's what we get with g++ 8.2:

| Benchmark        | Release,MB/s | Debug,MB/s  |
|:-----------------|-------------:|------------:|
| rapidjson_ro     |       509.9  |       43.4  |
| rapidjson_rw     |      1329.4  |       68.2  |
| sajson_rw        |       434.2  |      176.5  |
| sajson_ro        |       430.7  |      175.6  |
| jsoncpp_ro       |       183.6  |    ? 187.9  |
| nlohmann_json_ro |       115.8  |       21.5  |
| yamlcpp_ro       |        16.6  |        1.6  |
| libyaml_ro       |       113.9  |       35.7  |
| libyaml_ro_reuse |       114.6  |       35.9  |
| ryml_ro          |       388.6  |       36.9  |
| ryml_rw          |       393.7  |       36.9  |
| ryml_ro_reuse    |       446.2  |       74.6  |
| ryml_rw_reuse    |       457.1  |       74.9  |

You can verify that (at least for this test) ryml beats most json
parsers at their own game, with the only exception of
[rapidjson](https://github.com/Tencent/rapidjson). And actually, in
Debug, [rapidjson](https://github.com/Tencent/rapidjson) is slower
than ryml, and [sajson](https://github.com/chadaustin/sajson)
manages to be faster (but not sure about jsoncpp; need to scrutinize there
the suspicious fact that the Debug result is faster than the Release result).


### Performance emitting

Emitting benchmarks were not created yet, but feedback from some users
reports as much as 25x speedup from yaml-cpp [(eg,
here)](https://github.com/biojppm/rapidyaml/issues/28#issue-553855608).

If you have data or YAML code for this, please submit a pull request, or
just send us the files!


------

## Quick start

If you're wondering whether ryml's speed comes at a usage cost, you
need not: with ryml, you can have your cake and eat it too. Being
rapid is definitely NOT the same as being unpractical, so ryml was
written with easy AND efficient usage in mind, and comes with a two
level API for accessing and traversing the data tree.

The following snippet is a quick overview taken from [the quickstart
sample](samples/quickstart.cpp). After cloning ryml (don't forget the
`--recursive` flag for git), you can very
easily build and run this executable using any of the build samples,
eg the [`add_subdirectory()` sample](samples/add_subdirectory/).

```c++
// Parse YAML code in place, potentially mutating the buffer.
// It is also possible to:
//   - parse a read-only buffer
//   - reuse an existing tree (advised)
//   - reuse an existing parser (advised)
char yml_buf[] = "{foo: 1, bar: [2, 3], john: doe}";
ryml::Tree tree = ryml::parse(ryml::substr(yml_buf));

// Note: it will always be significantly faster to use mutable
// buffers and reuse tree+parser; in the quickstart sample you
// will find examples for this.


//------------------------------------------------------------------
// API overview

// ryml has a two-level API:
//
// The lower level index API is based on the indices of nodes,
// where the node's id is the node's position in the tree's data
// array. This API is very efficient, but somewhat difficult to use:
size_t root_id = tree.root_id();
size_t bar_id = tree.find_child(root_id, "bar"); // need to get the index right
CHECK(tree.is_map(root_id)); // all of the index methods are in the tree
CHECK(tree.is_seq(bar_id));  // ... and receive the subject index

// The node API is a lightweight abstraction sitting on top of the
// index API, but offering a much more convenient interaction:
ryml::NodeRef root = tree.rootref();
ryml::NodeRef bar = tree["bar"];
CHECK(root.is_map());
CHECK(bar.is_seq());
// NodeRef is a lightweight handle to the tree and associated id:
CHECK(root.tree() == &tree); // NodeRef points at its tree, WITHOUT refcount
CHECK(root.id() == root_id); // NodeRef's id is the index of the node
CHECK(bar.id() == bar_id);   // NodeRef's id is the index of the node

// The node API translates very cleanly to the index API, so most
// of the code examples below are using the node API.

// One significant point of the node API is that it holds a raw
// pointer to the tree. Care must be taken to ensure the lifetimes
// match, so that a node will never access the tree after the tree
// went out of scope.


//------------------------------------------------------------------
// To read the parsed tree

// Node::operator[] does a lookup, is O(num_children[node]).
// maps use string keys, seqs use integral keys.
CHECK(tree["foo"].is_keyval());
CHECK(tree["foo"].key() == "foo");
CHECK(tree["foo"].val() == "1");
CHECK(tree["bar"].is_seq());
CHECK(tree["bar"].has_key());
CHECK(tree["bar"].key() == "bar");
CHECK(tree["bar"][0].val() == "2");
CHECK(tree["bar"][1].val() == "3");
CHECK(tree["john"].val() == "doe");
// An integral key is the position of the child within its parent,
// so even maps can also use int keys, if the key position is
// known.
CHECK(tree[0].id() == tree["foo"].id());
CHECK(tree[1].id() == tree["bar"].id());
CHECK(tree[2].id() == tree["john"].id());
// Tree::operator[](int) searches a root child by its position.
CHECK(tree[0].id() == tree["foo"].id());  // 0: first child of root
CHECK(tree[1].id() == tree["bar"].id());  // 1: first child of root
CHECK(tree[2].id() == tree["john"].id()); // 2: first child of root
// NodeRef::operator[](int) searches a node child by its position
// on __the node__'s children list:
CHECK(bar[0].val() == "2"); // 0 means first child of bar
CHECK(bar[1].val() == "3"); // 1 means second child of bar
// NodeRef::operator[](string):
// A string key is the key of the node: lookup is by name. So it
// is only available for maps, and it is NOT available for seqs,
// since seq members do not have keys.
CHECK(tree["foo"].key() == "foo");
CHECK(tree["bar"].key() == "bar");
CHECK(tree["john"].key() == "john");
CHECK(bar.is_seq());
// CHECK(bar["BOOM!"].is_seed()); // error, seqs do not have key lookup

// Note that maps can also use index keys as well as string keys:
CHECK(root["foo"].id() == root[0].id());
CHECK(root["bar"].id() == root[1].id());
CHECK(root["john"].id() == root[2].id());

// Please note that since a ryml tree uses indexed linked lists for storing
// children, the complexity of `Tree::operator[csubstr]` and
// `Tree::operator[size_t]` is linear on the number of root children. If you use
// it with a large tree where the root has many children, you may get a
// performance hit. To avoid this hit, you can create your own accelerator
// structure. For example, before doing a lookup, do a single traverse at the
// root level to fill an `std::map<csubstr,size_t>` mapping key names to node
// indices; with a node index, a lookup (via `Tree::get()`) is O(1), so this way
// you can get O(log n) lookup from a key.
// 
// As for `NodeRef`, the difference from `NodeRef::operator[]`
// to `Tree::operator[]` is that the latter refers to the root node, whereas
// the former can be invoked on any node. But the lookup process is the same for
// both and their algorithmic complexity is the same: they are both linear in
// the number of direct children; but depending on the data, that number may
// be very different from one to another.


//------------------------------------------------------------------
// Hierarchy:

{
    ryml::NodeRef foo = root.first_child();
    ryml::NodeRef john = root.last_child();
    CHECK(tree.size() == 6); // O(1) number of nodes in the tree
    CHECK(root.num_children() == 3); // O(num_children[root])
    CHECK(foo.num_siblings() == 3); // O(num_children[parent(foo)])
    CHECK(foo.parent().id() == root.id()); // parent() is O(1)
    CHECK(root.first_child().id() == root["foo"].id()); // first_child() is O(1)
    CHECK(root.last_child().id() == root["john"].id()); // last_child() is O(1)
    CHECK(john.first_sibling().id() == foo.id());
    CHECK(foo.last_sibling().id() == john.id());
    // prev_sibling(), next_sibling(): (both are O(1))
    CHECK(foo.num_siblings() == root.num_children());
    CHECK(foo.prev_sibling().id() == ryml::NONE); // foo is the first_child()
    CHECK(foo.next_sibling().key() == "bar");
    CHECK(foo.next_sibling().next_sibling().key() == "john");
    CHECK(foo.next_sibling().next_sibling().next_sibling().id() == ryml::NONE); // john is the last_child()
}


//------------------------------------------------------------------
// Iterating:
{
    ryml::csubstr expected_keys[] = {"foo", "bar", "john"};
    // iterate children using the high-level node API:
    {
        size_t count = 0;
        for(ryml::NodeRef const& child : root.children())
            CHECK(child.key() == expected_keys[count++]);
    }
    // iterate siblings using the high-level node API:
    {
        size_t count = 0;
        for(ryml::NodeRef const& child : root["foo"].siblings())
            CHECK(child.key() == expected_keys[count++]);
    }
    // iterate children using the lower-level tree index API:
    {
        size_t count = 0;
        for(size_t child_id = tree.first_child(root_id); child_id != ryml::NONE; child_id = tree.next_sibling(child_id))
            CHECK(tree.key(child_id) == expected_keys[count++]);
    }
    // iterate siblings using the lower-level tree index API:
    // (notice the only difference from above is in the loop
    // preamble, which calls tree.first_sibling(bar_id) instead of
    // tree.first_child(root_id))
    {
        size_t count = 0;
        for(size_t child_id = tree.first_sibling(bar_id); child_id != ryml::NONE; child_id = tree.next_sibling(child_id))
            CHECK(tree.key(child_id) == expected_keys[count++]);
    }
}


//------------------------------------------------------------------
// Gotchas:
CHECK(!tree["bar"].has_val());          // seq is a container, so no val
CHECK(!tree["bar"][0].has_key());       // belongs to a seq, so no key
CHECK(!tree["bar"][1].has_key());       // belongs to a seq, so no key
//CHECK(tree["bar"].val() == BOOM!);    // ... so attempting to get a val is undefined behavior
//CHECK(tree["bar"][0].key() == BOOM!); // ... so attempting to get a key is undefined behavior
//CHECK(tree["bar"][1].key() == BOOM!); // ... so attempting to get a key is undefined behavior


//------------------------------------------------------------------
// Deserializing: use operator>>
{
    int foo = 0, bar0 = 0, bar1 = 0;
    std::string john;
    root["foo"] >> foo;
    root["bar"][0] >> bar0;
    root["bar"][1] >> bar1;
    root["john"] >> john; // requires from_chars(std::string). see serialization samples below.
    CHECK(foo == 1);
    CHECK(bar0 == 2);
    CHECK(bar1 == 3);
    CHECK(john == "doe");
}


//------------------------------------------------------------------
// Modifying existing nodes: operator<< vs operator=

// operator= assigns an existing string to the receiving node.
// This pointer will be in effect until the tree goes out of scope
// so beware to only assign from strings outliving the tree.
root["foo"] = "says you";
root["bar"][0] = "-2";
root["bar"][1] = "-3";
root["john"] = "ron";
// Now the tree is _pointing_ at the memory of the strings above.
// That is OK because those are static strings and will outlive
// the tree.
CHECK(root["foo"].val() == "says you");
CHECK(root["bar"][0].val() == "-2");
CHECK(root["bar"][1].val() == "-3");
CHECK(root["john"].val() == "ron");
// WATCHOUT: do not assign from temporary objects:
// {
//     std::string crash("will dangle");
//     root["john"] == ryml::to_csubstr(crash);
// }
// CHECK(root["john"] == "dangling"); // CRASH! the string was deallocated

// operator<< first serializes the input to the tree's arena, then
// assigns the serialized string to the receiving node. This avoids
// constraints with the lifetime, since the arena lives with the tree.
CHECK(tree.arena().empty());
root["foo"] << "says who";  // requires to_chars(). see serialization samples below.
root["bar"][0] << 20;
root["bar"][1] << 30;
root["john"] << "deere";
CHECK(root["foo"].val() == "says who");
CHECK(root["bar"][0].val() == "20");
CHECK(root["bar"][1].val() == "30");
CHECK(root["john"].val() == "deere");
CHECK(tree.arena() == "says who2030deere"); // the result of serializations to the tree arena
// using operator<< instead of operator=, the crash above is avoided:
{
    std::string ok("in_scope");
    // root["john"] == ryml::to_csubstr(ok); // don't, will dangle
    root["john"] << ryml::to_csubstr(ok); // OK, copy to the tree's arena
}
CHECK(root["john"] == "in_scope"); // OK!
CHECK(tree.arena() == "says who2030deerein_scope"); // the result of serializations to the tree arena


//------------------------------------------------------------------
// Adding new nodes:

// adding a keyval node to a map:
CHECK(root.num_children() == 3);
root["newkeyval"] = "shiny and new"; // using these strings
root.append_child() << ryml::key("newkeyval (serialized)") << "shiny and new (serialized)"; // serializes and assigns the serialization
CHECK(root.num_children() == 5);
CHECK(root["newkeyval"].key() == "newkeyval");
CHECK(root["newkeyval"].val() == "shiny and new");
CHECK(root["newkeyval (serialized)"].key() == "newkeyval (serialized)");
CHECK(root["newkeyval (serialized)"].val() == "shiny and new (serialized)");
CHECK( ! root["newkeyval"].key().is_sub(tree.arena())); // it's using directly the static string above
CHECK( ! root["newkeyval"].val().is_sub(tree.arena())); // it's using directly the static string above
CHECK(   root["newkeyval (serialized)"].key().is_sub(tree.arena())); // it's using a serialization of the string above
CHECK(   root["newkeyval (serialized)"].val().is_sub(tree.arena())); // it's using a serialization of the string above
// adding a val node to a seq:
CHECK(root["bar"].num_children() == 2);
root["bar"][2] = "oh so nice";
root["bar"][3] << "oh so nice (serialized)";
CHECK(root["bar"].num_children() == 4);
CHECK(root["bar"][2].val() == "oh so nice");
CHECK(root["bar"][3].val() == "oh so nice (serialized)");
// adding a seq node:
CHECK(root.num_children() == 5);
root["newseq"] |= ryml::SEQ;
root.append_child() << ryml::key("newseq (serialized)") |= ryml::SEQ;
CHECK(root.num_children() == 7);
CHECK(root["newseq"].num_children() == 0);
CHECK(root["newseq (serialized)"].num_children() == 0);
// adding a map node:
CHECK(root.num_children() == 7);
root["newmap"] |= ryml::MAP;
root.append_child() << ryml::key("newmap (serialized)") |= ryml::SEQ;
CHECK(root.num_children() == 9);
CHECK(root["newmap"].num_children() == 0);
CHECK(root["newmap (serialized)"].num_children() == 0);
// operator[] does not mutate the tree until the returned node is
// written to.
//
// Until such time, the NodeRef object keeps in itself the required
// information to write to the proper place in the tree. This is
// called being in a "seed" state.
//
// This means that passing a key/index which does not exist will
// not mutate the tree, but will instead store (in the node) the
// proper place of the tree to do so if and when it is required.
//
// This is a significant difference from eg, the behavior of
// std::map, which mutates the map immediately within the call to
// operator[].
CHECK(!root.has_child("I am nobody"));
ryml::NodeRef nobody = root["I am nobody"];
CHECK(nobody.valid());   // points at the tree, and a specific place in the tree
CHECK(nobody.is_seed()); // ... but nothing is there yet.
CHECK(!root.has_child("I am nobody")); // same as above
ryml::NodeRef somebody = root["I am somebody"];
CHECK(!root.has_child("I am somebody")); // same as above
CHECK(somebody.valid());
CHECK(somebody.is_seed()); // same as above
somebody = "indeed";  // this will commit to the tree, mutating at the proper place
CHECK(somebody.valid());
CHECK(!somebody.is_seed()); // now the tree has this node, and it is no longer a seed
CHECK(root.has_child("I am somebody"));
CHECK(root["I am somebody"].val() == "indeed");


//------------------------------------------------------------------
// Emitting:

// emit to a FILE*
ryml::emit(tree, stdout);
// emit to a stream
std::stringstream ss;
ss << tree;
std::string stream_result = ss.str();
// emit to a buffer:
std::string str_result = ryml::emitrs<std::string>(tree);
// can emit to any given buffer:
char buf[1024];
ryml::csubstr buf_result = ryml::emit(tree, buf);
// now check
ryml::csubstr expected_result = R"(foo: says who
bar:
  - 20
  - 30
  - oh so nice
  - oh so nice (serialized)
john: in_scope
newkeyval: shiny and new
newkeyval (serialized): shiny and new (serialized)
newseq: []
newseq (serialized): []
newmap: {}
newmap (serialized): []
I am somebody: indeed
)";
CHECK(buf_result == expected_result);
CHECK(str_result == expected_result);
CHECK(stream_result == expected_result);
// There are many possibilities to emit to buffer;
// please look at the quickstart sample functions below.
```

The [quickstart.cpp sample](./samples/quickstart.cpp) (from which the
above overview was taken) has many more detailed examples, and should
be your first port of call to find out any particular point about
ryml's API. It is tested in the CI, and thus has the correct behavior.
There you can find the following subjects being addressed:

```c++
sample_substr();               ///< about ryml's string views (from c4core)
sample_parse_read_only();      ///< parse a read-only YAML source buffer
sample_parse_in_situ();        ///< parse an immutable YAML source buffer
sample_parse_reuse_tree();     ///< parse into an existing tree, maybe into a node
sample_parse_reuse_parser();   ///< reuse an existing parser
sample_parse_reuse_tree_and_parser(); ///< how to reuse existing trees and parsers
sample_iterate_trees();        ///< visit individual nodes and iterate through trees
sample_create_trees();         ///< programatically create trees
sample_tree_arena();           ///< interact with the tree's serialization arena
sample_fundamental_types();    ///< serialize/deserialize fundamental types
sample_formatting();           ///< control formatting when serializing/deserializing
sample_base64();               ///< encode/decode base64
sample_user_scalar_types();    ///< serialize/deserialize scalar (leaf/string) types
sample_user_container_types(); ///< serialize/deserialize container (map or seq) types
sample_std_types();            ///< serialize/deserialize STL containers
sample_emit_to_container();    ///< emit to memory, eg a string or vector-like container
sample_emit_to_stream();       ///< emit to a stream, eg std::ostream
sample_emit_to_file();         ///< emit to a FILE*
sample_emit_nested_node();     ///< pick a nested node as the root when emitting
sample_json();                 ///< JSON parsing and emitting: notes and constraints
sample_anchors_and_aliases();  ///< deal with YAML anchors and aliases
sample_tags();                 ///< deal with YAML type tags
sample_docs();                 ///< deal with YAML docs
sample_error_handler();        ///< set a custom error handler
sample_global_allocator();     ///< set a global allocator for ryml
sample_per_tree_allocator();   ///< set per-tree allocators
```


------

## Using ryml in your project

As with any other library, you have the option to integrate ryml into
your project's build setup, thereby building ryml together with your
project, or -- prior to configuring your project -- you can have ryml
installed either manually or through package managers.

If you opt for package managers, here's where ryml is available so far (thanks to all the contributors!):
  * [vcpkg](https://vcpkg.io/en/packages.html): `vcpkg install ryml`
  * Arch Linux/Manjaro:
    * [rapidyaml-git (AUR)](https://aur.archlinux.org/packages/rapidyaml-git/)
    * [python-rapidyaml-git (AUR)](https://aur.archlinux.org/packages/python-rapidyaml-git/)
  * [PyPI](https://pypi.org/project/rapidyaml/)

Although package managers are very useful for quickly getting up to speed,
the advised way is still to bring ryml as a submodule of your project,
building both together. This makes it easy to track any upstream changes in ryml.
Also, ryml is fairly small, and is quick to build,
so there's not much of a cost for building it with your project.

Currently [cmake](https://cmake.org/) is required to build ryml; we
recommend a recent cmake version, at least 3.13.


### Usage samples

These samples show how to build an application using ryml. All the
samples use [the same quickstart executable
source](./samples/quickstart.cpp), but are built in different ways,
showing several alternatives to integrate ryml into your project. We
also encourage you to refer to the [quickstart source](./samples/quickstart.cpp) itself, which
extensively covers most of the functionality that you may want out of
ryml.

Each sample brings a `run.sh` script with the sequence of commands
required to successfully build and run the application (this is a bash
script and runs in Linux and MacOS, but it is also possible to run in
Windows via Git Bash or the WSL). Click on the links below to find out
more about each sample:

| Sample name        | ryml is part of build?   | cmake file   | commands     |
|:-------------------|--------------------------|:-------------|:-------------|
| [`add_subdirectory`](./samples/add_subdirectory) | **yes**                      | [`CMakeLists.txt`](./samples/add_subdirectory/CMakeLists.txt) | [`run.sh`](./samples/add_subdirectory/run.sh) |
| [`fetch_content`](./samples/fetch_content)      | **yes**                      | [`CMakeLists.txt`](./samples/fetch_content/CMakeLists.txt) | [`run.sh`](./samples/fetch_content/run.sh) |
| [`find_package`](./samples/find_package)        | **no**<br>needs prior install or package  | [`CMakeLists.txt`](./samples/find_package/CMakeLists.txt) | [`run.sh`](./samples/find_package/run.sh) |

### cmake build settings for ryml
The following cmake variables can be used to control the build behavior of
ryml:

  * `RYML_DEFAULT_CALLBACKS=ON/OFF`. Enable/disable ryml's default
    implementation of error and allocation callbacks. Defaults to `ON`.
  * `RYML_STANDALONE=ON/OFF`. ryml uses
    [c4core](https://github.com/biojppm/c4core), a C++ library with low-level
    multi-platform utilities for C++. When `RYML_STANDALONE=ON`, c4core is
    incorporated into ryml as if it is the same library. Defaults to `ON`.

If you're developing ryml or just debugging problems with ryml itself, the
following variables can be helpful:
  * `RYML_DEV=ON/OFF`: a bool variable which enables development targets such as
    unit tests, benchmarks, etc. Defaults to `OFF`.
  * `RYML_DBG=ON/OFF`: a bool variable which enables verbose prints from
    parsing code; can be useful to figure out parsing problems. Defaults to
    `OFF`.

#### Forcing ryml to use a different c4core version

ryml is strongly coupled to c4core, and this is reinforced by the fact
that c4core is a submodule of the current repo. However, it is still
possible to use a c4core version different from the one in the repo
(of course, only if there are no incompatibilities between the
versions). You can find out how to achieve this by looking at the [`custom_c4core` sample](samples/custom_c4core).


------

## Other languages

One of the aims of ryml is to provide an efficient YAML API for other
languages. There's already a cursory implementation for Python (using
only the low-level API). After ironing out the general approach, other
languages are likely to follow: probably (in order) JavaScript, C#,
Java, Ruby, PHP, Octave and R (all of this is possible because we're
using [SWIG](http://www.swig.org/), which makes it easy to do so).

### Python

(Note that this is a work in progress. Additions will be made and things will
be changed.) With that said, here's an example of the Python API:

```python
import ryml

# because ryml does not take ownership of the source buffer
# ryml cannot accept strings; only bytes or bytearrays
src = b"{HELLO: a, foo: b, bar: c, baz: d, seq: [0, 1, 2, 3]}"

def check(tree):
    # for now, only the index-based low-level API is implemented
    assert tree.size() == 10
    assert tree.root_id() == 0
    assert tree.first_child(0) == 1
    assert tree.next_sibling(1) == 2
    assert tree.first_sibling(5) == 2
    assert tree.last_sibling(1) == 5
    # use bytes objects for queries
    assert tree.find_child(0, b"foo") == 1
    assert tree.key(1) == b"foo")
    assert tree.val(1) == b"b")
    assert tree.find_child(0, b"seq") == 5
    assert tree.is_seq(5)
    # to loop over children:
    for i, ch in enumerate(ryml.children(tree, 5)):
        assert tree.val(ch) == [b"0", b"1", b"2", b"3"][i]
    # to loop over siblings:
    for i, sib in enumerate(ryml.siblings(tree, 5)):
        assert tree.key(sib) == [b"HELLO", b"foo", b"bar", b"baz", b"seq"][i]
    # to walk over all elements
    visited = [False] * tree.size()
    for n, indentation_level in ryml.walk(tree):
        # just a dumb emitter
        left = "  " * indentation_level
        if tree.is_keyval(n):
           print("{}{}: {}".format(left, tree.key(n), tree.val(n))
        elif tree.is_val(n):
           print("- {}".format(left, tree.val(n))
        elif tree.is_keyseq(n):
           print("{}{}:".format(left, tree.key(n))
        visited[inode] = True
    assert False not in visited
    # NOTE about encoding!
    k = tree.get_key(5)
    print(k)  # '<memory at 0x7f80d5b93f48>'
    assert k == b"seq"               # ok, as expected
    assert k != "seq"                # not ok - NOTE THIS! 
    assert str(k) != "seq"           # not ok
    assert str(k, "utf8") == "seq"   # ok again

# parse immutable buffer
tree = ryml.parse(src)
check(tree) # OK

# also works, but requires bytearrays or
# objects offering writeable memory
mutable = bytearray(src)
tree = ryml.parse_in_situ(mutable)
check(tree) # OK
```

As expected, the performance results so far are encouraging. In
a [timeit benchmark](api/python/parse_bm.py) compared
against [PyYaml](https://pyyaml.org/)
and [ruamel.yaml](https://yaml.readthedocs.io/en/latest/), ryml parses
quicker by a factor of 30x-50x:

```
+-----------------------+-------+----------+---------+----------------+
| case                  | iters | time(ms) | avg(ms) | avg_read(MB/s) |
+-----------------------+-------+----------+---------+----------------+
| parse:RuamelYaml      |    88 | 800.483  |  9.096  |      0.234     |
| parse:PyYaml          |    88 | 541.370  |  6.152  |      0.346     |
| parse:RymlRo          |  3888 | 776.020  |  0.200  |     10.667     |
| parse:RymlRoReuse     |  1888 | 381.558  |  0.202  |     10.535     |
| parse:RymlRw          |  3888 | 775.121  |  0.199  |     10.679     |
| parse:RymlRwReuse     |  3888 | 774.534  |  0.199  |     10.687     |
+-----------------------+-------+----------+---------+----------------+
```

(Note that the results above are somewhat biased towards ryml, because it does
not perform any type conversions: return types are merely `memoryviews` to
the source buffer.)


------

## YAML standard conformance

ryml is under active development, but is close to feature complete. The
following YAML core features are well covered in the unit tests:
* mappings
* sequences
* complex keys
* literal blocks
* quoted scalars
* tags
* anchors and references
* UTF8 is expected to mostly work
  
Of course, there are many dark corners in YAML, and there certainly can
appear cases which ryml fails to parse. Your [bug reports or pull
requests](https://github.com/biojppm/rapidyaml/issues) are very welcome.

See also [the roadmap](./ROADMAP.md) for a list of future work.


### Test suite status

Integration of the >300 cases in the [YAML test
suite](https://github.com/yaml/yaml-test-suite) is ongoing work. Each of
these tests have several subparts:
 * in-yaml: mildly, plainly or extremely difficult-to-parse yaml
 * in-json: equivalent json (where possible/meaningful)
 * out-yaml: equivalent standard yaml
 * events: equivalent libyaml events allowing to establish correctness of
   the parsed results

When testing, ryml tries to parse each of the 3 yaml/json parts. If the
parsing suceeds, then the ryml test will emit the parsed tree, then parse the
emitted result and verify that emission is idempotent, ie that the emitted
result is the same as its input without any loss of information. To ensure
correctness, this happens over four levels of parse/emission pairs, resulting
in ~200 checks per test case.

Please note that in [their own words](http://matrix.yaml.io/), the tests from
the YAML test suite *contain a lot of edge cases that don't play such an
important role in real world examples*. Despite the extreme focus of the test
suite, as of May 2020, ryml only fails to parse ~30 out of the ~1000=~3x300
cases from the test suite. Out of all other cases, all the ~200 checks per
case are 100% successful for consistency over parse/emit pairs --- but please
note that the events part is not yet read in and used to check for
correctness, and therefore that **even though ryml may suceed in parsing,
there still exists a minority of cases which may not be correct**. Currently,
I would estimate this fraction at somewhere around 5%. These are the suite
cases where ryml fails to parse any of its subparts:
[EXG3](https://github.com/yaml/yaml-test-suite/tree/master/test/EXG3.tml),
[M7A3](https://github.com/yaml/yaml-test-suite/tree/master/test/M7A3.tml),
[735Y](https://github.com/yaml/yaml-test-suite/tree/master/test/735Y.tml),
[82AN](https://github.com/yaml/yaml-test-suite/tree/master/test/82AN.tml),
[9YRD](https://github.com/yaml/yaml-test-suite/tree/master/test/9YRD.tml),
[EX5H](https://github.com/yaml/yaml-test-suite/tree/master/test/EX5H.tml),
[HS5T](https://github.com/yaml/yaml-test-suite/tree/master/test/HS5T.tml),
[7T8X](https://github.com/yaml/yaml-test-suite/tree/master/test/7T8X.tml),
[RZP5](https://github.com/yaml/yaml-test-suite/tree/master/test/RZP5.tml),
[FH7J](https://github.com/yaml/yaml-test-suite/tree/master/test/FH7J.tml),
[PW8X](https://github.com/yaml/yaml-test-suite/tree/master/test/PW8X.tml),
[CN3R](https://github.com/yaml/yaml-test-suite/tree/master/test/CN3R.tml),
[6BCT](https://github.com/yaml/yaml-test-suite/tree/master/test/6BCT.tml),
[G5U8](https://github.com/yaml/yaml-test-suite/tree/master/test/G5U8.tml),
[K858](https://github.com/yaml/yaml-test-suite/tree/master/test/K858.tml),
[NAT4](https://github.com/yaml/yaml-test-suite/tree/master/test/NAT4.tml),
[9MMW](https://github.com/yaml/yaml-test-suite/tree/master/test/9MMW.tml),
[DC7X](https://github.com/yaml/yaml-test-suite/tree/master/test/DC7X.tml),
[L94M](https://github.com/yaml/yaml-test-suite/tree/master/test/L94M.tml),

Except for the known limitations listed next, all other suite cases are
expected to work.


--------- 

## Known limitations

ryml makes no effort to follow the standard in the following situations:

* `%YAML` directives have no effect and are ignored.
* `%TAG` directives have no effect and are ignored. All schemas are assumed
  to be the default YAML 2002 schema.
* container elements are not accepted as mapping keys. keys must be
  simple strings and cannot themselves be mappings or sequences. But mapping
  values can be any of the above. [YAML test
  suite](https://github.com/yaml/yaml-test-suite) cases:
  [4FJ6](https://github.com/yaml/yaml-test-suite/tree/master/test/4FJ6.tml),
  [6BFJ](https://github.com/yaml/yaml-test-suite/tree/master/test/6BFJ.tml),
  [6PBE](https://github.com/yaml/yaml-test-suite/tree/master/test/6PBE.tml),
  [6PBE](https://github.com/yaml/yaml-test-suite/tree/master/test/6PBE.tml),
  [KK5P](https://github.com/yaml/yaml-test-suite/tree/master/test/KK5P.tml),
  [KZN9](https://github.com/yaml/yaml-test-suite/tree/master/test/KZN9.tml),
  [LX3P](https://github.com/yaml/yaml-test-suite/tree/master/test/LX3P.tml),
  [M5DY](https://github.com/yaml/yaml-test-suite/tree/master/test/M5DY.tml),
  [Q9WF](https://github.com/yaml/yaml-test-suite/tree/master/test/Q9WF.tml),
  [SBG9](https://github.com/yaml/yaml-test-suite/tree/master/test/SBG9.tml),
  [X38W](https://github.com/yaml/yaml-test-suite/tree/master/test/X38W.tml),
  [XW4D](https://github.com/yaml/yaml-test-suite/tree/master/test/XW4D.tml).


------

## Alternative libraries

Why this library? Because none of the existing libraries was quite what I
wanted. There are two C/C++ libraries that I know of:

* [libyaml](https://github.com/yaml/libyaml)
* [yaml-cpp](https://github.com/jbeder/yaml-cpp)

The standard [libyaml](https://github.com/yaml/libyaml) is a bare C
library. It does not create a representation of the data tree, so it can't
qualify as practical. My initial idea was to wrap parsing and emitting around
libyaml, but to my surprise I found out it makes heavy use of allocations and
string duplications when parsing. I briefly pondered on sending PRs to reduce
these allocation needs, but not having a permanent tree to store the parsed
data was too much of a downside.

[yaml-cpp](https://github.com/jbeder/yaml-cpp) is full of functionality, but
is heavy on the use of node-pointer-based structures like `std::map`,
allocations, string copies and slow C++ stream serializations. This is
generally a sure way of making your code slower, and strong evidence of this
can be seen in the benchmark results above.

When performance and low latency are important, using contiguous structures
for better cache behavior and to prevent the library from trampling over the
client's caches, parsing in place and using non-owning strings is of central
importance. Hence this Rapid YAML library which, with minimal compromise,
bridges the gap from efficiency to usability. This library takes inspiration
from [RapidJSON](https://github.com/Tencent/rapidjson)
and [RapidXML](http://rapidxml.sourceforge.net/).


------
## License

ryml is permissively licensed under the [MIT license](LICENSE.txt).
