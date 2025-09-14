// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MD5CALC_HPP
#define MD5CALC_HPP

#include "cbasetypes.hpp"

void MD5_String(const char * string, char * output);
void MD5_Binary(const char * string, unsigned char * output);
void MD5_Salt(size_t len, char * output);

#endif /* MD5CALC_HPP */
