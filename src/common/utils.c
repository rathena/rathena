#include <string.h>
#include "utils.h"
#include <stdio.h>

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

