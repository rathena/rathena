#ifndef C4CORE_SINGLE_HEADER
#include "c4/substr.hpp"
#include "c4/std/std.hpp"
#include "c4/dump.hpp"
#include "c4/format.hpp"
#endif

#include <c4/test.hpp>
#include "c4/libtest/supprwarn_push.hpp"

#ifdef __clang__
#   pragma clang diagnostic push
#elif defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

namespace c4 {


namespace example {

std::string test_dumper_target = {};
void test_dumper(csubstr str)
{
    test_dumper_target.append(str.str, str.len);
}

template<class ...Args>
void printf(csubstr fmt, Args&& ...args)
{
    static thread_local std::string writebuf(16, '\0');
    DumpResults results = format_dump_resume<&test_dumper>(c4::to_substr(writebuf), fmt, std::forward<Args>(args)...);
    if(C4_UNLIKELY(results.bufsize > writebuf.size())) // bufsize will be that of the largest element serialized. Eg int(1), will require 1 byte.
    {
        size_t dup = 2 * writebuf.size();
        writebuf.resize(dup > results.bufsize ? dup : results.bufsize);
        format_dump_resume<&test_dumper>(results, c4::to_substr(writebuf), fmt, std::forward<Args>(args)...);
    }
}
} // namespace example

TEST_CASE("printf_example")
{
    example::test_dumper_target.clear();
    SUBCASE("1")
    {
        example::printf("{} coffees per day.\n", 3);
        CHECK_EQ(example::test_dumper_target, "3 coffees per day.\n");
    }
    SUBCASE("2")
    {
        example::printf("{} would be {}.", "brecky", "nice");
        CHECK_EQ(example::test_dumper_target, "brecky would be nice.");
    }
    SUBCASE("resize writebuf")
    {
        // printed strings will not use the writebuf, so we write a zero-padded integer
        size_t dim = 128;  // pad with 128 zeroes
        std::string s1(dim, '0');
        std::string s2(dim, '0');
        s1.back() = '1';
        s2.back() = '2';
        example::printf("{} cannot be {}", fmt::zpad(1, dim), fmt::zpad(2, dim));
        CHECK_EQ(example::test_dumper_target, s1 + " cannot be " + s2);
    }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TEST_CASE("DumpResults")
{
    DumpResults dr = {};
    CHECK_EQ(dr.bufsize, 0u);
    CHECK_EQ(dr.lastok, DumpResults::noarg);
    CHECK_UNARY(dr.write_arg(0));
    CHECK_FALSE(dr.success_until(0));
    CHECK_EQ(dr.argfail(), 0);
}

struct DumpChecker
{
    static size_t s_num_calls;
    static char s_workspace[100];
    static size_t s_accum_pos;
    static char s_accum[100];
    static void s_reset()
    {
        s_num_calls = 0;
        s_accum_pos = 0;
        for(size_t i = 0; i < sizeof(s_workspace); ++i)
            s_workspace[i] = '+';
        for(size_t i = 0; i < sizeof(s_accum); ++i)
            s_accum[i] = '.';
    }
    static void s_dump(csubstr buf)
    {
        REQUIRE_LT(buf.len, sizeof(s_workspace));
        REQUIRE_LT(s_accum_pos + buf.len, sizeof(s_accum));
        ++s_num_calls;
        memcpy(s_accum + s_accum_pos, buf.str, buf.len);
        s_accum_pos += buf.len;
    }
};
size_t DumpChecker::s_num_calls = 0;
char DumpChecker::s_workspace[100] = {};
size_t DumpChecker::s_accum_pos = {};
char DumpChecker::s_accum[100] = {};

struct CatDumpTplArg
{
    template<class ...Args>
    static size_t call_cat_dump(Args&& ...args)
    {
        return cat_dump<&DumpChecker::s_dump>(std::forward<Args>(args)...);
    }
    template<class ...Args>
    static DumpResults call_cat_dump_resume(Args&& ...args)
    {
        return cat_dump_resume<&DumpChecker::s_dump>(std::forward<Args>(args)...);
    }
    template<class ...Args>
    static size_t call_catsep_dump(Args&& ...args)
    {
        return catsep_dump<&DumpChecker::s_dump>(std::forward<Args>(args)...);
    }
    template<class ...Args>
    static DumpResults call_catsep_dump_resume(Args&& ...args)
    {
        return catsep_dump_resume<&DumpChecker::s_dump>(std::forward<Args>(args)...);
    }
    template<class ...Args>
    static size_t call_format_dump(Args&& ...args)
    {
        return format_dump<&DumpChecker::s_dump>(std::forward<Args>(args)...);
    }
    template<class ...Args>
    static DumpResults call_format_dump_resume(Args&& ...args)
    {
        return format_dump_resume<&DumpChecker::s_dump>(std::forward<Args>(args)...);
    }
};

struct CatDumpFnArg
{
    template<class ...Args>
    static size_t call_cat_dump(Args&& ...args)
    {
        return cat_dump(&DumpChecker::s_dump, std::forward<Args>(args)...);
    }
    template<class ...Args>
    static DumpResults call_cat_dump_resume(Args&& ...args)
    {
        return cat_dump_resume(&DumpChecker::s_dump, std::forward<Args>(args)...);
    }
    template<class ...Args>
    static size_t call_catsep_dump(Args&& ...args)
    {
        return catsep_dump(&DumpChecker::s_dump, std::forward<Args>(args)...);
    }
    template<class ...Args>
    static DumpResults call_catsep_dump_resume(Args&& ...args)
    {
        return catsep_dump_resume(&DumpChecker::s_dump, std::forward<Args>(args)...);
    }
    template<class ...Args>
    static size_t call_format_dump(Args&& ...args)
    {
        return format_dump(&DumpChecker::s_dump, std::forward<Args>(args)...);
    }
    template<class ...Args>
    static DumpResults call_format_dump_resume(Args&& ...args)
    {
        return format_dump_resume(&DumpChecker::s_dump, std::forward<Args>(args)...);
    }
};

namespace buffers {
int b1 = 1;
int b2 = 22;
int b3 = 333;
int b4 = 4444;
int sep = 90009;
size_t seplen = 5;
}

TEST_CASE_TEMPLATE("cat_dump", T, CatDumpTplArg, CatDumpFnArg)
{
    using namespace buffers;
    substr buf = DumpChecker::s_workspace;
    auto accum = [&]{ return csubstr(DumpChecker::s_accum).first(DumpChecker::s_accum_pos); };
    SUBCASE("dont use the buffer with strings")
    {
        DumpChecker::s_reset();
        size_t needed_size = T::call_cat_dump(buf.first(0), b1);
        CHECK_EQ(needed_size, 1);
        CHECK_EQ(accum(), csubstr(""));
        needed_size = T::call_cat_dump(buf.first(0), b1, b2);
        CHECK_EQ(needed_size, 2);
        CHECK_EQ(accum(), csubstr(""));
        needed_size = T::call_cat_dump(buf.first(0), b1, b2, b3);
        CHECK_EQ(needed_size, 3);
        CHECK_EQ(accum(), csubstr(""));
        needed_size = T::call_cat_dump(buf.first(0), b1, b2, b3, b4);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(0), ("1"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr(""));
        needed_size = T::call_cat_dump(buf.first(0), ("1"), ("22"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr(""));
        needed_size = T::call_cat_dump(buf.first(0), ("1"), ("22"), ("333"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr(""));
        needed_size = T::call_cat_dump(buf.first(0), ("1"), ("22"), ("333"), ("4444"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(0), csubstr("1"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr(""));
        needed_size = T::call_cat_dump(buf.first(0), csubstr("1"), csubstr("22"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr(""));
        needed_size = T::call_cat_dump(buf.first(0), csubstr("1"), csubstr("22"), csubstr("333"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr(""));
        needed_size = T::call_cat_dump(buf.first(0), csubstr("1"), csubstr("22"), csubstr("333"), csubstr("4444"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(1), b1);
        CHECK_EQ(needed_size, 1);
        CHECK_EQ(accum(), csubstr("1"));
        needed_size = T::call_cat_dump(buf.first(1), b1, b2);
        CHECK_EQ(needed_size, 2);
        CHECK_EQ(accum(), csubstr("11"));
        needed_size = T::call_cat_dump(buf.first(1), b1, b2, b3);
        CHECK_EQ(needed_size, 3);
        CHECK_EQ(accum(), csubstr("111"));
        needed_size = T::call_cat_dump(buf.first(1), b1, b2, b3, b4);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(accum(), csubstr("1111"));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(1), ("1"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr("1"));
        needed_size = T::call_cat_dump(buf.first(1), ("1"), ("22"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr("1122"));
        needed_size = T::call_cat_dump(buf.first(1), ("1"), ("22"), ("333"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr("1122122333"));
        needed_size = T::call_cat_dump(buf.first(1), ("1"), ("22"), ("333"), ("4444"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr("11221223331223334444"));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(1), csubstr("1"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr("1"));
        needed_size = T::call_cat_dump(buf.first(1), csubstr("1"), csubstr("22"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr("1122"));
        needed_size = T::call_cat_dump(buf.first(1), csubstr("1"), csubstr("22"), csubstr("333"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr("1122122333"));
        needed_size = T::call_cat_dump(buf.first(1), csubstr("1"), csubstr("22"), csubstr("333"), csubstr("4444"));
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(accum(), csubstr("11221223331223334444"));
    }
    SUBCASE("1")
    {
        DumpChecker::s_reset();
        size_t needed_size = T::call_cat_dump(buf.first(0), b1);
        CHECK_EQ(needed_size, 1u);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(1), csubstr("+")); // nothing was written
        CHECK_EQ(accum(), csubstr("")); // nothing was written
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf, b1);
        CHECK_EQ(needed_size, 1u);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(1), csubstr("1"));
        CHECK_EQ(accum(), csubstr("1"));
    }
    SUBCASE("1 2")
    {
        DumpChecker::s_reset();
        size_t needed_size = T::call_cat_dump(buf.first(0), b1, b2);
        CHECK_EQ(needed_size, 2);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(2), csubstr("++")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(1), b1, b2);
        CHECK_EQ(needed_size, 2);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(2), csubstr("2+")); // only the first character of b2 was written
        CHECK_EQ(accum(), csubstr("1"));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf, b1, b2);
        CHECK_EQ(needed_size, 2);
        CHECK_EQ(DumpChecker::s_num_calls, 2);
        CHECK_EQ(buf.first(2), csubstr("22"));
        CHECK_EQ(accum(), csubstr("122"));
    }
    SUBCASE("2 1")
    {
        DumpChecker::s_reset();
        size_t needed_size = T::call_cat_dump(buf, b2, b1);
        CHECK_EQ(needed_size, 2);
        CHECK_EQ(DumpChecker::s_num_calls, 2);
        CHECK_EQ(buf.first(2), csubstr("12")); // wrote 2 then 1
        CHECK_EQ(accum(), csubstr("221"));
    }
    SUBCASE("1 2 3 4")
    {
        DumpChecker::s_reset();
        size_t needed_size = T::call_cat_dump(buf.first(0), b1, b2, b3, b4);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("++++"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(1), b1, b2, b3, b4);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(4), csubstr("2+++"));
        CHECK_EQ(accum(), csubstr("1"));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(2), b1, b2, b3, b4);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(DumpChecker::s_num_calls, 2);
        CHECK_EQ(buf.first(4), csubstr("33++"));
        CHECK_EQ(accum(), csubstr("122"));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(3), b1, b2, b3, b4);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(4), csubstr("444+"));
        CHECK_EQ(accum(), csubstr("122333"));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf, b1, b2, b3, b4);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(DumpChecker::s_num_calls, 4);
        CHECK_EQ(buf.first(4), csubstr("4444"));
        CHECK_EQ(accum(), csubstr("1223334444"));
    }
    SUBCASE("4 3 2 1")
    {
        DumpChecker::s_reset();
        size_t needed_size = T::call_cat_dump(buf.first(0), b4, b3, b2, b1);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("++++"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(1), b4, b3, b2, b1);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("4+++"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(2), b4, b3, b2, b1);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("44++"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf.first(3), b4, b3, b2, b1);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("444+"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_cat_dump(buf, b4, b3, b2, b1);
        CHECK_EQ(needed_size, 4);
        CHECK_EQ(DumpChecker::s_num_calls, 4);
        CHECK_EQ(buf.first(4), csubstr("1234"));
        CHECK_EQ(accum(), csubstr("4444333221"));
    }
}

TEST_CASE_TEMPLATE("cat_dump_resume", T, CatDumpTplArg, CatDumpFnArg)
{
    using namespace buffers;
    substr buf = DumpChecker::s_workspace;
    auto accum = [&]{ return csubstr(DumpChecker::s_accum).first(DumpChecker::s_accum_pos); };
    SUBCASE("1")
    {
        DumpChecker::s_reset();
        DumpResults ret = T::call_cat_dump_resume(buf.first(0), b1);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_EQ(ret.bufsize, 1);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(1), csubstr("+")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        DumpResults retry = T::call_cat_dump_resume(ret, buf.first(1), b1);
        CHECK_UNARY(retry.success_until(0));
        CHECK_UNARY(!retry.success_until(1));
        CHECK_UNARY(!retry.success_until(2));
        CHECK_EQ(retry.bufsize, 1);
        CHECK_EQ(retry.lastok, 0);
        CHECK_EQ(retry.argfail(), 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(1), csubstr("1"));
        CHECK_EQ(accum(), csubstr("1"));
    }
    SUBCASE("1 2")
    {
        DumpChecker::s_reset();
        DumpResults ret = T::call_cat_dump_resume(buf.first(0), b1, b2);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_EQ(ret.bufsize, 2); // finds the buf size at once
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(2), csubstr("++")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_cat_dump_resume(ret, buf.first(1), b1, b2);
        CHECK_UNARY(!ret.success_until(0)); // ret.bufsize signals buffer is at least 2
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_EQ(ret.bufsize, 2);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(2), csubstr("++"));
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_cat_dump_resume(ret, buf.first(2), b1, b2);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_EQ(ret.bufsize, 2);
        CHECK_EQ(ret.lastok, 1);
        CHECK_EQ(ret.argfail(), 2);
        CHECK_EQ(DumpChecker::s_num_calls, 2);
        CHECK_EQ(buf.first(2), csubstr("22"));
        CHECK_EQ(accum(), csubstr("122"));
    }
    SUBCASE("1 2 3 4")
    {
        DumpChecker::s_reset();
        DumpResults ret = T::call_cat_dump_resume(buf.first(0), b1, b2, b3, b4);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_UNARY(!ret.success_until(3));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(4), csubstr("++++")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_cat_dump_resume(ret, buf.first(1), b1, b2, b3, b4);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_UNARY(!ret.success_until(3));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("++++"));
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_cat_dump_resume(ret, buf.first(2), b1, b2, b3, b4);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_UNARY(!ret.success_until(3));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("++++"));
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_cat_dump_resume(ret, buf.first(3), b1, b2, b3, b4);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_UNARY(!ret.success_until(3));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("++++"));
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_cat_dump_resume(ret, buf.first(4), b1, b2, b3, b4);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(ret.success_until(2));
        CHECK_UNARY(ret.success_until(3));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, 3);
        CHECK_EQ(ret.argfail(), 4);
        CHECK_EQ(DumpChecker::s_num_calls, 4);
        CHECK_EQ(buf.first(4), csubstr("4444"));
        CHECK_EQ(accum(), csubstr("1223334444"));
    }
    SUBCASE("4 3 2 1")
    {
        DumpChecker::s_reset();
        DumpResults ret = T::call_cat_dump_resume(buf.first(0), b4, b3, b2, b1);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_UNARY(!ret.success_until(3));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(4), csubstr("++++")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_cat_dump_resume(ret, buf.first(1), b4, b3, b2, b1);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_UNARY(!ret.success_until(3));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("++++"));
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_cat_dump_resume(ret, buf.first(2), b4, b3, b2, b1);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_UNARY(!ret.success_until(3));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("++++"));
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_cat_dump_resume(ret, buf.first(3), b4, b3, b2, b1);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_UNARY(!ret.success_until(3));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("++++"));
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_cat_dump_resume(ret, buf.first(4), b4, b3, b2, b1);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(ret.success_until(2));
        CHECK_UNARY(ret.success_until(3));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, 3);
        CHECK_EQ(ret.argfail(), 4);
        CHECK_EQ(DumpChecker::s_num_calls, 4);
        CHECK_EQ(buf.first(4), csubstr("1234"));
        CHECK_EQ(accum(), csubstr("4444333221"));
    }
}



