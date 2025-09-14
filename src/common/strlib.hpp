// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef STRLIB_HPP
#define STRLIB_HPP

#include <cstdarg>
#if !defined(__USE_GNU)
#define __USE_GNU  // required to enable strnlen on some platforms
#define __USED_GNU
#endif
#include <cstring>
#if defined(__USED_GNU)
#undef __USE_GNU
#undef __USED_GNU
#endif

#include "cbasetypes.hpp"
#include "malloc.hpp"

int32 remove_control_chars(char* str);
char* trim(char* str);
char* normalize_name(char* str,const char* delims);
const char *stristr(const char *haystack, const char *needle);

#ifdef WIN32
#define HAVE_STRTOK_R
#define strtok_r(s,delim,save_ptr) _strtok_r((s),(delim),(save_ptr))
char* _strtok_r(char* s1, const char* s2, char** lasts);
#endif

#if !(defined(WIN32) && defined(_MSC_VER) && _MSC_VER >= 1400) && !defined(HAVE_STRNLEN)
size_t strnlen (const char* string, size_t maxlen);
#endif

#if defined(WIN32) && defined(_MSC_VER) && _MSC_VER <= 1200
uint64 strtoull(const char* str, char** endptr, int32 base);
#endif

int32 e_mail_check(char* email);
int32 config_switch(const char* str);

/// strncpy that always nul-terminates the string
char* safestrncpy(char* dst, const char* src, size_t n);

/// doesn't crash on null pointer
size_t safestrnlen(const char* string, size_t maxlen);

/// Works like snprintf, but always nul-terminates the buffer.
/// Returns the size of the string (without nul-terminator)
/// or -1 if the buffer is too small.
int32 safesnprintf(char* buf, size_t sz, const char* fmt, ...);

/// Returns the line of the target position in the string.
/// Lines start at 1.
int32 strline(const char* str, size_t pos);

/// Produces the hexadecimal representation of the given input.
/// The output buffer must be at least count*2+1 in size.
/// Returns true on success, false on failure.
bool bin2hex(char* output, unsigned char* input, size_t count);


/// Bitfield determining the behaviour of sv_parse and sv_split.
typedef enum e_svopt
{
	// default: no escapes and no line terminator
	SV_NOESCAPE_NOTERMINATE = 0,
	// Escapes according to the C compiler.
	SV_ESCAPE_C = 1,
	// Line terminators
	SV_TERMINATE_LF = 2,
	SV_TERMINATE_CRLF = 4,
	SV_TERMINATE_CR = 8,
	// If sv_split keeps the end of line terminator, instead of replacing with '\0'
	SV_KEEP_TERMINATOR = 16
} e_svopt;

/// Other escape sequences supported by the C compiler.
#define SV_ESCAPE_C_SUPPORTED "abtnvfr\?\"'\\"

/// Parse state.
/// The field is [start,end[
struct s_svstate
{
	const char* str; //< string to parse
	size_t len; //< string length
	size_t off; //< current offset in the string
	size_t start; //< where the field starts
	size_t end; //< where the field ends
	int32 opt; //< parse options
	char delim; //< field delimiter
	bool done; //< if all the text has been parsed
};

/// Parses a single field in a delim-separated string.
/// The delimiter after the field is skipped.
///
/// @param sv Parse state
/// @return 1 if a field was parsed, 0 if done, -1 on error.
int32 sv_parse_next( s_svstate& sv );

/// Parses a delim-separated string.
/// Starts parsing at startoff and fills the pos array with position pairs.
/// out_pos[0] and out_pos[1] are the start and end of line.
/// Other position pairs are the start and end of fields.
/// Returns the number of fields found or -1 if an error occurs.
size_t sv_parse( const char* str, size_t len, size_t startoff, char delim, size_t* out_pos, size_t npos, int32 opt, bool& error );

/// Splits a delim-separated string.
/// WARNING: this function modifies the input string
/// Starts splitting at startoff and fills the out_fields array.
/// out_fields[0] is the start of the next line.
/// Other entries are the start of fields (nul-teminated).
/// Returns the number of fields found or -1 if an error occurs.
size_t sv_split( char* str, size_t len, size_t startoff, char delim, char** out_fields, size_t nfields, int32 opt, bool& error );

/// Escapes src to out_dest according to the format of the C compiler.
/// Returns the length of the escaped string.
/// out_dest should be len*4+1 in size.
size_t sv_escape_c(char* out_dest, const char* src, size_t len, const char* escapes);

/// Unescapes src to out_dest according to the format of the C compiler.
/// Returns the length of the unescaped string.
/// out_dest should be len+1 in size and can be the same buffer as src.
size_t sv_unescape_c(char* out_dest, const char* src, size_t len);

/// Skips a C escape sequence (starting with '\\').
const char* skip_escaped_c(const char* p);

/// Opens and parses a file containing delim-separated columns, feeding them to the specified callback function row by row.
/// Tracks the progress of the operation (current line number, number of successfully processed rows).
/// Returns 'true' if it was able to process the specified file, or 'false' if it could not be read.
bool sv_readdb( const char* directory, const char* filename, char delim, size_t mincols, size_t maxcols, size_t maxrows, bool (*parseproc)( char* fields[], size_t columns, size_t current ), bool silent );


/// StringBuf - dynamic string
struct StringBuf
{
	char *buf_;
	char *ptr_;
	size_t max_;

	StringBuf();
	~StringBuf();
};

StringBuf* _StringBuf_Malloc(const char *file, int32 line, const char *func);
#define StringBuf_Malloc() _StringBuf_Malloc(ALC_MARK)
void _StringBuf_Init(const char *file, int32 line, const char *func, StringBuf* self);
#define StringBuf_Init(self) _StringBuf_Init(ALC_MARK,self)
size_t _StringBuf_Printf( const char* file, int32 line, const char* func, StringBuf* self, const char* fmt, ... );
#define StringBuf_Printf(self,fmt,...) _StringBuf_Printf(ALC_MARK,self,fmt, ## __VA_ARGS__)
size_t _StringBuf_Vprintf( const char* file, int32 line, const char* func, StringBuf* self, const char* fmt, va_list args );
#define StringBuf_Vprintf(self,fmt,args) _StringBuf_Vprintf(ALC_MARK,self,fmt,args)
size_t _StringBuf_Append(const char *file, int32 line, const char *func, StringBuf* self, const StringBuf *sbuf);
#define StringBuf_Append(self,sbuf) _StringBuf_Append(ALC_MARK,self,sbuf)
size_t _StringBuf_AppendStr(const char *file, int32 line, const char *func, StringBuf* self, const char* str);
#define StringBuf_AppendStr(self,str) _StringBuf_AppendStr(ALC_MARK,self,str)
int32 StringBuf_Length(StringBuf* self);
char* StringBuf_Value(StringBuf* self);
void StringBuf_Clear(StringBuf* self);
void StringBuf_Free(StringBuf* self);

#endif /* STRLIB_HPP */
