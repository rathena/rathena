cmake_minimum_required(VERSION 2.8)

## start setting
SET (this_target libbcrypt)
PROJECT(${this_target})

find_path( LIBBCRYPT_INCLUDE_DIRS "bcrypt/bcrypt.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include"
	NO_DEFAULT_PATH )
find_path( LIBBCRYPT_SOURCE_DIR "bcrypt.c"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src"
	NO_DEFAULT_PATH )
mark_as_advanced( LIBBCRYPT_INCLUDE_DIRS )
mark_as_advanced( LIBBCRYPT_SOURCE_DIR )

set( LIBBCRYPT_HEADERS
    "${LIBBCRYPT_INCLUDE_DIRS}/bcrypt/bcrypt.h"
    "${LIBBCRYPT_INCLUDE_DIRS}/bcrypt/BCrypt.hpp"
    "${LIBBCRYPT_INCLUDE_DIRS}/bcrypt/crypt_blowfish.h"
    "${LIBBCRYPT_INCLUDE_DIRS}/bcrypt/crypt_gensalt.h"
    "${LIBBCRYPT_INCLUDE_DIRS}/bcrypt/crypt.h"
    "${LIBBCRYPT_INCLUDE_DIRS}/bcrypt/ow-crypt.h"
    CACHE INTERNAL "libbcrypt headers" )
set( LIBBCRYPT_SOURCES
  "${LIBBCRYPT_SOURCE_DIR}/bcrypt.c"
  "${LIBBCRYPT_SOURCE_DIR}/crypt_blowfish.c"
  "${LIBBCRYPT_SOURCE_DIR}/crypt_gensalt.c"
  "${LIBBCRYPT_SOURCE_DIR}/wrapper.c"
  "${LIBBCRYPT_SOURCE_DIR}/bcrypt.c"
	CACHE INTERNAL "libbcrypt sources" )
set( LIBBCRYPT_DEFINITIONS
	"-std=c++11"
	CACHE INTERNAL "libbcrypt definitions" )
include_directories(${LIBBCRYPT_INCLUDE_DIRS}/bcrypt ${LIBBCRYPT_SOURCE_DIR})
#message(STATUS "LIBBCRYPT_INCLUDE_DIRS : ${LIBBCRYPT_INCLUDE_DIRS}, LIBBCRYPT_SOURCE_DIR=${LIBBCRYPT_SOURCE_DIR}")
ADD_LIBRARY(${this_target} STATIC ${LIBBCRYPT_SOURCES} )
