
#include "fast_float/fast_float.h"
#include <iostream>
#include <string>
#include <system_error>
 
int main() {
    const std::string input =  "3.1416 xyz ";
    double result;
    auto answer = fast_float::from_chars(input.data(), input.data()+input.size(), result);
    if((answer.ec != std::errc()) || ((result != 3.1416))) { std::cerr << "parsing failure\n"; return EXIT_FAILURE; }
    std::cout << "parsed the number " << result << std::endl;
    return EXIT_SUCCESS;
}
