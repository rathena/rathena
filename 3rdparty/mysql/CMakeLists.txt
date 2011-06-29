
#
# local copy
#
if( WIN32 )
message( STATUS "Detecting local MYSQL" )
find_path( LOCAL_MYSQL_INCLUDE_DIRS "mysql.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include"
	NO_DEFAULT_PATH )
find_library( LOCAL_MYSQL_LIBRARIES
	NAMES libmysql
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib"
	NO_DEFAULT_PATH )
mark_as_advanced( LOCAL_MYSQL_LIBRARIES )
mark_as_advanced( LOCAL_MYSQL_INCLUDE_DIRS )

if( LOCAL_MYSQL_LIBRARIES AND LOCAL_MYSQL_INCLUDE_DIRS )
	if( EXISTS "${LOCAL_MYSQL_INCLUDE_DIRS}/mysql_version.h" )
		file( STRINGS "${LOCAL_MYSQL_INCLUDE_DIRS}/mysql_version.h" MYSQL_VERSION_H REGEX "^#define MYSQL_SERVER_VERSION[ \t]+\"[^\"]+\".*$" )
		string( REGEX REPLACE "^.*MYSQL_SERVER_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" MYSQL_SERVER_VERSION "${MYSQL_VERSION_H}" )
		message( STATUS "Found MYSQL: ${LOCAL_MYSQL_LIBRARIES} (found version ${MYSQL_SERVER_VERSION})" )
	else()
		message( STATUS "Found MYSQL: ${LOCAL_MYSQL_LIBRARIES}" )
	endif()
	set( HAVE_LOCAL_MYSQL ON
		CACHE BOOL "mysql client is available as a local copy")
else()
	foreach( _VAR LOCAL_MYSQL_LIBRARIES LOCAL_MYSQL_INCLUDE_DIRS )
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
find_program( MYSQL_CONFIG_EXECUTABLE "mysql_config" )
mark_as_advanced( MYSQL_CONFIG_EXECUTABLE )
if( MYSQL_CONFIG_EXECUTABLE )
	# use mysql_config data
	message( "Found mysql_config: ${MYSQL_CONFIG_EXECUTABLE}" )
	execute_process( COMMAND "${MYSQL_CONFIG_EXECUTABLE} --libs"
		OUTPUT_VARIABLE MYSQL_LIBRARIES )
	execute_process( COMMAND "${MYSQL_CONFIG_EXECUTABLE} --include"
		OUTPUT_VARIABLE MYSQL_INCLUDE_DIRS )
	execute_process( COMMAND "${MYSQL_CONFIG_EXECUTABLE} --version"
		OUTPUT_VARIABLE MYSQL_VERSION )
	message( STATUS "Found MYSQL: ${MYSQL_LIBRARIES} (found version ${MYSQL_VERSION})" )
else()
	# find mysql package
	set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH} )
	find_package( MYSQL )
endif()
set( SYSTEM_MYSQL_LIBRARIES "${MYSQL_LIBRARIES}"
	CACHE PATH "system mysql libraries" )
set( SYSTEM_MYSQL_INCLUDE_DIRS "${MYSQL_INCLUDE_DIRS}"
	CACHE PATH "system mysql include directories" )
mark_as_advanced( SYSTEM_MYSQL_LIBRARIES )
mark_as_advanced( SYSTEM_MYSQL_INCLUDE_DIRS )

if( SYSTEM_MYSQL_LIBRARIES AND SYSTEM_MYSQL_INCLUDE_DIRS )
	set( HAVE_SYSTEM_MYSQL ON
		CACHE BOOL "mysql client is available on the system" )
endif()
message( STATUS "Detecting system MYSQL - done" )


# options

#
# configure
#
unset( MYSQL_LIBRARIES CACHE )
unset( MYSQL_INCLUDE_DIRS CACHE )
if( HAVE_LOCAL_MYSQL )
	set( USE_LOCAL_MYSQL ON
		CACHE BOOL "use local copy of mysql" )
else()
	unset( USE_LOCAL_MYSQL CACHE )
endif()
if( USE_LOCAL_MYSQL )
	message( STATUS "Configuring for local MYSQL" )
	set( MYSQL_LIBRARIES ${LOCAL_MYSQL_LIBRARIES} )
	set( MYSQL_INCLUDE_DIRS ${LOCAL_MYSQL_INCLUDE_DIRS} )
	message( STATUS "Configuring for local MYSQL - done" )
elseif( HAVE_SYSTEM_MYSQL )
	message( STATUS "Configuring for system MYSQL" )
	set( MYSQL_LIBRARIES ${SYSTEM_MYSQL_LIBRARIES} )
	set( MYSQL_INCLUDE_DIRS ${SYSTEM_MYSQL_INCLUDE_DIRS} )
	message( STATUS "Configuring for system MYSQL - done" )
endif()
if( USE_LOCAL_MYSQL OR HAVE_SYSTEM_MYSQL )
	set( USE_MYSQL ON
		CACHE BOOL "use mysql" )
else()
	unset( USE_MYSQL CACHE )
endif()
set( MYSQL_LIBRARIES ${MYSQL_LIBRARIES}
	CACHE PATH "mysql libraries" )
set( MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIRS}
	CACHE PATH "mysql include directories" )
mark_as_advanced( MYSQL_LIBRARIES )
mark_as_advanced( MYSQL_INCLUDE_DIRS )
