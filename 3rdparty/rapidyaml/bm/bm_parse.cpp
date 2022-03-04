#include "./bm_common.hpp"


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
    s_bm_case->run("PARSE", argc, argv);
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

#ifdef RYML_HAVE_LIBFYAML
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
#endif

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

BENCHMARK(bm_ryml_inplace_reuse);
BENCHMARK(bm_ryml_arena_reuse);
BENCHMARK(bm_ryml_inplace);
BENCHMARK(bm_ryml_arena);
BENCHMARK(bm_libyaml_arena);
BENCHMARK(bm_libyaml_arena_reuse);
#ifdef RYML_HAVE_LIBFYAML
BENCHMARK(bm_libfyaml_arena);
#endif
BENCHMARK(bm_yamlcpp_arena);
BENCHMARK(bm_rapidjson_arena);
BENCHMARK(bm_rapidjson_inplace);
BENCHMARK(bm_sajson_arena);
BENCHMARK(bm_sajson_inplace);
BENCHMARK(bm_jsoncpp_arena);
BENCHMARK(bm_nlohmann_arena);
