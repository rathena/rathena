// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/stat.h>
#endif

#ifdef UTIL_DUMP
void dump(const unsigned char* buffer, int num)
{
	int icnt, jcnt;

	printf("         Hex                                                  ASCII\n");
	printf("         -----------------------------------------------      ----------------");

	for (icnt = 0; icnt < num; icnt += 16)
	{
		printf("\n%p ", &buffer[icnt]);
		for (jcnt = icnt; jcnt < icnt + 16; ++jcnt)
		{
			if (jcnt < num)
				printf("%02hX ", buffer[jcnt]);
			else
				printf("   ");
		}

		printf("  |  ");

		for (jcnt = icnt; jcnt < icnt + 16; ++jcnt)
		{
			if (jcnt < num) {
				if (buffer[jcnt] > 31 && buffer[jcnt] < 127)
					printf("%c", buffer[jcnt]);
				else
					printf(".");
			} else
				printf(" ");
		}
	}
	printf("\n");
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

// vprintf into a StringBuf, moving the pointer [MouseJstr]
int StringBuf_Vprintf(struct StringBuf *sbuf,const char *fmt,va_list ap) 
{
	int n, size, off;

	while (1) {
		/* Try to print in the allocated space. */
		size = sbuf->max_ - (sbuf->ptr_ - sbuf->buf_);
		n = vsnprintf (sbuf->ptr_, size, fmt, ap);
		/* If that worked, return the length. */
		if (n > -1 && n < size) {
			sbuf->ptr_ += n;
			return (int)(sbuf->ptr_ - sbuf->buf_);
		}
		/* Else try again with more space. */
		sbuf->max_ *= 2; // twice the old size
		off = (int)(sbuf->ptr_ - sbuf->buf_);
		sbuf->buf_ = (char *) aRealloc(sbuf->buf_, sbuf->max_ + 1);
		sbuf->ptr_ = sbuf->buf_ + off;
	}
}

// printf into a StringBuf, moving the pointer [MouseJstr]
int StringBuf_Printf(struct StringBuf *sbuf,const char *fmt,...) 
{
	int len;
	va_list ap;

	va_start(ap,fmt);
	len = StringBuf_Vprintf(sbuf,fmt,ap);
	va_end(ap);

	return len;
}

// Append buf2 onto the end of buf1 [MouseJstr]
int StringBuf_Append(struct StringBuf *buf1,const struct StringBuf *buf2) 
{
	int buf1_avail = buf1->max_ - (buf1->ptr_ - buf1->buf_);
	int size2 = (int)(buf2->ptr_ - buf2->buf_);

	if (size2 >= buf1_avail)  {
		int off = (int)(buf1->ptr_ - buf1->buf_);
		buf1->max_ += size2;
		buf1->buf_ = (char *) aRealloc(buf1->buf_, buf1->max_ + 1);
		buf1->ptr_ = buf1->buf_ + off;
	}

	memcpy(buf1->ptr_, buf2->buf_, size2);
	buf1->ptr_ += size2;
	return (int)(buf1->ptr_ - buf1->buf_);
}

// Appends str onto the end of buf
int StringBuf_AppendStr(struct StringBuf* sbuf, const char* str) 
{
	int available = sbuf->max_ - (sbuf->ptr_ - sbuf->buf_);
	int size = (int)strlen(str);

	if( size >= available )
	{// not enough space, expand the buffer (minimum expansion = 1024)
		int off = (int)(sbuf->ptr_ - sbuf->buf_);
		sbuf->max_ += max(size, 1024);
		sbuf->buf_ = (char *) aRealloc(sbuf->buf_, sbuf->max_ + 1);
		sbuf->ptr_ = sbuf->buf_ + off;
	}

	memcpy(sbuf->ptr_, str, size);
	sbuf->ptr_ += size;
	return (int)(sbuf->ptr_ - sbuf->buf_);
}

// Returns the length of the data in a Stringbuf
int StringBuf_Length(struct StringBuf *sbuf) 
{
	return (int)(sbuf->ptr_ - sbuf->buf_);
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

#ifdef WIN32

static char* checkpath(char *path, const char *srcpath)
{	// just make sure the char*path is not const
	char *p=path;
	if(NULL!=path && NULL!=srcpath)
	while(*srcpath) {
		if (*srcpath=='/') {
			*p++ = '\\';
			srcpath++;
		}
		else
			*p++ = *srcpath++;
	}
	*p = *srcpath; //EOS
	return path;
}

void findfile(const char *p, const char *pat, void (func)(const char*))
{	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char tmppath[MAX_PATH+1];
	
	const char *path    = (p  ==NULL)? "." : p;
	const char *pattern = (pat==NULL)? "" : pat;
	
	checkpath(tmppath,path);
	if( PATHSEP != tmppath[strlen(tmppath)-1])
		strcat(tmppath, "\\*");
	else
		strcat(tmppath, "*");
	
	hFind = FindFirstFile(tmppath, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (strcmp(FindFileData.cFileName, ".") == 0)
				continue;
			if (strcmp(FindFileData.cFileName, "..") == 0)
				continue;

			sprintf(tmppath,"%s%c%s",path,PATHSEP,FindFileData.cFileName);

			if (FindFileData.cFileName && strstr(FindFileData.cFileName, pattern)) {
				func( tmppath );
			}


			if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				findfile(tmppath, pat, func);
			}
		}while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return;
}
#else

#define MAX_DIR_PATH 2048

static char* checkpath(char *path, const char*srcpath)
{	// just make sure the char*path is not const
	char *p=path;
	if(NULL!=path && NULL!=srcpath)
	while(*srcpath) {
		if (*srcpath=='\\') {
			*p++ = '/';
			srcpath++;
		}
		else
			*p++ = *srcpath++;
	}
	*p = *srcpath; //EOS
	return path;
}

void findfile(const char *p, const char *pat, void (func)(const char*))
{	
	DIR* dir;					// pointer to the scanned directory.
	struct dirent* entry;		// pointer to one directory entry.
	struct stat dir_stat;       // used by stat().
	char tmppath[MAX_DIR_PATH+1];
	char path[MAX_DIR_PATH+1]= ".";
	const char *pattern = (pat==NULL)? "" : pat;
	if(p!=NULL) strcpy(path,p);

	// open the directory for reading
	dir = opendir( checkpath(path, path) );
	if (!dir) {
		ShowError("Cannot read directory '%s'\n", path);
		return;
	}

	// scan the directory, traversing each sub-directory
	// matching the pattern for each file name.
	while ((entry = readdir(dir))) {
		// skip the "." and ".." entries.
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;

		sprintf(tmppath,"%s%c%s",path, PATHSEP, entry->d_name);

		// check if the pattern matchs.
		if (entry->d_name && strstr(entry->d_name, pattern)) {
			func( tmppath );
		}
		// check if it is a directory.
		if (stat(tmppath, &dir_stat) == -1) {
			ShowError("stat error %s\n': ", tmppath);
			continue;
		}
		// is this a directory?
		if (S_ISDIR(dir_stat.st_mode)) {
			// decent recursivly
			findfile(tmppath, pat, func);
		}
	}//end while
}
#endif

unsigned char GetByte(unsigned long val, size_t num)
{
	switch(num) {
	case 0:  return (unsigned char)((val & 0x000000FF)      );
	case 1:	 return (unsigned char)((val & 0x0000FF00)>>0x08);
	case 2:	 return (unsigned char)((val & 0x00FF0000)>>0x10);
	case 3:	 return (unsigned char)((val & 0xFF000000)>>0x18);
	default: return 0;	//better throw something here
	}
}
unsigned short GetWord(unsigned long val, size_t num)
{
	switch(num) {
	case 0:  return (unsigned short)((val & 0x0000FFFF)      );
	case 1:  return (unsigned short)((val & 0xFFFF0000)>>0x10);
	default: return 0;	//better throw something here
	}
}
unsigned short MakeWord(unsigned char byte0, unsigned char byte1)
{
	return byte0 | (byte1<<0x08);
}
unsigned long MakeDWord(unsigned short word0, unsigned short word1)
{
	return 	  ((unsigned long)word0)
			| ((unsigned long)word1<<0x10);
}
