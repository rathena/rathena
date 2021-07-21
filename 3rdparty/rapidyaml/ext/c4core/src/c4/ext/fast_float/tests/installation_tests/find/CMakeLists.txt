cmake_minimum_required(VERSION 3.15)

project(test_simdjson_install VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC_VERSION GREATER 1910)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -permissive-")
endif()
find_package(FastFloat REQUIRED)



file(WRITE main.cpp "
#include \"fast_float/fast_float.h\"
#include <iostream>
 
int main() {
    const std::string input =  \"3.1416 xyz \";
    double result;
    auto answer = fast_float::from_chars(input.data(), input.data()+input.size(), result);
    if(answer.ec != std::errc()) { std::cerr << \"parsing failure\\n\"; return EXIT_FAILURE; }
    std::cout << \"parsed the number \" << result << std::endl;
    return EXIT_SUCCESS;
}")



add_executable(repro main.cpp)
target_link_libraries(repro PUBLIC FastFloat::fast_float)