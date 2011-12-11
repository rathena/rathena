// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/des.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "grfio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zlib.h>

//----------------------------
//	file entry table struct
//----------------------------
typedef struct _FILELIST {
	int		srclen;				// compressed size
	int		srclen_aligned;
	int		declen;				// original size
	int		srcpos;				// position of entry in grf
	int		next;				// index of next filelist entry with same hash (-1: end of entry chain)
	int		cycle;
	char	type;
	char	fn[128-4*5];		// file name
	char*	fnd;				// if the file was cloned, contains name of original file
	char	gentry;				// read grf file select
} FILELIST;

//gentry ... > 0  : data read from a grf file (gentry_table[gentry-1])
//gentry ... 0    : data read from a local file (data directory)
//gentry ... < 0  : entry "-(gentry)" is marked for a local file check
//                  - if local file exists, gentry will be set to 0 (thus using the local file)
//                  - if local file doesn't exist, sign is inverted (thus using the original file inside a grf)
//                  (NOTE: this case is only used once (during startup) and only if GRFIO_LOCAL is enabled)
//                  (NOTE: probably meant to be used to override grf contents by files in the data directory)
//#define GRFIO_LOCAL

// stores info about every loaded file
FILELIST* filelist		= NULL;
int filelist_entrys		= 0;
int filelist_maxentry	= 0;

// stores grf file names
char** gentry_table		= NULL;
int gentry_entrys		= 0;
int gentry_maxentry		= 0;

// the path to the data directory
char data_dir[1024] = "";

//----------------------------
//	file list hash table
//----------------------------
int filelist_hash[256];

// little endian char array to uint conversion
static unsigned int getlong(unsigned char* p)
{
	return (p[0] | p[1] << 0x08 | p[2] << 0x10 | p[3] << 0x18);
}

static void NibbleSwap(unsigned char* Src, int len)
{
	for(;0<len;len--,Src++) {
		*Src = (*Src>>4) | (*Src<<4);
	}
}


/// Substitutes some specific values for others, leaves rest intact. Obfuscation.
/// NOTE: Operation is symmetric (calling it twice gives back the original input).
static uint8_t grf_substitution(uint8_t in)
{
	uint8_t out;
	
	switch( in )
	{
	case 0x00: out = 0x2B; break;
	case 0x2B: out = 0x00; break;
	case 0x6C: out = 0x80; break;
	case 0x01: out = 0x68; break;
	case 0x68: out = 0x01; break;
	case 0x48: out = 0x77; break;
	case 0x60: out = 0xFF; break;
	case 0x77: out = 0x48; break;
	case 0xB9: out = 0xC0; break;
	case 0xC0: out = 0xB9; break;
	case 0xFE: out = 0xEB; break;
	case 0xEB: out = 0xFE; break;
	case 0x80: out = 0x6C; break;
	case 0xFF: out = 0x60; break;
	default:   out = in;   break;
	}

	return out;
}


static void grf_shuffle_enc(BIT64* src)
{
	BIT64 out;

	out.b[0] = src->b[3];
	out.b[1] = src->b[4];
	out.b[2] = src->b[5];
	out.b[3] = src->b[0];
	out.b[4] = src->b[1];
	out.b[5] = src->b[6];
	out.b[6] = src->b[2];
	out.b[7] = grf_substitution(src->b[7]);

	*src = out;
}


static void grf_shuffle_dec(BIT64* src)
{
	BIT64 out;

	out.b[0] = src->b[3];
	out.b[1] = src->b[4];
	out.b[2] = src->b[6];
	out.b[3] = src->b[0];
	out.b[4] = src->b[1];
	out.b[5] = src->b[2];
	out.b[6] = src->b[5];
	out.b[7] = grf_substitution(src->b[7]);

	*src = out;
}


static void decode_des_etc(unsigned char* buf, size_t len, int type, int cycle)
{
	BIT64* p = (BIT64*)buf;

	size_t lop,cnt=0;
	if(cycle<3) cycle=3;
	else if(cycle<5) cycle++;
	else if(cycle<7) cycle+=9;
	else cycle+=15;

	for(lop=0; lop*8<len; lop++)
	{
		if(lop<20 || (type==0 && lop%cycle==0)) { // des
			des_decrypt_block(&p[lop]);
		} else {
			if(cnt==7 && type==0) {
				grf_shuffle_dec(&p[lop]);
				cnt=0;
			}
			cnt++;
		}
	}
}

