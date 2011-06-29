
#
# local copy
#
if( WIN32 )
message( STATUS "Detecting local PCRE" )
find_path( LOCAL_PCRE_INCLUDE_DIRS "pcre.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include"
	NO_DEFAULT_PATH )
find_library( LOCAL_PCRE_LIBRARIES
	NAMES pcre
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib"
	NO_DEFAULT_PATH )
mark_as_advanced( LOCAL_PCRE_LIBRARIES )
mark_as_advanced( LOCAL_PCRE_INCLUDE_DIRS )

if( LOCAL_PCRE_LIBRARIES AND LOCAL_PCRE_INCLUDE_DIRS )
	file( STRINGS "${LOCAL_PCRE_INCLUDE_DIRS}/pcre.h" PCRE_H REGEX "^#define[ \t]+PCRE_M[A-Z]+[ \t]+[0-9]+.*$" )
	string( REGEX REPLACE "^.*PCRE_MAJOR[ \t]+([0-9]+).*$" "\\1" PCRE_MAJOR "${PCRE_H}" )
	string( REGEX REPLACE "^.*PCRE_MINOR[ \t]+([0-9]+).*$" "\\1" PCRE_MINOR  "${PCRE_H}" )
	message( STATUS "Found PCRE: ${LOCAL_PCRE_LIBRARIES} (found version ${PCRE_MAJOR}.${PCRE_MINOR})" )
	set( HAVE_LOCAL_PCRE ON
		CACHE BOOL "pcre is available as a local copy" )
else()
	foreach( _VAR LOCAL_PCRE_LIBRARIES LOCAL_PCRE_INCLUDE_DIRS )
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
set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH} )
find_package( PCRE )
set( SYSTEM_PCRE_LIBRARIES ${PCRE_LIBRARIES}
	CACHE PATH "system pcre libraries" )
set( SYSTEM_PCRE_INCLUDE_DIRS ${PCRE_INCLUDE_DIRS}
	CACHE PATH "system pcre include directories" )
mark_as_advanced( SYSTEM_PCRE_LIBRARIES )
mark_as_advanced( SYSTEM_PCRE_INCLUDE_DIRS )

if( SYSTEM_PCRE_LIBRARIES AND SYSTEM_PCRE_INCLUDE_DIRS )
	set( HAVE_SYSTEM_PCRE ON
		CACHE BOOL "pcre is available on the system" )
else()
	unset( HAVE_SYSTEM_PCRE CACHE )
endif()
message( STATUS "Detecting system PCRE - done" )


#
# configure
#
unset( PCRE_LIBRARIES CACHE )
unset( PCRE_INCLUDE_DIRS CACHE )
if( HAVE_LOCAL_PCRE )
	set( USE_LOCAL_PCRE ON
		CACHE BOOL "use local copy of pcre" )
else()
	unset( USE_LOCAL_PCRE CACHE )
endif()
if( USE_LOCAL_PCRE )
	message( STATUS "Configuring for local PCRE" )
	set( PCRE_LIBRARIES ${LOCAL_PCRE_LIBRARIES} )
	set( PCRE_INCLUDE_DIRS ${LOCAL_PCRE_INCLUDE_DIRS} )
	message( STATUS "Configuring for local PCRE - done" )
elseif( HAVE_SYSTEM_PCRE )
	message( STATUS "Configuring for system PCRE" )
	set( PCRE_LIBRARIES ${SYSTEM_PCRE_LIBRARIES} )
	set( PCRE_INCLUDE_DIRS ${SYSTEM_PCRE_INCLUDE_DIRS} )
	message( STATUS "Configuring for system PCRE - done" )
endif()
if( USE_LOCAL_PCRE OR HAVE_SYSTEM_PCRE )
	set( USE_PCRE ON
		CACHE BOOL "use pcre" )
else()
	unset( USE_PCRE CACHE )
endif()
set( PCRE_LIBRARIES ${PCRE_LIBRARIES}
	CACHE PATH "pcre libraries" )
set( PCRE_INCLUDE_DIRS ${PCRE_INCLUDE_DIRS}
	CACHE PATH "pcre include directories" )
mark_as_advanced( PCRE_LIBRARIES )
mark_as_advanced( PCRE_INCLUDE_DIRS )