TEST_CASE_TEMPLATE("catsep_dump", T, CatDumpTplArg, CatDumpFnArg)
{
    using namespace buffers;
    size_t needed_size;
    substr buf = DumpChecker::s_workspace;
    auto accum = [&]{ return csubstr(DumpChecker::s_accum).first(DumpChecker::s_accum_pos); };
    SUBCASE("1")
    {
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(0), sep, b1);
        CHECK_EQ(needed_size, 1);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(1), csubstr("+")); // nothing was written
        CHECK_EQ(accum(), csubstr("")); // nothing was written
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(1), sep, b1);
        CHECK_EQ(needed_size, 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(1), csubstr("1"));
        CHECK_EQ(accum(), csubstr("1"));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(1 + seplen), sep, b1);
        CHECK_EQ(needed_size, 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1); // sep was not written
        CHECK_EQ(buf.first(1), csubstr("1"));
        CHECK_EQ(accum(), csubstr("1"));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf, sep, b1);
        CHECK_EQ(needed_size, 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1); // sep was not written
        CHECK_EQ(buf.first(1), csubstr("1"));
        CHECK_EQ(accum(), csubstr("1"));
    }
    SUBCASE("1 2")
    {
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(0), sep, b1, b2);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(2), csubstr("++")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(1), sep, b1, b2);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(2), csubstr("9+"));
        CHECK_EQ(accum(), csubstr("1"));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(seplen), sep, b1, b2);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("22"));
        CHECK_EQ(accum(), csubstr("19000922"));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf, sep, b1, b2);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("22"));
        CHECK_EQ(accum(), csubstr("19000922"));
    }
    SUBCASE("2 1")
    {
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(0), sep, b2, b1);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(2), csubstr("++")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(1), sep, b2, b1);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(2), csubstr("2+"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(seplen), sep, b2, b1);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("10"));
        CHECK_EQ(accum(), csubstr("22900091"));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf, sep, b2, b1);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("10"));
        CHECK_EQ(accum(), csubstr("22900091"));
    }
    SUBCASE("1 2 3 4")
    {
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(0), sep, b1, b2, b3, b4);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("++++"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(1), sep, b1, b2, b3, b4);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(4), csubstr("9+++"));
        CHECK_EQ(accum(), csubstr("1"));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(2), sep, b1, b2, b3, b4);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(4), csubstr("09++"));
        CHECK_EQ(accum(), csubstr("1"));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(3), sep, b1, b2, b3, b4);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(4), csubstr("009+"));
        CHECK_EQ(accum(), csubstr("1"));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf, sep, b1, b2, b3, b4);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 7);
        CHECK_EQ(buf.first(4), csubstr("4444"));
        CHECK_EQ(accum(), csubstr("1900092290009333900094444"));
    }
    SUBCASE("4 3 2 1")
    {
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(0), sep, b4, b3, b2, b1);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("++++"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(1), sep, b4, b3, b2, b1);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("4+++"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(2), sep, b4, b3, b2, b1);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("44++"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf.first(3), sep, b4, b3, b2, b1);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("444+"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        needed_size = T::call_catsep_dump(buf, sep, b4, b3, b2, b1);
        CHECK_EQ(needed_size, seplen);
        CHECK_EQ(DumpChecker::s_num_calls, 7);
        CHECK_EQ(buf.first(4), csubstr("1000"));
        CHECK_EQ(accum(), csubstr("4444900093339000922900091"));
    }
}


