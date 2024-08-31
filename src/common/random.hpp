// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <type_traits>
#include <random>

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

#endif /* RANDOM_HPP */
