// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_GRFIO_H_
#define	_GRFIO_H_

#ifdef __cplusplus
extern "C" {
#endif

void grfio_init(const char* fname);
void grfio_final(void);
void* grfio_reads(const char* fname, int* size);
char* grfio_find_file(const char* fname);
#define grfio_read(fn) grfio_reads(fn, NULL)

unsigned long grfio_crc32(const unsigned char *buf, unsigned int len);
int decode_zip(void* dest, unsigned long* destLen, const void* source, unsigned long sourceLen);
int encode_zip(void* dest, unsigned long* destLen, const void* source, unsigned long sourceLen);

#ifdef __cplusplus
}
#endif

#endif /* _GRFIO_H_ */
