// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "grfio.h"
#include <zlib.h>

#include "../common/cbasetypes.h"
#include "../common/showmsg.h"
#include "../common/malloc.h"


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

//----------------------------
//	grf decode data table
//----------------------------
static unsigned char BitMaskTable[8] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

static char	BitSwapTable1[64] = {
	58, 50, 42, 34, 26, 18, 10,  2, 60, 52, 44, 36, 28, 20, 12,  4,
	62, 54, 46, 38, 30, 22, 14,  6, 64, 56, 48, 40, 32, 24, 16,  8,
	57, 49, 41, 33, 25, 17,  9,  1, 59, 51, 43, 35, 27, 19, 11,  3,
	61, 53, 45, 37, 29, 21, 13,  5, 63, 55, 47, 39, 31, 23, 15,  7
};
static char	BitSwapTable2[64] = {
	40,  8, 48, 16, 56, 24, 64, 32, 39,  7, 47, 15, 55, 23, 63, 31,
	38,  6, 46, 14, 54, 22, 62, 30, 37,  5, 45, 13, 53, 21, 61, 29,
	36,  4, 44, 12, 52, 20, 60, 28, 35,  3, 43, 11, 51, 19, 59, 27,
	34,  2, 42, 10, 50, 18, 58, 26, 33,  1, 41,  9, 49, 17, 57, 25
};
static char	BitSwapTable3[32] = {
	16,  7, 20, 21, 29, 12, 28, 17,  1, 15, 23, 26,  5, 18, 31, 10,
     2,  8, 24, 14, 32, 27,  3,  9, 19, 13, 30,  6, 22, 11,  4, 25
};

static unsigned char NibbleData[4][64]={
	{
		0xef, 0x03, 0x41, 0xfd, 0xd8, 0x74, 0x1e, 0x47,  0x26, 0xef, 0xfb, 0x22, 0xb3, 0xd8, 0x84, 0x1e,
		0x39, 0xac, 0xa7, 0x60, 0x62, 0xc1, 0xcd, 0xba,  0x5c, 0x96, 0x90, 0x59, 0x05, 0x3b, 0x7a, 0x85,
		0x40, 0xfd, 0x1e, 0xc8, 0xe7, 0x8a, 0x8b, 0x21,  0xda, 0x43, 0x64, 0x9f, 0x2d, 0x14, 0xb1, 0x72,
		0xf5, 0x5b, 0xc8, 0xb6, 0x9c, 0x37, 0x76, 0xec,  0x39, 0xa0, 0xa3, 0x05, 0x52, 0x6e, 0x0f, 0xd9,
	}, {
		0xa7, 0xdd, 0x0d, 0x78, 0x9e, 0x0b, 0xe3, 0x95,  0x60, 0x36, 0x36, 0x4f, 0xf9, 0x60, 0x5a, 0xa3,
		0x11, 0x24, 0xd2, 0x87, 0xc8, 0x52, 0x75, 0xec,  0xbb, 0xc1, 0x4c, 0xba, 0x24, 0xfe, 0x8f, 0x19,
		0xda, 0x13, 0x66, 0xaf, 0x49, 0xd0, 0x90, 0x06,  0x8c, 0x6a, 0xfb, 0x91, 0x37, 0x8d, 0x0d, 0x78,
		0xbf, 0x49, 0x11, 0xf4, 0x23, 0xe5, 0xce, 0x3b,  0x55, 0xbc, 0xa2, 0x57, 0xe8, 0x22, 0x74, 0xce,
	}, {
		0x2c, 0xea, 0xc1, 0xbf, 0x4a, 0x24, 0x1f, 0xc2,  0x79, 0x47, 0xa2, 0x7c, 0xb6, 0xd9, 0x68, 0x15,
		0x80, 0x56, 0x5d, 0x01, 0x33, 0xfd, 0xf4, 0xae,  0xde, 0x30, 0x07, 0x9b, 0xe5, 0x83, 0x9b, 0x68,
		0x49, 0xb4, 0x2e, 0x83, 0x1f, 0xc2, 0xb5, 0x7c,  0xa2, 0x19, 0xd8, 0xe5, 0x7c, 0x2f, 0x83, 0xda,
		0xf7, 0x6b, 0x90, 0xfe, 0xc4, 0x01, 0x5a, 0x97,  0x61, 0xa6, 0x3d, 0x40, 0x0b, 0x58, 0xe6, 0x3d,
	}, {
		0x4d, 0xd1, 0xb2, 0x0f, 0x28, 0xbd, 0xe4, 0x78,  0xf6, 0x4a, 0x0f, 0x93, 0x8b, 0x17, 0xd1, 0xa4,
		0x3a, 0xec, 0xc9, 0x35, 0x93, 0x56, 0x7e, 0xcb,  0x55, 0x20, 0xa0, 0xfe, 0x6c, 0x89, 0x17, 0x62,
		0x17, 0x62, 0x4b, 0xb1, 0xb4, 0xde, 0xd1, 0x87,  0xc9, 0x14, 0x3c, 0x4a, 0x7e, 0xa8, 0xe2, 0x7d,
		0xa0, 0x9f, 0xf6, 0x5c, 0x6a, 0x09, 0x8d, 0xf0,  0x0f, 0xe3, 0x53, 0x25, 0x95, 0x36, 0x28, 0xcb,
	}
};

// little endian char array to uint conversion
static unsigned int getlong(unsigned char* p)
{
	return (p[0] | p[1] << 0x08 | p[2] << 0x10 | p[3] << 0x18);
}

/*==========================================
 *	Grf data decode : Subs
 *------------------------------------------*/
static void NibbleSwap(unsigned char* Src, int len)
{
	for(;0<len;len--,Src++) {
		*Src = (*Src>>4) | (*Src<<4);
	}
}

static void BitConvert(unsigned char* Src, char* BitSwapTable)
{
	int lop,prm;
	unsigned char tmp[8];
	memset(tmp,0,8);
	for(lop=0;lop!=64;lop++) {
		prm = BitSwapTable[lop]-1;
		if (Src[(prm >> 3) & 7] & BitMaskTable[prm & 7]) {
			tmp[(lop >> 3) & 7] |= BitMaskTable[lop & 7];
		}
	}
	memcpy(Src,tmp,8);
}

static void BitConvert4(unsigned char* Src)
{
	int lop,prm;
	unsigned char tmp[8];
	tmp[0] = ((Src[7]<<5) | (Src[4]>>3)) & 0x3f;	// ..0 vutsr
	tmp[1] = ((Src[4]<<1) | (Src[5]>>7)) & 0x3f;	// ..srqpo n
	tmp[2] = ((Src[4]<<5) | (Src[5]>>3)) & 0x3f;	// ..o nmlkj
	tmp[3] = ((Src[5]<<1) | (Src[6]>>7)) & 0x3f;	// ..kjihg f
	tmp[4] = ((Src[5]<<5) | (Src[6]>>3)) & 0x3f;	// ..g fedcb
	tmp[5] = ((Src[6]<<1) | (Src[7]>>7)) & 0x3f;	// ..cba98 7
	tmp[6] = ((Src[6]<<5) | (Src[7]>>3)) & 0x3f;	// ..8 76543
	tmp[7] = ((Src[7]<<1) | (Src[4]>>7)) & 0x3f;	// ..43210 v

	for(lop=0;lop!=4;lop++) {
		tmp[lop] = (NibbleData[lop][tmp[lop*2]] & 0xf0)
		         | (NibbleData[lop][tmp[lop*2+1]] & 0x0f);
	}

	memset(tmp+4,0,4);
	for(lop=0;lop!=32;lop++) {
		prm = BitSwapTable3[lop]-1;
		if (tmp[prm >> 3] & BitMaskTable[prm & 7]) {
			tmp[(lop >> 3) + 4] |= BitMaskTable[lop & 7];
		}
	}
	Src[0] ^= tmp[4];
	Src[1] ^= tmp[5];
	Src[2] ^= tmp[6];
	Src[3] ^= tmp[7];
}

static void decode_des_etc(unsigned char* buf, size_t len, int type, int cycle)
{
	size_t lop,cnt=0;
	if(cycle<3) cycle=3;
	else if(cycle<5) cycle++;
	else if(cycle<7) cycle+=9;
	else cycle+=15;

	for(lop=0; lop*8<len; lop++, buf+=8)
	{
		if(lop<20 || (type==0 && lop%cycle==0)) { // des
			BitConvert(buf,BitSwapTable1);
			BitConvert4(buf);
			BitConvert(buf,BitSwapTable2);
		} else {
			if(cnt==7 && type==0) {
				unsigned char a;
				unsigned char tmp[8];
				memcpy(tmp,buf,8);
				cnt=0;
				buf[0]=tmp[3];
				buf[1]=tmp[4];
				buf[2]=tmp[6];
				buf[3]=tmp[0];
				buf[4]=tmp[1];
				buf[5]=tmp[2];
				buf[6]=tmp[5];
				a=tmp[7];
				if(a==0x00) a=0x2b;
				else if(a==0x2b) a=0x00;
				else if(a==0x01) a=0x68;
				else if(a==0x68) a=0x01;
				else if(a==0x48) a=0x77;
				else if(a==0x77) a=0x48;
				else if(a==0x60) a=0xff;
				else if(a==0xff) a=0x60;
				else if(a==0x6c) a=0x80;
				else if(a==0x80) a=0x6c;
				else if(a==0xb9) a=0xc0;
				else if(a==0xc0) a=0xb9;
				else if(a==0xeb) a=0xfe;
				else if(a==0xfe) a=0xeb;
				buf[7]=a;
			}
			cnt++;
		}
	}
}
/*==========================================
 *	Grf data decode sub : zip
 *------------------------------------------*/
int decode_zip(unsigned char* dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen)
{
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	/* Check for source > 64K on 16-bit machine: */
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = (Bytef*) dest;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = inflateInit(&stream);
	if (err != Z_OK) return err;

	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		inflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*destLen = stream.total_out;

	err = inflateEnd(&stream);
	return err;
}

int encode_zip(unsigned char* dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen)
{
	z_stream stream;
	int err;
	memset(&stream, 0, sizeof(stream));
	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	/* Check for source > 64K on 16-bit machine: */
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = (Bytef*) dest;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = deflateInit(&stream,Z_DEFAULT_COMPRESSION);
	if (err != Z_OK) return err;

	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*destLen = stream.total_out;

	err = deflateEnd(&stream);
	return err;
}

unsigned long grfio_crc32 (const unsigned char* buf, unsigned int len)
{
	return crc32(crc32(0L, Z_NULL, 0), buf, len);
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

		snprintf(lfname, sizeof lfname, "%s%s", data_dir, fname);

		for (p = &lfname[0]; *p != 0; p++)
			if (*p=='\\') *p = '/';

		if (stat(lfname, &st) == 0) {
			strncpy(lentry.fn, fname, sizeof(lentry.fn) - 1);
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
		char lfname[256], *p;
		FILELIST lentry;

		snprintf(lfname, sizeof lfname, "%s%s", data_dir, fname);
		
		for (p = &lfname[0]; *p != 0; p++)
			if (*p == '\\') *p = '/';

		in = fopen(lfname, "rb");
		if (in != NULL) {
			if (entry != NULL && entry->gentry == 0) {
				lentry.declen = entry->declen;
			} else {
				fseek(in,0,SEEK_END);
				lentry.declen = ftell(in);
			}
			fseek(in,0,SEEK_SET);
			buf2 = (unsigned char *)aMallocA(lentry.declen + 1024);
			fread(buf2, 1, lentry.declen, in);
			fclose(in);
			strncpy(lentry.fn, fname, sizeof(lentry.fn) - 1);
			lentry.fnd = NULL;
			lentry.gentry = 0;	// 0:LocalFile
			entry = filelist_modify(&lentry);
		} else {
			if (entry != NULL && entry->gentry < 0) {
				entry->gentry = -entry->gentry;	// local file checked
			} else {
				ShowError("%s not found (grfio_reads - local file %s)\n", fname, lfname);
				return NULL;
			}
		}
	}
	if (entry != NULL && entry->gentry > 0) {	// Archive[GRF] File Read
		char* grfname = gentry_table[entry->gentry - 1];
		in = fopen(grfname, "rb");
		if(in != NULL) {
			unsigned char *buf = (unsigned char *)aMallocA(entry->srclen_aligned + 1024);
			fseek(in, entry->srcpos, 0);
			fread(buf, 1, entry->srclen_aligned, in);
			fclose(in);
			buf2 = (unsigned char *)aMallocA(entry->declen + 1024);
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
			aFree(buf);
		} else {
			ShowError("%s not found (grfio_reads - GRF file %s)\n", fname, grfname);
			return NULL;
		}
	}
	if (size != NULL && entry != NULL)
		*size = entry->declen;

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
		BitConvert(&buf[lop],BitSwapTable1);
		BitConvert4(&buf[lop]);
		BitConvert(&buf[lop],BitSwapTable2);
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
				strncpy(aentry.fn, fname,sizeof(aentry.fn)-1);
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
				strncpy(aentry.fn,fname,sizeof(aentry.fn)-1);
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
	char w1[256], w2[256], src[256], dst[256], restable[256], line[256];
	char *ptr, *buf;
	FILELIST* entry;
	int size;
	FILE* fp;
	int i = 0;

	// read resnametable from data directory and return if successful
	snprintf(restable, sizeof restable, "%sdata\\resnametable.txt", data_dir);
	for (ptr = &restable[0]; *ptr != 0; ptr++)
		if (*ptr == '\\') *ptr = '/';

	fp = fopen(restable, "rb");
	if (fp) {
		while(fgets(line, sizeof(line), fp))
		{
			if (sscanf(line, "%[^#]#%[^#]#", w1, w2) == 2 &&
				// we only need the maps' GAT and RSW files
				(strstr(w2, ".gat") || strstr(w2, ".rsw")))
			{
				snprintf(src, sizeof src, "data\\%s", w1);
				snprintf(dst, sizeof dst, "data\\%s", w2);
				entry = filelist_find(dst);
				// create new entries reusing the original's info
				if (entry != NULL) {
					FILELIST fentry;
					memcpy(&fentry, entry, sizeof(FILELIST));
					strncpy(fentry.fn, src, sizeof(fentry.fn) - 1);
					fentry.fnd = aStrdup(dst);
					filelist_modify(&fentry);
					i++;
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
				snprintf(src, sizeof src, "data\\%s", w1);
				snprintf(dst, sizeof dst, "data\\%s", w2);
				entry = filelist_find(dst);
				if (entry != NULL) {
					FILELIST fentry;
					memcpy(&fentry, entry, sizeof(FILELIST));
					strncpy(fentry.fn, src, sizeof(fentry.fn) - 1);
					fentry.fnd = aStrdup(dst);
					filelist_modify(&fentry);
					i++;
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
				strcpy(data_dir, w2);
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
