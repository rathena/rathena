#include <string>
#include <c4/c4_push.hpp>
#include <c4/std/std.hpp>
#include <c4/format.hpp>
#include <c4/dump.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <benchmark/benchmark.h>

namespace bm = benchmark;

double getmax(std::vector<double> const& v)
{
    return *(std::max_element(std::begin(v), std::end(v)));
}
double getmin(std::vector<double> const& v)
{
    return *(std::min_element(std::begin(v), std::end(v)));
}
double getrange(std::vector<double> const& v)
{
    auto min_max = std::minmax_element(std::begin(v), std::end(v));
    return *min_max.second - *min_max.first;
}

#define _c4bm_stats                                                     \
    /*->Repetitions(20)*/                                               \
    ->DisplayAggregatesOnly(true)                                       \
    ->ComputeStatistics("range", &getrange)                             \
    ->ComputeStatistics("max", &getmax)                                 \
    ->ComputeStatistics("min", &getmin)

#define C4BM(fn) BENCHMARK(fn) _c4bm_stats

/** convenience wrapper to avoid boilerplate code */
void report(bm::State &st, size_t sz)
{
    st.SetBytesProcessed(static_cast<int64_t>(st.iterations() * sz));
    st.SetItemsProcessed(static_cast<int64_t>(st.iterations()));
}


const c4::csubstr sep = " --- ";


#define _c4argbundle_fmt "hello here you have some numbers: "\
    "1={}, 2={}, 3={}, 4={}, 5={}, 6={}, 7={}, 8={}, 9={}, size_t(283482349)={}, "\
    "\" \"=\"{}\", \"haha\"=\"{}\", std::string(\"hehe\")=\"{}\", "\
    "str=\"{}\""

#define _c4argbundle_fmt_printf "hello here you have some numbers: "\
    "1=%d, 2=%d, 3=%d, 4=%d, 5=%d, 6=%d, 7=%d, 8=%d, 9=%d, size_t(283482349)=%zu, "\
    "\" \"=\"%s\", \"haha\"=\"%s\", std::string(\"hehe\")=\"%s\", "\
    "str=\"%s\""

#define _c4argbundle_fmt_printf_sep "hello here you have some numbers: "\
    "1=%d%s2=%d%s3=%d%s4=%d%s5=%d%s6=%d%s7=%d%s8=%d%s9=%d%ssize_t(283482349)=%zu%s"\
    "\" \"=\"%s\"%s\"haha\"=\"%s\"%sstd::string(\"hehe\")=\"%s\"%s"\
    "str=\"%s\""

#define _c4argbundle \
    1, 2, 3, 4, 5, 6, 7, 8, 9, size_t(283482349),\
    " ", "haha", std::string("hehe"),\
    std::string("asdlklkasdlkjasd asdlkjasdlkjasdlkjasdoiasdlkjasldkj")

#define _c4argbundle_printf \
    1, 2, 3, 4, 5, 6, 7, 8, 9, size_t(283482349),\
    " ", "haha", std::string("hehe").c_str(),\
    std::string("asdlklkasdlkjasd asdlkjasdlkjasdlkjasdoiasdlkjasldkj").c_str()

#define _c4argbundle_printf_sep \
    1, sep.str, 2, sep.str, 3, sep.str, 4, sep.str, 5, sep.str, 6, sep.str, 7, sep.str, 8, sep.str, 9, sep.str, size_t(283482349), sep.str,\
    " ", sep.str, "haha", sep.str, std::string("hehe").c_str(), sep.str,\
    std::string("asdlklkasdlkjasd asdlkjasdlkjasdlkjasdoiasdlkjasldkj").c_str()

#define _c4argbundle_lshift \
    1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << size_t(283482349)\
      << " " << "haha" << std::string("hehe")\
      << std::string("asdlklkasdlkjasd asdlkjasdlkjasdlkjasdoiasdlkjasldkj")

