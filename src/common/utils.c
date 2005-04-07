#include <string.h>
#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "malloc.h"
#include "mmo.h"

void dump(unsigned char *buffer, int num)
{
   int     icnt,jcnt;

   printf("         Hex                                                  ASCII\n");
   printf("         -----------------------------------------------      ----------------");

   for (icnt=0;icnt<num;icnt+=16) {
	printf("\n%p ",&buffer[icnt]);
    for (jcnt=icnt;jcnt<icnt+16;++jcnt) {
	    if (jcnt < num) {
              printf("%02hX ",buffer[jcnt]);
		}  else
              printf("   ");
    }

    printf("  |  ");

	for (jcnt=icnt;jcnt<icnt+16;++jcnt) {
        if (jcnt < num) {
            if (buffer[jcnt] > 31 && buffer[jcnt] < 127)
                   printf("%c",buffer[jcnt]);
               else
                   printf(".");
           }  else
               printf(" ");
       }
   }
   printf("\n");
}


#ifdef _WIN32
char *rindex(char *str, char c)
{
        char *sptr;

        sptr = str;
        while(*sptr)
                ++sptr;
        if (c == '\0')
                return(sptr);
        while(str != sptr)
                if (*sptr-- == c)
                        return(++sptr);
        return(NULL);
}

int strcasecmp(const char *arg1, const char *arg2)
{
  int chk, i;

  if (arg1 == NULL || arg2 == NULL) {
    printf("SYSERR: str_cmp() passed a NULL pointer, %p or %p.\n", arg1, arg2);
    return (0);
  }

  for (i = 0; arg1[i] || arg2[i]; i++)
    if ((chk = LOWER(arg1[i]) - LOWER(arg2[i])) != 0)
      return (chk);	/* not equal */

  return (0);
}

int strncasecmp(const char *arg1, const char *arg2, int n)
{
  int chk, i;

  if (arg1 == NULL || arg2 == NULL) {
    printf("SYSERR: strn_cmp() passed a NULL pointer, %p or %p.\n", arg1, arg2);
    return (0);
  }

  for (i = 0; (arg1[i] || arg2[i]) && (n > 0); i++, n--)
    if ((chk = LOWER(arg1[i]) - LOWER(arg2[i])) != 0)
      return (chk);	/* not equal */

  return (0);
}

void str_upper(char *name)
{

  int len = strlen(name);
  while (len--) {
	if (*name >= 'a' && *name <= 'z')
    	*name -= ('a' - 'A');
     name++;
  }
}

void str_lower(char *name)
{
  int len = strlen(name);

  while (len--) {
	if (*name >= 'A' && *name <= 'Z')
    	*name += ('a' - 'A');
    name++;
  }
}

#endif

// Allocate a StringBuf  [MouseJstr]
struct StringBuf * StringBuf_Malloc() 
{
	struct StringBuf * ret = (struct StringBuf *) aMallocA(sizeof(struct StringBuf));
	StringBuf_Init(ret);
	return ret;
}

// Initialize a previously allocated StringBuf [MouseJstr]
void StringBuf_Init(struct StringBuf * sbuf)  {
	sbuf->max_ = 1024;
	sbuf->ptr_ = sbuf->buf_ = (char *) aMallocA(sbuf->max_ + 1);
}

// printf into a StringBuf, moving the pointer [MouseJstr]
int StringBuf_Printf(struct StringBuf *sbuf,const char *fmt,...) 
{
	va_list ap;
        int n, size, off;

	while (1) {
		/* Try to print in the allocated space. */
		va_start(ap, fmt);
		size = sbuf->max_ - (sbuf->ptr_ - sbuf->buf_);
		n = vsnprintf (sbuf->ptr_, size, fmt, ap);
		va_end(ap);
		/* If that worked, return the length. */
		if (n > -1 && n < size) {
			sbuf->ptr_ += n;
			return sbuf->ptr_ - sbuf->buf_;
		}
		/* Else try again with more space. */
		sbuf->max_ *= 2; // twice the old size
		off = sbuf->ptr_ - sbuf->buf_;
		sbuf->buf_ = (char *) aRealloc(sbuf->buf_, sbuf->max_ + 1);
		sbuf->ptr_ = sbuf->buf_ + off;
	}
}

// Append buf2 onto the end of buf1 [MouseJstr]
int StringBuf_Append(struct StringBuf *buf1,const struct StringBuf *buf2) 
{
	int buf1_avail = buf1->max_ - (buf1->ptr_ - buf1->buf_);
	int size2 = buf2->ptr_ - buf2->buf_;

	if (size2 >= buf1_avail)  {
		int off = buf1->ptr_ - buf1->buf_;
		buf1->max_ += size2;
		buf1->buf_ = (char *) aRealloc(buf1->buf_, buf1->max_ + 1);
		buf1->ptr_ = buf1->buf_ + off;
	}

	memcpy(buf1->ptr_, buf2->buf_, size2);
	buf1->ptr_ += size2;
	return buf1->ptr_ - buf1->buf_;
}

// Destroy a StringBuf [MouseJstr]
void StringBuf_Destroy(struct StringBuf *sbuf) 
{
	aFree(sbuf->buf_);
	sbuf->ptr_ = sbuf->buf_ = 0;
}

// Free a StringBuf returned by StringBuf_Malloc [MouseJstr]
void StringBuf_Free(struct StringBuf *sbuf) 
{
	StringBuf_Destroy(sbuf);
	aFree(sbuf);
}

// Return the built string from the StringBuf [MouseJstr]
char * StringBuf_Value(struct StringBuf *sbuf) 
{
	*sbuf->ptr_ = '\0';
	return sbuf->buf_;
}
