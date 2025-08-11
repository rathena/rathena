// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "strlib.hpp"

#include <algorithm>
#include <cstdlib>

#include "cbasetypes.hpp"
#include "malloc.hpp"
#include "showmsg.hpp"

// Function to suppress control characters in a string.
int32 remove_control_chars(char* str)
{
	int32 i;
	int32 change = 0;

	for(i = 0; str[i]; i++) {
		if (ISCNTRL(str[i])) {
			str[i] = '_';
			change = 1;
		}
	}

	return change;
}

// Removes characters identified by ISSPACE from the start and end of the string
// NOTE: make sure the string is not const!!
char* trim(char* str)
{
	size_t start;
	size_t end;

	if( str == nullptr )
		return str;

	// get start position
	for( start = 0; str[start] && ISSPACE(str[start]); ++start )
		;
	// get end position
	for( end = strlen(str); start < end && str[end-1] && ISSPACE(str[end-1]); --end )
		;
	// trim
	if( start == end )
		*str = '\0';// empty string
	else
	{// move string with nul terminator
		str[end] = '\0';
		memmove(str,str+start,end-start+1);
	}
	return str;
}

// Converts one or more consecutive occurences of the delimiters into a single space
// and removes such occurences from the beginning and end of string
// NOTE: make sure the string is not const!!
char* normalize_name(char* str,const char* delims)
{
	char* in = str;
	char* out = str;
	int32 put_space = 0;

	if( str == nullptr || delims == nullptr )
		return str;

	// trim start of string
	while( *in && strchr(delims,*in) )
		++in;
	while( *in )
	{
		if( put_space )
		{// replace trim characters with a single space
			*out = ' ';
			++out;
		}
		// copy non trim characters
		while( *in && !strchr(delims,*in) )
		{
			*out = *in;
			++out;
			++in;
		}
		// skip trim characters
		while( *in && strchr(delims,*in) )
			++in;
		put_space = 1;
	}
	*out = '\0';
	return str;
}

//stristr: Case insensitive version of strstr, code taken from
//http://www.daniweb.com/code/snippet313.html, Dave Sinkula
//
const char* stristr(const char* haystack, const char* needle)
{
	if ( !*needle )
	{
		return haystack;
	}
	for ( ; *haystack; ++haystack )
	{
		if ( TOUPPER(*haystack) == TOUPPER(*needle) )
		{
			// matched starting char -- loop through remaining chars
			const char *h, *n;
			for ( h = haystack, n = needle; *h && *n; ++h, ++n )
			{
				if ( TOUPPER(*h) != TOUPPER(*n) )
				{
					break;
				}
			}
			if ( !*n ) // matched all of 'needle' to null termination
			{
				return haystack; // return the start of the match
			}
		}
	}
	return 0;
}

#ifdef __WIN32
char* _strtok_r(char *s1, const char *s2, char **lasts)
{
	char *ret;

	if (s1 == nullptr)
		s1 = *lasts;
	while(*s1 && strchr(s2, *s1))
		++s1;
	if(*s1 == '\0')
		return nullptr;
	ret = s1;
	while(*s1 && !strchr(s2, *s1))
		++s1;
	if(*s1)
		*s1++ = '\0';
	*lasts = s1;
	return ret;
}
#endif

#if !(defined(WIN32) && defined(_MSC_VER) && _MSC_VER >= 1400) && !defined(HAVE_STRNLEN)
/* Find the length of STRING, but scan at most MAXLEN characters.
   If no '\0' terminator is found in that many characters, return MAXLEN.  */
size_t strnlen (const char* string, size_t maxlen)
{
  const char* end = (const char*)memchr(string, '\0', maxlen);
  return end ? (size_t) (end - string) : maxlen;
}
#endif

