# - Find mysqlclient
# Find the native MySQL includes and library
#
#  MYSQL_INCLUDE_DIR - where to find mysql.h, etc.
#  MYSQL_LIBRARIES   - List of libraries when using MySQL.
#  MYSQL_FOUND       - True if MySQL found.
#
# Based on: http://www.itk.org/Wiki/CMakeUserFindMySQL

find_path( MYSQL_INCLUDE_DIR "mysql.h"
	PATH_SUFFIXES "mysql" )

set( MYSQL_NAMES mysqlclient mysqlclient_r )
find_library( MYSQL_LIBRARY
	NAMES ${MYSQL_NAMES}
	PATH_SUFFIXES "mysql" )
mark_as_advanced( MYSQL_LIBRARY MYSQL_INCLUDE_DIR )

if( MYSQL_INCLUDE_DIR AND EXISTS "${MYSQL_INCLUDE_DIR}/mysql_version.h" )
	file( STRINGS "${MYSQL_INCLUDE_DIR}/mysql_version.h" MYSQL_VERSION_H REGEX "^#define[ \t]+MYSQL_SERVER_VERSION[ \t]+\"[^\"]+\".*$" )
	string( REGEX REPLACE "^.*MYSQL_SERVER_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" MYSQL_VERSION_STRING "${MYSQL_VERSION_H}" )
endif()

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if 
# all listed variables are TRUE
include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( MYSQL
	REQUIRED_VARS MYSQL_LIBRARY MYSQL_INCLUDE_DIR
	VERSION_VAR MYSQL_VERSION_STRING )

if( MYSQL_FOUND )
	set( MYSQL_LIBRARIES ${PCRE_LIBRARY} )
	set( MYSQL_INCLUDE_DIRS ${PCRE_INCLUDE_DIR} )
endif()