#define _c4argbundle_lshift_sep \
    1 << sep << 2 << sep << 3 << sep << 4 << sep << 5 << sep << 6 << sep << 7 << sep << 8 << sep << 9 << sep << size_t(283482349)\
      << sep << " " << sep << "haha" << sep << std::string("hehe")\
      << sep << std::string("asdlklkasdlkjasd asdlkjasdlkjasdlkjasdoiasdlkjasldkj")


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace dump2str {
std::string c_style_subject;
void c_style(c4::csubstr s) { c_style_subject.append(s.str, s.len); }
struct cpp_style
{
    std::string subject = {};
    void operator() (c4::csubstr s) { subject.append(s.str, s.len); }
};
struct lambda_style
{
    std::string subject = {};
};
} // namespace dump2str

namespace dump2file {
FILE * c_style_subject;
void c_style(c4::csubstr s) { fwrite(s.str, 1, s.len, c_style_subject); }
struct cpp_style
{
    FILE * subject;
    cpp_style() : subject(fopen("asdkjhasdkjhsdfoiefkjn", "wb")) {}
    ~cpp_style() { fclose(subject); }
    void operator() (c4::csubstr s) { fwrite(s.str, 1, s.len, subject); }
};
struct lambda_style
{
    lambda_style() : subject(fopen("asdkjhasdkjhsdfoiefkjn", "wb")) {}
    ~lambda_style() { fclose(subject); }
    FILE * subject;
};
} // namespace dump2fil

template<class T>
C4_ALWAYS_INLINE typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type
std_to_string(T const& a)
{
    return std::to_string(a);
}

template<class T>
C4_ALWAYS_INLINE typename std::enable_if<std::is_same<T, std::string>::value, std::string const&>::type
std_to_string(std::string const& a)
{
    return a;
}

C4_ALWAYS_INLINE std::string std_to_string(c4::csubstr a)
{
    return std::string(a.str, a.len);
}

template<class T>
C4_ALWAYS_INLINE typename std::enable_if< ! std::is_arithmetic<T>::value, std::string>::type
std_to_string(T const& a)
{
    return std::string(a);
}

C4_ALWAYS_INLINE void cat_std_string_impl(std::string *)
{
}

C4_ALWAYS_INLINE void catsep_std_string_impl(std::string *)
{
}

template<class Arg, class... Args>
void cat_std_string_impl(std::string *s, Arg const& a, Args const& ...args)
{
    *s += std_to_string(a);
    cat_std_string_impl(s, args...);
}

template<class Arg, class... Args>
void catsep_std_string_impl(std::string *s, Arg const& a, Args const& ...args)
{
    *s += std_to_string(a);
    if(sizeof...(args) > 0)
    {
        s->append(sep.str, sep.len);
        catsep_std_string_impl(s, args...);
    }
}

void cat_std_stringstream_impl(std::stringstream &)
{
}
void catsep_std_stringstream_impl(std::stringstream &)
{
}

template<class Arg, class... Args>
void cat_std_stringstream_impl(std::stringstream &ss, Arg const& a, Args const& ...args)
{
    ss << a;
    cat_std_stringstream_impl(ss, args...);
}

template<class Arg, class... Args>
void catsep_std_stringstream_impl(std::stringstream &ss, Arg const& a, Args const& ...args)
{
    ss << sep << a;
    cat_std_stringstream_impl(ss, args...);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void cat_c4cat_substr(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = 0;
    for(auto _ : st)
    {
        sz = cat(buf, _c4argbundle);
    }
    report(st, sz);
}

void cat_c4catrs_reuse(bm::State &st)
{
    std::string buf;
    size_t sz = 0;
    for(auto _ : st)
    {
        c4::catrs(&buf, _c4argbundle);
        sz = buf.size();
    }
    report(st, sz);
}

void cat_c4catrs_no_reuse(bm::State &st)
{
    size_t sz = 0;
    for(auto _ : st)
    {
        auto buf = c4::catrs<std::string>(_c4argbundle);
        sz = buf.size();
    }
    report(st, sz);
}

void cat_c4catdump_c_style_static_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::cat(buf, _c4argbundle);
    for(auto _ : st)
    {
        c4::cat_dump<&dump2str::c_style>(buf, _c4argbundle);
    }
    report(st, sz);
}

void cat_c4catdump_c_style_dynamic_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::cat(buf, _c4argbundle);
    for(auto _ : st)
    {
        sz = c4::cat_dump(&dump2str::c_style, buf, _c4argbundle);
    }
    report(st, sz);
}

