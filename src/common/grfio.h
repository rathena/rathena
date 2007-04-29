// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_GRFIO_H_
#define	_GRFIO_H_

int decode_zip(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);
int encode_zip(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);

#endif /* _GRFIO_H_ */
