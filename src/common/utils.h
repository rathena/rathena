// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdarg.h>

// generate a hex dump of the first 'length' bytes of 'buffer'
void dump(FILE* fp, const unsigned char* buffer, int length);

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
int StringBuf_AppendStr(struct StringBuf* sbuf, const char* str);
int StringBuf_Length(struct StringBuf* sbuf);
char * StringBuf_Value(struct StringBuf *);
void StringBuf_Destroy(struct StringBuf *);
void StringBuf_Free(struct StringBuf *);

void findfile(const char *p, const char *pat, void (func)(const char*));

//Caps values to min/max
#define cap_value(a, min, max) ((a >= max) ? max : (a <= min) ? min : a)

//////////////////////////////////////////////////////////////////////////
// byte word dword access [Shinomori]
//////////////////////////////////////////////////////////////////////////

extern uint8 GetByte(uint32 val, size_t num);
extern uint16 GetWord(uint32 val, size_t num);
extern uint16 MakeWord(uint8 byte0, uint8 byte1);
extern uint32 MakeDWord(uint16 word0, uint16 word1);

#endif /* _UTILS_H_ */
