cmake_minimum_required(VERSION 2.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_SOURCE_DIR}/")

project (ChainedLibconfigExample)
file(GLOB SOURCES *.cpp *.h ../*.h ../*.md)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
else()
	find_package(libconfig)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	#set(CMAKE_BUILD_TYPE Debug)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories(
	${CMAKE_SOURCE_DIR}/../../../lib/
)

if(MSVC)
 link_libraries (
    ${CMAKE_SOURCE_DIR}/../../../Debug/libconfig++.lib
 )
else()
link_libraries (
	${LIBCONFIG_LIBRARY}
)
endif()


add_executable (
   ChainedLibconfigExample
#   WIN32 # Only if you don't want the DOS prompt to appear in the background in Windows
#   MACOSX_BUNDLE
   ${SOURCES} # We could've listed the source files here directly instead of using a variable to store them
   #${INCLUDES}
)
