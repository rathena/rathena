cmake_minimum_required(VERSION 3.15)

project(test_simdjson_install VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC_VERSION GREATER 1910)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -permissive-")
endif()

find_package(FastFloat REQUIRED)



file(WRITE test.h "
#pragma once
#include \"fast_float/fast_float.h\"")

file(WRITE main.cpp "
#include \"test.h\"
int main() { return 0; }")

file(WRITE foo.cpp "
#include \"test.h\"
void foo() { }")
add_executable(issue72 main.cpp main.cpp)
target_link_libraries(issue72 PUBLIC FastFloat::fast_float)