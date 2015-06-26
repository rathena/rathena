// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/showmsg.h"
#include "socket.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <math.h> // floor()

#ifdef WIN32
	#include "../common/winapi.h"
	#ifndef F_OK
		#define F_OK   0x0
	#endif  /* F_OK */
#else
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/stat.h>
#endif


/// Dumps given buffer into file pointed to by a handle.
void WriteDump(FILE* fp, const void* buffer, size_t length)
{
	size_t i;
	char hex[48+1], ascii[16+1];

	fprintf(fp, "--- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F   0123456789ABCDEF\n");
	ascii[16] = 0;

	for( i = 0; i < length; i++ )
	{
		char c = RBUFB(buffer,i);

		ascii[i%16] = ISCNTRL(c) ? '.' : c;
		sprintf(hex+(i%16)*3, "%02X ", RBUFB(buffer,i));

		if( (i%16) == 15 )
		{
			fprintf(fp, "%03X %s  %s\n", (unsigned int)(i/16), hex, ascii);
		}
	}

	if( (i%16) != 0 )
	{
		ascii[i%16] = 0;
		fprintf(fp, "%03X %-48s  %-16s\n", (unsigned int)(i/16), hex, ascii);
	}
}


/// Dumps given buffer on the console.
void ShowDump(const void* buffer, size_t length)
{
	size_t i;
	char hex[48+1], ascii[16+1];

	ShowDebug("--- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F   0123456789ABCDEF\n");
	ascii[16] = 0;
        
	for( i = 0; i < length; i++ )
	{
		char c = RBUFB(buffer,i);

		ascii[i%16] = ISCNTRL(c) ? '.' : c;
		sprintf(hex+(i%16)*3, "%02X ", RBUFB(buffer,i));

		if( (i%16) == 15 )
		{
			ShowDebug("%03X %s  %s\n", i/16, hex, ascii);
		}
	}

	if( (i%16) != 0 )
	{
		ascii[i%16] = 0;
		ShowDebug("%03X %-48s  %-16s\n", i/16, hex, ascii);
	}
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
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	char tmppath[MAX_PATH+1];

	const char *path    = (p  ==NULL)? "." : p;
	const char *pattern = (pat==NULL)? "" : pat;

	checkpath(tmppath,path);
	if( PATHSEP != tmppath[strlen(tmppath)-1])
		strcat(tmppath, "\\*");
	else
		strcat(tmppath, "*");

	hFind = FindFirstFileA(tmppath, &FindFileData);
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
		}while (FindNextFileA(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return;
}

int check_filepath(const char* filepath){
	DWORD Attribute;
	if( Attribute = GetFileAttributes(filepath) ){
		if( (Attribute & INVALID_FILE_ATTRIBUTES) && GetLastError() == ERROR_FILE_NOT_FOUND ) return 3;
		else if( Attribute & FILE_ATTRIBUTE_DIRECTORY ) return 1;
		else return 2;
	}
	return 0;
}

#else

#define MAX_DIR_PATH 2048


/**
 * Check if the path is a directory or file
 * @param filepath
 * @return 1=dir, 2=file, 3=else, 0=error
 */
int check_filepath(const char* filepath){
    struct stat s;

    if( stat(filepath,&s) == 0 ){
            if( s.st_mode & S_IFDIR ) return 1;
            else if( s.st_mode & S_IFREG )return 2;
            else return 3;
    }
    else  {
        return 0;
    }
}

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

	closedir(dir);
}
#endif

bool exists(const char* filename)
{
	return !access(filename, F_OK);
}

uint8 GetByte(uint32 val, int idx)
{
	switch( idx )
	{
	case 0: return (uint8)( (val & 0x000000FF)         );
	case 1: return (uint8)( (val & 0x0000FF00) >> 0x08 );
	case 2: return (uint8)( (val & 0x00FF0000) >> 0x10 );
	case 3: return (uint8)( (val & 0xFF000000) >> 0x18 );
	default:
#if defined(DEBUG)
		ShowDebug("GetByte: invalid index (idx=%d)\n", idx);
#endif
		return 0;
	}
}

uint16 GetWord(uint32 val, int idx)
{
	switch( idx )
	{
	case 0: return (uint16)( (val & 0x0000FFFF)         );
	case 1: return (uint16)( (val & 0xFFFF0000) >> 0x10 );
	default:
#if defined(DEBUG)
		ShowDebug("GetWord: invalid index (idx=%d)\n", idx);
#endif
		return 0;
	}
}
uint16 MakeWord(uint8 byte0, uint8 byte1)
{
	return byte0 | (byte1 << 0x08);
}

uint32 MakeDWord(uint16 word0, uint16 word1)
{
	return
		( (uint32)(word0        ) )|
		( (uint32)(word1 << 0x10) );
}

/*************************************
* Big-endian compatibility functions *
*************************************/

