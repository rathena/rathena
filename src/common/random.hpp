// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include "cbasetypes.hpp"

void rnd_init(void);

int32 rnd(void);// [0, SINT32_MAX]
int32 rnd_value(int32 min, int32 max);// [min, max]
bool rnd_chance( uint16 chance, uint16 base );

#endif /* RANDOM_HPP */
