cmake_minimum_required(VERSION 3.13)

if ( NOT ENABLE_WEB_SERVER )
    return()
endif( NOT ENABLE_WEB_SERVER )

SET (this_target httplib)
PROJECT(${this_target})

find_path ( HTTPLIB_INCLUDE_DIRS "httplib.h"
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}"
    NO_DEFAULT_PATH )

find_path ( HTTPLIB_SOURCE_DIR "httplib.cc"
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}"
    NO_DEFAULT_PATH )

mark_as_advanced( HTTPLIB_INCLUDE_DIRS )
mark_as_advanced( HTTPLIB_SOURCE_DIR )

set ( HTTPLIB_HEADERS
    "${HTTPLIB_INCLUDE_DIRS}/httplib.h"
    CACHE INTERNAL "httplib headers" )

set ( HTTPLIB_SOURCES
    "${HTTPLIB_SOURCE_DIR}/httplib.cc"
    CACHE INTERNAL "httplib sources" )

set ( HTTPLIB_DEFINITIONS
    "-std=c++11"
    CACHE INTERNAL "http definitions" )
include_directories( ${HTTPLIB_INCLUDE_DIRS} ${HTTPLIB_SOURCE_DIR} )

ADD_LIBRARY( ${this_target} STATIC ${HTTPLIB_SOURCES} )

if (NOT WIN32)
    target_compile_definitions(${this_target} PUBLIC "CPPHTTPLIB_SEND_FLAGS=MSG_NOSIGNAL")
endif()
