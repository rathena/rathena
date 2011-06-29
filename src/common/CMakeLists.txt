
set( COMMON_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
	CACHE PATH "common include directories" )
set( COMMON_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
	CACHE PATH "common source directory" )
mark_as_advanced( COMMON_INCLUDE_DIRS )
mark_as_advanced( COMMON_SOURCE_DIR )


#
# Find svnversion
#
message( STATUS "Detecting svnversion" )
find_program( SVNVERSION_EXECUTABLE svnversion )
mark_as_advanced( SVNVERSION_EXECUTABLE )
message( STATUS "Detecting svnversion - done" )


#
# Create svnversion.h
#
message( STATUS "Creating svnversion.h" )
if( SVNVERSION_EXECUTABLE )
	execute_process( COMMAND ${SVNVERSION_EXECUTABLE} ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE SVNVERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE )
endif()
if( SVNVERSION )
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/svnversion.h
		"#ifndef SVNVERSION\n#define SVNVERSION ${SVNVERSION}\n#endif\n" )
else()
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/svnversion.h "" )
endif()
set( GLOBAL_INCLUDE_DIRS "${GLOBAL_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "" )
set( SVNVERSION ${SVNVERSION}
	CACHE STRING "SVN version of the source code" )
if( INSTALL_BUILD_DATA )
	install( FILES ${CMAKE_CURRENT_BINARY_DIR}/svnversion.h DESTINATION ${CMAKE_INSTALL_PREFIX}/src/common )
endif()
message( STATUS "Creating svnversion.h - done" )