#if defined(WIN32) && defined(_MSC_VER) && _MSC_VER <= 1200
uint64 strtoull(const char* str, char** endptr, int32 base)
{
	uint64 result;
	int32 count;
	int32 n;

	if( base == 0 )
	{
		if( str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
			base = 16;
		else
		if( str[0] == '0' )
			base = 8;
		else
			base = 10;
	}

	if( base == 8 )
		count = sscanf(str, "%I64o%n", &result, &n);
	else
	if( base == 10 )
		count = sscanf(str, "%I64u%n", &result, &n);
	else
	if( base == 16 )
		count = sscanf(str, "%I64x%n", &result, &n);
	else
		count = 0; // fail

	if( count < 1 )
	{
		errno = EINVAL;
		result = 0;
		n = 0;
	}

	if( endptr )
		*endptr = (char*)str + n;

	return result;
}
#endif

//----------------------------------------------------
// E-mail check: return 0 (not correct) or 1 (valid).
//----------------------------------------------------
int32 e_mail_check(char* email)
{
	char ch;
	char* last_arobas;
	size_t len = strlen(email);

	// athena limits
	if (len < 3 || len > 39)
		return 0;

	// part of RFC limits (official reference of e-mail description)
	if (strchr(email, '@') == nullptr || email[len-1] == '@')
		return 0;

	if (email[len-1] == '.')
		return 0;

	last_arobas = strrchr(email, '@');

	if (strstr(last_arobas, "@.") != nullptr || strstr(last_arobas, "..") != nullptr)
		return 0;

	for(ch = 1; ch < 32; ch++)
		if (strchr(last_arobas, ch) != nullptr)
			return 0;

	if (strchr(last_arobas, ' ') != nullptr || strchr(last_arobas, ';') != nullptr)
		return 0;

	// all correct
	return 1;
}

//--------------------------------------------------
// Return numerical value of a switch configuration
// on/off, english, fran�ais, deutsch, espa�ol, portuguese
//--------------------------------------------------
int32 config_switch(const char* str)
{
	if (strcmpi(str, "on") == 0 || strcmpi(str, "yes") == 0 || strcmpi(str, "oui") == 0 || strcmpi(str, "ja") == 0 || strcmpi(str, "si") == 0 || strcmpi(str, "sim") == 0)
		return 1;
	if (strcmpi(str, "off") == 0 || strcmpi(str, "no") == 0 || strcmpi(str, "non") == 0 || strcmpi(str, "nein") == 0 || strcmpi(str, "nao") == 0)
		return 0;

	return (int32)strtol(str, nullptr, 0);
}

/// strncpy that always nul-terminates the string
char* safestrncpy(char* dst, const char* src, size_t n)
{
	if( n > 0 )
	{
		char* d = dst;
		const char* s = src;
		d[--n] = '\0';/* nul-terminate string */
		for( ; n > 0; --n )
		{
			if( (*d++ = *s++) == '\0' )
			{/* nul-pad remaining bytes */
				while( --n > 0 )
					*d++ = '\0';
				break;
			}
		}
	}
	return dst;
}

/// doesn't crash on null pointer
size_t safestrnlen(const char* string, size_t maxlen)
{
	return ( string != nullptr ) ? strnlen(string, maxlen) : 0;
}

/// Works like snprintf, but always nul-terminates the buffer.
/// Returns the size of the string (without nul-terminator)
/// or -1 if the buffer is too small.
///
/// @param buf Target buffer
/// @param sz Size of the buffer (including nul-terminator)
/// @param fmt Format string
/// @param ... Format arguments
/// @return The size of the string or -1 if the buffer is too small
int32 safesnprintf(char* buf, size_t sz, const char* fmt, ...)
{
	va_list ap;
	int32 ret;

	va_start(ap,fmt);
	ret = vsnprintf(buf, sz, fmt, ap);
	va_end(ap);
	if( ret < 0 || (size_t)ret >= sz )
	{// overflow
		buf[sz-1] = '\0';// always nul-terminate
		return -1;
	}
	return ret;
}

/// Returns the line of the target position in the string.
/// Lines start at 1.
int32 strline(const char* str, size_t pos)
{
	const char* target;
	int32 line;

	if( str == nullptr || pos == 0 )
		return 1;

	target = str+pos;
	for( line = 1; ; ++line )
	{
		str = strchr(str, '\n');
		if( str == nullptr || target <= str )
			break;// found target line
		++str;// skip newline
	}
	return line;
}

/// Produces the hexadecimal representation of the given input.
/// The output buffer must be at least count*2+1 in size.
/// Returns true on success, false on failure.
///
/// @param output Output string
/// @param input Binary input buffer
/// @param count Number of bytes to convert
bool bin2hex(char* output, unsigned char* input, size_t count)
{
	char toHex[] = "0123456789abcdef";
	size_t i;

	for( i = 0; i < count; ++i )
	{
		*output++ = toHex[(*input & 0xF0) >> 4];
		*output++ = toHex[(*input & 0x0F) >> 0];
		++input;
	}
	*output = '\0';
	return true;
}



/////////////////////////////////////////////////////////////////////
/// Parses a single field in a delim-separated string.
/// The delimiter after the field is skipped.
///
/// @param sv Parse state
/// @return 1 if a field was parsed, 0 if already done, -1 on error.
int32 sv_parse_next( s_svstate& sv ){
	enum {
		START_OF_FIELD,
		PARSING_FIELD,
		PARSING_C_ESCAPE,
		END_OF_FIELD,
		TERMINATE,
		END
	} state;

	const char* str = sv.str;
	size_t len = sv.len;
	int32 opt = sv.opt;
	char delim = sv.delim;

	// check opt
	if( delim == '\n' && (opt&(SV_TERMINATE_CRLF|SV_TERMINATE_LF)) )
	{
		ShowError("sv_parse_next: delimiter '\\n' is not compatible with options SV_TERMINATE_LF or SV_TERMINATE_CRLF.\n");
		return -1;// error
	}
	if( delim == '\r' && (opt&(SV_TERMINATE_CRLF|SV_TERMINATE_CR)) )
	{
		ShowError("sv_parse_next: delimiter '\\r' is not compatible with options SV_TERMINATE_CR or SV_TERMINATE_CRLF.\n");
		return -1;// error
	}

	if( sv.done || str == nullptr )
	{
		sv.done = true;
		return 0;// nothing to parse
	}

#define IS_END() ( i >= len )
#define IS_DELIM() ( str[i] == delim )
#define IS_TERMINATOR() ( \
	((opt&SV_TERMINATE_LF) && str[i] == '\n') || \
	((opt&SV_TERMINATE_CR) && str[i] == '\r') || \
	((opt&SV_TERMINATE_CRLF) && i+1 < len && str[i] == '\r' && str[i+1] == '\n') )
#define IS_C_ESCAPE() ( (opt&SV_ESCAPE_C) && str[i] == '\\' )
#define SET_FIELD_START() sv.start = i
#define SET_FIELD_END() sv.end = i

	size_t i = sv.off;
	state = START_OF_FIELD;
	while( state != END )
	{
		switch( state )
		{
		case START_OF_FIELD:// record start of field and start parsing it
			SET_FIELD_START();
			state = PARSING_FIELD;
			break;

		case PARSING_FIELD:// skip field character
			if( IS_END() || IS_DELIM() || IS_TERMINATOR() )
				state = END_OF_FIELD;
			else if( IS_C_ESCAPE() )
				state = PARSING_C_ESCAPE;
			else
				++i;// normal character
			break;

		case PARSING_C_ESCAPE:// skip escape sequence (validates it too)
			{
				++i;// '\\'
				if( IS_END() )
				{
					ShowError("sv_parse_next: empty escape sequence\n");
					return -1;
				}
				if( str[i] == 'x' )
				{// hex escape
					++i;// 'x'
					if( IS_END() || !ISXDIGIT(str[i]) )
					{
						ShowError("sv_parse_next: \\x with no following hex digits\n");
						return -1;
					}
					do{
						++i;// hex digit
					}while( !IS_END() && ISXDIGIT(str[i]));
				}
				else if( str[i] == '0' || str[i] == '1' || str[i] == '2' )
				{// octal escape
					++i;// octal digit
					if( !IS_END() && str[i] >= '0' && str[i] <= '7' )
						++i;// octal digit
					if( !IS_END() && str[i] >= '0' && str[i] <= '7' )
						++i;// octal digit
				}
				else if( strchr(SV_ESCAPE_C_SUPPORTED, str[i]) )
				{// supported escape character
					++i;
				}
				else
				{
					ShowError("sv_parse_next: unknown escape sequence \\%c\n", str[i]);
					return -1;
				}
				state = PARSING_FIELD;
				break;
			}

		case END_OF_FIELD:// record end of field and stop
			SET_FIELD_END();
			state = END;
			if( IS_END() )
				;// nothing else
			else if( IS_DELIM() )
				++i;// delim
			else if( IS_TERMINATOR() )
				state = TERMINATE;
			break;

		case TERMINATE:
#if 0
			// skip line terminator
			if( (opt&SV_TERMINATE_CRLF) && i+1 < len && str[i] == '\r' && str[i+1] == '\n' )
				i += 2;// CRLF
			else
				++i;// CR or LF
#endif
			sv.done = true;
			state = END;
			break;
		}
	}
	if( IS_END() )
		sv.done = true;
	sv.off = i;

#undef IS_END
#undef IS_DELIM
#undef IS_TERMINATOR
#undef IS_C_ESCAPE
#undef SET_FIELD_START
#undef SET_FIELD_END

	return 1;
}


