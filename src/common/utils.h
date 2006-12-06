// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H


#ifndef NULL
#define NULL (void *)0
#endif

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

/* strcasecmp -> stricmp -> str_cmp */
#if defined(_WIN32) && !defined(MINGW)
	int	strcasecmp(const char *arg1, const char *arg2);
	int	strncasecmp(const char *arg1, const char *arg2, size_t n);
	void str_upper(char *name);
	void str_lower(char *name);
    char *rindex(char *str, char c);
#endif

void dump(unsigned char *buffer, int num);
int newt_sqrt(int value); //Newton aproximation for getting a fast sqrt.

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

#endif
