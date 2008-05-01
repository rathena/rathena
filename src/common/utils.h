// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _UTILS_H_
#define _UTILS_H_

#ifndef _CBASETYPES_H_
#include "../common/cbasetypes.h"
#endif

#include <stdio.h> // FILE*

// generate a hex dump of the first 'length' bytes of 'buffer'
void dump(FILE* fp, const unsigned char* buffer, int length);

void findfile(const char *p, const char *pat, void (func)(const char*));

//Caps values to min/max
#define cap_value(a, min, max) ((a >= max) ? max : (a <= min) ? min : a)

/// calculates the value of A / B, in percent (rounded down)
unsigned int get_percentage(const unsigned int A, const unsigned int B);

//////////////////////////////////////////////////////////////////////////
// byte word dword access [Shinomori]
//////////////////////////////////////////////////////////////////////////

extern uint8 GetByte(uint32 val, int idx);
extern uint16 GetWord(uint32 val, int idx);
extern uint16 MakeWord(uint8 byte0, uint8 byte1);
extern uint32 MakeDWord(uint16 word0, uint16 word1);

#endif /* _UTILS_H_ */
