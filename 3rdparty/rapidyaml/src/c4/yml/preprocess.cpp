#include "c4/yml/preprocess.hpp"
#include "c4/yml/detail/parser_dbg.hpp"

/** @file preprocess.hpp Functions for preprocessing YAML prior to parsing. */

namespace c4 {
namespace yml {

C4_SUPPRESS_WARNING_GCC_WITH_PUSH("-Wuseless-cast")

namespace {
struct _SubstrWriter
{
    substr buf;
    size_t pos;
    _SubstrWriter(substr buf_, size_t pos_=0) : buf(buf_), pos(pos_) {}
    void append(csubstr s)
    {
        C4_ASSERT(!s.overlaps(buf));
        if(pos + s.len <= buf.len)
            memcpy(buf.str + pos, s.str, s.len);
        pos += s.len;
    }
    void append(char c)
    {
        if(pos < buf.len)
            buf.str[pos] = c;
        ++pos;
    }
    size_t slack() const { return pos <= buf.len ? buf.len - pos : 0; }
    size_t excess() const { return pos > buf.len ? pos - buf.len : 0; }
    //! get the part written so far
    csubstr curr() const { return pos <= buf.len ? buf.first(pos) : buf; }
    //! get the part that is still free to write to
    substr rem() { return pos <= buf.len ? buf.sub(pos) : substr(buf.end(), size_t(0u)); }

    size_t advance(size_t more) { pos += more; return pos; }
};

// adapted from csubstr::pair_range_nested()
csubstr extract_json_container_range(csubstr s, char open, char close)
{
    RYML_ASSERT(s.begins_with(open));
    for(size_t curr = 1, count = 0; curr < s.len; ++curr)
    {
        char c = s[curr];
        if(c == open)
        {
            ++count;
        }
        else if(c == close)
        {
            if(count == 0)
                return s.first(curr+1);
            --count;
        }
        else if(c == '\'' || c == '"')  // consume quoted strings at once
        {
            csubstr ss = s.sub(curr).pair_range_esc(c, '\\');
            RYML_CHECK(ss.len > 0);
            curr += ss.len - 1;
        }
    }
    c4::yml::error("container range was opened but not closed");
    return {};
}

} // empty namespace

C4_SUPPRESS_WARNING_GCC_POP


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

size_t preprocess_json(csubstr s, substr buf)
{
    _SubstrWriter writer(buf);
    size_t last = 0; // the index of the last character in s that was copied to buf

    for(size_t i = 0; i < s.len; ++i)
    {
        // append everything that was not written yet
        #define _apfromlast() { csubstr _s_ = s.range(last, i); writer.append(_s_); last += _s_.len; }
        // append element from the buffer
        #define _apelm(c) { writer.append(c); ++last; }
        #define _adv(nsrc, ndst) { writer.advance(ndst); i += nsrc; last += nsrc; }
        const char curr = s[i];
        const char next = i+1 < s.len ? s[i+1] : '\0';
        if(curr == ':')  // if a space is missing after a semicolon, add it
        {
            bool insert = false;
            if(next == '"' || next == '\'' || next == '{' || next == '[' || (next >= '0' && next <= '9'))
            {
                insert = true;
            }
            else if(i+1 < s.len)
            {
                csubstr rem = s.sub(i+1);
                if(rem.begins_with("true") || rem.begins_with("false") || rem.begins_with("null"))
                    insert = true;
            }
            if(insert)
            {
                _apfromlast();
                _apelm(curr);
                writer.append(' ');
            }
        }
        else if((curr == '{' || curr == '[') && next != '\0') // recurse into substructures
        {
            // get the close-character matching the open-character.
            // In ascii: '{'=123,'}'=125 and '['=91,']'=93.
            // So just add 2!
            const char close = static_cast<char>(curr + 2);
            RYML_ASSERT((curr == '{' && close == '}') || (curr == '[' && close == ']'));
            // get the contents inside the brackets
            csubstr ss = extract_json_container_range(s.sub(i), curr, close);
            RYML_ASSERT(ss.size() >= 2);
            RYML_ASSERT(ss.ends_with(close));
            ss = ss.offs(1, 1); // skip the open-close bracket characters
            _apfromlast();
            _apelm(curr);
            if(!ss.empty())  // recurse into the substring
            {
                size_t ret = preprocess_json(ss, writer.rem());
                _adv(ss.len, ret);
            }
            _apelm(close);
        }
        else if(curr == '\'' || curr == '"')  // consume quoted strings at once
        {
            csubstr ss = s.sub(i).pair_range_esc(curr, '\\');
            RYML_ASSERT(ss.begins_with(curr) && ss.ends_with(curr));
            i += ss.len;
            _apfromlast();
            --i;
        }
        #undef _apfromlast
        #undef _apelm
        #undef _adv
    }

    if(last + 1 < s.len)
        writer.append(s.sub(last));

    return writer.pos;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace {
bool _is_idchar(char c)
{
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9')
        || (c == '_' || c == '-' || c == '~' || c == '$');
}

typedef enum { kReadPending = 0, kKeyPending = 1, kValPending = 2 } _ppstate;
_ppstate _next(_ppstate s)
{
    int n = (int)s + 1;
    return (_ppstate)(n <= (int)kValPending ? n : 0);
}
} // empty namespace


//-----------------------------------------------------------------------------

size_t preprocess_rxmap(csubstr s, substr buf)
{
    _SubstrWriter writer(buf);
    _ppstate state = kReadPending;
    size_t last = 0;

    if(s.begins_with('{'))
    {
        RYML_CHECK(s.ends_with('}'));
        s = s.offs(1, 1);
    }

    writer.append('{');

    for(size_t i = 0; i < s.len; ++i)
    {
        const char curr = s[i];
        const char next = i+1 < s.len ? s[i+1] : '\0';

        if(curr == '\'' || curr == '"')
        {
            csubstr ss = s.sub(i).pair_range_esc(curr, '\\');
            i += static_cast<size_t>(ss.end() - (s.str + i));
            state = _next(state);
        }
        else if(state == kReadPending && _is_idchar(curr))
        {
            state = _next(state);
        }

        switch(state)
        {
        case kKeyPending:
        {
            if(curr == ':' && next == ' ')
            {
                state = _next(state);
            }
            else if(curr == ',' && next == ' ')
            {
                writer.append(s.range(last, i));
                writer.append(": 1, ");
                last = i + 2;
            }
            break;
        }
        case kValPending:
        {
            if(curr == '[' || curr == '{' || curr == '(')
            {
                csubstr ss = s.sub(i).pair_range_nested(curr, '\\');
                i += static_cast<size_t>(ss.end() - (s.str + i));
                state = _next(state);
            }
            else if(curr == ',' && next == ' ')
            {
                state = _next(state);
            }
            break;
        }
        default:
            // nothing to do
            break;
        }
    }

    writer.append(s.sub(last));
    if(state == kKeyPending)
        writer.append(": 1");
    writer.append('}');

    return writer.pos;
}


} // namespace yml
} // namespace c4
