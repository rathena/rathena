// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <algorithm>
#include <random>
#include <type_traits>
#include <vector>

#include "cbasetypes.hpp"

inline std::random_device device;
inline std::mt19937 generator = std::mt19937(device());

int32 rnd(void);// [0, SINT32_MAX]

/*
 * Generates a random number in the interval [min, max]
 * @return random number
 */
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type rnd_value(T min, T max) {
	if (min > max) {
		std::swap(min, max);
	}
	std::uniform_int_distribution<T> dist(min, max);
	return dist(generator);
}

/*
 * Simulates a chance based on a given probability
 * @return true if succeeded / false if it didn't
 */
template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type rnd_chance(T chance, T base) {
	return rnd_value<T>(1, base) <= chance;
}

/*
 * Simulates a chance based on a given probability
 * This considers the official inaccuracy where a random value between 0 and 20000 is generated first
 * and the taken modulo to the base. That means there's always an increased chance that the result is 0.
 * For example if base is 10000, there is a 3/20001 chance that the value is 0 (0, 10000 and 20000).
 * @return true if succeeded / false if it didn't
 */
template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type rnd_chance_official(T chance, T base) {
	return rnd_value<T>(0, 20000)%base < chance;
}

template <typename T>
void rnd_vector_order( std::vector<T>& vec ){
	std::shuffle( std::begin( vec ), std::end( vec ), generator );
}

#endif /* RANDOM_HPP */
