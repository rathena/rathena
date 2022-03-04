
#include "fast_float/fast_float.h"
#include <iostream>
#include <string>
#include <system_error>
 
int main() {
    const std::string input =  "3,1416 xyz ";
    double result;
    fast_float::parse_options options{fast_float::chars_format::general, ','};
    auto answer = fast_float::from_chars_advanced(input.data(), input.data()+input.size(), result, options);
    if((answer.ec != std::errc()) || ((result != 3.1416))) { std::cerr << "parsing failure\n"; return EXIT_FAILURE; }
    std::cout << "parsed the number " << result << std::endl;
    return EXIT_SUCCESS;
}