TEST_CASE_TEMPLATE("catsep_dump_resume", T, CatDumpTplArg, CatDumpFnArg)
{
    using namespace buffers;
    substr buf = DumpChecker::s_workspace;
    auto accum = [&]{ return csubstr(DumpChecker::s_accum).first(DumpChecker::s_accum_pos); };
    SUBCASE("1")
    {
        DumpChecker::s_reset();
        DumpResults ret = T::call_catsep_dump_resume(buf.first(0), sep, b1);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_EQ(ret.bufsize, 1);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(1), csubstr("+")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_catsep_dump_resume(ret, buf.first(1), sep, b1);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_EQ(ret.bufsize, 1);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(1), csubstr("1"));
        CHECK_EQ(accum(), csubstr("1"));
        ret = T::call_catsep_dump_resume(ret, buf, sep, b1);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_EQ(ret.bufsize, 1);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(1), csubstr("1"));
        CHECK_EQ(accum(), csubstr("1"));
    }
    SUBCASE("1 2")
    {
        DumpChecker::s_reset();
        DumpResults ret = T::call_catsep_dump_resume(buf.first(0), sep, b1, b2);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_EQ(ret.bufsize, seplen); // finds the buf size at once
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(2), csubstr("++")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_catsep_dump_resume(ret, buf.first(1), sep, b1, b2);
        CHECK_UNARY(ret.success_until(0)); // b1
        CHECK_UNARY(!ret.success_until(1)); // sep
        CHECK_UNARY(!ret.success_until(2)); // b2
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1); // sep
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(2), csubstr("9+"));
        CHECK_EQ(accum(), csubstr("1"));
        ret = T::call_catsep_dump_resume(ret, buf.first(2), sep, b1, b2);
        CHECK_UNARY(ret.success_until(0));  // b1
        CHECK_UNARY(!ret.success_until(1)); // sep
        CHECK_UNARY(!ret.success_until(2)); // b2
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1); // sep
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(2), csubstr("09"));
        CHECK_EQ(accum(), csubstr("1"));
        ret = T::call_catsep_dump_resume(ret, buf.first(seplen), sep, b1, b2);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(ret.success_until(2));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, 2);
        CHECK_EQ(ret.argfail(), 3);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("22"));
        CHECK_EQ(accum(), csubstr("19000922"));
    }
    SUBCASE("1 2 3 4")
    {
        DumpChecker::s_reset();
        DumpResults ret = T::call_catsep_dump_resume(buf.first(0), sep, b1, b2, b3, b4);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(4), csubstr("++++")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_catsep_dump_resume(ret, buf.first(1), sep, b1, b2, b3, b4);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(4), csubstr("9+++")); // failed while writing sep
        CHECK_EQ(accum(), csubstr("1"));
        ret = T::call_catsep_dump_resume(ret, buf.first(2), sep, b1, b2, b3, b4);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(4), csubstr("09++")); // failed while writing sep
        CHECK_EQ(accum(), csubstr("1"));
        ret = T::call_catsep_dump_resume(ret, buf.first(3), sep, b1, b2, b3, b4);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(4), csubstr("009+"));
        CHECK_EQ(accum(), csubstr("1"));
        ret = T::call_catsep_dump_resume(ret, buf.first(4), sep, b1, b2, b3, b4);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(4), csubstr("0009"));
        CHECK_EQ(accum(), csubstr("1"));
        ret = T::call_catsep_dump_resume(ret, buf.first(seplen), sep, b1, b2, b3, b4);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(ret.success_until(2));
        CHECK_UNARY(ret.success_until(3));
        CHECK_UNARY(ret.success_until(4));
        CHECK_UNARY(ret.success_until(5));
        CHECK_UNARY(ret.success_until(6));
        CHECK_UNARY(!ret.success_until(7));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, 6);
        CHECK_EQ(ret.argfail(), 7);
        CHECK_EQ(DumpChecker::s_num_calls, 7);
        CHECK_EQ(buf.first(seplen), csubstr("44449"));
        CHECK_EQ(accum(), csubstr("1900092290009333900094444"));
    }
    SUBCASE("4 3 2 1")
    {
        DumpChecker::s_reset();
        DumpResults ret = T::call_catsep_dump_resume(buf.first(0), sep, b4, b3, b2, b1);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(4), csubstr("++++")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_catsep_dump_resume(ret, buf.first(1), sep, b4, b3, b2, b1);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("4+++"));
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_catsep_dump_resume(ret, buf.first(2), sep, b4, b3, b2, b1);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(4), csubstr("44++"));
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_catsep_dump_resume(ret, buf.first(seplen), sep, b4, b3, b2, b1);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(ret.success_until(2));
        CHECK_UNARY(ret.success_until(3));
        CHECK_UNARY(ret.success_until(4));
        CHECK_UNARY(ret.success_until(5));
        CHECK_UNARY(ret.success_until(6));
        CHECK_UNARY(!ret.success_until(7));
        CHECK_EQ(ret.bufsize, seplen);
        CHECK_EQ(ret.lastok, 6);
        CHECK_EQ(ret.argfail(), 7);
        CHECK_EQ(DumpChecker::s_num_calls, 7);
        CHECK_EQ(buf.first(seplen), csubstr("10009"));
        CHECK_EQ(accum(), csubstr("4444900093339000922900091"));
    }
    SUBCASE("1 2 3 4 with seplen==3")
    {
        int s = 999;
        DumpChecker::s_reset();
        DumpResults ret = T::call_catsep_dump_resume(buf.first(0), s, b1, b2, b3, b4);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(4), csubstr("++++")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_catsep_dump_resume(ret, buf.first(1), s, b1, b2, b3, b4);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(4), csubstr("9+++")); // failed while writing sep
        CHECK_EQ(accum(), csubstr("1"));
        ret = T::call_catsep_dump_resume(ret, buf.first(2), s, b1, b2, b3, b4);
        CHECK_UNARY(ret.success_until(0)); // b1
        CHECK_UNARY(!ret.success_until(1)); // s
        CHECK_UNARY(!ret.success_until(2)); // b2
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(4), csubstr("99++")); // failed while writing sep
        CHECK_EQ(accum(), csubstr("1"));
        ret = T::call_catsep_dump_resume(ret, buf.first(3), s, b1, b2, b3, b4);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(ret.success_until(2));
        CHECK_UNARY(ret.success_until(3));
        CHECK_UNARY(ret.success_until(4));
        CHECK_UNARY(ret.success_until(5));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, 5);
        CHECK_EQ(ret.argfail(), 6);
        CHECK_EQ(DumpChecker::s_num_calls, 6);
        CHECK_EQ(buf.first(4), csubstr("444+")); // failed while writing b4
        CHECK_EQ(accum(), csubstr("199922999333999"));
        ret = T::call_catsep_dump_resume(ret, buf.first(4), s, b1, b2, b3, b4);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(ret.success_until(2));
        CHECK_UNARY(ret.success_until(3));
        CHECK_UNARY(ret.success_until(4));
        CHECK_UNARY(ret.success_until(5));
        CHECK_UNARY(ret.success_until(6));
        CHECK_UNARY(!ret.success_until(7));
        CHECK_EQ(ret.bufsize, 4);
        CHECK_EQ(ret.lastok, 6);
        CHECK_EQ(ret.argfail(), 7);
        CHECK_EQ(DumpChecker::s_num_calls, 7);
        CHECK_EQ(buf.first(5), csubstr("4444+"));
        CHECK_EQ(accum(), csubstr("1999229993339994444"));
    }
}


