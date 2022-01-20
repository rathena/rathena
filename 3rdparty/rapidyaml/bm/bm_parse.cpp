#include <ryml.hpp>
#include <ryml_std.hpp>
#include <c4/fs/fs.hpp>
#include "c4/yml/parse.hpp"

#include <libfyaml.h>

#include <vector>
#include <iostream>

#include "./libyaml.hpp"
#include <benchmark/benchmark.h>

// warning suppressions for thirdparty code
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4100) // sajson.h(1313,41): warning C4100: 'input_document_size_in_bytes': unreferenced formal parameter
#   pragma warning(disable : 4127) // conditional expression is constant
#   pragma warning(disable : 4200) // sajson.h(209,28): warning C4200: nonstandard extension used: zero-sized array in struct/union
#   pragma warning(disable : 4242) // sajson.h(2295,1): warning C4242: '=': conversion from 'unsigned int' to 'char', possible loss of data
#   pragma warning(disable : 4244) // sajson.h(2295,26): warning C4244: '=': conversion from 'unsigned int' to 'char', possible loss of data
#   pragma warning(disable : 4389) // '==': signed/unsigned mismatch
#   pragma warning(disable : 4996) // warning C4996: 'Json::Reader': Use CharReader and CharReaderBuilder instead.
#elif defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wc99-extensions"
#   pragma clang diagnostic ignored "-Wfloat-equal"
#   pragma clang diagnostic ignored "-Wshadow"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wconversion"
#   if __clang_major__ >= 8
#       pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#   endif
#elif defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wshadow"
#   pragma GCC diagnostic ignored "-Wunused-parameter"
#   pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#   pragma GCC diagnostic ignored "-Wfloat-equal"
#   pragma GCC diagnostic ignored "-Wpedantic"
#   pragma GCC diagnostic ignored "-Wuseless-cast"
#   pragma GCC diagnostic ignored "-Wconversion"
#   pragma GCC diagnostic ignored "-Wsign-conversion"
#   if __GNUC__ >= 8
#       pragma GCC diagnostic ignored "-Wclass-memaccess" // rapidjson/document.h:1952:24
#   endif
#endif
#include <rapidjson/document.h>
#include <sajson.h>
#include <json/json.h>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#if defined(_MSC_VER)
#   pragma warning(pop)
#elif defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif


namespace bm = benchmark;


// now for our code
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4996) // warning C4996: 'Json::Reader': Use CharReader and CharReaderBuilder instead.
#elif defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   pragma clang diagnostic ignored "-Wunused-variable"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#elif defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#   pragma GCC diagnostic ignored "-Wunused-variable"
#   pragma GCC diagnostic ignored "-Wsign-conversion"
#endif


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum : int {
      kClearTree=1,
      kClearTreeArena=2,
      kResetInPlace=4,
      kAll=kClearTree|kClearTreeArena|kResetInPlace,
};

struct BmCase
{
    c4::csubstr            filename;
    std::vector<char>      src;
    std::vector<char>      in_place;
    ryml::Parser           ryml_parser;
    ryml::Tree             ryml_tree;
    bool                   is_json;
    rapidjson::Document    rapidjson_doc;
    ryml::Tree             libyaml_tree;

    void run(int argc, char **argv)
    {
        bm::Initialize(&argc, argv);
        C4_CHECK(argc == 2);
        load(argv[1]);
        bm::RunSpecifiedBenchmarks();
    }

    void load(const char* file)
    {
        filename = c4::to_csubstr(file);
        is_json = filename.ends_with(".json");
        std::cout << "-----------------------------------\n";
        std::cout << "running case: " << filename.basename() << "\n";
        std::cout << "file: " << filename << "\n";
        std::cout << "-----------------------------------\n";
        c4::fs::file_get_contents(file, &src);
        if(src.back() != '\0')
        {
            src.push_back('\0');
        }
        in_place = src;
        C4_ASSERT_MSG(strlen(in_place.data()) == in_place.size()-1,
                      "len=%zu sz=%zu",
                      strlen(in_place.data()), in_place.size());
    }

