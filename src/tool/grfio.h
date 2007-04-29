// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_GRFIO_H_
#define	_GRFIO_H_

void grfio_init(char*);			// GRFIO Initialize
void grfio_final(void);			// GRFIO Finalize
void* grfio_reads(char*,int*);	// GRFIO data file read & size get
char *grfio_find_file(char *fname);

#define grfio_read(fn) grfio_reads(fn, NULL)

int grfio_size(char*);			// GRFIO data file size get
unsigned long grfio_crc32(const unsigned char *buf, unsigned int len);

int decode_zip(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);
int encode_zip(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);

#endif /* _GRFIO_H_ */
