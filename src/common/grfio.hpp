// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef GRFIO_HPP
#define GRFIO_HPP

#include "cbasetypes.hpp"

const int32 RSW_NO_WATER = 1000000;

void grfio_init(const char* fname);
void grfio_final(void);
void* grfio_reads(const char* fname, size_t* size = nullptr);
char* grfio_find_file(const char* fname);
int32 grfio_read_rsw_water_level( const char* fname );

unsigned long grfio_crc32(const unsigned char *buf, uint32 len);
int32 decode_zip(void* dest, unsigned long* destLen, const void* source, unsigned long sourceLen);
int32 encode_zip(void* dest, unsigned long* destLen, const void* source, unsigned long sourceLen);

#endif /* GRFIO_HPP */