unsigned long grfio_crc32 (const unsigned char* buf, unsigned int len)
{
	return crc32(crc32(0L, Z_NULL, 0), buf, len);
}


///////////////////////////////////////////////////////////////////////////////
///	Grf data sub : zip decode
int decode_zip(void* dest, unsigned long* destLen, const void* source, unsigned long sourceLen)
{
	return uncompress((Bytef*)dest, destLen, (const Bytef*)source, sourceLen);
}


///////////////////////////////////////////////////////////////////////////////
///	Grf data sub : zip encode 
int encode_zip(void* dest, unsigned long* destLen, const void* source, unsigned long sourceLen)
{
	return compress((Bytef*)dest, destLen, (const Bytef*)source, sourceLen);
}


/***********************************************************
 ***                File List Subroutines                ***
 ***********************************************************/

// initializes the table that holds the first elements of all hash chains
static void hashinit(void)
{
	int i;
	for (i = 0; i < 256; i++)
		filelist_hash[i] = -1;
}

// hashes a filename string into a number from {0..255}
static int filehash(char* fname)
{
	unsigned int hash = 0;
	while(*fname) {
		hash = (hash<<1) + (hash>>7)*9 + TOLOWER(*fname);
		fname++;
	}
	return hash & 255;
}

// finds a FILELIST entry with the specified file name
static FILELIST* filelist_find(char* fname)
{
	int hash, index;

	if (!filelist)
		return NULL;

	hash = filelist_hash[filehash(fname)];
	for (index = hash; index != -1; index = filelist[index].next)
		if(!strcmpi(filelist[index].fn, fname))
			break;

	return (index >= 0) ? &filelist[index] : NULL;
}

// returns the original file name
char* grfio_find_file(char* fname)
{
	FILELIST *filelist = filelist_find(fname);
	if (!filelist) return NULL;
	return (!filelist->fnd ? filelist->fn : filelist->fnd);
}

// adds a FILELIST entry into the list of loaded files
static FILELIST* filelist_add(FILELIST* entry)
{
	int hash;

	#define	FILELIST_ADDS	1024	// number increment of file lists `

	if (filelist_entrys >= filelist_maxentry) {
		filelist = (FILELIST *)aRealloc(filelist, (filelist_maxentry + FILELIST_ADDS) * sizeof(FILELIST));
		memset(filelist + filelist_maxentry, '\0', FILELIST_ADDS * sizeof(FILELIST));
		filelist_maxentry += FILELIST_ADDS;
	}

	memcpy (&filelist[filelist_entrys], entry, sizeof(FILELIST));

	hash = filehash(entry->fn);
	filelist[filelist_entrys].next = filelist_hash[hash];
	filelist_hash[hash] = filelist_entrys;

	filelist_entrys++;

	return &filelist[filelist_entrys - 1];
}

// adds a new FILELIST entry or overwrites an existing one
static FILELIST* filelist_modify(FILELIST* entry)
{
	FILELIST* fentry = filelist_find(entry->fn);
	if (fentry != NULL) {
		int tmp = fentry->next;
		memcpy(fentry, entry, sizeof(FILELIST));
		fentry->next = tmp;
	} else {
		fentry = filelist_add(entry);
	}
	return fentry;
}

// shrinks the file list array if too long
static void filelist_adjust(void)
{
	if (filelist == NULL)
		return;
	
	if (filelist_entrys < filelist_maxentry) {
		filelist = (FILELIST *)aRealloc(filelist, filelist_entrys * sizeof(FILELIST));
		filelist_maxentry = filelist_entrys;
	}
}


