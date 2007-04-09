// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "grfio.h"
#include "malloc.h"
#include "../zlib/unzip.h"

#ifdef __WIN32
	#include "../zlib/zlib.h"
	#include "../zlib/iowin32.h"
#else
	#ifndef __FREEBSD__
		#include <zlib.h>
	#endif
#endif

int decode_zip(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen)
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

int encode_zip(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen) {
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
		inflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*destLen = stream.total_out;

	err = deflateEnd(&stream);
	return err;
}

/* ===================================
* Unzips a file. 1: success, 0: error
* Adapted from miniunz.c [Celest]
* Version 1.01b, May 30th, 2004
* Copyright (C) 1998-2004 Gilles Vollant
* -------------------------------------
*/
int deflate_file (const char *source, const char *filename)
{
#ifdef _WIN32
	zlib_filefunc_def ffunc;
#endif
	unzFile uf = NULL;
	int err = UNZ_OK;
	uInt size_buf = 8192;
	FILE *fout = NULL;
	void *buf;

#ifdef _WIN32
	fill_win32_filefunc(&ffunc);
	uf = unzOpen2(source, &ffunc);
#else
	uf = unzOpen(source);
#endif

	if (uf == NULL) {
		//printf("Cannot open %s\n", source);
		return 0;
	}
	//printf("%s opened\n", source);

	if (unzLocateFile(uf, filename, 0) != UNZ_OK) {
		//printf("file %s not found in the zipfile\n", filename);
		return 0;
	}

	err = unzOpenCurrentFilePassword(uf, NULL);
	//if (err != UNZ_OK)
	//	printf("error %d with zipfile in unzOpenCurrentFilePassword\n", err);

	fout = fopen(filename,"wb");
	if (fout == NULL) {
		//printf("error opening %s\n", filename);
		return 0;
	}

	buf = (void *)aMalloc(size_buf);
	do {
		err = unzReadCurrentFile(uf, buf, size_buf);
		if (err < 0) {
			//printf("error %d with zipfile in unzReadCurrentFile\n", err);
			break;
		}
		if (err > 0 &&
			fwrite(buf, err, 1, fout)!=1)
		{
			//printf("error in writing extracted file\n");
			err = UNZ_ERRNO;
			break;
		}
	} while (err > 0);
	
	if (fout) fclose(fout);

	if (err == UNZ_OK) {
		err = unzCloseCurrentFile (uf);
		//if (err != UNZ_OK)
		//	printf("error %d with zipfile in unzCloseCurrentFile\n", err);
		aFree(buf);
		return (err == UNZ_OK);
	}
	
	unzCloseCurrentFile(uf); /* don't lose the error */

	return 0;
}
