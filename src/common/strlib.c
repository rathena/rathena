// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "strlib.h"
#include "utils.h"
#include "../common/malloc.h"

//-----------------------------------------------
// string lib.
char* jstrescape (char* pt) {
	//copy from here
	char *ptr;
	int i =0, j=0;

	//copy string to temporary
	CREATE(ptr, char, J_MAX_MALLOC_SIZE);
	strcpy(ptr,pt);

	while (ptr[i] != '\0') {
		switch (ptr[i]) {
			case '\'':
				pt[j++] = '\\';
				pt[j++] = ptr[i++];
				break;
			case '\\':
				pt[j++] = '\\';
				pt[j++] = ptr[i++];
				break;
			case '%':
				pt[j++] = '_'; i++;
				break;
			default:
				pt[j++] = ptr[i++];
		}
	}
	pt[j++] = '\0';
	aFree(ptr);
	return &pt[0];
}

char* jstrescapecpy (char* pt,char* spt) {
	//copy from here
	//WARNING: Target string pt should be able to hold strlen(spt)*2, as each time
	//a escape character is found, the target's final length increases! [Skotlex]
	int i =0, j=0;

	if (!spt) {	//Return an empty string [Skotlex]
		pt[0] = '\0';
		return &pt[0];
	}
	
	while (spt[i] != '\0') {
		switch (spt[i]) {
			case '\'':
				pt[j++] = '\\';
				pt[j++] = spt[i++];
				break;
			case '\\':
				pt[j++] = '\\';
				pt[j++] = spt[i++];
				break;
			case '%':
				pt[j++] = '_'; i++;
				break;
			default:
				pt[j++] = spt[i++];
		}
	}
	pt[j++] = '\0';
	return &pt[0];
}
int jmemescapecpy (char* pt,char* spt, int size) {
	//copy from here
	int i =0, j=0;

	while (i < size) {
		switch (spt[i]) {
			case '\'':
				pt[j++] = '\\';
				pt[j++] = spt[i++];
				break;
			case '\\':
				pt[j++] = '\\';
				pt[j++] = spt[i++];
				break;
			case '%':
				pt[j++] = '_'; i++;
				break;
			default:
				pt[j++] = spt[i++];
		}
	}
	// copy size is 0 ~ (j-1)
	return j;
}

//-----------------------------------------------------
// Function to suppress control characters in a string.
//-----------------------------------------------------
//int remove_control_chars(char *str) {
int remove_control_chars(unsigned char *str) {
	int i;
	int change = 0;

	for(i = 0; str[i]; i++) {
		if (str[i] < 32) {
			str[i] = '_';
			change = 1;
		}
	}

	return change;
}

//Trims a string, also removes illegal characters such as \t and reduces continous spaces to a single one. by [Foruken]
char *trim(char *str, const char *delim)
{
	char *strp = strtok(str,delim);
	char buf[1024];
	char *bufp = buf;
	memset(buf,0,sizeof buf);

	while(strp) {
		strcpy(bufp, strp);
		bufp = bufp + strlen(strp);
		strp = strtok(NULL, delim);
		if (strp) {
			strcpy(bufp," ");
			bufp++;
		}
	}
	strcpy(str,buf);
	return str;
}


//stristr: Case insensitive version of strstr, code taken from 
//http://www.daniweb.com/code/snippet313.html, Dave Sinkula
//
const char *stristr(const char *haystack, const char *needle)
{
	if ( !*needle )
	{
		return haystack;
	}
	for ( ; *haystack; ++haystack )
	{
		if ( toupper(*haystack) == toupper(*needle) )
		{
			/*
			* Matched starting char -- loop through remaining chars.
			*/
			const char *h, *n;
			for ( h = haystack, n = needle; *h && *n; ++h, ++n )
			{
				if ( toupper(*h) != toupper(*n) )
				{
					break;
				}
			}
			if ( !*n ) /* matched all of 'needle' to null termination */
			{
				return haystack; /* return the start of the match */
			}
		}
	}
	return 0;
}

#ifdef __WIN32
char *_strtok_r(char *s1, const char *s2, char **lasts)
{
	char *ret;

	if (s1 == NULL)
		s1 = *lasts;
	while(*s1 && strchr(s2, *s1))
		++s1;
	if(*s1 == '\0')
		return NULL;
	ret = s1;
	while(*s1 && !strchr(s2, *s1))
		++s1;
	if(*s1)
		*s1++ = '\0';
	*lasts = s1;
	return ret;
}
#endif
