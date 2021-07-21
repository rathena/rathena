#ifndef _C4_YML_COMMON_HPP_
#define _C4_YML_COMMON_HPP_

#include <cstddef>
#include <c4/substr.hpp>
#include <c4/yml/export.hpp>


#define RYML_INLINE inline


#ifndef RYML_USE_ASSERT
#   define RYML_USE_ASSERT C4_USE_ASSERT
#endif


#if RYML_USE_ASSERT
#   define RYML_ASSERT(cond) RYML_CHECK(cond)
#   define RYML_ASSERT_MSG(cond, msg) RYML_CHECK_MSG(cond, msg)
#else
#   define RYML_ASSERT(cond)
#   define RYML_ASSERT_MSG(cond, msg)
#endif


#define RYML_CHECK(cond)                                                \
    do {                                                                \
        if(!(cond))                                                     \
        {                                                               \
            C4_DEBUG_BREAK();                                           \
            c4::yml::error("check failed: " #cond, c4::yml::Location(__FILE__, __LINE__, 0)); \
        }                                                               \
    } while(0)

#define RYML_CHECK_MSG(cond, msg)                                       \
    do                                                                  \
    {                                                                   \
        if(!(cond))                                                     \
        {                                                               \
            C4_DEBUG_BREAK();                                           \
            c4::yml::error(msg ": check failed: " #cond, c4::yml::Location(__FILE__, __LINE__, 0)); \
        }                                                               \
    } while(0)


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace c4 {
namespace yml {

enum : size_t {
    /** a null position */
    npos = size_t(-1),
    /** an index to none */
    NONE = size_t(-1)
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//! holds a position into a source buffer
struct RYML_EXPORT LineCol
{
    //!< number of bytes from the beginning of the source buffer
    size_t offset;
    //!< line
    size_t line;
    //!< column
    size_t col;

    LineCol() : offset(), line(), col() {}
    //! construct from line and column
    LineCol(size_t l, size_t c) : offset(0), line(l), col(c) {}
    //! construct from offset, line and column
    LineCol(size_t o, size_t l, size_t c) : offset(o), line(l), col(c) {}
};


//! a source file position
struct RYML_EXPORT Location : public LineCol
{
    csubstr name;

    operator bool () const { return !name.empty() || line != 0 || offset != 0; }

    Location() : LineCol(), name() {}
    Location(                         size_t l, size_t c) : LineCol{   l, c}, name( ) {}
    Location(    csubstr n,           size_t l, size_t c) : LineCol{   l, c}, name(n) {}
    Location(    csubstr n, size_t b, size_t l, size_t c) : LineCol{b, l, c}, name(n) {}
    Location(const char *n,           size_t l, size_t c) : LineCol{   l, c}, name(to_csubstr(n)) {}
    Location(const char *n, size_t b, size_t l, size_t c) : LineCol{b, l, c}, name(to_csubstr(n)) {}
};


//-----------------------------------------------------------------------------

/** the type of the function used to report errors. This function must
 * interrupt execution, either by raising an exception or calling
 * std::abort(). */
using pfn_error = void (*)(const char* msg, size_t msg_len, Location location, void *user_data);

/** trigger an error: call the current error callback. */
RYML_EXPORT void error(const char *msg, size_t msg_len, Location loc);
/** @overload error */
inline void error(const char *msg, size_t msg_len)
{
    error(msg, msg_len, Location{});
}
/** @overload error */
template<size_t N>
inline void error(const char (&msg)[N], Location loc)
{
    error(msg, N-1, loc);
}
/** @overload error */
template<size_t N>
inline void error(const char (&msg)[N])
{
    error(msg, N-1, Location{});
}

//-----------------------------------------------------------------------------

/** the type of the function used to allocate memory */
using pfn_allocate = void* (*)(size_t len, void* hint, void *user_data);
/** the type of the function used to free memory */
using pfn_free = void (*)(void* mem, size_t size, void *user_data);

/// a c-style callbacks class
struct RYML_EXPORT Callbacks
{
    void *       m_user_data;
    pfn_allocate m_allocate;
    pfn_free     m_free;
    pfn_error    m_error;

    Callbacks();
    Callbacks(void *user_data, pfn_allocate alloc, pfn_free free, pfn_error error_);

    inline void* allocate(size_t len, void* hint) const
    {
        void* mem = m_allocate(len, hint, m_user_data);
        if(mem == nullptr)
        {
            this->error("out of memory", {});
        }
        return mem;
    }

    inline void free(void *mem, size_t len) const
    {
        m_free(mem, len, m_user_data);
    }

    void error(const char *msg, size_t msg_len, Location loc) const
    {
        m_error(msg, msg_len, loc, m_user_data);
    }

    void error(const char *msg, size_t msg_len) const
    {
        m_error(msg, msg_len, {}, m_user_data);
    }

    template<size_t N>
    inline void error(const char (&msg)[N], Location loc) const
    {
        error(msg, N-1, loc);
    }

    template<size_t N>
    inline void error(const char (&msg)[N]) const
    {
        error(msg, N-1, {});
    }

};

/// get the global callbacks
RYML_EXPORT Callbacks const& get_callbacks();
/// set the global callbacks
RYML_EXPORT void set_callbacks(Callbacks const& c);
/// set the global callbacks to their defaults
RYML_EXPORT void reset_callbacks();


//-----------------------------------------------------------------------------

class RYML_EXPORT MemoryResource
{
public:

    virtual ~MemoryResource() = default;

    virtual void * allocate(size_t num_bytes, void *hint) = 0;
    virtual void   free(void *mem, size_t num_bytes) = 0;
};

/** set the global memory resource */
RYML_EXPORT void set_memory_resource(MemoryResource *r);
/** get the global memory resource */
RYML_EXPORT MemoryResource *get_memory_resource();


//-----------------------------------------------------------------------------

/** a memory resource adapter to the c-style allocator */
class RYML_EXPORT MemoryResourceCallbacks : public MemoryResource
{
public:

    Callbacks m_callbacks;

    MemoryResourceCallbacks() : m_callbacks(get_callbacks()) {}
    MemoryResourceCallbacks(Callbacks const& c) : m_callbacks(c) {}

    void* allocate(size_t len, void* hint) override final
    {
        return m_callbacks.allocate(len, hint);
    }

    void free(void *mem, size_t len) override final
    {
        return m_callbacks.free(mem, len);
    }
};


//-----------------------------------------------------------------------------

/** an allocator is a lightweight non-owning handle to a memory resource */
struct RYML_EXPORT Allocator
{
    MemoryResource *r;

    Allocator() : r(get_memory_resource()) {}
    Allocator(MemoryResource *m) : r(m) {}

    inline void *allocate(size_t num_bytes, void *hint)
    {
        void *mem = r->allocate(num_bytes, hint);
        if(mem == nullptr)
            error("out of memory");
        return mem;
    }

    inline void free(void *mem, size_t num_bytes)
    {
        RYML_ASSERT(r != nullptr);
        r->free(mem, num_bytes);
    }
};


} // namespace yml
} // namespace c4

#endif /* _C4_YML_COMMON_HPP_ */