TEST_CASE_TEMPLATE("format_dump", T, CatDumpTplArg, CatDumpFnArg)
{
    using namespace buffers;
    size_t needed_size;
    substr buf = DumpChecker::s_workspace;
    auto accum = [&]{ return csubstr(DumpChecker::s_accum).first(DumpChecker::s_accum_pos); };
    SUBCASE("no buffer is needed for strings")
    {
        csubstr fmt = "{}-{}-{}-{}";
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(0), fmt, "1", "22", "333", "4444"); // no buffer!
        CHECK_EQ(needed_size, 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(1), csubstr("+"));
        CHECK_EQ(accum(), csubstr(""));
        DumpChecker::s_reset();
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        needed_size = T::call_format_dump(buf.first(1), fmt, "1", "22", "333", "4444"); // 1-len buffer, unused
        CHECK_EQ(needed_size, 0); // no intermediate serialization is needed, since these are strings
        CHECK_EQ(DumpChecker::s_num_calls, 7); // calls everything even when the buffer is empty
        CHECK_EQ(accum(), csubstr("1-22-333-4444")); // dumped the full format string
    }
    SUBCASE("0")
    {
        csubstr fmt = "01234567890123456789";
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(0), fmt, b1, b2, b3, b4);
        CHECK_EQ(needed_size, 0); // the longest sized argument format argument
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls when the buffer is empty
        CHECK_EQ(buf.first(needed_size), csubstr("")); // nothing was written
        CHECK_EQ(accum(), csubstr("")); // dumped the full format string
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf, fmt, b1, b2, b3, b4);
        CHECK_EQ(needed_size, 0); // the longest sized argument format argument
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(needed_size), csubstr("")); // nothing was written
        CHECK_EQ(accum(), fmt); // dumped the full format string
    }
    SUBCASE("1")
    {
        //             ____1____ 2  __3__
        csubstr fmt = "012345678_{}_34567";
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(0), fmt, b1);
        CHECK_EQ(needed_size, 1); // the longest sized argument format argument
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(2), csubstr("++")); // nothing was written
        CHECK_EQ(accum(), csubstr("")); // dumped first part of the format string
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(1), fmt, b1);
        CHECK_EQ(needed_size, 1);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("1+"));
        CHECK_EQ(accum(), csubstr("012345678_1_34567"));
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(1), fmt, b1, b2, b3, b4); // check that extra arguments are ignored
        CHECK_EQ(needed_size, 1);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("1+"));
        CHECK_EQ(accum(), csubstr("012345678_1_34567"));
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(1), fmt); // check that missing arguments are skipped
        CHECK_EQ(needed_size, 0u);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(2), csubstr("++"));
        CHECK_EQ(accum(), csubstr("012345678_{}_34567"));
    }
    SUBCASE("1 2")
    {
        //             ____1____ 2  __3__ 4  _5_
        csubstr fmt = "012345678_{}_34567_{}_aaa";
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(0), fmt, b1, b2);
        CHECK_EQ(needed_size, 2); // the longest sized argument format argument
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(2), csubstr("++")); // nothing was written
        CHECK_EQ(accum(), csubstr("")); // dumped first part of the format string
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(1), fmt, b1, b2);
        CHECK_EQ(needed_size, 2);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("2+"));
        CHECK_EQ(accum(), csubstr("012345678_1_34567_"));
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(2), fmt, b1, b2);
        CHECK_EQ(needed_size, 2);
        CHECK_EQ(DumpChecker::s_num_calls, 5);
        CHECK_EQ(buf.first(2), csubstr("22"));
        CHECK_EQ(accum(), csubstr("012345678_1_34567_22_aaa"));
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(1), fmt); // check that missing arguments are skipped
        CHECK_EQ(needed_size, 0u);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(2), csubstr("++"));
        CHECK_EQ(accum(), csubstr("012345678_{}_34567_{}_aaa"));
    }
    SUBCASE("1 2 3")
    {
        //             1         2  3     4  5     6
        csubstr fmt = "012345678_{}_34567_{}_aaa___{}";
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(0), fmt, b1, b2, b3);
        CHECK_EQ(needed_size, 3); // the longest sized argument format argument
        CHECK_EQ(DumpChecker::s_num_calls, 0);
        CHECK_EQ(buf.first(2), csubstr("++")); // nothing was written
        CHECK_EQ(accum(), csubstr("")); // dumped first part of the format string
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(1), fmt, b1, b2, b3);
        CHECK_EQ(needed_size, 3);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("2+"));
        CHECK_EQ(accum(), csubstr("012345678_1_34567_"));
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(2), fmt, b1, b2, b3);
        CHECK_EQ(needed_size, 3);
        CHECK_EQ(DumpChecker::s_num_calls, 5);
        CHECK_EQ(buf.first(2), csubstr("33"));
        CHECK_EQ(accum(), csubstr("012345678_1_34567_22_aaa___"));
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf.first(3), fmt, b1, b2, b3);
        CHECK_EQ(needed_size, 3);
        CHECK_EQ(DumpChecker::s_num_calls, 6);
        CHECK_EQ(buf.first(2), csubstr("33"));
        CHECK_EQ(accum(), csubstr("012345678_1_34567_22_aaa___333"));
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf, fmt); // check that missing arguments are skipped
        CHECK_EQ(needed_size, 0u);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(2), csubstr("++"));
        CHECK_EQ(accum(), csubstr("012345678_{}_34567_{}_aaa___{}"));
        DumpChecker::s_reset();
        needed_size = T::call_format_dump(buf, fmt, b1); // check that missing arguments are skipped
        CHECK_EQ(needed_size, 1);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("1+"));
        CHECK_EQ(accum(), csubstr("012345678_1_34567_{}_aaa___{}"));
    }
}


