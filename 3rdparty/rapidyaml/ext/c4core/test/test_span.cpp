#ifndef C4CORE_SINGLE_HEADER
#include "c4/span.hpp"
#endif

#include "c4/libtest/supprwarn_push.hpp"

#include <c4/test.hpp>

namespace c4 {

//-----------------------------------------------------------------------------
TEST_CASE_TEMPLATE("span.default_init", SpanClass, span<int>, spanrs<int>, spanrsl<int>)
{
    SpanClass s;
    CHECK_EQ(s.size(), 0);
    CHECK_EQ(s.capacity(), 0);
    CHECK_EQ(s.data(), nullptr);
}


template<template<class, class> class Span, class T, class I>
Span<const T, I> cvt_to_const(Span<T, I> const& s)
{
    Span<const T, I> ret = s;
    return ret;
}

TEST_CASE_TEMPLATE("span.convert_to_const", SpanClass, span<int>, spanrs<int>, spanrsl<int>)
{
    SpanClass s;
    auto cs = cvt_to_const(s);
    CHECK_EQ(s.size(), cs.size());
    CHECK_EQ(s.data(), cs.data());
    CHECK_EQ(s.end(), cs.end());
}


//-----------------------------------------------------------------------------
TEST_CASE("span.empty_init")
{
    int arr[10];
    span<int> s(arr, 0);
    CHECK_EQ(s.size(), 0);
    CHECK_EQ(s.capacity(), 0);
    CHECK_NE(s.data(), nullptr);
}

TEST_CASE("spanrs.empty_init")
{
    int arr[10];

    {
        spanrs<int> s(arr, 0);
        CHECK_EQ(s.size(), 0);
        CHECK_EQ(s.capacity(), 0);
        CHECK_EQ(s.data(), arr);
    }

    {
        spanrs<int> s(arr, 0, C4_COUNTOF(arr));
        CHECK_EQ(s.size(), 0);
        CHECK_EQ(s.capacity(), 10);
        CHECK_EQ(s.data(), arr);
    }
}

TEST_CASE("spanrsl.empty_init")
{
    int arr[10];

    {
        spanrsl<int> s(arr, 0);
        CHECK_EQ(s.size(), 0);
        CHECK_EQ(s.capacity(), 0);
        CHECK_EQ(s.data(), arr);
        CHECK_EQ(s.offset(), 0);
    }

    {
        spanrsl<int> s(arr, 0, C4_COUNTOF(arr));
        CHECK_EQ(s.size(), 0);
        CHECK_EQ(s.capacity(), 10);
        CHECK_EQ(s.data(), arr);
        CHECK_EQ(s.offset(), 0);
    }
}

//-----------------------------------------------------------------------------

TEST_CASE_TEMPLATE("span.fromArray", SpanClass,
                   span<char>, span<int>, span<uint32_t>,
                   spanrs<char>, spanrs<int>, spanrs<uint32_t>,
                   spanrsl<char>, spanrsl<int>, spanrsl<uint32_t>
    )
{
    using ConstSpanClass = typename SpanClass::const_type;
    using T = typename SpanClass::value_type;
    T arr1[10];
    T arr2[20];

    T a = 0;
    for(auto &v : arr1) { v = a; ++a; }
    for(auto &v : arr2) { v = a; ++a; }

    {
        SpanClass s(arr1);
        CHECK_EQ(s.size(), C4_COUNTOF(arr1));
        CHECK_EQ(s.capacity(), C4_COUNTOF(arr1));
        CHECK_EQ(s.data(), arr1);
    }

    {
        ConstSpanClass s(arr1);
        CHECK_EQ(s.size(), C4_COUNTOF(arr1));
        CHECK_EQ(s.capacity(), C4_COUNTOF(arr1));
        CHECK_EQ(s.data(), arr1);
    }

    {
        SpanClass s = arr1;
        CHECK_EQ(s.size(), C4_COUNTOF(arr1));
        CHECK_EQ(s.capacity(), C4_COUNTOF(arr1));
        CHECK_EQ(s.data(), arr1);
    }

    {
        ConstSpanClass s = arr1;
        CHECK_EQ(s.size(), C4_COUNTOF(arr1));
        CHECK_EQ(s.capacity(), C4_COUNTOF(arr1));
        CHECK_EQ(s.data(), arr1);
    }

    {
        SpanClass s = arr1;
        CHECK_EQ(s.size(), C4_COUNTOF(arr1));
        CHECK_EQ(s.capacity(), C4_COUNTOF(arr1));
        CHECK_EQ(s.data(), arr1);
        s = arr2;
        CHECK_EQ(s.size(), C4_COUNTOF(arr2));
        CHECK_EQ(s.capacity(), C4_COUNTOF(arr2));
        CHECK_EQ(s.data(), arr2);
    }

    {
        ConstSpanClass s = arr1;
        CHECK_EQ(s.size(), C4_COUNTOF(arr1));
        CHECK_EQ(s.capacity(), C4_COUNTOF(arr1));
        CHECK_EQ(s.data(), arr1);
        s = arr2;
        CHECK_EQ(s.size(), C4_COUNTOF(arr2));
        CHECK_EQ(s.capacity(), C4_COUNTOF(arr2));
        CHECK_EQ(s.data(), arr2);
    }
}


//-----------------------------------------------------------------------------
TEST_CASE("span.subspan")
{
    int arr[10];
    span<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.subspan(0)), decltype(s)>::value));

    {
        auto ss = s.subspan(0, 5);
        CHECK_EQ(ss.size(), 5);
        CHECK_EQ(ss.capacity(), 5);
        CHECK_EQ(ss.data(), arr);

        ss = s.subspan(5);
        CHECK_EQ(ss.size(), 5);
        CHECK_EQ(ss.capacity(), 5);
        CHECK_EQ(ss.data(), &arr[5]);

        // fine to obtain an empty span at the end
        ss = s.subspan(10);
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.capacity(), 0);
        CHECK_EQ(ss.data(), std::end(arr));
        // fine to obtain an empty span at the end
        ss = s.subspan(10, 0);
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.capacity(), 0);
        CHECK_EQ(ss.data(), std::end(arr));
    }
    {
        int buf10[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int buf_5[]  = {-1, 0, 1, 2, 3, 4};
        int *buf5 = buf_5 + 1; // to make sure that one does not immediately follow the other in memory

        span<int> n(buf10);
        span<int> m(buf5, 5);

        auto ss = n.subspan(0);
        CHECK_EQ(ss.data(), buf10);
        CHECK_EQ(ss.size(), 10);
        ss = m.subspan(0);
        CHECK_EQ(ss.data(), buf5);
        CHECK_EQ(ss.size(), 5);
        ss = n.subspan(0, 0);
        CHECK_NE(ss.data(), nullptr);
        CHECK_EQ(ss.data(), &buf10[0]);
        CHECK_EQ(ss.size(), 0);
        ss = m.subspan(0, 0);
        CHECK_NE(ss.data(), nullptr);
        CHECK_EQ(ss.data(), &buf5[0]);
        CHECK_EQ(ss.size(), 0);
    }
}
TEST_CASE("spanrs.subspan")
{
    int arr[10];
    spanrs<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.subspan(0)), decltype(s)>::value));

    auto ss = s.subspan(0, 5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss = s.subspan(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), &arr[5]);

    // fine to obtain an empty span at the end
    ss = s.subspan(10);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), std::end(arr));
    // fine to obtain an empty span at the end
    ss = s.subspan(10, 0);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), std::end(arr));
}
TEST_CASE("spanrsl.subspan")
{
    int arr[10];
    spanrsl<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.subspan(0)), decltype(s)>::value));

    auto ss = s.subspan(0, 5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);
    CHECK_EQ(ss.offset(), 0);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);
    CHECK_EQ(ss.offset(), 0);

    ss = s.subspan(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), &arr[5]);
    CHECK_EQ(ss.offset(), 5);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);
    CHECK_EQ(ss.offset(), 0);

    // fine to obtain an empty span at the end
    ss = s.subspan(10);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), std::end(arr));
    // fine to obtain an empty span at the end
    ss = s.subspan(10, 0);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), std::end(arr));
}