void cat_c4catdump_cpp_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::cat(buf, _c4argbundle);
    dump2str::cpp_style dumper;
    for(auto _ : st)
    {
        sz = c4::cat_dump(dumper, buf, _c4argbundle);
    }
    report(st, sz);
}

void cat_c4catdump_lambda_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::cat(buf, _c4argbundle);
    dump2str::lambda_style dumper;
    auto lambda = [&dumper](c4::csubstr s) { dumper.subject.append(s.str, s.len); };
    for(auto _ : st)
    {
        sz = c4::cat_dump(lambda, buf, _c4argbundle);
    }
    report(st, sz);
}

void cat_stdsstream_reuse(bm::State &st)
{
    size_t sz = 0;
    std::stringstream ss;
    for(auto _ : st)
    {
        ss.clear();
        ss.str("");
        cat_std_stringstream_impl(ss, _c4argbundle);
        sz = ss.str().size();
    }
    report(st, sz);
}

void cat_stdsstream_no_reuse(bm::State &st)
{
    size_t sz = 0;
    for(auto _ : st)
    {
        std::stringstream ss;
        cat_std_stringstream_impl(ss, _c4argbundle);
        sz = ss.str().size();
    }
    report(st, sz);
}

void cat_std_to_string_reuse(bm::State &st)
{
    size_t sz = 0;
    std::string s;
    for(auto _ : st)
    {
        s.clear();
        cat_std_string_impl(&s, _c4argbundle);
        sz = s.size();
    }
    report(st, sz);
}

void cat_std_to_string_no_reuse(bm::State &st)
{
    size_t sz = 0;
    for(auto _ : st)
    {
        std::string s;
        cat_std_string_impl(&s, _c4argbundle);
        sz = s.size();
    }
    report(st, sz);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void catfile_c4catdump_c_style_static_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    dump2file::cpp_style fileowner;
    dump2file::c_style_subject = fileowner.subject;
    size_t sz = c4::cat(buf, _c4argbundle);
    for(auto _ : st)
    {
        c4::cat_dump<&dump2file::c_style>(buf, _c4argbundle);
    }
    report(st, sz);
}

void catfile_c4catdump_c_style_dynamic_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    dump2file::cpp_style fileowner;
    dump2file::c_style_subject = fileowner.subject;
    size_t sz = c4::cat(buf, _c4argbundle);
    for(auto _ : st)
    {
        c4::cat_dump(&dump2file::c_style, buf, _c4argbundle);
    }
    report(st, sz);
}

void catfile_c4catdump_cpp_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::cat(buf, _c4argbundle);
    dump2file::cpp_style dumper;
    for(auto _ : st)
    {
        sz = c4::cat_dump(dumper, buf, _c4argbundle);
    }
    report(st, sz);
}

void catfile_c4catdump_lambda_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::cat(buf, _c4argbundle);
    dump2file::lambda_style dumper;
    auto lambda = [&dumper](c4::csubstr s) { fwrite(s.str, 1, s.len, dumper.subject); };
    for(auto _ : st)
    {
        sz = c4::cat_dump(lambda, buf, _c4argbundle);
    }
    report(st, sz);
}

void catfile_fprintf(bm::State &st)
{
    char buf[256];
    size_t sz = c4::cat(buf, _c4argbundle);
    dump2file::cpp_style dumper;
    for(auto _ : st)
    {
        fprintf(dumper.subject, _c4argbundle_fmt_printf, _c4argbundle_printf);
    }
    report(st, sz);
}