TEST_CASE_TEMPLATE("format_dump_resume", T, CatDumpTplArg, CatDumpFnArg)
{
    using namespace buffers;
    substr buf = DumpChecker::s_workspace;
    auto accum = [&]{ return csubstr(DumpChecker::s_accum).first(DumpChecker::s_accum_pos); };
    SUBCASE("1")
    {
        csubstr fmt = "aaa_then_{}_then_bbb";
        DumpChecker::s_reset();
        DumpResults ret = T::call_format_dump_resume(buf.first(0), fmt, b1);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_UNARY(!ret.success_until(2));
        CHECK_EQ(ret.bufsize, 1);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(1), csubstr("+")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_format_dump_resume(ret, buf.first(1), fmt, b1);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(ret.success_until(2));
        CHECK_EQ(ret.bufsize, 1);
        CHECK_EQ(ret.lastok, 2);
        CHECK_EQ(ret.argfail(), 3);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("1+"));
        CHECK_EQ(accum(), csubstr("aaa_then_1_then_bbb"));
    }
    SUBCASE("2")
    {
        csubstr fmt = "aaa_then_{}_then_bbb_then_{}__then_epilogue";
        DumpChecker::s_reset();
        DumpResults ret = T::call_format_dump_resume(buf.first(0), fmt, b1, b2);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_EQ(ret.bufsize, 2);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(1), csubstr("+")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_format_dump_resume(ret, buf.first(1), fmt, b1, b2);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(ret.success_until(2));
        CHECK_UNARY(!ret.success_until(3));
        CHECK_EQ(ret.bufsize, 2);
        CHECK_EQ(ret.lastok, 2);
        CHECK_EQ(ret.argfail(), 3);
        CHECK_EQ(DumpChecker::s_num_calls, 3);
        CHECK_EQ(buf.first(2), csubstr("2+"));
        CHECK_EQ(accum(), csubstr("aaa_then_1_then_bbb_then_"));
        ret = T::call_format_dump_resume(ret, buf.first(2), fmt, b1, b2);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(ret.success_until(1));
        CHECK_UNARY(ret.success_until(2));
        CHECK_UNARY(ret.success_until(3));
        CHECK_UNARY(ret.success_until(4));
        CHECK_EQ(ret.bufsize, 2);
        CHECK_EQ(ret.lastok, 4);
        CHECK_EQ(ret.argfail(), 5);
        CHECK_EQ(DumpChecker::s_num_calls, 5);
        CHECK_EQ(buf.first(2), csubstr("22"));
        CHECK_EQ(accum(), csubstr("aaa_then_1_then_bbb_then_22__then_epilogue"));
    }
    SUBCASE("no args")
    {
        csubstr fmt = "no args { -- }";
        DumpChecker::s_reset();
        DumpResults ret = T::call_format_dump_resume(buf.first(0), fmt, b1, b2);
        CHECK_UNARY(!ret.success_until(0));
        CHECK_EQ(ret.bufsize, 0);
        CHECK_EQ(ret.lastok, DumpResults::noarg);
        CHECK_EQ(ret.argfail(), 0);
        CHECK_EQ(DumpChecker::s_num_calls, 0); // no calls to dump
        CHECK_EQ(buf.first(1), csubstr("+")); // nothing was written
        CHECK_EQ(accum(), csubstr(""));
        ret = T::call_format_dump_resume(ret, buf.first(1), fmt, b1, b2);
        CHECK_UNARY(ret.success_until(0));
        CHECK_UNARY(!ret.success_until(1));
        CHECK_EQ(ret.bufsize, 0);
        CHECK_EQ(ret.lastok, 0);
        CHECK_EQ(ret.argfail(), 1);
        CHECK_EQ(DumpChecker::s_num_calls, 1);
        CHECK_EQ(buf.first(2), "++");
        CHECK_EQ(accum(), fmt);
    }
}

} // namespace c4

#ifdef __clang__
#   pragma clang diagnostic pop
#elif defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif

#include "c4/libtest/supprwarn_pop.hpp"