//-----------------------------------------------------------------------------
TEST_CASE("span.range")
{
    int arr[10];
    span<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.range(0)), decltype(s)>::value));

    auto ss = s.range(0, 5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), arr);

    ss = s.range(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), &arr[5]);

    ss = s.range(5, 10);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), &arr[5]);

    ss = s.range(10, 10);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), std::end(arr));

    SUBCASE("empty_span")
    {
        s = {};
        ss = s.range(0, 0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), nullptr);
        s = arr;
        ss = s.range(0, 0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr);
        ss = s.range(10, 10);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr + 10);
        ss = s.range(10);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr + 10);
    }
}
TEST_CASE("spanrs.range")
{
    int arr[10];
    spanrs<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.range(0)), decltype(s)>::value));

    auto ss = s.range(0, 5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss = s.range(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), &arr[5]);

    ss = s.range(5, 10);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), &arr[5]);

    ss = s.range(10, 10);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), std::end(arr));

    SUBCASE("empty_span")
    {
        s = {};
        ss = s.range(0, 0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), nullptr);
        s = arr;
        ss = s.range(0, 0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr);
        ss = s.range(10, 10);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr + 10);
        ss = s.range(10);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr + 10);
    }
}
TEST_CASE("spanrsl.range")
{
    int arr[10];
    spanrsl<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.range(0)), decltype(s)>::value));

    auto ss = s.range(0, 5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss = s.range(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), &arr[5]);

    ss = s.range(5, 10);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), &arr[5]);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss = s.range(10, 10);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), std::end(arr));

    SUBCASE("empty_span")
    {
        s = {};
        ss = s.range(0, 0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), nullptr);
        s = arr;
        ss = s.range(0, 0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr);
        ss = s.range(10, 10);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr + 10);
        ss = s.range(10);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr + 10);
    }
}

