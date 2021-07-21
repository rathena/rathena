#include "c4/test.hpp"
#include "c4/std/string.hpp"
#include "c4/std/vector.hpp"
#include "c4/format.hpp"
#include "c4/base64.hpp"

#include "c4/libtest/supprwarn_push.hpp"

C4_SUPPRESS_WARNING_MSVC_WITH_PUSH(4310)  // cast truncates constant value

namespace c4 {

namespace detail {
void base64_test_tables();
TEST_CASE("base64.infrastructure")
{
    #ifndef NDEBUG
    detail::base64_test_tables();
    #endif
}
} // namespace detail


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class T, class U>
void test_base64_str_roundtrip(T const& val, csubstr expected, U *ws)
{
    char buf_[512];
    substr buf(buf_);

    csubstr encoded = to_chars_sub(buf, fmt::base64(val));
    CHECK(base64_valid(encoded));
    CHECK_EQ(encoded, expected);
    CHECK_EQ(encoded.len % 4, 0);

    auto req = fmt::base64(*ws);
    size_t written = from_chars(encoded, &req);
    CHECK_EQ(ws->first(written), val);
}

template<class T>
void test_base64_roundtrip(T const& val, csubstr expected)
{
    char buf_[512];
    substr buf(buf_);

    csubstr encoded = to_chars_sub(buf, fmt::base64(val));
    CHECK(base64_valid(encoded));
    CHECK_EQ(encoded, expected);
    CHECK_EQ(encoded.len % 4, 0);

    T ws = {};
    auto req = fmt::base64(ws);
    size_t written = from_chars(encoded, &req);
    CHECK_EQ(written, sizeof(T));
    CHECK_EQ(ws, val);
}

template<class T>
struct base64_test_pair
{
    T val;
    csubstr encoded;
};

base64_test_pair<csubstr> base64_str_pairs[] = {
#define __(val, expected) {csubstr(val), csubstr(expected)}
    __(""                    , ""                            ),
    __("0"                   , "MA=="                        ),
    __("1"                   , "MQ=="                        ),
    __("2"                   , "Mg=="                        ),
    __("3"                   , "Mw=="                        ),
    __("4"                   , "NA=="                        ),
    __("5"                   , "NQ=="                        ),
    __("6"                   , "Ng=="                        ),
    __("7"                   , "Nw=="                        ),
    __("8"                   , "OA=="                        ),
    __("9"                   , "OQ=="                        ),
    __("10"                  , "MTA="                        ),
    __("123"                 , "MTIz"                        ),
    __("1234"                , "MTIzNA=="                    ),
    __("1235"                , "MTIzNQ=="                    ),
    __("Man"                 , "TWFu"                        ),
    __("Ma"                  , "TWE="                        ),
    __("M"                   , "TQ=="                        ),
    __("deadbeef"            , "ZGVhZGJlZWY="                ),
    __("any carnal pleasure.", "YW55IGNhcm5hbCBwbGVhc3VyZS4="),
    __("any carnal pleasure" , "YW55IGNhcm5hbCBwbGVhc3VyZQ=="),
    __("any carnal pleasur"  , "YW55IGNhcm5hbCBwbGVhc3Vy"    ),
    __("any carnal pleasu"   , "YW55IGNhcm5hbCBwbGVhc3U="    ),
    __("any carnal pleas"    , "YW55IGNhcm5hbCBwbGVhcw=="    ),
    __(           "pleasure.", "cGxlYXN1cmUu"                ),
    __(            "leasure.", "bGVhc3VyZS4="                ),
    __(             "easure.", "ZWFzdXJlLg=="                ),
    __(              "asure.", "YXN1cmUu"                    ),
    __(               "sure.", "c3VyZS4="                    ),
#undef __
};


TEST_CASE("base64.str")
{
    char buf_[512];
    substr buf(buf_);
    for(auto p : base64_str_pairs)
    {
        INFO(p.val);
        test_base64_str_roundtrip(p.val, p.encoded, &buf);
    }
}

TEST_CASE_TEMPLATE("base64.8bit", T, int8_t, uint8_t)
{
    base64_test_pair<T> pairs[] = {
        {(T)  0, csubstr("AA==")},
        {(T)  1, csubstr("AQ==")},
        {(T)  2, csubstr("Ag==")},
        {(T)  3, csubstr("Aw==")},
        {(T)  4, csubstr("BA==")},
        {(T)  5, csubstr("BQ==")},
        {(T)  6, csubstr("Bg==")},
        {(T)  7, csubstr("Bw==")},
        {(T)  8, csubstr("CA==")},
        {(T)  9, csubstr("CQ==")},
        {(T) 10, csubstr("Cg==")},
        {(T) 11, csubstr("Cw==")},
        {(T) 12, csubstr("DA==")},
        {(T) 13, csubstr("DQ==")},
        {(T) 14, csubstr("Dg==")},
        {(T) 15, csubstr("Dw==")},
        {(T) 16, csubstr("EA==")},
        {(T) 17, csubstr("EQ==")},
        {(T) 18, csubstr("Eg==")},
        {(T) 19, csubstr("Ew==")},
        {(T) 20, csubstr("FA==")},
        {(T)127, csubstr("fw==")},
        {(T)128, csubstr("gA==")},
        {(T)254, csubstr("/g==")},
        {(T)255, csubstr("/w==")},
    };
    for(auto p : pairs)
    {
        INFO("val=" << (int)p.val << " expected=" << p.encoded);
        test_base64_roundtrip(p.val, p.encoded);
    }
}

TEST_CASE_TEMPLATE("base64.16bit", T, int16_t, uint16_t)
{
    base64_test_pair<T> pairs[] = {
        {   0, csubstr("AAA=")},
        {   1, csubstr("AQA=")},
        {   2, csubstr("AgA=")},
        {   3, csubstr("AwA=")},
        {   4, csubstr("BAA=")},
        {   5, csubstr("BQA=")},
        {   6, csubstr("BgA=")},
        {   7, csubstr("BwA=")},
        {   8, csubstr("CAA=")},
        {   9, csubstr("CQA=")},
        {  10, csubstr("CgA=")},
        {1234, csubstr("0gQ=")},
    };
    for(auto p : pairs)
    {
        INFO("val=" << p.val << " expected=" << p.encoded);
        test_base64_roundtrip(p.val, p.encoded);
    }
}

TEST_CASE_TEMPLATE("base64.32bit", T, int32_t, uint32_t)
{
    base64_test_pair<T> pairs[] = {
        {   0, csubstr("AAAAAA==")},
        {   1, csubstr("AQAAAA==")},
        {   2, csubstr("AgAAAA==")},
        {   3, csubstr("AwAAAA==")},
        {   4, csubstr("BAAAAA==")},
        {   5, csubstr("BQAAAA==")},
        {   6, csubstr("BgAAAA==")},
        {   7, csubstr("BwAAAA==")},
        {   8, csubstr("CAAAAA==")},
        {   9, csubstr("CQAAAA==")},
        {  10, csubstr("CgAAAA==")},
        {1234, csubstr("0gQAAA==")},
    };
    for(auto p : pairs)
    {
        INFO("val=" << p.val << " expected=" << p.encoded);
        test_base64_roundtrip(p.val, p.encoded);
    }
}

TEST_CASE_TEMPLATE("base64.64bit", T, int64_t, uint64_t)
{
    base64_test_pair<T> pairs[] = {
        {   0, csubstr("AAAAAAAAAAA=")},
        {   1, csubstr("AQAAAAAAAAA=")},
        {   2, csubstr("AgAAAAAAAAA=")},
        {   3, csubstr("AwAAAAAAAAA=")},
        {   4, csubstr("BAAAAAAAAAA=")},
        {   5, csubstr("BQAAAAAAAAA=")},
        {   6, csubstr("BgAAAAAAAAA=")},
        {   7, csubstr("BwAAAAAAAAA=")},
        {   8, csubstr("CAAAAAAAAAA=")},
        {   9, csubstr("CQAAAAAAAAA=")},
        {  10, csubstr("CgAAAAAAAAA=")},
        {1234, csubstr("0gQAAAAAAAA=")},
        {0xdeadbeef, csubstr("776t3gAAAAA=")},
    };
    for(auto p : pairs)
    {
        INFO("val=" << p.val << " expected=" << p.encoded);
        test_base64_roundtrip(p.val, p.encoded);
    }
}

TEST_CASE("base64.high_bits_u32")
{
    test_base64_roundtrip(UINT32_C(0xdeadbeef), "776t3g==");
    test_base64_roundtrip(UINT32_MAX, "/////w==");
}

TEST_CASE("base64.high_bits_i32")
{
    test_base64_roundtrip(INT32_C(0x7fffffff), "////fw==");
    test_base64_roundtrip(INT32_MAX, "////fw==");
}

TEST_CASE("base64.high_bits_u64")
{
    test_base64_roundtrip(UINT64_MAX, "//////////8=");
}

TEST_CASE("base64.high_bits_i64")
{
    test_base64_roundtrip(INT64_MAX, "/////////38=");
}

} // namespace c4

C4_SUPPRESS_WARNING_MSVC_POP

#include "c4/libtest/supprwarn_pop.hpp"