    void prepare(bm::State &st, int what)
    {
        st.PauseTiming();
        prepare(what);
        st.ResumeTiming();
    }

    void prepare(int what)
    {
        if(what & kClearTree)
        {
            ryml_tree.clear();
        }
        if(what & kClearTreeArena)
        {
            ryml_tree.clear_arena();
        }
        if(what & kResetInPlace)
        {
            C4_ASSERT(in_place.size() == src.size());
            memcpy(in_place.data(), src.data(), src.size());
        }
    }

    void report(bm::State &st) const
    {
        // number of times the source was parsed
        st.SetItemsProcessed(st.iterations());
        // number of bytes parsed in the source
        st.SetBytesProcessed(st.iterations() * src.size());
    }

    bool skip_libyaml_if_needed(bm::State &st) const
    {
        if(filename.basename() == "trusty.yml")
        {
            st.SkipWithError("trusty.yml has anchors/references, which do not parse successfully in our parser wrapper for libyaml");
            return true;
        }
        return false;
    }

    bool skip_libfyaml_if_needed(bm::State &st) const
    {
        if(filename.basename() == "typeIDs.yaml")
        {
            st.SkipWithError("libfyaml fails to parse this file");
            return true;
        }
        return false;
    }
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


/** this is used by the benchmarks.
 *
 * @note We cannot declare the case as value-static as there is no guarantee
 * that the allocator's lifetime starts before and ends after the case's
 * lifetime. So use a pointer to control the lifetime. */
static BmCase * C4_RESTRICT s_bm_case = nullptr;


int main(int argc, char** argv)
{
    BmCase fixture;
    s_bm_case = &fixture;
    s_bm_case->run(argc, argv);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define ONLY_FOR_JSON \
    if(!s_bm_case->is_json) { st.SkipWithError("not a json file"); return; }


void bm_rapidjson_arena(bm::State& st)
{
    const char *src = s_bm_case->src.data();
    for(auto _ : st)
    {
        ONLY_FOR_JSON;
        rapidjson::Document doc;
        doc.Parse(src);
    }
    s_bm_case->report(st);
}

void bm_rapidjson_inplace(bm::State& st)
{
    char *src = s_bm_case->in_place.data();
    for(auto _ : st)
    {
        ONLY_FOR_JSON;
        s_bm_case->prepare(st, kResetInPlace);
        rapidjson::Document doc;
        doc.ParseInsitu(src);
    }
    s_bm_case->report(st);
}

void bm_sajson_arena(bm::State& st)
{
    sajson::string src = {s_bm_case->src.data(), s_bm_case->src.size()};
    for(auto _ : st)
    {
        ONLY_FOR_JSON;
        sajson::document document = sajson::parse(sajson::dynamic_allocation(), src);
    }
    s_bm_case->report(st);
}

void bm_sajson_inplace(bm::State& st)
{
    sajson::mutable_string_view src = {s_bm_case->in_place.size(), s_bm_case->in_place.data()};
    for(auto _ : st)
    {
        ONLY_FOR_JSON;
        s_bm_case->prepare(st, kResetInPlace);
        sajson::document document = sajson::parse(sajson::dynamic_allocation(), src);
    }
    s_bm_case->report(st);
}

void bm_jsoncpp_arena(bm::State& st)
{
    const char *b = &s_bm_case->src.front(), *e = &s_bm_case->src.back();
    for(auto _ : st)
    {
        ONLY_FOR_JSON;
        Json::Value root;
        Json::Reader reader;
        reader.parse(b, e, root);
    }
    s_bm_case->report(st);
}

void bm_nlohmann_arena(bm::State& st)
{
    const char* src = s_bm_case->src.data();
    for(auto _ : st)
    {
        ONLY_FOR_JSON;
        auto root = nlohmann::json::parse(src);
    }
    s_bm_case->report(st);
}

void bm_yamlcpp_arena(bm::State& st)
{
    const char* src = s_bm_case->src.data();
    for(auto _ : st)
    {
        YAML::Node node = YAML::Load(src);
    }
    s_bm_case->report(st);
}

void bm_libyaml_arena(bm::State& st)
{
    if(s_bm_case->skip_libyaml_if_needed(st))
        return;
    c4::csubstr src = c4::to_csubstr(s_bm_case->src.data());
    for(auto _ : st)
    {
        c4::yml::LibyamlParser p;
        c4::yml::Tree t;
        p.parse(&t, src);
    }
    s_bm_case->report(st);
}

void bm_libyaml_arena_reuse(bm::State& st)
{
    if(s_bm_case->skip_libyaml_if_needed(st))
        return;
    c4::csubstr src = c4::to_csubstr(s_bm_case->src.data());
    for(auto _ : st)
    {
        c4::yml::LibyamlParser libyaml_parser;
        s_bm_case->prepare(st, kClearTree|kClearTreeArena);
        libyaml_parser.parse(&s_bm_case->libyaml_tree, src);
    }
    s_bm_case->report(st);
}

void bm_libfyaml_arena(bm::State& st)
{
    if(s_bm_case->skip_libfyaml_if_needed(st))
        return;
    c4::csubstr src = c4::to_csubstr(s_bm_case->src.data());
    for(auto _ : st)
    {
        struct fy_document *fyd = fy_document_build_from_string(nullptr, src.str, src.len);
        fy_document_destroy(fyd);
    }
    s_bm_case->report(st);
}

void bm_ryml_arena(bm::State& st)
{
    c4::csubstr src = c4::to_csubstr(s_bm_case->src);
    for(auto _ : st)
    {
        ryml::Tree tree = ryml::parse_in_arena(s_bm_case->filename, src);
    }
    s_bm_case->report(st);
}

void bm_ryml_inplace(bm::State& st)
{
    c4::substr src = c4::to_substr(s_bm_case->in_place);
    for(auto _ : st)
    {
        s_bm_case->prepare(st, kResetInPlace);
        ryml::Tree tree = ryml::parse_in_place(s_bm_case->filename, src);
    }
    s_bm_case->report(st);
}

void bm_ryml_arena_reuse(bm::State& st)
{
    c4::csubstr src = c4::to_csubstr(s_bm_case->src);
    for(auto _ : st)
    {
        s_bm_case->prepare(st, kClearTree|kClearTreeArena);
        s_bm_case->ryml_parser.parse_in_arena(s_bm_case->filename, src, &s_bm_case->ryml_tree);
    }
    s_bm_case->report(st);
}

void bm_ryml_inplace_reuse(bm::State& st)
{
    c4::substr src = c4::to_substr(s_bm_case->in_place);
    for(auto _ : st)
    {
        s_bm_case->prepare(st, kResetInPlace|kClearTree|kClearTreeArena);
        s_bm_case->ryml_parser.parse_in_place(s_bm_case->filename, src, &s_bm_case->ryml_tree);
    }
    s_bm_case->report(st);
}

BENCHMARK(bm_ryml_arena);
BENCHMARK(bm_ryml_inplace);
BENCHMARK(bm_ryml_arena_reuse);
BENCHMARK(bm_ryml_inplace_reuse);
BENCHMARK(bm_libyaml_arena);
BENCHMARK(bm_libyaml_arena_reuse);
BENCHMARK(bm_yamlcpp_arena);
BENCHMARK(bm_libfyaml_arena);
BENCHMARK(bm_rapidjson_arena);
BENCHMARK(bm_rapidjson_inplace);
BENCHMARK(bm_sajson_arena);
BENCHMARK(bm_sajson_inplace);
BENCHMARK(bm_jsoncpp_arena);
BENCHMARK(bm_nlohmann_arena);

#if defined(_MSC_VER)
#   pragma warning(pop)
#elif defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif
