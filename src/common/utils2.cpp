#include "utils2.hpp"

#include <chrono>
#include <string>
#include <algorithm>
#include <iostream>
#include <numeric> //iota


using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::microseconds;

struct cScopeTimer::sPimpl {
    time_point<Clock> start;
    time_point<Clock> end;
    
    sPimpl()
    {
        start = Clock::now();
    }
    
    ~sPimpl(){
        end = Clock::now();
        microseconds diff = duration_cast<microseconds>(end - start);
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
int levenshtein(const std::string &s1, const std::string &s2)
{
	// To change the type this function manipulates and returns, change
	// the return type and the types of the two variables below.
	int s1len = static_cast<int>(s1.size());
	int s2len = static_cast<int>(s2.size());
	
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