/// Parses a delim-separated string.
/// Starts parsing at startoff and fills the pos array with position pairs.
/// out_pos[0] and out_pos[1] are the start and end of line.
/// Other position pairs are the start and end of fields.
/// Returns the number of fields found or -1 if an error occurs.
///
/// out_pos can be nullptr.
/// If a line terminator is found, the end position is placed there.
/// out_pos[2] and out_pos[3] for the first field, out_pos[4] and out_pos[5]
/// for the seconds field and so on.
/// Unfilled positions are set to -1.
///
/// @param str String to parse
/// @param len Length of the string
/// @param startoff Where to start parsing
/// @param delim Field delimiter
/// @param out_pos Array of resulting positions
/// @param npos Size of the pos array
/// @param opt Options that determine the parsing behaviour
/// @return Number of fields found in the string or -1 if an error occured
size_t sv_parse( const char* str, size_t len, size_t startoff, char delim, size_t* out_pos, size_t npos, int32 opt, bool& error ){
	// initialize
	error = false;

	if( out_pos == nullptr ){
		npos = 0;
	}

	for( size_t i = 0; i < npos; ++i ){
		out_pos[i] = -1;
	}

	s_svstate sv = {};

	sv.str = str;
	sv.len = len;
	sv.off = startoff;
	sv.opt = opt;
	sv.delim = delim;
	sv.done = false;

	if( npos > 0 ){
		out_pos[0] = startoff;
	}

	// parse
	size_t count = 0;
	
	while( !sv.done ){
		++count;

		if( sv_parse_next( sv ) <= 0 ){
			error = true;
			return 0;
		}

		if( npos > count * 2 ){
			out_pos[count * 2] = sv.start;
		}

		if( npos > count * 2 + 1 ){
			out_pos[count * 2 + 1] = sv.end;
		}
	}

	if( npos > 1 ){
		out_pos[1] = sv.off;
	}

	return count;
}

