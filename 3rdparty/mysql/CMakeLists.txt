
#
# local copy
#
if( WIN32 )
message( STATUS "Detecting local MYSQL" )

# external_mysql
message( STATUS "Creating target external_mysql" )
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set( _URL "${CMAKE_CURRENT_SOURCE_DIR}/mysql-5.5.24-winx64-libmysql.tar.gz" )
	set( _URL_MD5 "36ae333e56ff6ae99de13edf893a1792" )
elseif( CMAKE_SIZEOF_VOID_P EQUAL 4 )
	set( _URL "${CMAKE_CURRENT_SOURCE_DIR}/mysql-5.5.24-win32-libmysql.tar.gz" )
	set( _URL_MD5 "e9abe2055bba07995c89e66c7c6d1e2c" )
else()
	message( FATAL_ERROR "Not supported: CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}" )
endif()
set( _INSTALL_DIR "${CMAKE_BINARY_DIR}/external/mysql" )


message( STATUS "Using URL=${_URL}" )
message( STATUS "Using URL_MD5=${_URL_MD5}" )
message( STATUS "Using INSTALL_DIR=${_INSTALL_DIR}" )
include(ExternalProject)
ExternalProject_Add( external_mysql
	URL "${_URL}"
	URL_MD5 "${_URL_MD5}"
	SOURCE_DIR "${_INSTALL_DIR}"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)
set( TARGET_LIST ${TARGET_LIST} external_mysql  CACHE INTERNAL "" )
message( STATUS "Creating target external_mysql - done" )

# libmysql
message( STATUS "Creating import shared library libmysql" )
add_library( libmysql SHARED IMPORTED GLOBAL )
set_target_properties( libmysql PROPERTIES
	IMPORTED_LOCATION "${_INSTALL_DIR}/lib/libmysql.dll"
	IMPORTED_IMPLIB "${_INSTALL_DIR}/lib/libmysql.lib"  )
message( STATUS "Creating import shared library libmysql - done" )

set( HAVE_LOCAL_MYSQL ON
	CACHE BOOL "mysql client is available as a local copy" )
set( MYSQL_LOCAL_DEPENDENCIES external_mysql
	CACHE STRING "local mysql dependencies" )
set( MYSQL_LOCAL_LIBRARIES libmysql
	CACHE PATH "local mysql libraries" )
set( MYSQL_LOCAL_INCLUDE_DIRS "${_INSTALL_DIR}/include"
	CACHE PATH "local mysql include directories" )
set( MYSQL_LOCAL_DEFINITIONS
	CACHE STRING "local mysql definitions" )
mark_as_advanced( HAVE_LOCAL_MYSQL )
mark_as_advanced( MYSQL_LOCAL_DEPENDENCIES )
mark_as_advanced( MYSQL_LOCAL_LIBRARIES )
mark_as_advanced( MYSQL_LOCAL_INCLUDE_DIRS )
mark_as_advanced( MYSQL_LOCAL_DEFINITIONS )
message( STATUS "Detecting local MYSQL - done" )
else( WIN32 )
message( STATUS "Skipping local MYSQL (requires WIN32)" )
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
if( WITH_LOCAL_MYSQL AND INSTALL_COMPONENT_RUNTIME )
	install( FILES "${_INSTALL_DIR}/lib/libmysql.dll"
		DESTINATION "."
		COMPONENT Runtime_base )
endif( WITH_LOCAL_MYSQL AND INSTALL_COMPONENT_RUNTIME )