/// Combines are resource path with the data folder location to
/// create local resource path.
static void grfio_localpath_create(char* buffer, size_t size, const char* filename)
{
	unsigned int i;
	size_t len;

	len = strlen(data_dir);

	if( data_dir[0] == 0 || data_dir[len-1] == '/' || data_dir[len-1] == '\\' )
	{
		safesnprintf(buffer, size, "%s%s", data_dir, filename);
	}
	else
	{
		safesnprintf(buffer, size, "%s/%s", data_dir, filename);
	}

	for( i = 0; buffer[i]; i++ )
	{// normalize path
		if( buffer[i] == '\\' )
		{
			buffer[i] = '/';
		}
	}
}


/***********************************************************
 ***                  Grfio Sobroutines                  ***
 ***********************************************************/

// returns a file's size
/*
int grfio_size(char* fname)
{
	FILELIST* entry;

	entry = filelist_find(fname);

	if (entry == NULL || entry->gentry < 0) {	// LocalFileCheck
		char lfname[256], *p;
		FILELIST lentry;
		struct stat st;

		grfio_localpath_create(lfname, sizeof(lfname), fname);

		if (stat(lfname, &st) == 0) {
			safestrncpy(lentry.fn, fname, sizeof(lentry.fn));
			lentry.fnd = NULL;
			lentry.declen = st.st_size;
			lentry.gentry = 0;	// 0:LocalFile
			entry = filelist_modify(&lentry);
		} else if (entry == NULL) {
			ShowError("%s not found (grfio_size)\n", fname);
			return -1;
		}
	}
	return entry->declen;
}
*/

// reads a file into a newly allocated buffer (from grf or data directory)
void* grfio_reads(char* fname, int* size)
{
	FILE* in;
	FILELIST* entry;
	unsigned char* buf2 = NULL;

	entry = filelist_find(fname);

	if (entry == NULL || entry->gentry <= 0) {	// LocalFileCheck
		char lfname[256];
		int declen;

		grfio_localpath_create(lfname, sizeof(lfname), ( entry && entry->fnd ) ? entry->fnd : fname);

		in = fopen(lfname, "rb");
		if (in != NULL) {
			fseek(in,0,SEEK_END);
			declen = ftell(in);
			fseek(in,0,SEEK_SET);
			buf2 = (unsigned char *)aMallocA(declen+1);  // +1 for resnametable zero-termination
			fread(buf2, 1, declen, in);
			fclose(in);
			if( size )
			{
				size[0] = declen;
			}
		} else {
			if (entry != NULL && entry->gentry < 0) {
				entry->gentry = -entry->gentry;	// local file checked
			} else {
				ShowError("grfio_reads: %s not found (local file: %s)\n", fname, lfname);
				return NULL;
			}
		}
	}
	if (entry != NULL && entry->gentry > 0) {	// Archive[GRF] File Read
		char* grfname = gentry_table[entry->gentry - 1];
		in = fopen(grfname, "rb");
		if(in != NULL) {
			unsigned char *buf = (unsigned char *)aMallocA(entry->srclen_aligned);
			fseek(in, entry->srcpos, 0);
			fread(buf, 1, entry->srclen_aligned, in);
			fclose(in);
			buf2 = (unsigned char *)aMallocA(entry->declen+1);  // +1 for resnametable zero-termination
			if (entry->type == 1 || entry->type == 3 || entry->type == 5) {
				uLongf len;
				if (entry->cycle >= 0)
					decode_des_etc(buf, entry->srclen_aligned, entry->cycle == 0, entry->cycle);
				len = entry->declen;
				decode_zip(buf2, &len, buf, entry->srclen);
				if (len != (uLong)entry->declen) {
					ShowError("decode_zip size mismatch err: %d != %d\n", (int)len, entry->declen);
					aFree(buf);
					aFree(buf2);
					return NULL;
				}
			} else {
				memcpy(buf2, buf, entry->declen);
			}
			if( size )
			{
				size[0] = entry->declen;
			}
			aFree(buf);
		} else {
			ShowError("grfio_reads: %s not found (GRF file: %s)\n", fname, grfname);
			return NULL;
		}
	}

	return buf2;
}

/*==========================================
 *	Resource filename decode
 *------------------------------------------*/
static char* decode_filename(unsigned char* buf, int len)
{
	int lop;
	for(lop=0;lop<len;lop+=8) {
		NibbleSwap(&buf[lop],8);
		des_decrypt(&buf[lop],8);
	}
	return (char*)buf;
}

