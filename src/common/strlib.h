// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STRLIB_H_
#define _STRLIB_H_

#ifndef _CBASETYPES_H_
#include "../common/cbasetypes.h"
#endif

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

#endif /* _STRLIB_H_ */