/// Splits a delim-separated string.
/// WARNING: this function modifies the input string
/// Starts splitting at startoff and fills the out_fields array.
/// out_fields[0] is the start of the next line.
/// Other entries are the start of fields (nul-teminated).
/// Returns the number of fields found or -1 if an error occurs.
///
/// out_fields can be nullptr.
/// Fields that don't fit in out_fields are not nul-terminated.
/// Extra entries in out_fields are filled with the end of the last field (empty string).
///
/// @param str String to parse
/// @param len Length of the string
/// @param startoff Where to start parsing
/// @param delim Field delimiter
/// @param out_fields Array of resulting fields
/// @param nfields Size of the field array
/// @param opt Options that determine the parsing behaviour
/// @return Number of fields found in the string or -1 if an error occured
size_t sv_split( char* str, size_t len, size_t startoff, char delim, char** out_fields, size_t nfields, int32 opt, bool& error ){
	if( out_fields == nullptr || nfields <= 0 ){
		return 0; // nothing to do
	}

	size_t pos[1024];
	size_t ret = sv_parse( str, len, startoff, delim, pos, ARRAYLENGTH( pos ), opt, error );

	// An error occurred
	if( error ){
		return 0;
	}

	// next line
	char* end = str + pos[1];
	if( end[0] == '\0' )
	{
		*out_fields = end;
	}
	else if( (opt&SV_TERMINATE_LF) && end[0] == '\n' )
	{
		if( !(opt&SV_KEEP_TERMINATOR) )
			end[0] = '\0';
		*out_fields = end + 1;
	}
	else if( (opt&SV_TERMINATE_CRLF) && end[0] == '\r' && end[1] == '\n' )
	{
		if( !(opt&SV_KEEP_TERMINATOR) )
			end[0] = end[1] = '\0';
		*out_fields = end + 2;
	}
	else if( (opt&SV_TERMINATE_CR) && end[0] == '\r' )
	{
		if( !(opt&SV_KEEP_TERMINATOR) )
			end[0] = '\0';
		*out_fields = end + 1;
	}
	else
	{
		ShowError("sv_split: unknown line delimiter 0x02%x.\n", (unsigned char)end[0]);
		return -1;// error
	}
	++out_fields;
	--nfields;

	// fields
	size_t i = 2;
	size_t done = 0;
	while( done < ret && nfields > 0 )
	{
		if( i < ARRAYLENGTH(pos) )
		{// split field
			*out_fields = str + pos[i];
			end = str + pos[i+1];
			*end = '\0';
			// next field
			i += 2;
			++done;
			++out_fields;
			--nfields;
		}
		else
		{// get more fields
			sv_parse( str, len, pos[i - 1] + 1, delim, pos, ARRAYLENGTH( pos ), opt, error );

			// An error occurred
			if( error ){
				return 0;
			}

			i = 2;
		}
	}
	// remaining fields
	for( i = 0; i < nfields; ++i )
		out_fields[i] = end;
	return ret;
}

