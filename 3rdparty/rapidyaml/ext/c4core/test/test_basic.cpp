#include <c4/fs/fs.hpp>
#include <c4/std/std.hpp>
#include <c4/substr.hpp>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <stdlib.h>
#include <string>
#include <thread>

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable : 4996) // 'strncpy', fopen, etc: This function or variable may be unsafe
#elif defined(__clang__)
#elif defined(__GNUC__)
#endif

namespace c4 {
namespace fs {

struct ScopedTestFile
{
    char m_name[32];
    operator const char* () const { return m_name; }
    ScopedTestFile(const char* access="wb") : m_name()
    {
        tmpnam(m_name, sizeof(m_name), "scoped_file.XXXXXX.test");
        ::FILE *f = fopen(m_name, access);
        fclose(f);
    }
    ~ScopedTestFile()
    {
        c4::fs::rmfile(m_name);
    }
};

struct ScopedTestDir
{
    char m_name[32];
    operator const char* () const { return m_name; }
    ScopedTestDir() : m_name()
    {
        tmpnam(m_name, sizeof(m_name), "scoped_dir.XXXXXX.test");
        c4::fs::mkdir(m_name);
    }
    ~ScopedTestDir()
    {
        c4::fs::rmdir(m_name);
    }
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TEST_CASE("path_exists.file")
{
    std::string p;
    {
        auto file = ScopedTestFile();
        p.assign(file);
        CHECK(path_exists(file));
        CHECK(file_exists(file));
        CHECK_FALSE(dir_exists(file));
    }
    CHECK_FALSE(path_exists(p.c_str()));
    CHECK_FALSE(file_exists(p.c_str()));
    CHECK_FALSE(dir_exists(p.c_str()));
}

TEST_CASE("path_exists.dir")
{
    std::string p;
    {
        auto dir = ScopedTestDir();
        CHECK(path_exists(dir));
        CHECK_FALSE(file_exists(dir));
        CHECK(dir_exists(dir));
        p.assign(dir);
    }
    CHECK_FALSE(path_exists(p.c_str()));
    CHECK_FALSE(file_exists(p.c_str()));
    CHECK_FALSE(dir_exists(p.c_str()));
}

TEST_CASE("path_type.file")
{
    auto file = ScopedTestFile();
    CHECK_EQ(path_type(file), REGFILE);
    CHECK(is_file(file));
    CHECK(file_exists(file));
    CHECK_FALSE(is_dir(file));
}

TEST_CASE("path_type.dir")
{
    auto dir = ScopedTestDir();
    CHECK_EQ(path_type(dir), DIR);
    CHECK(is_dir(dir));
    CHECK(dir_exists(dir));
    CHECK(is_dir(dir));
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TEST_CASE("path_times")
{
    auto file = ScopedTestFile();
    auto t0 = times(file);
    CHECK_EQ(t0.creation, ctime(file));
    CHECK_EQ(t0.modification, mtime(file));
    CHECK_EQ(t0.access, atime(file));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    file_put_contents(file, csubstr("THE CONTENTS"));
    auto t1 = times(file);
    CHECK_EQ(t1.creation, ctime(file));
    CHECK_EQ(t1.modification, mtime(file));
    CHECK_EQ(t1.access, atime(file));
    #ifdef C4_WIN
    CHECK_GE(t1.creation, t0.creation);
    #else
    CHECK_GT(t1.creation, t0.creation);
    #endif
    CHECK_GT(t1.modification, t0.modification);
    // CHECK_GT(t1.access, t0.access); // not required by the system
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TEST_CASE("mkdir.basic")
{
    // setup
    if(dir_exists("c4fdx"))
        CHECK_EQ(rmtree("c4fdx"), 0);
    CHECK_FALSE(dir_exists("c4fdx"));
    CHECK_FALSE(dir_exists("c4fdx/a"));
    CHECK_FALSE(dir_exists("c4fdx/a/b"));
    // mkdir
    CHECK_EQ(mkdir("c4fdx"), 0);
    CHECK(dir_exists("c4fdx"));
    CHECK_FALSE(dir_exists("c4fdx/a"));
    CHECK_FALSE(dir_exists("c4fdx/a/b"));
    CHECK_EQ(mkdir("c4fdx/a"), 0);
    CHECK(dir_exists("c4fdx"));
    CHECK(dir_exists("c4fdx/a"));
    CHECK_FALSE(dir_exists("c4fdx/a/b"));
    CHECK_EQ(mkdir("c4fdx/a/b"), 0);
    CHECK(dir_exists("c4fdx"));
    CHECK(dir_exists("c4fdx/a"));
    CHECK(dir_exists("c4fdx/a/b"));
    CHECK_EQ(rmtree("c4fdx"), 0);
}

TEST_CASE("rmdir.basic")
{
    // setup
    if(dir_exists("c4fdx"))
        CHECK_EQ(rmtree("c4fdx"), 0);
    CHECK_EQ(mkdir("c4fdx"), 0);
    CHECK_EQ(mkdir("c4fdx/a"), 0);
    CHECK_EQ(mkdir("c4fdx/a/b"), 0);
    CHECK_EQ(rmdir("c4fdx/a/b"), 0);
    // rmdir
    CHECK(dir_exists("c4fdx"));
    CHECK(dir_exists("c4fdx/a"));
    CHECK_FALSE(dir_exists("c4fdx/a/b"));
    CHECK_EQ(rmdir("c4fdx/a"), 0);
    CHECK(dir_exists("c4fdx"));
    CHECK_FALSE(dir_exists("c4fdx/a"));
    CHECK_FALSE(dir_exists("c4fdx/a/b"));
    CHECK_EQ(rmdir("c4fdx"), 0);
    CHECK_FALSE(dir_exists("c4fdx"));
    CHECK_FALSE(dir_exists("c4fdx/a"));
    CHECK_FALSE(dir_exists("c4fdx/a/b"));
}

TEST_CASE("mkdirs.basic")
{
    if(dir_exists("c4fdx"))
        CHECK_EQ(rmtree("c4fdx"), 0);
    CHECK_FALSE(dir_exists("c4fdx"));
    CHECK_FALSE(dir_exists("c4fdx/a"));
    CHECK_FALSE(dir_exists("c4fdx/a/b"));
    CHECK_FALSE(dir_exists("c4fdx/a/b/c"));
    char buf[32] = "c4fdx/a/b/c\0";
    mkdirs(buf);
    CHECK(dir_exists("c4fdx"));
    CHECK(dir_exists("c4fdx/a"));
    CHECK(dir_exists("c4fdx/a/b"));
    CHECK(dir_exists("c4fdx/a/b/c"));
    CHECK_EQ(rmtree("c4fdx"), 0);
    CHECK_FALSE(dir_exists("c4fdx"));
}

TEST_CASE("rmfile")
{
    SUBCASE("existing")
    {
        const char filename[] = "adslkjasdlkj";
        file_put_contents(filename, csubstr("THE CONTENTS"));
        CHECK(file_exists(filename));
        CHECK_EQ(rmfile(filename), 0);
        CHECK(!file_exists(filename));
    }
    SUBCASE("nonexisting")
    {
        const char filename[] = "adslkjasdlkj";
        CHECK(!file_exists(filename));
        CHECK_NE(rmfile(filename), 0);
    }
}

const char * _make_tree()
{
    if(dir_exists("c4fdx"))
        CHECK_EQ(rmtree("c4fdx"), 0);
    auto fpcon = [](const char* path){
        file_put_contents(path, to_csubstr(path));
        CHECK(file_exists(path));
    };
    mkdir("c4fdx");
    fpcon("c4fdx/file1");
    fpcon("c4fdx/file2");
    mkdir("c4fdx/a");
    fpcon("c4fdx/a/file1");
    fpcon("c4fdx/a/file2");
    mkdir("c4fdx/a/1");
    fpcon("c4fdx/a/1/file1");
    fpcon("c4fdx/a/1/file2");
    mkdir("c4fdx/a/1/a");
    fpcon("c4fdx/a/1/a/file1");
    fpcon("c4fdx/a/1/a/file2");
    mkdir("c4fdx/a/1/b");
    fpcon("c4fdx/a/1/b/file1");
    fpcon("c4fdx/a/1/b/file2");
    mkdir("c4fdx/a/1/c");
    fpcon("c4fdx/a/1/c/file1");
    fpcon("c4fdx/a/1/c/file2");
    mkdir("c4fdx/a/2");
    fpcon("c4fdx/a/2/file1");
    fpcon("c4fdx/a/2/file2");
    mkdir("c4fdx/a/2/a");
    fpcon("c4fdx/a/2/a/file1");
    fpcon("c4fdx/a/2/a/file2");
    mkdir("c4fdx/a/2/b");
    fpcon("c4fdx/a/2/b/file1");
    fpcon("c4fdx/a/2/b/file2");
    mkdir("c4fdx/a/2/c");
    fpcon("c4fdx/a/2/c/file1");
    fpcon("c4fdx/a/2/c/file2");
    mkdir("c4fdx/b");
    fpcon("c4fdx/b/file1");
    fpcon("c4fdx/b/file2");
    mkdir("c4fdx/c");
    fpcon("c4fdx/c/file1");
    fpcon("c4fdx/c/file2");
    return "c4fdx";
}

TEST_CASE("rmtree")
{
    if(dir_exists("c4fdx"))
        CHECK_EQ(rmtree("c4fdx"), 0);
    SUBCASE("existing")
    {
        CHECK(!dir_exists("c4fdx"));
        auto treename = _make_tree();
        CHECK(dir_exists(treename));
        CHECK_EQ(rmtree(treename), 0);
        CHECK(!dir_exists(treename));
    }
    SUBCASE("nonexisting")
    {
        CHECK(!dir_exists("nonexisting"));
        CHECK_NE(rmtree("nonexisting"), 0);
    }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

u32 file_count = 0;
u32 dir_count = 0;
int entry_visitor(VisitedFile const& p)
{
    std::cout << p.name << std::endl;
    CHECK(path_exists(p.name));
    if(is_file(p.name))
        ++file_count;
    if(is_dir(p.name))
        ++dir_count;
    return 0;
}

TEST_CASE("walk_entries")
{
    const std::string cwd_orig = cwd<std::string>();
    constexpr const char dirname[] = "c4fdx";
    CHECK_EQ(mkdir(dirname), 0);
    file_put_contents("c4fdx/file0", csubstr("asdasdasd"));
    file_put_contents("c4fdx/file1", csubstr("dsfgsdfds"));
    SUBCASE("empty_name_buffer")
    {
        dir_count = file_count = 0;
        maybe_buf<char> buf;
        bool ok = walk_entries(dirname, entry_visitor, &buf);
        CHECK(!ok);
        CHECK_EQ(file_count, 0);
        CHECK_EQ(dir_count, 0);
    }
    SUBCASE("small_name_buffer")
    {
        char buf_[sizeof(dirname) + 3]; // not enough for the children files
        maybe_buf<char> buf(buf_);
        dir_count = file_count = 0;
        bool ok = walk_entries(dirname, entry_visitor, &buf);
        CHECK(!ok);
        CHECK_EQ(file_count, 0);
        CHECK_EQ(dir_count, 0);
    }
    SUBCASE("vanilla")
    {
        char buf_[100];
        maybe_buf<char> buf(buf_);
        dir_count = file_count = 0;
        bool ok = walk_entries(dirname, entry_visitor, &buf);
        CHECK(ok);
        CHECK_EQ(file_count, 2);
        CHECK_EQ(dir_count, 0);
    }
    SUBCASE("dont_descend_into_subdirs")
    {
        mkdir("c4fdx/dir");
        file_put_contents("c4fdx/dir/file2", csubstr("asdasdasd"));
        file_put_contents("c4fdx/dir/file3", csubstr("asdasdasd"));
        mkdir("c4fdx/dir2");
        file_put_contents("c4fdx/dir2/file4", csubstr("asdasdasd"));
        file_put_contents("c4fdx/dir2/file5", csubstr("asdasdasd"));
        dir_count = file_count = 0;
        char buf_[100];
        maybe_buf<char> buf(buf_);
        bool ok = walk_entries(dirname, entry_visitor, &buf);
        CHECK(ok);
        CHECK_EQ(file_count, 2); // must not have changed
        CHECK_EQ(dir_count, 2); // but must see the new subdirs
    }
    CHECK_EQ(rmtree(dirname), 0);
    CHECK_EQ(cwd<std::string>(), cwd_orig);
}

TEST_CASE("list_entries")
{
    char dirname[] = "c4fdx/dir\0";
    mkdirs(dirname);
    file_put_contents("c4fdx/dir/file09", csubstr("asdasdasd"));
    file_put_contents("c4fdx/dir/file00", csubstr("asdasdasd"));
    file_put_contents("c4fdx/dir/file08", csubstr("asdasdasd"));
    file_put_contents("c4fdx/dir/file01", csubstr("asdasdasd"));
    file_put_contents("c4fdx/dir/file07", csubstr("asdasdasd"));
    file_put_contents("c4fdx/dir/file02", csubstr("asdasdasd"));
    file_put_contents("c4fdx/dir/file06", csubstr("asdasdasd"));
    file_put_contents("c4fdx/dir/file03", csubstr("asdasdasd"));
    file_put_contents("c4fdx/dir/file05", csubstr("asdasdasd"));
    file_put_contents("c4fdx/dir/file04", csubstr("asdasdasd"));
    size_t num_files = 10u;
    size_t arena_size = (num_files + 1u) * strlen("c4fdx/dir/file00");
    SUBCASE("no_buffer")
    {
        maybe_buf<char> scratch;
        EntryList el = {};
        bool ok = list_entries("c4fdx/dir", &el, &scratch);
        CHECK(!ok);
        CHECK_EQ(el.names.required_size, 0u);
        CHECK_GE(el.arena.required_size, 0u);
        CHECK_GT(scratch.required_size, strlen("c4fdx/dir/"));
        CHECK(el.arena.valid());
        CHECK(el.names.valid());
        CHECK(el.valid());
        CHECK(!scratch.valid());
    }
    SUBCASE("buffer_ok")
    {
        char namebuf[1000] = {};
        char *namesbuf[20] = {};
        char scratchbuf[256] = {};
        maybe_buf<char> scratch(scratchbuf);
        EntryList el(namebuf, namesbuf);
        bool ok = list_entries("c4fdx/dir", &el, &scratch);
        CHECK(ok);
        CHECK(el.arena.valid());
        CHECK(el.names.valid());
        CHECK(el.valid());
        CHECK(scratch.valid());
        CHECK_LE(el.arena.required_size, arena_size);
        REQUIRE_EQ(el.names.required_size, num_files);
        std::sort(namesbuf, namesbuf+num_files,
                  [](const char *lhs, const char *rhs){
                      return strcmp(lhs, rhs) < 0;
                  });
        CHECK_EQ(to_csubstr(namesbuf[0]), csubstr("c4fdx/dir/file00"));
        CHECK_EQ(to_csubstr(namesbuf[1]), csubstr("c4fdx/dir/file01"));
        CHECK_EQ(to_csubstr(namesbuf[2]), csubstr("c4fdx/dir/file02"));
        CHECK_EQ(to_csubstr(namesbuf[3]), csubstr("c4fdx/dir/file03"));
        CHECK_EQ(to_csubstr(namesbuf[4]), csubstr("c4fdx/dir/file04"));
        CHECK_EQ(to_csubstr(namesbuf[5]), csubstr("c4fdx/dir/file05"));
        CHECK_EQ(to_csubstr(namesbuf[6]), csubstr("c4fdx/dir/file06"));
        CHECK_EQ(to_csubstr(namesbuf[7]), csubstr("c4fdx/dir/file07"));
        CHECK_EQ(to_csubstr(namesbuf[8]), csubstr("c4fdx/dir/file08"));
        CHECK_EQ(to_csubstr(namesbuf[9]), csubstr("c4fdx/dir/file09"));
    }
    SUBCASE("iter_entry_list")
    {
        char namebuf[1000] = {};
        char *namesbuf[20] = {};
        char scratchbuf[256] = {};
        maybe_buf<char> scratch(scratchbuf);
        EntryList el(namebuf, namesbuf);
        bool ok = list_entries("c4fdx/dir", &el, &scratch);
        CHECK(ok);
        CHECK(el.arena.valid());
        CHECK(el.names.valid());
        CHECK(el.valid());
        CHECK(scratch.valid());
        CHECK_LT(el.arena.required_size, arena_size);
        REQUIRE_EQ(el.names.required_size, num_files);
        el.sort();
        CHECK_EQ(to_csubstr(namesbuf[0]), csubstr("c4fdx/dir/file00"));
        CHECK_EQ(to_csubstr(namesbuf[1]), csubstr("c4fdx/dir/file01"));
        CHECK_EQ(to_csubstr(namesbuf[2]), csubstr("c4fdx/dir/file02"));
        CHECK_EQ(to_csubstr(namesbuf[3]), csubstr("c4fdx/dir/file03"));
        CHECK_EQ(to_csubstr(namesbuf[4]), csubstr("c4fdx/dir/file04"));
        CHECK_EQ(to_csubstr(namesbuf[5]), csubstr("c4fdx/dir/file05"));
        CHECK_EQ(to_csubstr(namesbuf[6]), csubstr("c4fdx/dir/file06"));
        CHECK_EQ(to_csubstr(namesbuf[7]), csubstr("c4fdx/dir/file07"));
        CHECK_EQ(to_csubstr(namesbuf[8]), csubstr("c4fdx/dir/file08"));
        CHECK_EQ(to_csubstr(namesbuf[9]), csubstr("c4fdx/dir/file09"));
    }
    rmtree("c4fdx");
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int path_visitor(VisitedPath const& p)
{
    CHECK(path_exists(p.name));
    if(is_file(p.name))
        ++file_count;
    if(is_dir(p.name))
        ++dir_count;
    return 0;
}

TEST_CASE("walk_tree")
{
    SUBCASE("existing")
    {
        auto treename = _make_tree();
        CHECK(dir_exists(treename));
        int ret = walk_tree(treename, path_visitor);
        CHECK(ret == 0);
        CHECK_EQ(file_count, 26);
        CHECK_EQ(dir_count, 14);
        CHECK_EQ(rmtree(treename), 0);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

csubstr test_contents = R"(
0
1
2
3
4
5
6
7
8
9
10
\0
)";

TEST_CASE("ScopedTmpFile.c_str")
{
    auto wfile = ScopedTmpFile(test_contents.str, test_contents.len);
    CHECK_EQ(file_size(wfile.m_name), test_contents.len);

    std::string out = file_get_contents<std::string>(wfile.m_name);
    CHECK_EQ(out.size(), test_contents.len);
    CHECK_EQ(to_csubstr(out), test_contents);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TEST_CASE("file_put_contents.basic")
{
    std::string filename_buf = tmpnam<std::string>();
    const char *filename = filename_buf.c_str();

    file_put_contents(filename, test_contents);
    CHECK_EQ(file_size(filename), test_contents.len);
    std::string cmp = file_get_contents<std::string>(filename);
    CHECK_EQ(to_csubstr(cmp), test_contents);
}

TEST_CASE("file_get_contents.std_string")
{
    auto wfile = ScopedTmpFile(test_contents.str, test_contents.len);
    std::string s = file_get_contents<std::string>(wfile.m_name);
    CHECK_EQ(to_csubstr(s), test_contents);
}

TEST_CASE("file_get_contents.std_vector_char")
{
    auto wfile = ScopedTmpFile(test_contents.str, test_contents.len);
    std::vector<char> s = file_get_contents<std::vector<char>>(wfile.m_name);
    CHECK_EQ(to_csubstr(s), test_contents);
}


} // namespace fs
} // namespace c4

#ifdef _MSC_VER
#   pragma warning(pop)
#elif defined(__clang__)
#elif defined(__GNUC__)
#endif