void catfile_ofstream(bm::State &st)
{
    char buf[256];
    size_t sz = c4::cat(buf, _c4argbundle);
    std::ofstream ofs("ddofgufgbmn4g0rtglf", std::ios::out|std::ios::binary);
    for(auto _ : st)
    {
        ofs << _c4argbundle_lshift;
    }
    report(st, sz);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void catsep_c4cat_substr(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = 0;
    for(auto _ : st)
    {
        sz = catsep(buf, _c4argbundle);
    }
    report(st, sz);
}

void catsep_c4catrs_reuse(bm::State &st)
{
    std::string buf;
    size_t sz = 0;
    for(auto _ : st)
    {
        c4::catseprs(&buf, _c4argbundle);
        sz = buf.size();
    }
    report(st, sz);
}

void catsep_c4catrs_no_reuse(bm::State &st)
{
    size_t sz = 0;
    for(auto _ : st)
    {
        auto buf = c4::catseprs<std::string>(sep, _c4argbundle);
        sz = buf.size();
    }
    report(st, sz);
}

void catsep_c4catdump_c_style_static_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::catsep(buf, _c4argbundle);
    for(auto _ : st)
    {
        c4::catsep_dump<&dump2str::c_style>(buf, sep, _c4argbundle);
    }
    report(st, sz);
}

void catsep_c4catdump_c_style_dynamic_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::catsep(buf, _c4argbundle);
    for(auto _ : st)
    {
        sz = c4::catsep_dump(&dump2str::c_style, buf, sep, _c4argbundle);
    }
    report(st, sz);
}

void catsep_c4catdump_cpp_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::catsep(buf, _c4argbundle);
    dump2str::cpp_style dumper;
    for(auto _ : st)
    {
        sz = c4::catsep_dump(dumper, buf, sep, _c4argbundle);
    }
    report(st, sz);
}

void catsep_c4catdump_lambda_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::catsep(buf, sep, _c4argbundle);
    dump2str::lambda_style dumper;
    auto lambda = [&dumper](c4::csubstr s) { dumper.subject.append(s.str, s.len); };
    for(auto _ : st)
    {
        sz = c4::catsep_dump(lambda, buf, _c4argbundle);
    }
    report(st, sz);
}

void catsep_stdsstream_reuse(bm::State &st)
{
    size_t sz = 0;
    std::stringstream ss;
    for(auto _ : st)
    {
        ss.clear();
        ss.str("");
        catsep_std_stringstream_impl(ss, sep, _c4argbundle);
        sz = ss.str().size();
    }
    report(st, sz);
}

void catsep_stdsstream_no_reuse(bm::State &st)
{
    size_t sz = 0;
    for(auto _ : st)
    {
        std::stringstream ss;
        catsep_std_stringstream_impl(ss, sep, _c4argbundle);
        sz = ss.str().size();
    }
    report(st, sz);
}

void catsep_std_to_string_reuse(bm::State &st)
{
    size_t sz = 0;
    std::string s;
    for(auto _ : st)
    {
        s.clear();
        catsep_std_string_impl(&s, sep, _c4argbundle);
        sz = s.size();
    }
    report(st, sz);
}

void catsep_std_to_string_no_reuse(bm::State &st)
{
    size_t sz = 0;
    for(auto _ : st)
    {
        std::string s;
        catsep_std_string_impl(&s, sep, _c4argbundle);
        sz = s.size();
    }
    report(st, sz);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void catsepfile_c4catsepdump_c_style_static_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    dump2file::cpp_style fileowner;
    dump2file::c_style_subject = fileowner.subject;
    size_t sz = c4::catsep(buf, sep, _c4argbundle);
    for(auto _ : st)
    {
        c4::catsep_dump<&dump2file::c_style>(buf, sep, _c4argbundle);
    }
    report(st, sz);
}

void catsepfile_c4catsepdump_c_style_dynamic_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    dump2file::cpp_style fileowner;
    dump2file::c_style_subject = fileowner.subject;
    size_t sz = c4::catsep(buf, sep, _c4argbundle);
    for(auto _ : st)
    {
        c4::catsep_dump(&dump2file::c_style, buf, sep, _c4argbundle);
    }
    report(st, sz);
}

void catsepfile_c4catsepdump_cpp_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::catsep(buf, sep, _c4argbundle);
    dump2file::cpp_style dumper;
    for(auto _ : st)
    {
        c4::catsep_dump(dumper, buf, sep, _c4argbundle);
    }
    report(st, sz);
}

void catsepfile_c4catsepdump_lambda_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::catsep(buf, sep, _c4argbundle);
    dump2file::lambda_style dumper;
    auto lambda = [&dumper](c4::csubstr s) { fwrite(s.str, 1, s.len, dumper.subject); };
    for(auto _ : st)
    {
        c4::catsep_dump(lambda, buf, sep, _c4argbundle);
    }
    report(st, sz);
}

