
#
# local copy
#
if( WIN32 )
message( STATUS "Detecting local ZLIB" )
find_path( LOCAL_ZLIB_INCLUDE_DIRS "zlib.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include"
	NO_DEFAULT_PATH )
find_library( LOCAL_ZLIB_LIBRARIES
	NAMES zdll
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib"
	NO_DEFAULT_PATH )
mark_as_advanced( LOCAL_ZLIB_LIBRARIES )
mark_as_advanced( LOCAL_ZLIB_INCLUDE_DIRS )

if( LOCAL_ZLIB_LIBRARIES AND LOCAL_ZLIB_INCLUDE_DIRS )
	file( STRINGS "${LOCAL_ZLIB_INCLUDE_DIRS}/zlib.h" ZLIB_H REGEX "^#define[ \t]+ZLIB_VERSION[ \t]+\"[^\"]+\".*$" )
	string( REGEX REPLACE "^.*ZLIB_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" ZLIB_VERSION "${ZLIB_H}" )
	message( STATUS "Found local ZLIB: ${LOCAL_ZLIB_LIBRARIES} (found version ${ZLIB_VERSION})" )
	set( HAVE_LOCAL_ZLIB ON
		CACHE BOOL "zlib is available as a local copy" )
else()
	foreach( _VAR LOCAL_ZLIB_LIBRARIES LOCAL_ZLIB_INCLUDE_DIRS )
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
find_package( ZLIB )
set( SYSTEM_ZLIB_LIBRARIES ${ZLIB_LIBRARIES}
	CACHE PATH "system zlib libraries" )
set( SYSTEM_ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS}
	CACHE PATH "system zlib include directories" )
mark_as_advanced( SYSTEM_ZLIB_LIBRARIES )
mark_as_advanced( SYSTEM_ZLIB_INCLUDE_DIRS )

if( SYSTEM_ZLIB_LIBRARIES AND SYSTEM_ZLIB_INCLUDE_DIRS )
	set( HAVE_SYSTEM_ZLIB ON
		CACHE BOOL "zlib is available on the system" )
else()
	unset( HAVE_SYSTEM_ZLIB CACHE )
endif()
message( STATUS "Detecting system ZLIB - done" )


#
# configure
#
unset( ZLIB_LIBRARIES CACHE )
unset( ZLIB_INCLUDE_DIRS CACHE )
if( HAVE_LOCAL_ZLIB )
	set( USE_LOCAL_ZLIB ON
		CACHE BOOL "use local copy of zlib" )
else()
	unset( USE_LOCAL_ZLIB CACHE )
endif()
if( USE_LOCAL_ZLIB )
	message( STATUS "Configuring for local ZLIB" )
	set( ZLIB_LIBRARIES ${LOCAL_ZLIB_LIBRARIES} )
	set( ZLIB_INCLUDE_DIRS ${LOCAL_ZLIB_INCLUDE_DIRS} )
	message( STATUS "Configuring for local ZLIB - done" )
elseif( HAVE_SYSTEM_ZLIB )
	message( STATUS "Configuring for system ZLIB" )
	set( ZLIB_LIBRARIES ${SYSTEM_ZLIB_LIBRARIES} )
	set( ZLIB_INCLUDE_DIRS ${SYSTEM_ZLIB_INCLUDE_DIRS} )
	message( STATUS "Configuring for system ZLIB - done" )
endif()
if( USE_LOCAL_ZLIB OR HAVE_SYSTEM_ZLIB )
	set( USE_ZLIB ON
		CACHE BOOL "use zlib" )
else()
	unset( USE_ZLIB CACHE )
endif()
set( ZLIB_LIBRARIES ${ZLIB_LIBRARIES}
	CACHE PATH "zlib libraries" )
set( ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS}
	CACHE PATH "zlib include directories" )
mark_as_advanced( ZLIB_LIBRARIES )
mark_as_advanced( ZLIB_INCLUDE_DIRS )
