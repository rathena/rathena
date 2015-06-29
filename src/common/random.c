// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/showmsg.h"
#include "../common/timer.h" // gettick
#include "random.h"
#if defined(WIN32)
	#include "../common/winapi.h"
#elif defined(HAVE_GETPID) || defined(HAVE_GETTID)
	#include <sys/types.h>
	#include <unistd.h>
#endif

#include <mt19937ar.h> // init_genrand, genrand_int32, genrand_res53


/// Initializes the random number generator with an appropriate seed.
void rnd_init(void)
{
	uint32 seed = gettick();
	seed += (uint32)time(NULL);
#if defined(WIN32)
	seed += GetCurrentProcessId();
	seed += GetCurrentThreadId();
#else
#if defined(HAVE_GETPID)
	seed += (uint32)getpid();
#endif // HAVE_GETPID
#if defined(HAVE_GETTID)
	seed += (uint32)gettid();
#endif // HAVE_GETTID
#endif
	init_genrand(seed);
}


/// Initializes the random number generator.
void rnd_seed(uint32 seed)
{
	init_genrand(seed);
}


/// Generates a random number in the interval [0, SINT32_MAX]
int32 rnd(void)
{
	return (int32)genrand_int31();
}


/// Generates a random number in the interval [0, dice_faces)
/// NOTE: interval is open ended, so dice_faces is excluded (unless it's 0)
uint32 rnd_roll(uint32 dice_faces)
{
	return (uint32)(rnd_uniform()*dice_faces);
}


/// Generates a random number in the interval [min, max]
/// Returns min if range is invalid.
int32 rnd_value(int32 min, int32 max)
{
	if( min >= max )
		return min;
	return min + (int32)(rnd_uniform()*(max-min+1));
}


/// Generates a random number in the interval [0.0, 1.0)
/// NOTE: interval is open ended, so 1.0 is excluded
double rnd_uniform(void)
{
	return ((uint32)genrand_int32())*(1.0/4294967296.0);// divided by 2^32
}


/// Generates a random number in the interval [0.0, 1.0) with 53-bit resolution
/// NOTE: interval is open ended, so 1.0 is excluded
/// NOTE: 53 bits is the maximum precision of a double
double rnd_uniform53(void)
{
	return genrand_res53();
}
