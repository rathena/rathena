# - Find libmysql/mysqlclient
# Find the native MySQL includes and library
#
#  MYSQL_INCLUDE_DIRS - where to find mysql.h, etc.
#  MYSQL_LIBRARIES   - mysqlclient library.
#  MYSQL_FOUND       - True if mysqlclient is found.
#

find_path( MYSQL_INCLUDE_DIRS
	NAMES "mysql.h" "mariadb/mysql.h"
	PATHS
		"/usr/include/mysql"
		"/usr/include/mariadb"
		"/usr/local/include/mysql"
		"/usr/local/include/mariadb"
		"/usr/mysql/include/mysql"
		"/opt/homebrew/include/mysql" # for Apple Silicon (M1, M2)
		"/opt/homebrew/include/mariadb"
		"/usr/local/include/mysql" # for Intel-based Macs
		"/usr/local/include/mariadb"
		"$ENV{PROGRAMFILES}/MySQL/*/include"
		"$ENV{SYSTEMDRIVE}/MySQL/*/include"
		"$ENV{PROGRAMFILES}/MariaDB/*/include"
		"$ENV{SYSTEMDRIVE}/MariaDB/*/include"
	)

find_library( MYSQL_LIBRARIES
	NAMES "libmysql" "mysqlclient" "mysqlclient_r" "mariadb" "libmariadb"
	PATHS
		"/usr/lib/mysql"
		"/usr/lib/mariadb"
		"/usr/local/lib/mysql"
		"/usr/local/lib/mariadb"
		"/usr/mysql/lib/mysql"
		"/opt/homebrew/opt/mysql/lib" # for Apple Silicon (M1, M2)
		"/opt/homebrew/opt/mariadb-connector-c/lib"
		"/usr/local/opt/mysql/lib" # for Intel-based Macs
		"/usr/local/opt/mariadb-connector-c/lib"
		"$ENV{PROGRAMFILES}/MySQL/*/lib"
		"$ENV{SYSTEMDRIVE}/MySQL/*/lib"
		"$ENV{PROGRAMFILES}/MariaDB/*/lib"
		"$ENV{SYSTEMDRIVE}/MariaDB/*/lib"
	)
mark_as_advanced( MYSQL_LIBRARIES  MYSQL_INCLUDE_DIRS )

set( _MYSQL_VERSION_HEADER "" )
if( EXISTS "${MYSQL_INCLUDE_DIRS}/mysql_version.h" )
	set( _MYSQL_VERSION_HEADER "${MYSQL_INCLUDE_DIRS}/mysql_version.h" )
elseif( EXISTS "${MYSQL_INCLUDE_DIRS}/mariadb/mysql_version.h" )
	set( _MYSQL_VERSION_HEADER "${MYSQL_INCLUDE_DIRS}/mariadb/mysql_version.h" )
endif()

if( _MYSQL_VERSION_HEADER )
	file( STRINGS "${_MYSQL_VERSION_HEADER}" MYSQL_VERSION_H REGEX "^#define[ \t]+MYSQL_SERVER_VERSION[ \t]+\"[^\"]+\".*$" )
	string( REGEX REPLACE "^.*MYSQL_SERVER_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" MYSQL_VERSION_STRING "${MYSQL_VERSION_H}" )
endif()

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if 
# all listed variables are TRUE
include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( MYSQL
	REQUIRED_VARS MYSQL_LIBRARIES MYSQL_INCLUDE_DIRS
	VERSION_VAR MYSQL_VERSION_STRING )