void catsepfile_fprintf(bm::State &st)
{
    char buf[256];
    size_t sz = c4::catsep(buf, sep, _c4argbundle);
    dump2file::cpp_style dumper;
    for(auto _ : st)
    {
        fprintf(dumper.subject, _c4argbundle_fmt_printf_sep, _c4argbundle_printf_sep);
    }
    report(st, sz);
}

void catsepfile_ofstream(bm::State &st)
{
    char buf[256];
    size_t sz = c4::catsep(buf, sep, _c4argbundle);
    std::ofstream ofs("ddofgufgbmn4g0rtglf", std::ios::out|std::ios::binary);
    for(auto _ : st)
    {
        ofs << _c4argbundle_lshift_sep;
    }
    report(st, sz);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void format_c4format(bm::State &st)
{
    char buf_[512];
    c4::substr buf(buf_);
    size_t sz = 0;
    for(auto _ : st)
    {
        sz = format(buf, _c4argbundle_fmt, _c4argbundle);
    }
    report(st, sz);
}

void format_c4formatrs_reuse(bm::State &st)
{
    std::string buf;
    size_t sz = 0;
    for(auto _ : st)
    {
        c4::formatrs(&buf, _c4argbundle_fmt, _c4argbundle);
        sz = buf.size();
    }
    report(st, sz);
}

void format_c4formatrs_no_reuse(bm::State &st)
{
    size_t sz = 0;
    for(auto _ : st)
    {
        auto buf = c4::formatrs<std::string>(_c4argbundle_fmt, _c4argbundle);
        sz = buf.size();
    }
    report(st, sz);
}

void format_c4formatdump_c_style_static_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::format(buf, _c4argbundle_fmt, _c4argbundle);
    for(auto _ : st)
    {
        c4::format_dump<&dump2str::c_style>(buf, _c4argbundle_fmt, _c4argbundle);
    }
    report(st, sz);
}

void format_c4formatdump_c_style_dynamic_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::format(buf, _c4argbundle_fmt, _c4argbundle);
    for(auto _ : st)
    {
        c4::format_dump(&dump2str::c_style, buf, _c4argbundle_fmt, _c4argbundle);
    }
    report(st, sz);
}

void format_c4formatdump_cpp_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::format(buf, _c4argbundle_fmt, _c4argbundle);
    dump2str::cpp_style dumper;
    for(auto _ : st)
    {
        c4::format_dump(dumper, buf, _c4argbundle_fmt, _c4argbundle);
    }
    report(st, sz);
}

void format_c4formatdump_lambda_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::format(buf, _c4argbundle_fmt, _c4argbundle);
    dump2str::lambda_style dumper;
    auto lambda = [&dumper](c4::csubstr s) { dumper.subject.append(s.str, s.len); };
    for(auto _ : st)
    {
        c4::format_dump(lambda, buf, _c4argbundle_fmt, _c4argbundle);
    }
    report(st, sz);
}

void format_snprintf(bm::State &st)
{
    char buf_[512];
    c4::substr buf(buf_);
    size_t sz = 0;
    for(auto _ : st)
    {
        sz = (size_t) snprintf(buf.str, buf.len, _c4argbundle_fmt_printf, _c4argbundle_printf);
    }
    report(st, sz);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void formatfile_c4formatdump_c_style_static_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    dump2file::cpp_style fileowner;
    dump2file::c_style_subject = fileowner.subject;
    size_t sz = c4::format(buf, _c4argbundle_fmt, _c4argbundle);
    for(auto _ : st)
    {
        c4::format_dump<&dump2file::c_style>(buf, _c4argbundle_fmt, _c4argbundle);
    }
    report(st, sz);
}

void formatfile_c4formatdump_c_style_dynamic_dispatch(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    dump2file::cpp_style fileowner;
    dump2file::c_style_subject = fileowner.subject;
    size_t sz = c4::format(buf, _c4argbundle_fmt, _c4argbundle);
    for(auto _ : st)
    {
        c4::format_dump(&dump2file::c_style, buf, _c4argbundle_fmt, _c4argbundle);
    }
    report(st, sz);
}