//-----------------------------------------------------------------------------
TEST_CASE("span.first")
{
    int arr[10];
    span<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.first(1)), decltype(s)>::value));

    auto ss = s.first(0);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), arr);

    ss = s.first(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), arr);

    SUBCASE("empty")
    {
        s = {};
        ss = s.first(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), nullptr);
        s = arr;
        ss = s.first(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr);
    }
}
TEST_CASE("spanrs.first")
{
    int arr[10];
    spanrs<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.first(1)), decltype(s)>::value));

    auto ss = s.first(0);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss = s.first(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    SUBCASE("empty")
    {
        s = {};
        ss = s.first(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), nullptr);
        s = arr;
        ss = s.first(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr);
    }
}
TEST_CASE("spanrsl.first")
{
    int arr[10];
    spanrsl<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.first(1)), decltype(s)>::value));

    auto ss = s.first(0);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss = s.first(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    SUBCASE("empty")
    {
        s = {};
        ss = s.first(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), nullptr);
        s = arr;
        ss = s.first(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr);
    }
}

//-----------------------------------------------------------------------------
TEST_CASE("span.last")
{
    int arr[10];
    span<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.last(1)), decltype(s)>::value));

    auto ss = s.last(0);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), arr + s.size());

    ss = s.last(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), arr + 5);

    SUBCASE("empty")
    {
        s = {};
        ss = s.last(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), nullptr);
        s = arr;
        ss = s.last(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr + 10);
    }
}
TEST_CASE("spanrs.last")
{
    int arr[10];
    spanrs<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.last(1)), decltype(s)>::value));

    auto ss = s.last(0);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), arr + s.size());

    ss = s.last(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), arr + 5);

    SUBCASE("empty")
    {
        s = {};
        ss = s.last(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), nullptr);
        s = arr;
        ss = s.last(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr + 10);
    }
}
TEST_CASE("spanrsl.last")
{
    int arr[10];
    spanrsl<int> s(arr);
    C4_STATIC_ASSERT((std::is_same<decltype(s.last(1)), decltype(s)>::value));

    auto ss = s.last(0);
    CHECK_EQ(ss.size(), 0);
    CHECK_EQ(ss.capacity(), 0);
    CHECK_EQ(ss.data(), arr + s.size());

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss = s.last(5);
    CHECK_EQ(ss.size(), 5);
    CHECK_EQ(ss.capacity(), 5);
    CHECK_EQ(ss.data(), arr + 5);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    SUBCASE("empty")
    {
        s = {};
        ss = s.last(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), nullptr);
        s = arr;
        ss = s.last(0);
        CHECK(ss.empty());
        CHECK_EQ(ss.size(), 0);
        CHECK_EQ(ss.data(), arr + 10);
    }
}


