cmake_minimum_required(VERSION 3.1...4.0)

# Extract version from configure.ac.
set(VERSION_REGEX "^AC_INIT\\(\\[libconfig\\],[ \t]*\\[([0-9.]+)\\],.*")
file(STRINGS "configure.ac"
  VERSION_STRING REGEX ${VERSION_REGEX})
string(REGEX REPLACE ${VERSION_REGEX} "\\1" VERSION_STRING "${VERSION_STRING}")

project(libconfig LANGUAGES C CXX VERSION ${VERSION_STRING})
option(BUILD_EXAMPLES "Enable examples" OFF)
option(BUILD_SHARED_LIBS  "Enable shared library" OFF)
option(BUILD_TESTS "Enable tests" OFF)
option(BUILD_FUZZERS "Enable fuzzers" OFF)

set_property(GLOBAL	PROPERTY USE_FOLDERS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)

#Disabled rA
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

include(GNUInstallDirs)
include(CheckSymbolExists)
add_subdirectory(lib)

if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if(BUILD_TESTS)
	enable_testing()
	add_subdirectory(tinytest)
	add_subdirectory(tests)
endif()

if (BUILD_FUZZERS AND DEFINED ENV{LIB_FUZZING_ENGINE})
	add_subdirectory(fuzz)
endif()
