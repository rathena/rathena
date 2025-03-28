// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "utilities.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric> //iota
#include <string>

#include "cbasetypes.hpp"

#ifndef __has_builtin
	#define __has_builtin(x) 0
#endif

struct cScopeTimer::sPimpl {
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point end;
    
    sPimpl()
    {
        start = std::chrono::steady_clock::now();
    }
    
    ~sPimpl(){
        end = std::chrono::steady_clock::now();
        std::chrono::microseconds diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << " took=" << diff.count() << "ms !\n";
    }
};

cScopeTimer::cScopeTimer() 
	: aPimpl(new sPimpl())
{}

/**
 * Calculates the Levenshtein distance of two strings.
 * @author http://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C.2B.2B
 * comparison test was done here http://cpp.sh/2o7w
 */
int32 levenshtein(const std::string &s1, const std::string &s2)
{
	// To change the type this function manipulates and returns, change
	// the return type and the types of the two variables below.
	int32 s1len = static_cast<int32>(s1.size());
	int32 s2len = static_cast<int32>(s2.size());
	
	auto column_start = (decltype(s1len))1;
	
	auto column = new decltype(s1len)[s1len + 1];
	std::iota(column + column_start, column + s1len + 1, column_start);
	
	for (auto x = column_start; x <= s2len; x++) {
		column[0] = x;
		auto last_diagonal = x - column_start;
		for (auto y = column_start; y <= s1len; y++) {
			auto old_diagonal = column[y];
			auto possibilities = {
				column[y] + 1,
				column[y - 1] + 1,
				last_diagonal + (s1[y - 1] == s2[x - 1]? 0 : 1)
			};
			column[y] = std::min(possibilities);
			last_diagonal = old_diagonal;
		}
	}
	auto result = column[s1len];
	delete[] column;
	return result;
}

bool rathena::util::safe_substraction( int64 a, int64 b, int64& result ){
#if __has_builtin( __builtin_sub_overflow ) || ( defined( __GNUC__ ) && !defined( __clang__ ) && defined( GCC_VERSION  ) && GCC_VERSION >= 50100 )
	return __builtin_sub_overflow( a, b, &result );
#else
	bool overflow = false;

	if( b < 0 ){
		if( a > ( INT64_MAX + b ) ){
			overflow = true;
		}
	}else{
		if( a < ( INT64_MIN + b ) ){
			overflow = true;
		}
	}

	result = a - b;

	return overflow;
#endif
}

bool rathena::util::safe_multiplication( int64 a, int64 b, int64& result ){
#if __has_builtin( __builtin_mul_overflow ) || ( defined( __GNUC__ ) && !defined( __clang__ ) && defined( GCC_VERSION  ) && GCC_VERSION >= 50100 )
	return __builtin_mul_overflow( a, b, &result );
#else
	result = a * b;

	if( a > 0 ){
		if( b > 0 ){
			return result < 0;
		}else if( b < 0 ){
			return result > 0;
		}
	}else if( a < 0 ){
		if( b > 0 ){
			return result > 0;
		}else if( b < 0 ){
			return result < 0;
		}
	}

	return false;
#endif
}

void rathena::util::string_left_pad_inplace(std::string& str, char padding, size_t num)
{
	str.insert( 0, std::min( static_cast<size_t>( 0 ), num - str.length() ), padding );
}

std::string rathena::util::string_left_pad(const std::string& original, char padding, size_t num)
{
	return std::string( num - std::min( num, original.length() ), padding ) + original;
}

int32 rathena::util::strtoint32def(const char* str, int32 def) {
	char* str_end{};
	int32 result = std::strtol(str, &str_end, 10);

	if (str_end != nullptr && *str_end != '\0') {
		return def;
	}

	if (errno == ERANGE) {
		return def;
	}

	return result;
}

constexpr char base62_dictionary[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
	'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z'
};

std::string rathena::util::base62_encode( uint32 val ){
	std::string result = "";
	while (val != 0) {
		result = base62_dictionary[(val % 62)] + result;
		val /= 62;
	}
	return result;
}