/// Escapes src to out_dest according to the format of the C compiler.
/// Returns the length of the escaped string.
/// out_dest should be len*4+1 in size.
///
/// @param out_dest Destination buffer
/// @param src Source string
/// @param len Length of the source string
/// @param escapes Extra characters to be escaped
/// @return Length of the escaped string
size_t sv_escape_c(char* out_dest, const char* src, size_t len, const char* escapes)
{
	size_t i;
	size_t j;

	if( out_dest == nullptr )
		return 0;// nothing to do
	if( src == nullptr )
	{// nothing to escape
		*out_dest = 0;
		return 0;
	}
	if( escapes == nullptr )
		escapes = "";

	for( i = 0, j = 0; i < len; ++i )
	{
		switch( src[i] )
		{
		case '\0':// octal 0
			out_dest[j++] = '\\';
			out_dest[j++] = '0';
			out_dest[j++] = '0';
			out_dest[j++] = '0';
			break;
		case '\r':// carriage return
			out_dest[j++] = '\\';
			out_dest[j++] = 'r';
			break;
		case '\n':// line feed
			out_dest[j++] = '\\';
			out_dest[j++] = 'n';
			break;
		case '\\':// escape character
			out_dest[j++] = '\\';
			out_dest[j++] = '\\';
			break;
		default:
			if( strchr(escapes,src[i]) )
			{// escape
				out_dest[j++] = '\\';
				switch( src[i] )
				{
				case '\a': out_dest[j++] = 'a'; break;
				case '\b': out_dest[j++] = 'b'; break;
				case '\t': out_dest[j++] = 't'; break;
				case '\v': out_dest[j++] = 'v'; break;
				case '\f': out_dest[j++] = 'f'; break;
				case '\?': out_dest[j++] = '?'; break;
				default:// to octal
					out_dest[j++] = '0'+((char)(((unsigned char)src[i]&0700)>>6));
					out_dest[j++] = '0'+((char)(((unsigned char)src[i]&0070)>>3));
					out_dest[j++] = '0'+((char)(((unsigned char)src[i]&0007)   ));
					break;
				}
			}
			else
				out_dest[j++] = src[i];
			break;
		}
	}
	out_dest[j] = 0;
	return j;
}

