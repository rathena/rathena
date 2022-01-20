#include "c4/test.hpp"
#ifndef C4CORE_SINGLE_HEADER
#include "c4/std/vector_fwd.hpp"
#include "c4/std/vector.hpp"
#endif

namespace c4 {

template<size_t N>
std::vector<char> ctor(const char (&s)[N])
{
    return std::vector<char>(s, s+N-1);
}

TEST_CASE("std_vector.to_csubstr")
{
    std::vector<char> s = ctor("barnabe");
    csubstr ss = to_csubstr(s);
    CHECK_EQ(ss, csubstr("barnabe"));
    CHECK_EQ(ss.str, s.data());
    CHECK_EQ(ss.len, s.size());
}

TEST_CASE("std_vector.to_substr")
{
    std::vector<char> s = ctor("barnabe");
    substr ss = to_substr(s);
    CHECK_EQ(ss, csubstr("barnabe"));
    CHECK_EQ(ss.str, s.data());
    CHECK_EQ(ss.len, s.size());
    //
    CHECK_EQ(s[0], 'b');
    ss[0] = 'B';
    CHECK_EQ(s[0], 'B');
    ss[0] = 'A';
    CHECK_EQ(s[0], 'A');
}

TEST_CASE("std_vector.compare_csubstr")
{
    std::vector<char> s0 = ctor("000");
    std::vector<char> s1 = ctor("111");
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

TEST_CASE("std_vector.compare_substr")
{
    std::vector<char> s0 = ctor("000");
    std::vector<char> s1 = ctor("111");
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

TEST_CASE("std_vector.to_chars")
{
    const std::vector<char> s0 = ctor("000");
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

TEST_CASE("std_vector.from_chars")
{
    std::vector<char> s0;
    csubstr buf = "0123456798";
    CHECK_NE(buf.data(), s0.data());
    bool ok = from_chars(buf, &s0);
    CHECK(ok);
    CHECK_EQ(buf, s0);
}

} // namespace c4
