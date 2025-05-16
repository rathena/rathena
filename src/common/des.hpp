// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef DES_HPP
#define DES_HPP

#include <common/cbasetypes.hpp>

/// One 64-bit block.
typedef struct BIT64 { uint8_t b[8]; } BIT64;

void des_decrypt_block(BIT64* block);
void des_decrypt(unsigned char* data, size_t size);

#endif /* DES_HPP */
