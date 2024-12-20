// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "random.hpp"

std::uniform_int_distribution<int32> int31_distribution = std::uniform_int_distribution<int32>(0, SINT32_MAX);

/// Generates a random number in the interval [0, SINT32_MAX]
int32 rnd( void ){
	return int31_distribution( generator );
}