/// Unescapes src to out_dest according to the format of the C compiler.
/// Returns the length of the unescaped string.
/// out_dest should be len+1 in size and can be the same buffer as src.
///
/// @param out_dest Destination buffer
/// @param src Source string
/// @param len Length of the source string
/// @return Length of the escaped string
size_t sv_unescape_c(char* out_dest, const char* src, size_t len)
{
	static unsigned char low2hex[256] = {
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0x0?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0x1?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0x2?
		0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 0, 0, 0, 0, 0, 0,// 0x3?
		0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0x4?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0x5?
		0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0x6?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0x7?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0x8?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0x9?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0xA?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0xB?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0xC?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0xD?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 0xE?
		0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 0xF?
	};
	size_t i;
	size_t j;

	for( i = 0, j = 0; i < len; )
	{
		if( src[i] == '\\' )
		{
			++i;// '\\'
			if( i >= len )
				ShowWarning("sv_unescape_c: empty escape sequence\n");
			else if( src[i] == 'x' )
			{// hex escape sequence
				unsigned char c = 0;
				unsigned char inrange = 1;

				++i;// 'x'
				if( i >= len || !ISXDIGIT(src[i]) )
				{
					ShowWarning("sv_unescape_c: \\x with no following hex digits\n");
					continue;
				}
				do{
					if( c > 0x0F && inrange )
					{
						ShowWarning("sv_unescape_c: hex escape sequence out of range\n");
						inrange = 0;
					}
					c = (c<<4)|low2hex[(unsigned char)src[i]];// hex digit
					++i;
				}while( i < len && ISXDIGIT(src[i]) );
				out_dest[j++] = (char)c;
			}
			else if( src[i] == '0' || src[i] == '1' || src[i] == '2' || src[i] == '3' )
			{// octal escape sequence (255=0377)
				unsigned char c = src[i]-'0';
				++i;// '0', '1', '2' or '3'
				if( i < len && src[i] >= '0' && src[i] <= '7' )
				{
					c = (c<<3)|(src[i]-'0');
					++i;// octal digit
				}
				if( i < len && src[i] >= '0' && src[i] <= '7' )
				{
					c = (c<<3)|(src[i]-'0');
					++i;// octal digit
				}
				out_dest[j++] = (char)c;
			}
			else
			{// other escape sequence
				if( strchr(SV_ESCAPE_C_SUPPORTED, src[i]) == nullptr )
					ShowWarning("sv_unescape_c: unknown escape sequence \\%c\n", src[i]);
				switch( src[i] )
				{
				case 'a': out_dest[j++] = '\a'; break;
				case 'b': out_dest[j++] = '\b'; break;
				case 't': out_dest[j++] = '\t'; break;
				case 'n': out_dest[j++] = '\n'; break;
				case 'v': out_dest[j++] = '\v'; break;
				case 'f': out_dest[j++] = '\f'; break;
				case 'r': out_dest[j++] = '\r'; break;
				case '?': out_dest[j++] = '\?'; break;
				default: out_dest[j++] = src[i]; break;
				}
				++i;// escaped character
			}
		}
		else
			out_dest[j++] = src[i++];// normal character
	}
	out_dest[j] = 0;
	return j;
}

/// Skips a C escape sequence (starting with '\\').
const char* skip_escaped_c(const char* p)
{
	if( p && *p == '\\' )
	{
		++p;
		switch( *p )
		{
		case 'x':// hexadecimal
			++p;
			while( ISXDIGIT(*p) )
				++p;
			break;
		case '0':
		case '1':
		case '2':
		case '3':// octal
			++p;
			if( *p >= '0' && *p <= '7' )
				++p;
			if( *p >= '0' && *p <= '7' )
				++p;
			break;
		default:
			if( *p && strchr(SV_ESCAPE_C_SUPPORTED, *p) )
				++p;
		}
	}
	return p;
}


