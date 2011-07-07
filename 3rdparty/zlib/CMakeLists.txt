
#
# local copy
#
if( WIN32 )
message( STATUS "Detecting local ZLIB" )
find_path( ZLIB_LOCAL_INCLUDE_DIRS "zlib.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include"
	NO_DEFAULT_PATH )
find_library( ZLIB_LOCAL_LIBRARIES
	NAMES zdll
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib"
	NO_DEFAULT_PATH )
mark_as_advanced( ZLIB_LOCAL_LIBRARIES )
mark_as_advanced( ZLIB_LOCAL_INCLUDE_DIRS )

if( ZLIB_LOCAL_LIBRARIES AND ZLIB_LOCAL_INCLUDE_DIRS )
	file( STRINGS "${ZLIB_LOCAL_INCLUDE_DIRS}/zlib.h" ZLIB_H REGEX "^#define[ \t]+ZLIB_VERSION[ \t]+\"[^\"]+\".*$" )
	string( REGEX REPLACE "^.*ZLIB_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" ZLIB_VERSION "${ZLIB_H}" )
	message( STATUS "Found local ZLIB: ${ZLIB_LOCAL_LIBRARIES} (found version ${ZLIB_VERSION})" )
	set( HAVE_LOCAL_ZLIB ON
		CACHE BOOL "zlib is available as a local copy" )
	mark_as_advanced( HAVE_LOCAL_ZLIB )
else()
	foreach( _VAR ZLIB_LOCAL_LIBRARIES ZLIB_LOCAL_INCLUDE_DIRS )
		if( NOT "${_VAR}" )
			set( MISSING_VARS ${MISSING_VARS} ${_VAR} )
		endif()
	endforeach()
	message( STATUS "Could NOT find local ZLIB (missing: ${MISSING_VARS})" )
	unset( HAVE_LOCAL_ZLIB CACHE )
endif()
message( STATUS "Detecting local ZLIB - done" )
endif( WIN32 )


#
# system
#
message( STATUS "Detecting system ZLIB" )
unset( ZLIB_LIBRARIES CACHE )
unset( ZLIB_INCLUDE_DIRS CACHE )
find_package( ZLIB )
set( ZLIB_SYSTEM_LIBRARIES ${ZLIB_LIBRARIES}
	CACHE PATH "system zlib libraries" )
set( ZLIB_SYSTEM_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS}
	CACHE PATH "system zlib include directories" )
mark_as_advanced( ZLIB_SYSTEM_LIBRARIES )
mark_as_advanced( ZLIB_SYSTEM_INCLUDE_DIRS )

if( ZLIB_SYSTEM_LIBRARIES AND ZLIB_SYSTEM_INCLUDE_DIRS )
	set( HAVE_SYSTEM_ZLIB ON
		CACHE BOOL "zlib is available on the system" )
	mark_as_advanced( HAVE_SYSTEM_ZLIB )
else()
	unset( HAVE_SYSTEM_ZLIB CACHE )
endif()
message( STATUS "Detecting system ZLIB - done" )


#
# configure
#
CONFIGURE_WITH_LOCAL_OR_SYSTEM( ZLIB )