//-----------------------------------------------------------------------------
TEST_CASE_TEMPLATE("span.is_subspan", SpanClass, span<int>, spanrs<int>, spanrsl<int>)
{
    int buf10[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int buf_5[]  = {-1, 0, 1, 2, 3, 4};
    int *buf5 = buf_5 + 1; // to make sure that one does not immediately follow the other in memory

    SpanClass n(buf10);
    SpanClass m(buf5, 5);

    CHECK_EQ(n.data(), buf10);
    CHECK_EQ(m.data(), buf5);

    CHECK_UNARY(n.is_subspan(n.subspan(0   )));
    CHECK_UNARY(n.is_subspan(n.subspan(0, 3)));
    CHECK_UNARY(n.is_subspan(n.subspan(0, 0)));

    CHECK_FALSE(n.is_subspan(m.subspan(0   )));
    CHECK_FALSE(n.is_subspan(m.subspan(0, 3)));
    CHECK_FALSE(n.is_subspan(m.subspan(0, 0)));
}

//-----------------------------------------------------------------------------
TEST_CASE_TEMPLATE("span.compll", SpanClass, span<int>, spanrs<int>, spanrsl<int>)
{
    int buf10[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    SpanClass n(buf10);

    CHECK_EQ(n.compll(n.subspan(0)), n.subspan(0, 0));
    CHECK_EQ(n.is_subspan(n.compll(n.subspan(0))), true);
    CHECK_EQ(n.compll(n.subspan(0, 0)), n.subspan(0, 0));
    CHECK_EQ(n.is_subspan(n.compll(n.subspan(0, 0))), true);

    CHECK_EQ(n.compll(n.subspan(0, 1)), n.subspan(0, 0));
    CHECK_EQ(n.compll(n.subspan(0, 3)), n.subspan(0, 0));

    CHECK_EQ(n.compll(n.range(5, 10)), n.subspan(0, 5));
    CHECK_EQ(n.compll(n.range(5, 5)),  n.subspan(0, 5));

    CHECK_EQ(n.compll(n.subspan(n.size(), 0)), n);
    CHECK_EQ(n.compll(n.range(n.size(), n.size())), n);
}


//-----------------------------------------------------------------------------

TEST_CASE_TEMPLATE("span.complr", SpanClass, span<int>, spanrs<int>, spanrsl<int>)
{
    int buf10[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    SpanClass n(buf10);

    CHECK_EQ(n.complr(n.subspan(0)), n.subspan(0, 0));
    CHECK_EQ(n.is_subspan(n.complr(n.subspan(0))), true);
    CHECK_EQ(n.complr(n.subspan(0, 0)), n.subspan(0));
    CHECK_EQ(n.is_subspan(n.complr(n.subspan(0, 0))), true);

    CHECK_EQ(n.complr(n.subspan(0, 1)), n.subspan(1));
    CHECK_EQ(n.complr(n.subspan(0, 3)), n.subspan(3));

    CHECK_EQ(n.complr(n.subspan(5)), n.subspan(0, 0));
    CHECK_EQ(n.complr(n.range(5, 10)), n.subspan(0, 0));

    CHECK_EQ(n.complr(n.subspan(5, 0)), n.subspan(5));
    CHECK_EQ(n.complr(n.range(5, 5)), n.subspan(5));

    CHECK_EQ(n.complr(n.subspan(0, 0)), n);
    CHECK_EQ(n.complr(n.range(0, 0)), n);
}


//-----------------------------------------------------------------------------
TEST_CASE("span.rtrim")
{
    int arr[10];
    span<int> s(arr);
    auto ss = s;

    ss.rtrim(0);
    CHECK_EQ(ss.size(), s.size());
    CHECK_EQ(ss.capacity(), s.capacity());
    CHECK_EQ(ss.data(), arr);

    ss.rtrim(5);
    CHECK_EQ(ss.size(), s.size() - 5);
    CHECK_EQ(ss.capacity(), s.capacity() - 5);
    CHECK_EQ(ss.data(), arr);
}
TEST_CASE("spanrs.rtrim")
{
    int arr[10];
    spanrs<int> s(arr);
    auto ss = s;

    ss.rtrim(0);
    CHECK_EQ(ss.size(), s.size());
    CHECK_EQ(ss.capacity(), s.capacity());
    CHECK_EQ(ss.data(), arr);

    ss.rtrim(5);
    CHECK_EQ(ss.size(), s.size() - 5);
    CHECK_EQ(ss.capacity(), s.capacity());
    CHECK_EQ(ss.data(), arr);
}
TEST_CASE("spanrsl.rtrim")
{
    int arr[10];
    spanrsl<int> s(arr);
    auto ss = s;

    ss.rtrim(0);
    CHECK_EQ(ss.size(), s.size());
    CHECK_EQ(ss.capacity(), s.capacity());
    CHECK_EQ(ss.data(), arr);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss.rtrim(5);
    CHECK_EQ(ss.size(), s.size() - 5);
    CHECK_EQ(ss.capacity(), s.capacity());
    CHECK_EQ(ss.data(), arr);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);
}

//-----------------------------------------------------------------------------
TEST_CASE("span.ltrim")
{
    int arr[10];
    span<int> s(arr);
    auto ss = s;

    ss.ltrim(0);
    CHECK_EQ(ss.size(), s.size());
    CHECK_EQ(ss.capacity(), s.capacity());
    CHECK_EQ(ss.data(), arr);

    ss.ltrim(5);
    CHECK_EQ(ss.size(), s.size() - 5);
    CHECK_EQ(ss.capacity(), s.capacity() - 5);
    CHECK_EQ(ss.data(), arr + 5);
}
TEST_CASE("spanrs.ltrim")
{
    int arr[10];
    spanrs<int> s(arr);
    auto ss = s;

    ss.ltrim(0);
    CHECK_EQ(ss.size(), s.size());
    CHECK_EQ(ss.capacity(), ss.capacity());
    CHECK_EQ(ss.data(), arr);

    ss.ltrim(5);
    CHECK_EQ(ss.size(), s.size() - 5);
    CHECK_EQ(ss.capacity(), s.size() - 5);
    CHECK_EQ(ss.data(), arr + 5);
}
TEST_CASE("spanrsl.ltrim")
{
    int arr[10];
    spanrsl<int> s(arr);
    auto ss = s;

    ss.ltrim(0);
    CHECK_EQ(ss.size(), s.size());
    CHECK_EQ(ss.capacity(), ss.capacity());
    CHECK_EQ(ss.data(), arr);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);

    ss.ltrim(5);
    CHECK_EQ(ss.size(), s.size() - 5);
    CHECK_EQ(ss.capacity(), s.size() - 5);
    CHECK_EQ(ss.data(), arr + 5);

    ss = ss.original();
    CHECK_EQ(ss.size(), 10);
    CHECK_EQ(ss.capacity(), 10);
    CHECK_EQ(ss.data(), arr);
}

//-----------------------------------------------------------------------------
const char larrc[11] = "0123456789";
const char rarrc[11] = "1234567890";
const int larri[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const int rarri[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
TEST_CASE("span.reverse_iter")
{
    cspan<int> s(larri);
    REQUIRE_EQ(s.data(), larri);
    REQUIRE_EQ(s.begin(), std::begin(larri));
    REQUIRE_EQ(s.end(), std::end(larri));
    REQUIRE_EQ(&*s.rbegin(), s.end()-1);
    using rit = cspan<int>::const_reverse_iterator;
    int pos = szconv<int>(s.size()) - 1;
    size_t count = 0;
    //for(rit b = s.rbegin(), e = s.rend(); b != e; ++b) // BUG: b != e is never true on arm-eabi-g++-7
    for(rit b = s.rbegin(), e = s.rend(); b < e; ++b)
    {
        CHECK_EQ(&(*b), s.data() + pos);
        auto spos = szconv<size_t>(pos);
        REQUIRE_EQ(&*b, &s[spos]);
        REQUIRE_GE(pos, 0);
        REQUIRE_LT(pos, szconv<int>(s.size()));
        REQUIRE_LT(count, s.size());
        CHECK_EQ(*b, s[spos]);
        --pos;
        ++count;
    }
    CHECK_EQ(pos, -1);
    CHECK_EQ(count, s.size());
}

//-----------------------------------------------------------------------------
TEST_CASE("span_impl.eq")
{
    CHECK_EQ(cspan  <char>(larrc), cspan  <char>(larrc));
    CHECK_EQ(cspanrs<char>(larrc), cspan  <char>(larrc));
    CHECK_EQ(cspan  <char>(larrc), cspanrs<char>(larrc));
    CHECK_EQ(cspanrs<char>(larrc), cspanrs<char>(larrc));

    CHECK_EQ(cspan  <int>(larri) , cspan  <int>(larri));
    CHECK_EQ(cspanrs<int>(larri) , cspan  <int>(larri));
    CHECK_EQ(cspan  <int>(larri) , cspanrs<int>(larri));
    CHECK_EQ(cspanrs<int>(larri) , cspanrs<int>(larri));
}

TEST_CASE("span_impl.lt")
{
    CHECK_LT(cspan  <char>(larrc), cspan  <char>(rarrc));
    CHECK_LT(cspanrs<char>(larrc), cspan  <char>(rarrc));
    CHECK_LT(cspan  <char>(larrc), cspanrs<char>(rarrc));
    CHECK_LT(cspanrs<char>(larrc), cspanrs<char>(rarrc));

    CHECK_LT(cspan  <int>(larri) , cspan  <int>(rarri));
    CHECK_LT(cspanrs<int>(larri) , cspan  <int>(rarri));
    CHECK_LT(cspan  <int>(larri) , cspanrs<int>(rarri));
    CHECK_LT(cspanrs<int>(larri) , cspanrs<int>(rarri));
}
TEST_CASE("span_impl.gt")
{
    CHECK_GT(cspan  <char>(rarrc), cspan  <char>(larrc));
    CHECK_GT(cspan  <char>(rarrc), cspanrs<char>(larrc));
    CHECK_GT(cspanrs<char>(rarrc), cspan  <char>(larrc));
    CHECK_GT(cspanrs<char>(rarrc), cspanrs<char>(larrc));

    CHECK_GT(cspan  <int>(rarri) , cspan  <int>(larri));
    CHECK_GT(cspan  <int>(rarri) , cspanrs<int>(larri));
    CHECK_GT(cspanrs<int>(rarri) , cspan  <int>(larri));
    CHECK_GT(cspanrs<int>(rarri) , cspanrs<int>(larri));
}

TEST_CASE("span_impl.ge")
{
    CHECK_GE(cspan  <char>(rarrc), cspan  <char>(larrc));
    CHECK_GE(cspan  <char>(rarrc), cspanrs<char>(larrc));
    CHECK_GE(cspanrs<char>(rarrc), cspan  <char>(larrc));
    CHECK_GE(cspanrs<char>(rarrc), cspanrs<char>(larrc));
    CHECK_GE(cspan  <char>(larrc), cspan  <char>(larrc));
    CHECK_GE(cspan  <char>(larrc), cspanrs<char>(larrc));
    CHECK_GE(cspanrs<char>(larrc), cspan  <char>(larrc));
    CHECK_GE(cspanrs<char>(larrc), cspanrs<char>(larrc));
    CHECK_GE(cspan  <int>(rarri) , cspan  <int>(larri));
    CHECK_GE(cspan  <int>(rarri) , cspanrs<int>(larri));
    CHECK_GE(cspanrs<int>(rarri) , cspan  <int>(larri));
    CHECK_GE(cspanrs<int>(rarri) , cspanrs<int>(larri));
    CHECK_GE(cspan  <int>(larri) , cspan  <int>(larri));
    CHECK_GE(cspan  <int>(larri) , cspanrs<int>(larri));
    CHECK_GE(cspanrs<int>(larri) , cspan  <int>(larri));
    CHECK_GE(cspanrs<int>(larri) , cspanrs<int>(larri));
}
TEST_CASE("span_impl.le")
{
    CHECK_LE(cspan  <char>(larrc), cspan  <char>(rarrc));
    CHECK_LE(cspanrs<char>(larrc), cspan  <char>(rarrc));
    CHECK_LE(cspan  <char>(larrc), cspanrs<char>(rarrc));
    CHECK_LE(cspanrs<char>(larrc), cspanrs<char>(rarrc));
    CHECK_LE(cspan  <char>(larrc), cspan  <char>(larrc));
    CHECK_LE(cspanrs<char>(larrc), cspan  <char>(larrc));
    CHECK_LE(cspan  <char>(larrc), cspanrs<char>(larrc));
    CHECK_LE(cspanrs<char>(larrc), cspanrs<char>(larrc));
    CHECK_LE(cspan  <int>(larri) , cspan  <int>(rarri));
    CHECK_LE(cspanrs<int>(larri) , cspan  <int>(rarri));
    CHECK_LE(cspan  <int>(larri) , cspanrs<int>(rarri));
    CHECK_LE(cspanrs<int>(larri) , cspanrs<int>(rarri));
    CHECK_LE(cspan  <int>(larri) , cspan  <int>(larri));
    CHECK_LE(cspanrs<int>(larri) , cspan  <int>(larri));
    CHECK_LE(cspan  <int>(larri) , cspanrs<int>(larri));
    CHECK_LE(cspanrs<int>(larri) , cspanrs<int>(larri));
}

} // namespace c4

#include "c4/libtest/supprwarn_pop.hpp"
