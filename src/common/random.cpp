// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "random.hpp"

#include <random>

std::mt19937 generator;
std::uniform_int_distribution<int32> int31_distribution;
std::uniform_int_distribution<uint32> uint32_distribution;

/// Initializes the random number generator
void rnd_init( void ){
	std::random_device device;
	generator = std::mt19937( device() );
	int31_distribution = std::uniform_int_distribution<int32>( 0, SINT32_MAX );
	uint32_distribution = std::uniform_int_distribution<uint32>( 0, UINT32_MAX );
}

/// Generates a random number in the interval [0, SINT32_MAX]
int32 rnd( void ){
	return int31_distribution( generator );
}

/// Generates a random number in the interval [0, UINT32_MAX]
uint32 rnd_uint32( void ){
	return uint32_distribution( generator );
}

/// Generates a random number in the interval [0.0, 1.0)
/// NOTE: interval is open ended, so 1.0 is excluded
double rnd_uniform( void ){
	return rnd_uint32() * ( 1.0 / 4294967296.0 );// divided by 2^32
}

/// Generates a random number in the interval [min, max]
/// Returns min if range is invalid.
int32 rnd_value( int32 min, int32 max ){
	if( min >= max ){
		return min;
	}

	return min + (int32)( rnd_uniform() * ( max - min + 1 ) );
}

