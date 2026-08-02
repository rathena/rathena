# - Find libmysql/mysqlclient
# Find the native MySQL includes and library
#
#  MYSQL_INCLUDE_DIRS - where to find mysql.h, etc.
#  MYSQL_LIBRARIES   - mysqlclient library.
#  MYSQL_FOUND       - True if mysqlclient is found.
#

find_path( MYSQL_INCLUDE_DIRS "mysql.h"
	PATHS
		"/usr/include/mysql"
		"/usr/local/include/mysql"
		"/usr/mysql/include/mysql"
		"/opt/homebrew/include/mysql" # for Apple Silicon (M1, M2)
		"/usr/local/include/mysql" # for Intel-based Macs
		"$ENV{PROGRAMFILES}/MySQL/*/include"
		"$ENV{SYSTEMDRIVE}/MySQL/*/include" )

find_library( MYSQL_LIBRARIES
	NAMES "libmysql" "mysqlclient" "mysqlclient_r"
	PATHS
		"/usr/lib/mysql"
		"/usr/local/lib/mysql"
		"/usr/mysql/lib/mysql"
		"/opt/homebrew/opt/mysql/lib" # for Apple Silicon (M1, M2)
		"/usr/local/opt/mysql/lib" # for Intel-based Macs
		"$ENV{PROGRAMFILES}/MySQL/*/lib"
		"$ENV{SYSTEMDRIVE}/MySQL/*/lib" )
mark_as_advanced( MYSQL_LIBRARIES  MYSQL_INCLUDE_DIRS )

if( MYSQL_INCLUDE_DIRS AND EXISTS "${MYSQL_INCLUDE_DIRS}/mysql_version.h" )
	file( STRINGS "${MYSQL_INCLUDE_DIRS}/mysql_version.h" MYSQL_VERSION_H REGEX "^#define[ \t]+MYSQL_SERVER_VERSION[ \t]+\"[^\"]+\".*$" )
	string( REGEX REPLACE "^.*MYSQL_SERVER_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" MYSQL_VERSION_STRING "${MYSQL_VERSION_H}" )
endif()

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if 
# all listed variables are TRUE
include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( MYSQL
	REQUIRED_VARS MYSQL_LIBRARIES MYSQL_INCLUDE_DIRS
	VERSION_VAR MYSQL_VERSION_STRING )
