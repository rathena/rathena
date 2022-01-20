#include "c4/test.hpp"
#ifndef C4CORE_SINGLE_HEADER
#include "c4/std/string_fwd.hpp"
#include "c4/std/string.hpp"
#endif

namespace c4 {

TEST_CASE("std_string.to_substr")
{
    std::string s("barnabe");
    substr ss = to_substr(s);
    CHECK_EQ(ss.str, s.data());
    CHECK_EQ(ss.len, s.size());
    s[0] = 'B';
    CHECK_EQ(ss[0], 'B');
    ss[0] = 'b';
    CHECK_EQ(s[0], 'b');
}

TEST_CASE("std_string.to_csubstr")
{
    std::string s("barnabe");
    csubstr ss = to_csubstr(s);
    CHECK_EQ(ss.str, s.data());
    CHECK_EQ(ss.len, s.size());
    s[0] = 'B';
    CHECK_EQ(ss[0], 'B');
}

TEST_CASE("std_string.compare_csubstr")
{
    std::string s0 = "000";
    std::string s1 = "111";
    csubstr ss0 = "000";
    csubstr ss1 = "111";
    CHECK_NE(s0.data(), ss0.data());
    CHECK_NE(s1.data(), ss1.data());
    //
    CHECK_EQ(s0, ss0);
    CHECK_EQ(s1, ss1);
    CHECK_EQ(ss0, s0);
    CHECK_EQ(ss1, s1);
    //
    CHECK_NE(s1, ss0);
    CHECK_NE(s0, ss1);
    CHECK_NE(ss1, s0);
    CHECK_NE(ss0, s1);
    //
    CHECK_GE(s0, ss0);
    CHECK_LE(s1, ss1);
    CHECK_GE(ss0, s0);
    CHECK_LE(ss1, s1);
    CHECK_GE(s1, ss0);
    CHECK_LE(s0, ss1);
    CHECK_GE(ss1, s0);
    CHECK_LE(ss0, s1);
    //
    CHECK_GT(s1, ss0);
    CHECK_LT(s0, ss1);
    CHECK_GT(ss1, s0);
    CHECK_LT(ss0, s1);
}

TEST_CASE("std_string.compare_substr")
{
    std::string s0 = "000";
    std::string s1 = "111";
    char buf0[] = "000";
    char buf1[] = "111";
    substr ss0 = buf0;
    substr ss1 = buf1;
    CHECK_NE(s0.data(), ss0.data());
    CHECK_NE(s1.data(), ss1.data());
    //
    CHECK_EQ(s0, ss0);
    CHECK_EQ(s1, ss1);
    CHECK_EQ(ss0, s0);
    CHECK_EQ(ss1, s1);
    //
    CHECK_NE(s1, ss0);
    CHECK_NE(s0, ss1);
    CHECK_NE(ss1, s0);
    CHECK_NE(ss0, s1);
    //
    CHECK_GE(s0, ss0);
    CHECK_LE(s1, ss1);
    CHECK_GE(ss0, s0);
    CHECK_LE(ss1, s1);
    CHECK_GE(s1, ss0);
    CHECK_LE(s0, ss1);
    CHECK_GE(ss1, s0);
    CHECK_LE(ss0, s1);
    //
    CHECK_GT(s1, ss0);
    CHECK_LT(s0, ss1);
    CHECK_GT(ss1, s0);
    CHECK_LT(ss0, s1);
}

TEST_CASE("std_string.to_chars")
{
    const std::string s0 = "000";
    char buf_[100] = {};
    substr buf = buf_;
    CHECK_NE(buf.data(), s0.data());
    size_t ret = to_chars({}, s0);
    CHECK_EQ(ret, s0.size());
    CHECK_NE(buf.first(ret), s0);
    ret = to_chars(buf, s0);
    CHECK_EQ(ret, s0.size());
    CHECK_EQ(buf.first(ret), s0);
}

TEST_CASE("std_string.from_chars")
{
    std::string s0;
    csubstr buf = "0123456798";
    CHECK_NE(buf.data(), s0.data());
    bool ok = from_chars(buf, &s0);
    CHECK(ok);
    CHECK_EQ(buf, s0);
}

} // namespace c4
