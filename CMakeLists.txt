cmake_minimum_required(VERSION 3.11)

project(rAthena)

# Configure CMake Modules
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake")


# options
set(PACKETVER 20211103 CACHE STRING "Sets the PACKETVER define of the servers (see src/common/mmo.hpp)")
option(ENABLE_PRERENEWAL "Whether or not to enable Pre-renewal (default=OFF)" OFF)
option(ENABLE_WEB_SERVER "Build web-server (default=ON)" ON)
option(ENABLE_RDTSC "Enable RDTSC instruction as a timing source (default=OFF)" OFF)
option(ENABLE_EXTRA_DEBUG_CODE "Enable extra debug code (default=OFF)" OFF)
option(ENABLE_MEMMGR "Enable memory manager (default=ON)" ON)
# TODO(vstumpf): If no one uses this, we can just remove it
# set(ENABLE_MEMORY "system" CACHE STRING "Enable memory library (default=system)")
option(ENABLE_PROFILER "Enable profiler (default=OFF)" OFF)
option(ENABLE_EXTRA_BUILDBOT_CODE "Enable extra buildbot code (default=OFF)" OFF)
option(ENABLE_EPOLL "Use epoll instead of select (default=OFF)" OFF)


# Set a default build type if none was specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel" FORCE)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
endif()

#
# Prevent building in the source directory by default
#
if( CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR )
    message( FATAL_ERROR
        "Do not use the source directory to build your files, instead delete CMakeCache.txt, create a separate folder and build there.\n"
        "Example: (build in subdir 'build' and install to source dir)\n"
        "  rm -f CMakeCache.txt\n"
        "  mkdir build\n"
        "  cd build\n"
        "  cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..\n"
        "  make install\n"
        "  cd ..\n"
        "  rm -rf build\n")
endif()

if(WIN32)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# Configure C++ Standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set build directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${suffixInstallStr})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${suffixInstallStr})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


add_subdirectory(db)
add_subdirectory(conf)
add_subdirectory(3rdparty)
add_subdirectory(src)
add_subdirectory(tools)

add_custom_target(server
    DEPENDS login-server char-server map-server web-server scripts
)