/**
 * Opens and parses a file containing delim-separated columns, feeding them to the specified callback function row by row.
 * Tracks the progress of the operation (current line number, number of successfully processed rows).
 * Returns 'true' if it was able to process the specified file, or 'false' if it could not be read.
 * @param directory : Directory
 * @param filename : filename File to process
 * @param delim : delim Field delimiter
 * @param mincols : mincols Minimum number of columns of a valid row
 * @param maxcols : maxcols Maximum number of columns of a valid row
 * @param maxrows : maxcols Maximum number of columns of a valid row
 * @param parseproc : parseproc User-supplied row processing function
 * @param silent : should we display error if file not found ?
 * @return true on success, false if file could not be opened
 */
bool sv_readdb( const char* directory, const char* filename, char delim, size_t mincols, size_t maxcols, size_t maxrows, bool (*parseproc)( char* fields[], size_t columns, size_t current ), bool silent ){
	FILE* fp;
	int32 lines = 0;
	size_t entries = 0;
	char** fields; // buffer for fields ([0] is reserved)
	char path[1024], *line;
	const int16 colsize=512;

	snprintf(path, sizeof(path), "%s/%s", directory, filename);

	// open file
	fp = fopen(path, "r");
	if( fp == nullptr )
	{
		if(silent == 0) ShowError("sv_readdb: can't read %s\n", path);
		return false;
	}

	// allocate enough memory for the maximum requested amount of columns plus the reserved one
	size_t nb_cols = maxcols + 1;
	fields = (char**)aMalloc(nb_cols*sizeof(char*));
	line = (char*)aMalloc(nb_cols*colsize);

	// process rows one by one
	while( fgets( line, static_cast<int32>( maxcols * colsize ), fp ) ){
		char *match;
		lines++;

		if( ( match = strstr(line, "//") ) != nullptr )
		{// strip comments
			match[0] = 0;
		}

		//trim(line); //TODO: strip trailing whitespace
		//trim2(line,1); //removing trailing actually break mob_skill_db
		if( line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;

		bool error;
		size_t columns = sv_split( line, strlen( line ), 0, delim, fields, nb_cols, SV_TERMINATE_LF|SV_TERMINATE_CRLF, error );

		if( error ){
			ShowError( "sv_readdb: error in line %d of \"%s\".\n", lines, path );
			continue;
		}

		if( columns < mincols )
		{
			ShowError("sv_readdb: Insufficient columns in line %d of \"%s\" (found %" PRIuPTR ", need at least %" PRIuPTR ").\n", lines, path, columns, mincols);
			continue; // not enough columns
		}
		if( columns > maxcols )
		{
			ShowError("sv_readdb: Too many columns in line %d of \"%s\" (found %" PRIuPTR ", maximum is %" PRIuPTR ").\n", lines, path, columns, maxcols );
			continue; // too many columns
		}
		if( entries == maxrows )
		{
			ShowError("sv_readdb: Reached the maximum allowed number of entries (%" PRIuPTR ") when parsing file \"%s\".\n", maxrows, path);
			break;
		}

		// parse this row
		if( !parseproc(fields+1, columns, entries) )
		{
			ShowError("sv_readdb: Could not process contents of line %d of \"%s\".\n", lines, path);
                        //perhaps call a provided function to clean entries if we have fail
                        //clearproc(fields+1, columns, entries)
			continue; // invalid row contents
		}

		// success!
		entries++;
	}

	aFree(fields);
	aFree(line);
	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%" PRIuPTR CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", entries, path);

	return true;
}


/////////////////////////////////////////////////////////////////////
// StringBuf - dynamic string
//
// @author MouseJstr (original)

/// Allocates a StringBuf
StringBuf* _StringBuf_Malloc(const char *file, int32 line, const char *func)
{
	StringBuf* self;
	self = (StringBuf *)aCalloc2(1, sizeof(StringBuf), file, line, func);
	new (self) StringBuf();
	_StringBuf_Init(file, line, func, self);
	return self;
}

/// Initializes a previously allocated StringBuf
void _StringBuf_Init(const char *file, int32 line, const char *func,StringBuf* self)
{
	self->max_ = 1024;
	self->ptr_ = self->buf_ = (char*)aMalloc2(self->max_ + 1, file, line, func);
}