void formatfile_c4formatdump_cpp_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::format(buf, _c4argbundle_fmt, _c4argbundle);
    dump2file::cpp_style dumper;
    for(auto _ : st)
    {
        c4::format_dump(dumper, buf, _c4argbundle_fmt, _c4argbundle);
    }
    report(st, sz);
}

void formatfile_c4formatdump_lambda_style(bm::State &st)
{
    char buf_[256];
    c4::substr buf(buf_);
    size_t sz = c4::format(buf, _c4argbundle_fmt, _c4argbundle);
    dump2file::lambda_style dumper;
    auto lambda = [&dumper](c4::csubstr s) { fwrite(s.str, 1, s.len, dumper.subject); };
    for(auto _ : st)
    {
        c4::format_dump(lambda, buf, _c4argbundle_fmt, _c4argbundle);
    }
    report(st, sz);
}

void formatfile_fprintf(bm::State &st)
{
    char buf[256];
    size_t sz = c4::format(buf, _c4argbundle_fmt, _c4argbundle);
    dump2file::cpp_style dumper;
    for(auto _ : st)
    {
        fprintf(dumper.subject, _c4argbundle_fmt_printf, _c4argbundle_printf);
    }
    report(st, sz);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

C4BM(cat_c4cat_substr);
C4BM(cat_c4catrs_reuse);
C4BM(cat_c4catrs_no_reuse);
C4BM(cat_c4catdump_c_style_static_dispatch);
C4BM(cat_c4catdump_c_style_dynamic_dispatch);
C4BM(cat_c4catdump_cpp_style);
C4BM(cat_c4catdump_lambda_style);
C4BM(cat_std_to_string_reuse);
C4BM(cat_std_to_string_no_reuse);
C4BM(cat_stdsstream_reuse);
C4BM(cat_stdsstream_no_reuse);

C4BM(catfile_c4catdump_c_style_static_dispatch);
C4BM(catfile_c4catdump_c_style_dynamic_dispatch);
C4BM(catfile_c4catdump_cpp_style);
C4BM(catfile_c4catdump_lambda_style);
C4BM(catfile_fprintf);
C4BM(catfile_ofstream);


C4BM(catsep_c4cat_substr);
C4BM(catsep_c4catrs_reuse);
C4BM(catsep_c4catrs_no_reuse);
C4BM(catsep_c4catdump_c_style_static_dispatch);
C4BM(catsep_c4catdump_c_style_dynamic_dispatch);
C4BM(catsep_c4catdump_cpp_style);
C4BM(catsep_c4catdump_lambda_style);
C4BM(catsep_std_to_string_reuse);
C4BM(catsep_std_to_string_no_reuse);
C4BM(catsep_stdsstream_reuse);
C4BM(catsep_stdsstream_no_reuse);

C4BM(catsepfile_c4catsepdump_c_style_static_dispatch);
C4BM(catsepfile_c4catsepdump_c_style_dynamic_dispatch);
C4BM(catsepfile_c4catsepdump_cpp_style);
C4BM(catsepfile_c4catsepdump_lambda_style);
C4BM(catsepfile_fprintf);
C4BM(catsepfile_ofstream);


C4BM(format_c4format);
C4BM(format_c4formatrs_reuse);
C4BM(format_c4formatrs_no_reuse);
C4BM(format_c4formatdump_c_style_static_dispatch);
C4BM(format_c4formatdump_c_style_dynamic_dispatch);
C4BM(format_c4formatdump_cpp_style);
C4BM(format_c4formatdump_lambda_style);
C4BM(format_snprintf);

C4BM(formatfile_c4formatdump_c_style_static_dispatch);
C4BM(formatfile_c4formatdump_c_style_dynamic_dispatch);
C4BM(formatfile_c4formatdump_cpp_style);
C4BM(formatfile_c4formatdump_lambda_style);
C4BM(formatfile_fprintf);


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    bm::Initialize(&argc, argv);
    bm::RunSpecifiedBenchmarks();
    return 0;
}


#include <c4/c4_pop.hpp>


