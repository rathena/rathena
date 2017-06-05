# - Find pcre
# Find the native PCRE includes and library
#
#  PCRE_INCLUDE_DIRS - where to find pcre.h
#  PCRE_LIBRARIES    - List of libraries when using pcre.
#  PCRE_FOUND        - True if pcre found.


find_path( PCRE_INCLUDE_DIR pcre.h
	PATHS
		"/usr/include/pcre" )
set( PCRE_NAMES pcre )
find_library( PCRE_LIBRARY NAMES ${PCRE_NAMES} )
mark_as_advanced( PCRE_LIBRARY PCRE_INCLUDE_DIR )

if( PCRE_INCLUDE_DIR AND EXISTS "${PCRE_INCLUDE_DIR}/pcre.h" )
	file( STRINGS "${PCRE_INCLUDE_DIR}/pcre.h" PCRE_H REGEX "^#define[ \t]+PCRE_M[A-Z]+[ \t]+[0-9]+.*$" )
	string( REGEX REPLACE "^.*PCRE_MAJOR[ \t]+([0-9]+).*$" "\\1" PCRE_MAJOR "${PCRE_H}" )
	string( REGEX REPLACE "^.*PCRE_MINOR[ \t]+([0-9]+).*$" "\\1" PCRE_MINOR  "${PCRE_H}" )

	set( PCRE_VERSION_STRING "${PCRE_MAJOR}.${PCRE_MINOR}" )
	set( PCRE_VERSION_MAJOR "${PCRE_MAJOR}" )
	set( PCRE_VERSION_MINOR "${PCRE_MINOR}" )
endif()

# handle the QUIETLY and REQUIRED arguments and set PCRE_FOUND to TRUE if 
# all listed variables are TRUE
include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( PCRE
	REQUIRED_VARS PCRE_LIBRARY PCRE_INCLUDE_DIR
	VERSION_VAR PCRE_VERSION_STRING )

if( PCRE_FOUND )
	set( PCRE_LIBRARIES ${PCRE_LIBRARY} )
	set( PCRE_INCLUDE_DIRS ${PCRE_INCLUDE_DIR} )
endif()