// loads all entries in the specified grf file into the filelist
// gentry - index of the grf file name in the gentry_table
static int grfio_entryread(char* grfname, int gentry)
{
	FILE* fp;
	long grf_size,list_size;
	unsigned char grf_header[0x2e];
	int lop,entry,entrys,ofs,grf_version;
	char *fname;
	unsigned char *grf_filelist;

	fp = fopen(grfname, "rb");
	if (fp == NULL) {
		ShowWarning("GRF data file not found: '%s'\n",grfname);
		return 1;	// 1:not found error
	} else
		ShowInfo("GRF data file found: '%s'\n",grfname);

	fseek(fp,0,SEEK_END);
	grf_size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	fread(grf_header,1,0x2e,fp);
	if (strcmp((const char *) grf_header,"Master of Magic") ||
		fseek(fp,getlong(grf_header+0x1e),SEEK_CUR))
	{
		fclose(fp);
		ShowError("GRF %s read error\n", grfname);
		return 2;	// 2:file format error
	}

	grf_version = getlong(grf_header+0x2a) >> 8;

	if (grf_version == 0x01) {	//****** Grf version 01xx ******
		list_size = grf_size - ftell(fp);
		grf_filelist = (unsigned char *) aMallocA(list_size);
		fread(grf_filelist,1,list_size,fp);
		fclose(fp);

		entrys = getlong(grf_header+0x26) - getlong(grf_header+0x22) - 7;

		// Get an entry
		for (entry = 0,ofs = 0; entry < entrys; entry++) {
			int ofs2, srclen, srccount;
			unsigned char type;
			char* period_ptr;
			FILELIST aentry;

			ofs2 = ofs+getlong(grf_filelist+ofs)+4;
			type = grf_filelist[ofs2+12];
			if (type != 0) {	// Directory Index ... skip
				fname = decode_filename(grf_filelist+ofs+6, grf_filelist[ofs]-6);
				if (strlen(fname) > sizeof(aentry.fn) - 1) {
					ShowFatalError("GRF file name %s is too long\n", fname);
					aFree(grf_filelist);
					exit(EXIT_FAILURE);
				}
				srclen = 0;
				if ((period_ptr = strrchr(fname, '.')) != NULL) {
					for(lop = 0; lop < 4; lop++) {
						if (strcmpi(period_ptr, ".gnd\0.gat\0.act\0.str"+lop*5) == 0)
							break;
					}
					srclen = getlong(grf_filelist+ofs2) - getlong(grf_filelist+ofs2+8) - 715;
					if(lop == 4) {
						for(lop = 10, srccount = 1; srclen >= lop; lop = lop * 10, srccount++);
					} else {
						srccount = 0;
					}
				} else {
					srccount = 0;
				}

				aentry.srclen         = srclen;
				aentry.srclen_aligned = getlong(grf_filelist+ofs2+4)-37579;
				aentry.declen         = getlong(grf_filelist+ofs2+8);
				aentry.srcpos         = getlong(grf_filelist+ofs2+13)+0x2e;
				aentry.cycle          = srccount;
				aentry.type           = type;
				safestrncpy(aentry.fn, fname, sizeof(aentry.fn));
				aentry.fnd			  = NULL;
#ifdef	GRFIO_LOCAL
				aentry.gentry         = -(gentry+1);	// As Flag for making it a negative number carrying out the first time LocalFileCheck
#else
				aentry.gentry         = gentry+1;		// With no first time LocalFileCheck
#endif
				filelist_modify(&aentry);
			}
			ofs = ofs2 + 17;
		}
		aFree(grf_filelist);

	} else if (grf_version == 0x02) {	//****** Grf version 02xx ******
		unsigned char eheader[8];
		unsigned char *rBuf;
		uLongf rSize, eSize;

		fread(eheader,1,8,fp);
		rSize = getlong(eheader);	// Read Size
		eSize = getlong(eheader+4);	// Extend Size

		if ((long)rSize > grf_size-ftell(fp)) {
			fclose(fp);
			ShowError("Illegal data format: GRF compress entry size\n");
			return 4;
		}

		rBuf = (unsigned char *)aMallocA(rSize);	// Get a Read Size
		grf_filelist = (unsigned char *)aMallocA(eSize);	// Get a Extend Size
		fread(rBuf,1,rSize,fp);
		fclose(fp);
		decode_zip(grf_filelist, &eSize, rBuf, rSize);	// Decode function
		list_size = eSize;
		aFree(rBuf);

		entrys = getlong(grf_header+0x26) - 7;

		// Get an entry
		for(entry = 0, ofs = 0; entry < entrys; entry++) {
			int ofs2, srclen, srccount, type;
			FILELIST aentry;

			fname = (char*)(grf_filelist+ofs);
			if (strlen(fname) > sizeof(aentry.fn)-1) {
				ShowFatalError("GRF file name %s is too long\n", fname);
				aFree(grf_filelist);
				exit(EXIT_FAILURE);
			}
			ofs2 = ofs + (int)strlen(fname)+1;
			type = grf_filelist[ofs2+12];
			if (type == 1 || type == 3 || type == 5) {
				srclen = getlong(grf_filelist+ofs2);
				if (grf_filelist[ofs2+12] == 3) {
					for (lop = 10, srccount = 1; srclen >= lop; lop = lop * 10, srccount++);
				} else if (grf_filelist[ofs2+12] == 5) {
					srccount = 0;
				} else {	// if (grf_filelist[ofs2+12]==1) {
					srccount = -1;
				}

				aentry.srclen         = srclen;
				aentry.srclen_aligned = getlong(grf_filelist+ofs2+4);
				aentry.declen         = getlong(grf_filelist+ofs2+8);
				aentry.srcpos         = getlong(grf_filelist+ofs2+13)+0x2e;
				aentry.cycle          = srccount;
				aentry.type           = type;
				safestrncpy(aentry.fn, fname, sizeof(aentry.fn));
				aentry.fnd			  = NULL;
#ifdef	GRFIO_LOCAL
				aentry.gentry         = -(gentry+1);	// As Flag for making it a negative number carrying out the first time LocalFileCheck
#else
				aentry.gentry         = gentry+1;		// With no first time LocalFileCheck
#endif
				filelist_modify(&aentry);
			}
			ofs = ofs2 + 17;
		}
		aFree(grf_filelist);

	} else {	//****** Grf Other version ******
		fclose(fp);
		ShowError("GRF version %04x not supported\n",getlong(grf_header+0x2a));
		return 4;
	}

	filelist_adjust();	// Unnecessary area release of filelist

	return 0;	// 0:no error
}

