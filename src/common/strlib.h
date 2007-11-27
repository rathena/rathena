// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STRLIB_H_
#define _STRLIB_H_

#ifndef _CBASETYPES_H_
#include "../common/cbasetypes.h"
#endif
#include <stdarg.h>

char* jstrescape (char* pt);
char* jstrescapecpy (char* pt, const char* spt);
int jmemescapecpy (char* pt, const char* spt, int size);

int remove_control_chars(char* str);
char* trim(char* str);
char* normalize_name(char* str,const char* delims);
const char *stristr(const char *haystack, const char *needle);

#ifdef WIN32
#define HAVE_STRTOK_R
#define strtok_r(s,delim,save_ptr) _strtok_r((s),(delim),(save_ptr))
char* _strtok_r(char* s1, const char* s2, char** lasts);
#endif

#if !(defined(WIN32) && defined(_MSC_VER) && _MSC_VER >= 1400)
size_t strnlen (const char* string, size_t maxlen);
#endif

int e_mail_check(char* email);
int config_switch(const char* str);

/// always nul-terminates the string
char* safestrncpy(char* dst, const char* src, size_t n);

/// doesn't crash on null pointer
size_t safestrnlen(const char* string, size_t maxlen);

/// Works like snprintf, but always nul-terminates the buffer.
/// Returns the size of the string (without nul-terminator)
/// or -1 if the buffer is too small.
int safesnprintf(char* buf, size_t sz, const char* fmt, ...);

/// Returns the line of the target position in the string.
/// Lines start at 1.
int strline(const char* str, size_t pos);

/// StringBuf - dynamic string
struct StringBuf
{
	char *buf_;
	char *ptr_;
	unsigned int max_;
};
typedef struct StringBuf StringBuf;

StringBuf* StringBuf_Malloc(void);
void StringBuf_Init(StringBuf* self);
int StringBuf_Printf(StringBuf* self, const char* fmt, ...);
int StringBuf_Vprintf(StringBuf* self, const char* fmt, va_list args);
int StringBuf_Append(StringBuf* self, const StringBuf *sbuf);
int StringBuf_AppendStr(StringBuf* self, const char* str);
int StringBuf_Length(StringBuf* self);
char* StringBuf_Value(StringBuf* self);
void StringBuf_Clear(StringBuf* self);
void StringBuf_Destroy(StringBuf* self);
void StringBuf_Free(StringBuf* self);

#endif /* _STRLIB_H_ */
