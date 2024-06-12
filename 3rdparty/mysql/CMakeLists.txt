
#
# local copy
#
if( WIN32 )
message( STATUS "Detecting local MYSQL" )
find_path( MYSQL_LOCAL_INCLUDE_DIRS "mysql.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include"
	NO_DEFAULT_PATH )
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
find_library( MYSQL_LOCAL_LIBRARIES
	NAMES libmysql
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib/Win32"
	NO_DEFAULT_PATH )
else()
find_library( MYSQL_LOCAL_LIBRARIES
	NAMES libmysql
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64"
	NO_DEFAULT_PATH )
endif()
mark_as_advanced( MYSQL_LOCAL_LIBRARIES )
mark_as_advanced( MYSQL_LOCAL_INCLUDE_DIRS )

if( MYSQL_LOCAL_LIBRARIES AND MYSQL_LOCAL_INCLUDE_DIRS )
	if( EXISTS "${MYSQL_LOCAL_INCLUDE_DIRS}/mysql_version.h" )
		file( STRINGS "${MYSQL_LOCAL_INCLUDE_DIRS}/mysql_version.h" MYSQL_VERSION_H REGEX "^#define MYSQL_SERVER_VERSION[ \t]+\"[^\"]+\".*$" )
		string( REGEX REPLACE "^.*MYSQL_SERVER_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" MYSQL_SERVER_VERSION "${MYSQL_VERSION_H}" )
		message( STATUS "Found MYSQL: ${MYSQL_LOCAL_LIBRARIES} (found version ${MYSQL_SERVER_VERSION})" )
	else()
		message( STATUS "Found MYSQL: ${MYSQL_LOCAL_LIBRARIES}" )
	endif()
	set( HAVE_LOCAL_MYSQL ON
		CACHE BOOL "mysql client is available as a local copy")
	mark_as_advanced( HAVE_LOCAL_MYSQL )
else()
	foreach( _VAR MYSQL_LOCAL_LIBRARIES MYSQL_LOCAL_INCLUDE_DIRS )
		if( NOT "${_VAR}" )
			set( MISSING_VARS ${MISSING_VARS} ${_VAR} )
		endif()
	endforeach()
	message( STATUS "Could NOT find MYSQL (missing: ${MISSING_VARS})" )
	unset( HAVE_LOCAL_MYSQL CACHE )
endif()
message( STATUS "Detecting local MYSQL - done" )
endif( WIN32 )


#
# system
#
message( STATUS "Detecting system MYSQL" )
unset( MYSQL_LIBRARIES CACHE )
unset( MYSQL_INCLUDE_DIRS CACHE )
find_package( MYSQL )
set( MYSQL_SYSTEM_LIBRARIES "${MYSQL_LIBRARIES}"
	CACHE PATH "system mysql libraries" )
set( MYSQL_SYSTEM_INCLUDE_DIRS "${MYSQL_INCLUDE_DIRS}"
	CACHE PATH "system mysql include directories" )
mark_as_advanced( MYSQL_SYSTEM_LIBRARIES )
mark_as_advanced( MYSQL_SYSTEM_INCLUDE_DIRS )

if( MYSQL_SYSTEM_LIBRARIES AND MYSQL_SYSTEM_INCLUDE_DIRS )
	set( HAVE_SYSTEM_MYSQL ON
		CACHE BOOL "mysql client is available on the system" )
	mark_as_advanced( HAVE_SYSTEM_MYSQL )
else()
	unset( HAVE_SYSTEM_MYSQL CACHE )
endif()
message( STATUS "Detecting system MYSQL - done" )


#
# configure
#
CONFIGURE_WITH_LOCAL_OR_SYSTEM( MYSQL )
