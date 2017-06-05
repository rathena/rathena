
#
# local copy
#
if( WIN32 )
message( STATUS "Detecting local PCRE" )
find_path( PCRE_LOCAL_INCLUDE_DIRS "pcre.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include"
	NO_DEFAULT_PATH )
find_library( PCRE_LOCAL_LIBRARIES
	NAMES pcre
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib"
	NO_DEFAULT_PATH )
mark_as_advanced( PCRE_LOCAL_LIBRARIES )
mark_as_advanced( PCRE_LOCAL_INCLUDE_DIRS )

if( PCRE_LOCAL_LIBRARIES AND PCRE_LOCAL_INCLUDE_DIRS )
	file( STRINGS "${PCRE_LOCAL_INCLUDE_DIRS}/pcre.h" PCRE_H REGEX "^#define[ \t]+PCRE_M[A-Z]+[ \t]+[0-9]+.*$" )
	string( REGEX REPLACE "^.*PCRE_MAJOR[ \t]+([0-9]+).*$" "\\1" PCRE_MAJOR "${PCRE_H}" )
	string( REGEX REPLACE "^.*PCRE_MINOR[ \t]+([0-9]+).*$" "\\1" PCRE_MINOR  "${PCRE_H}" )
	message( STATUS "Found PCRE: ${PCRE_LOCAL_LIBRARIES} (found version ${PCRE_MAJOR}.${PCRE_MINOR})" )
	set( HAVE_LOCAL_PCRE ON
		CACHE BOOL "pcre is available as a local copy" )
	mark_as_advanced( HAVE_LOCAL_PCRE )
else()
	foreach( _VAR PCRE_LOCAL_LIBRARIES PCRE_LOCAL_INCLUDE_DIRS )
		if( NOT "${_VAR}" )
			set( MISSING_VARS ${MISSING_VARS} ${_VAR} )
		endif()
	endforeach()
	message( STATUS "Could NOT find PCRE (missing: ${MISSING_VARS})" )
	unset( HAVE_LOCAL_PCRE CACHE )
endif()
message( STATUS "Detecting local PCRE - done" )
endif( WIN32 )


#
# system
#
message( STATUS "Detecting system PCRE" )
unset( PCRE_LIBRARIES CACHE )
unset( PCRE_INCLUDE_DIRS CACHE )
find_package( PCRE )
set( PCRE_SYSTEM_LIBRARIES ${PCRE_LIBRARIES}
	CACHE PATH "system pcre libraries" )
set( PCRE_SYSTEM_INCLUDE_DIRS ${PCRE_INCLUDE_DIRS}
	CACHE PATH "system pcre include directories" )
mark_as_advanced( PCRE_SYSTEM_LIBRARIES )
mark_as_advanced( PCRE_SYSTEM_INCLUDE_DIRS )

if( PCRE_SYSTEM_LIBRARIES AND PCRE_SYSTEM_INCLUDE_DIRS )
	set( HAVE_SYSTEM_PCRE ON
		CACHE BOOL "pcre is available on the system" )
	mark_as_advanced( HAVE_SYSTEM_PCRE )
else()
	unset( HAVE_SYSTEM_PCRE CACHE )
endif()
message( STATUS "Detecting system PCRE - done" )


#
# configure
#
CONFIGURE_WITH_LOCAL_OR_SYSTEM( PCRE )
