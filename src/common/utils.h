// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdarg.h>

#ifndef NULL
#define NULL (void *)0
#endif

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

void dump(unsigned char *buffer, int num);

struct StringBuf {
	char *buf_;
	char *ptr_;
	unsigned int max_;
};

struct StringBuf * StringBuf_Malloc(void);
void StringBuf_Init(struct StringBuf *);
int StringBuf_Vprintf(struct StringBuf *,const char *,va_list);
int StringBuf_Printf(struct StringBuf *,const char *,...);
int StringBuf_Append(struct StringBuf *,const struct StringBuf *);
char * StringBuf_Value(struct StringBuf *);
void StringBuf_Destroy(struct StringBuf *);
void StringBuf_Free(struct StringBuf *);

void findfile(const char *p, const char *pat, void (func)(const char*));

//////////////////////////////////////////////////////////////////////////
// byte word dword access [Shinomori]
//////////////////////////////////////////////////////////////////////////

extern unsigned char GetByte(unsigned long val, size_t num);
extern unsigned short GetWord(unsigned long val, size_t num);
extern unsigned short MakeWord(unsigned char byte0, unsigned char byte1);
extern unsigned long MakeDWord(unsigned short word0, unsigned short word1);

#endif /* _UTILS_H_ */