/*==========================================
 * Grfio : Resource file check
 *------------------------------------------*/
static void grfio_resourcecheck(void)
{
	char w1[256], w2[256], src[256], dst[256], restable[256], line[256], local[256];
	char *ptr, *buf;
	FILELIST* entry;
	FILELIST fentry;
	int size;
	FILE* fp;
	int i = 0;

	// read resnametable from data directory and return if successful
	grfio_localpath_create(restable, sizeof(restable), "data\\resnametable.txt");

	fp = fopen(restable, "rb");
	if (fp) {
		while(fgets(line, sizeof(line), fp))
		{
			if (sscanf(line, "%[^#]#%[^#]#", w1, w2) == 2 &&
				// we only need the maps' GAT and RSW files
				(strstr(w2, ".gat") || strstr(w2, ".rsw")))
			{
				sprintf(src, "data\\%s", w1);
				sprintf(dst, "data\\%s", w2);
				entry = filelist_find(dst);
				// create new entries reusing the original's info
				if (entry != NULL)
				{// alias for GRF resource
					memcpy(&fentry, entry, sizeof(FILELIST));
					safestrncpy(fentry.fn, src, sizeof(fentry.fn));
					fentry.fnd = aStrdup(dst);
					filelist_modify(&fentry);
					i++;
				}
				else
				{
					grfio_localpath_create(local, sizeof(local), dst);

					if( exists(local) )
					{// alias for local resource
						memset(&fentry, 0, sizeof(fentry));
						//fentry.gentry = 0;
						safestrncpy(fentry.fn, src, sizeof(fentry.fn));
						fentry.fnd = aStrdup(dst);
						filelist_modify(&fentry);
						i++;
					}
				}
			}
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", i, "resnametable.txt");
		return;	// we're done here!
	}
	
	// read resnametable from loaded GRF's, only if it cannot be loaded from the data directory
	buf = (char *)grfio_reads("data\\resnametable.txt", &size);
	if (buf) {
		buf[size] = 0;

		ptr = buf;
		while (ptr - buf < size) {
			if (sscanf(ptr, "%[^#]#%[^#]#", w1, w2) == 2 &&
				(strstr(w2, ".gat") || strstr(w2, ".rsw")))
			{
				sprintf(src, "data\\%s", w1);
				sprintf(dst, "data\\%s", w2);
				entry = filelist_find(dst);
				if (entry != NULL)
				{// alias for GRF resource
					memcpy(&fentry, entry, sizeof(FILELIST));
					safestrncpy(fentry.fn, src, sizeof(fentry.fn));
					fentry.fnd = aStrdup(dst);
					filelist_modify(&fentry);
					i++;
				}
				else
				{
					grfio_localpath_create(local, sizeof(local), dst);

					if( exists(local) )
					{// alias for local resource
						memset(&fentry, 0, sizeof(fentry));
						//fentry.gentry = 0;
						safestrncpy(fentry.fn, src, sizeof(fentry.fn));
						fentry.fnd = aStrdup(dst);
						filelist_modify(&fentry);
						i++;
					}
				}
			}
			ptr = strchr(ptr, '\n');	// Next line
			if (!ptr) break;
			ptr++;
		}
		aFree(buf);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", i, "data\\resnametable.txt");
		return;
	}

}

// reads a grf file and adds it to the list
static int grfio_add(char* fname)
{
	#define	GENTRY_ADDS	4	// The number increment of gentry_table entries

	if (gentry_entrys >= gentry_maxentry) {
		gentry_maxentry += GENTRY_ADDS;
		gentry_table = (char**)aRealloc(gentry_table, gentry_maxentry * sizeof(char*));
		memset(gentry_table + (gentry_maxentry - GENTRY_ADDS), 0, sizeof(char*) * GENTRY_ADDS);
	}

	gentry_table[gentry_entrys++] = aStrdup(fname);

	return grfio_entryread(fname, gentry_entrys - 1);
}

// removes all entries
void grfio_final(void)
{
	if (filelist != NULL) {
		int i;
		for (i = 0; i < filelist_entrys; i++)
			if (filelist[i].fnd != NULL)
				aFree(filelist[i].fnd);

		aFree(filelist);
		filelist = NULL;
	}
	filelist_entrys = filelist_maxentry = 0;

	if (gentry_table != NULL) {
		int i;
		for (i = 0; i < gentry_entrys; i++)
			if (gentry_table[i] != NULL)
				aFree(gentry_table[i]);

		aFree(gentry_table);
		gentry_table = NULL;
	}
	gentry_entrys = gentry_maxentry = 0;
}

/*==========================================
 * Grfio : Initialize
 *------------------------------------------*/
void grfio_init(char* fname)
{
	FILE* data_conf;
	char line[1024], w1[1024], w2[1024];
	int grf_num = 0;

	hashinit();	// hash table initialization

	data_conf = fopen(fname, "r");
	// It will read, if there is grf-files.txt.
	if (data_conf) {
		while(fgets(line, sizeof(line), data_conf))
		{
			if (line[0] == '/' && line[1] == '/')
				continue;
			if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
				continue;
			// Entry table reading
			if(strcmp(w1, "grf") == 0)	// GRF file
				grf_num += (grfio_add(w2) == 0);
			else if(strcmp(w1,"data_dir") == 0) {	// Data directory
				safestrncpy(data_dir, w2, sizeof(data_dir));
			}
		}
		fclose(data_conf);
		ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n", fname);
	} // end of reading grf-files.txt

	if (grf_num == 0) {
		ShowInfo("No GRF loaded, using default data directory\n");
	}

	// Unneccessary area release of filelist
	filelist_adjust();

	// Resource check
	grfio_resourcecheck();

	return;
}