// Converts an int16 from current machine order to little-endian
int16 MakeShortLE(int16 val)
{
	unsigned char buf[2];
	buf[0] = (unsigned char)( (val & 0x00FF)         );
	buf[1] = (unsigned char)( (val & 0xFF00) >> 0x08 );
	return *((int16*)buf);
}

// Converts an int32 from current machine order to little-endian
int32 MakeLongLE(int32 val)
{
	unsigned char buf[4];
	buf[0] = (unsigned char)( (val & 0x000000FF)         );
	buf[1] = (unsigned char)( (val & 0x0000FF00) >> 0x08 );
	buf[2] = (unsigned char)( (val & 0x00FF0000) >> 0x10 );
	buf[3] = (unsigned char)( (val & 0xFF000000) >> 0x18 );
	return *((int32*)buf);
}

// Reads an uint16 in little-endian from the buffer
uint16 GetUShort(const unsigned char* buf)
{
	return	 ( ((uint16)(buf[0]))         )
			|( ((uint16)(buf[1])) << 0x08 );
}

// Reads an uint32 in little-endian from the buffer
uint32 GetULong(const unsigned char* buf)
{
	return	 ( ((uint32)(buf[0]))         )
			|( ((uint32)(buf[1])) << 0x08 )
			|( ((uint32)(buf[2])) << 0x10 )
			|( ((uint32)(buf[3])) << 0x18 );
}

// Reads an int32 in little-endian from the buffer
int32 GetLong(const unsigned char* buf)
{
	return (int32)GetULong(buf);
}

// Reads a float (32 bits) from the buffer
float GetFloat(const unsigned char* buf)
{
	uint32 val = GetULong(buf);
	return *((float*)(void*)&val);
}

uint32 date2version(int date) {
	if(date < 20040906) return 5;
	else if(date < 20040920) return 10;
	else if(date < 20041005) return 11;
	else if(date < 20041025) return 12;
	else if(date < 20041129) return 13;
	else if(date < 20050110) return 14;
	else if(date < 20050509) return 15;
	else if(date < 20050628) return 16;
	else if(date < 20050718) return 17;
	else if(date < 20050719) return 18;
	else if(date < 20060327) return 19;
	else if(date < 20070108) return 20;
	else if(date < 20070212) return 21;
	//wtf @FIXME
	//else if(date < 20080910) return 22;
	else if(date < 20080827) return 23;
	else if(date < 20080910) return 24;
	//unable to solve from date
	else if(date < 20101124) return 25;
	else if(date < 20111005) return 26;
	else if(date < 20111102) return 27;
	else if(date < 20120307) return 28;
	else if(date < 20120410) return 29;
	else if(date < 20120418) return 30;
	else if(date < 20120618) return 31;
	else if(date < 20120702) return 32;
	else if(date < 20130320) return 33;
	else if(date < 20130515) return 34;
	else if(date < 20130522) return 35;
	else if(date < 20130529) return 36;
	else if(date < 20130605) return 37;
	else if(date < 20130612) return 38;
	else if(date < 20130618) return 39;
	else if(date < 20130626) return 40;
	else if(date < 20130703) return 41;
	else if(date < 20130710) return 42;
	else if(date < 20130717) return 43;
	else if(date < 20130807) return 44;
	else if(date < 20131223) return 45;
	else if(date < 20140212) return 46;
	//else if(date < 20140613) return 47;
	//else if(date < 20141016) return 48;
	else if(date < 20141022) return 50;
	else if(date >= 20141022) return 51;

	else return 30; //default
}

/// calculates the value of A / B, in percent (rounded down)
unsigned int get_percentage(const unsigned int A, const unsigned int B)
{
	double result;

	if( B == 0 )
	{
		ShowError("get_percentage(): divison by zero! (A=%u,B=%u)\n", A, B);
		return ~0U;
	}

	result = 100 * ((double)A / (double)B);

	if( result > UINT_MAX )
	{
		ShowError("get_percentage(): result percentage too high! (A=%u,B=%u,result=%g)\n", A, B, result);
		return UINT_MAX;
	}

	return (unsigned int)floor(result);
}

/**
 * Calculates the Levenshtein distance of two strings.
 * @author http://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C
 */
int levenshtein(const char *s1, const char *s2) {
	unsigned int s1len, s2len, x, y, lastdiag, olddiag, i;
	unsigned int *column;
	s1len = strlen(s1);
	s2len = strlen(s2);
	column = malloc((s1len+1) * sizeof(unsigned int));
	for (y = 1; y <= s1len; y++)
		column[y] = y;
	for (x = 1; x <= s2len; x++) {
		column[0] = x;
		for (y = 1, lastdiag = x-1; y <= s1len; y++) {
			olddiag = column[y];
			column[y] = min(min(column[y] + 1, column[y-1] + 1), lastdiag + (s1[y-1] == s2[x-1] ? 0 : 1));
			lastdiag = olddiag;
		}
	}
	i = column[s1len];
	free(column);
	return i;
}