/// Appends the result of printf to the StringBuf
size_t _StringBuf_Printf( const char* file, int32 line, const char* func, StringBuf* self, const char* fmt, ... ){
	va_list ap;

	va_start(ap, fmt);
	size_t len = _StringBuf_Vprintf(file,line,func,self, fmt, ap);
	va_end(ap);

	return len;
}

/// Appends the result of vprintf to the StringBuf
size_t _StringBuf_Vprintf( const char* file, int32 line, const char* func, StringBuf* self, const char* fmt, va_list ap ){
	for(;;)
	{
		va_list apcopy;
		/* Try to print in the allocated space. */
		size_t size = self->max_ - (self->ptr_ - self->buf_);
		va_copy(apcopy, ap);
		int32 n = vsnprintf( self->ptr_, size, fmt, apcopy );
		va_end(apcopy);
		/* If that worked, return the length. */
		if( n > -1 && static_cast<size_t>(n) < size )
		{
			self->ptr_ += n;
			return self->ptr_ - self->buf_;
		}
		/* Else try again with more space. */
		self->max_ *= 2; // twice the old size
		size_t off = self->ptr_ - self->buf_;
		self->buf_ = (char*)aRealloc2(self->buf_, self->max_ + 1, file, line, func);
		self->ptr_ = self->buf_ + off;
	}
}

/// Appends the contents of another StringBuf to the StringBuf
size_t _StringBuf_Append(const char *file, int32 line, const char *func,StringBuf* self, const StringBuf* sbuf)
{
	size_t available = self->max_ - (self->ptr_ - self->buf_);
	size_t needed = sbuf->ptr_ - sbuf->buf_;

	if( needed >= available )
	{
		size_t off = self->ptr_ - self->buf_;
		self->max_ += needed;
		self->buf_ = (char*)aRealloc2(self->buf_, self->max_ + 1, file, line, func);
		self->ptr_ = self->buf_ + off;
	}

	memcpy(self->ptr_, sbuf->buf_, needed);
	self->ptr_ += needed;
	return self->ptr_ - self->buf_;
}

// Appends str to the StringBuf
size_t _StringBuf_AppendStr(const char *file, int32 line, const char *func,StringBuf* self, const char* str)
{
	size_t available = self->max_ - ( self->ptr_ - self->buf_ );
	size_t needed = strlen( str );

	if( needed >= available )
	{// not enough space, expand the buffer (minimum expansion = 1024)
		size_t off = self->ptr_ - self->buf_;
		self->max_ += std::max( needed, static_cast<size_t>( 1024 ) );
		self->buf_ = (char*)aRealloc2(self->buf_, self->max_ + 1, file, line, func);
		self->ptr_ = self->buf_ + off;
	}

	memcpy(self->ptr_, str, needed);
	self->ptr_ += needed;
	return self->ptr_ - self->buf_;
}

// Returns the length of the data in the Stringbuf
int32 StringBuf_Length(StringBuf* self)
{
	return (int32)(self->ptr_ - self->buf_);
}

/// Returns the data in the StringBuf
char* StringBuf_Value(StringBuf* self)
{
	*self->ptr_ = '\0';
	return self->buf_;
}

/// Clears the contents of the StringBuf
void StringBuf_Clear(StringBuf* self)
{
	self->ptr_ = self->buf_;
}

///  Constructs the StringBuf
StringBuf::StringBuf(){
	this->buf_ = nullptr;
	this->ptr_ = nullptr;
	this->max_ = 0;
}

/// Destroys the StringBuf
StringBuf::~StringBuf(){
	if( this->buf_ != nullptr ){
		aFree( this->buf_ );
		this->buf_ = nullptr;
	}
	
	this->ptr_ = nullptr;
	this->max_ = 0;
}

// Frees a StringBuf returned by StringBuf_Malloc
void StringBuf_Free(StringBuf* self)
{
	if( self == nullptr ){
		return;
	}

	self->~StringBuf();
	aFree(self);
}
