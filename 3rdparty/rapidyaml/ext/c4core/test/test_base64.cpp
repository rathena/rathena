#include "c4/test.hpp"
#ifndef C4CORE_SINGLE_HEADER
#include "c4/std/string.hpp"
#include "c4/std/vector.hpp"
#include "c4/format.hpp"
#include "c4/base64.hpp"
#endif

#include "c4/libtest/supprwarn_push.hpp"

#include <cstring>

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
// Since some the macros in c4/cpu.cpp cannot identify endanness at compile
// time, we use a simple runtime endianness-detection routine.
bool is_little_endian()
{
    unsigned long const v = 1UL;
    unsigned char b[sizeof(v)];
    std::memcpy(&b[0], &v, sizeof(v));
    return !!b[0];
}
} // namespace detail

csubstr native(csubstr little_endian, csubstr big_endian)
{
    return detail::is_little_endian() ? little_endian : big_endian;
}


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
        {   0, native("AAA=", "AAA=")},
        {   1, native("AQA=", "AAE=")},
        {   2, native("AgA=", "AAI=")},
        {   3, native("AwA=", "AAM=")},
        {   4, native("BAA=", "AAQ=")},
        {   5, native("BQA=", "AAU=")},
        {   6, native("BgA=", "AAY=")},
        {   7, native("BwA=", "AAc=")},
        {   8, native("CAA=", "AAg=")},
        {   9, native("CQA=", "AAk=")},
        {  10, native("CgA=", "AAo=")},
        {1234, native("0gQ=", "BNI=")},
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
        {   0, native("AAAAAA==", "AAAAAA==")},
        {   1, native("AQAAAA==", "AAAAAQ==")},
        {   2, native("AgAAAA==", "AAAAAg==")},
        {   3, native("AwAAAA==", "AAAAAw==")},
        {   4, native("BAAAAA==", "AAAABA==")},
        {   5, native("BQAAAA==", "AAAABQ==")},
        {   6, native("BgAAAA==", "AAAABg==")},
        {   7, native("BwAAAA==", "AAAABw==")},
        {   8, native("CAAAAA==", "AAAACA==")},
        {   9, native("CQAAAA==", "AAAACQ==")},
        {  10, native("CgAAAA==", "AAAACg==")},
        {1234, native("0gQAAA==", "AAAE0g==")},
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
        {   0, native("AAAAAAAAAAA=", "AAAAAAAAAAA=")},
        {   1, native("AQAAAAAAAAA=", "AAAAAAAAAAE=")},
        {   2, native("AgAAAAAAAAA=", "AAAAAAAAAAI=")},
        {   3, native("AwAAAAAAAAA=", "AAAAAAAAAAM=")},
        {   4, native("BAAAAAAAAAA=", "AAAAAAAAAAQ=")},
        {   5, native("BQAAAAAAAAA=", "AAAAAAAAAAU=")},
        {   6, native("BgAAAAAAAAA=", "AAAAAAAAAAY=")},
        {   7, native("BwAAAAAAAAA=", "AAAAAAAAAAc=")},
        {   8, native("CAAAAAAAAAA=", "AAAAAAAAAAg=")},
        {   9, native("CQAAAAAAAAA=", "AAAAAAAAAAk=")},
        {  10, native("CgAAAAAAAAA=", "AAAAAAAAAAo=")},
        {1234, native("0gQAAAAAAAA=", "AAAAAAAABNI=")},
	{0xdeadbeef, native("776t3gAAAAA=", "AAAAAN6tvu8=")},
    };
    for(auto p : pairs)
    {
        INFO("val=" << p.val << " expected=" << p.encoded);
        test_base64_roundtrip(p.val, p.encoded);
    }
}

TEST_CASE("base64.high_bits_u32")
{
    test_base64_roundtrip(UINT32_C(0xdeadbeef), native("776t3g==", "3q2+7w=="));
    test_base64_roundtrip(UINT32_MAX, native("/////w==", "/////w=="));
}

TEST_CASE("base64.high_bits_i32")
{
    test_base64_roundtrip(INT32_C(0x7fffffff), native("////fw==", "f////w=="));
    test_base64_roundtrip(INT32_MAX, native("////fw==", "f////w=="));
}

TEST_CASE("base64.high_bits_u64")
{
    test_base64_roundtrip(UINT64_MAX, native("//////////8=", "//////////8="));
}

TEST_CASE("base64.high_bits_i64")
{
    test_base64_roundtrip(INT64_MAX, native("/////////38=", "f/////////8="));
}

} // namespace c4

C4_SUPPRESS_WARNING_MSVC_POP

#include "c4/libtest/supprwarn_pop.hpp"
