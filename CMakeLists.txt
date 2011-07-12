#####################################################################
#
# "Getting Started with CMake", a tutorial video by Eric Wing.
#   Part 1 of 6: http://www.youtube.com/watch?v=CLvZTyji_Uw
#   Part 2 of 6: http://www.youtube.com/watch?v=gUW-RrRQjEg
#   Part 3 of 6: http://www.youtube.com/watch?v=sz6cPhbuTk4
#   Part 4 of 6: http://www.youtube.com/watch?v=JICZOkyNXbg
#   Part 5 of 6: http://www.youtube.com/watch?v=lAiuLHy4dCk
#   Part 6 of 6: http://www.youtube.com/watch?v=fAtJNzDZdH8
#
# You can use notepad++ for syntax highlighting.
# Naming conventions:
#   WITH_*   : option to use an external package or not
#   ENABLE_* : option to use an internal feature/code or not
#   HAVE_*   : internal variable indicating if we have and are using something
#
#####################################################################


#cmake_minimum_required( VERSION 2.8.4 )
# Functional changes from 2.8.3 to 2.8.4:
#   string(SUBSTRING) works with length -1 as "rest of string"
#   changes to some CPack generators
#   CYGWIN no longer defines WIN32
#   CMP0017: Prefer files from the CMake module directory when including from there.
set( CMAKE_LEGACY_CYGWIN_WIN32 0 )
cmake_minimum_required( VERSION 2.8.3 )
project( eAthena C )
if( CYGWIN )
	unset( WIN32 )
endif()


#
# Prevent building in the source directory by default
#
if( ALLOW_SAME_DIRECTORY )
elseif( "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}" )
	option( ALLOW_SAME_DIRECTORY "Allow CMake to build in the source directory." OFF )
	message( FATAL_ERROR
		"Do not use the source directory to build your files, instead delete CMakeCache.txt, create a separate folder and build there.\n"
		"Example: (build in subdir 'build' and install to source dir)\n"
		"  mkdir build && cd build\n"
		"  cmake -G\"Unix Makefiles\" -DINSTALL_TO_SOURCE:bool=ON ..\n"
		"  make install\n"
		"  cd .. && rm -rf build\n"
		"To skip this check, set ALLOW_SAME_DIRECTORY to ON (-DALLOW_SAME_DIRECTORY:bool=ON)" )
endif()


#
# Global stuff
#
set( GLOBAL_LIBRARIES ${LINK_LIBRARIES}  CACHE INTERNAL "" )# list (comma separated values)
set( GLOBAL_INCLUDE_DIRS ${INCLUDE_DIRECTORIES}  CACHE INTERNAL "" )# list (comma separated values)
set( GLOBAL_DEFINITIONS ${COMPILE_DEFINITIONS}  CACHE INTERNAL "" )# string (space separated values -DFOO=bar)
mark_as_advanced( GLOBAL_LIBRARIES  GLOBAL_INCLUDE_DIRS  GLOBAL_DEFINITIONS )
if( WIN32 )
	set_property( CACHE GLOBAL_DEFINITIONS  PROPERTY VALUE "${GLOBAL_DEFINITIONS} -DFD_SETSIZE=4096" )
	set_property( CACHE GLOBAL_LIBRARIES    PROPERTY VALUE ${GLOBAL_LIBRARIES} "oldnames.lib" "ws2_32.lib" )
endif()
if( MSVC )
	set_property( CACHE GLOBAL_DEFINITIONS  PROPERTY VALUE "${GLOBAL_DEFINITIONS} -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DDB_MANUAL_CAST_TO_UNION" )
endif()


#
# Find svnversion
#
message( STATUS "Detecting svnversion" )
find_program( SVNVERSION_EXECUTABLE svnversion )
mark_as_advanced( SVNVERSION_EXECUTABLE )
if( SVNVERSION_EXECUTABLE )
	message( STATUS "Found svnversion: ${SVNVERSION_EXECUTABLE}" )
endif()
message( STATUS "Detecting svnversion - done" )


#
# Find Subversion
#
message( STATUS "Detecting Subversion" )
find_package( Subversion )
message( STATUS "Detecting Subversion - done" )

#
# PACKETVER
#
set( PACKETVER CACHE STRING "Sets the PACKETVER define of the servers. (see src/common/mmo.h)" )
if( PACKETVER )
	list( APPEND GLOBAL_DEFINITIONS  PACKETVER=${PACKETVER} )
endif()


#
# SVNVERSION
#
if( SVNVERSION_EXECUTABLE )
	execute_process( COMMAND ${SVNVERSION_EXECUTABLE} ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE SVNVERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE )
	string( REGEX REPLACE "[^1234567890MSexported]" "_" SVNVERSION ${SVNVERSION} )
endif()
if( Subversion_FOUND AND SVNVERSION )
	Subversion_WC_INFO( ${PROJECT_SOURCE_DIR} eAthena )
	if( eAthena_WC_URL )
		string( REGEX MATCH "[^/]+$" BRANCH ${eAthena_WC_URL} )
		set( SVNVERSION "${BRANCH}-${SVNVERSION}" )
	endif()
endif()


#
# 3rdparty and library tests
#
add_subdirectory( 3rdparty )
include( FindFunctionLibrary )


#
# math library (FreeBSD/Linux)
#
message( STATUS "Detecting m library (math)" )
set( CMAKE_REQUIRED_LIBRARIES ${GLOBAL_LIBRARIES} )
find_function_library( floor FUNCTION_FLOOR_LIBRARIES m )
mark_as_advanced( FUNCTION_FLOOR_LIBRARIES )
if( FUNCTION_FLOOR_LIBRARIES )
	message( STATUS "Adding global library: ${FUNCTION_FLOOR_LIBRARIES}" )
	list( APPEND GLOBAL_LIBRARIES ${FUNCTION_FLOOR_LIBRARIES} )
endif()
message( STATUS "Detecting m library (math) - done" )


#
# dynamic loading library (Linux)
#
if( NOT WIN32 )
message( STATUS "Detecting dl library (dynamic loading)" )
set( CMAKE_REQUIRED_LIBRARIES ${GLOBAL_LIBRARIES} )
find_function_library( dlopen FUNCTION_DLOPEN_LIBRARIES dl )
mark_as_advanced( FUNCTION_DLOPEN_LIBRARIES )
if( FUNCTION_DLOPEN_LIBRARIES )
	message( STATUS "Adding global library: ${FUNCTION_DLOPEN_LIBRARIES}" )
	list( APPEND GLOBAL_LIBRARIES ${FUNCTION_DLOPEN_LIBRARIES} )
endif()
message( STATUS "Detecting dl library (dynamic loading) - done" )
endif()


#####################################################################
# package stuff
#
set( CPACK_PACKAGE_NAME "eAthena" )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "MMORPG server package" )
set( CPACK_PACKAGE_VERSION ${SVNVERSION} )
set( CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE )
#set( CPACK_MONOLITHIC_INSTALL ON )
include( CPACK  OPTIONAL  RESULT_VARIABLE HAVE_CPACK )
if( NOT HAVE_CPACK )
	# empty replacements
	macro( cpack_add_component_group )
	endmacro()
	macro( cpack_add_component )
	endmacro()
	message( STATUS "CPACK not found, package creation disabled" )
endif()

set( Runtime "Runtime files"  CACHE INTERNAL "" )
set( Runtime_base "configurations, dbs, npcs, docs, ..."  CACHE INTERNAL "" )
set( Runtime_templates "conf/import and save (generated from conf/import-tmpl and save-tmpl)"  CACHE INTERNAL "" )
cpack_add_component_group( Runtime DESCRIPTION ${Runtime} DISPLAY_NAME "Runtime" )
cpack_add_component( Runtime_base DESCRIPTION ${Runtime_base} DISPLAY_NAME "Base files" GROUP Runtime )
cpack_add_component( Runtime_templates DESCRIPTION ${Runtime_templates} DISPLAY_NAME "Base templates" GROUP Runtime )

set( Development "Development files"  CACHE INTERNAL "" )
set( Development_base "projects, 3rdparty, sources, templates"  CACHE INTERNAL "" )
cpack_add_component_group( Development DESCRIPTION ${Development} DISPLAY_NAME "Development" )
cpack_add_component( Development_base DESCRIPTION ${Development_base} DISPLAY_NAME "Base files" GROUP Development )


#
# install stuff
#
option( WITH_COMPONENT_RUNTIME "install/package files needed to run the project" ON )
option( WITH_COMPONENT_DEVELOPMENT "install/package files needed to build the project" OFF )
option( INSTALL_TO_PATH "copy files to INSTALL_PATH" OFF )
option( INSTALL_TO_SOURCE "copy files to source directory, skips what is already there (${CMAKE_CURRENT_SOURCE_DIR})" OFF )
option( INSTALL_TO_SUBDIR "copy files to subdirectory (${CMAKE_CURRENT_BINARY_DIR}/install)" OFF )
set( INSTALL_PATH "${CMAKE_INSTALL_PREFIX}"  CACHE STRING "install path (only used when INSTALL_TO_PATH is set)" )
mark_as_advanced( CMAKE_INSTALL_PREFIX )
if( INSTALL_TO_PATH AND NOT ("${INSTALL_TO}" STREQUAL "path") )# changed to path
	set_property( CACHE INSTALL_TO_SOURCE INSTALL_TO_SUBDIR  PROPERTY VALUE OFF )
elseif( INSTALL_TO_SOURCE AND NOT ("${INSTALL_TO}" STREQUAL "source") )# changed to source
	set_property( CACHE INSTALL_TO_PATH INSTALL_TO_SUBDIR  PROPERTY VALUE OFF )
elseif( INSTALL_TO_SUBDIR AND NOT ("${INSTALL_TO}" STREQUAL "subdir") )# changed to subdir
	set_property( CACHE INSTALL_TO_PATH INSTALL_TO_SOURCE  PROPERTY VALUE OFF )
elseif( NOT INSTALL_TO_PATH AND NOT INSTALL_TO_SOURCE AND NOT INSTALL_TO_SUBDIR )# default
	set_property( CACHE INSTALL_TO_SUBDIR  PROPERTY VALUE ON )
endif()
if( INSTALL_TO_PATH )
	set( INSTALL_TO "path"  CACHE INTERNAL "" )
	set_property( CACHE CMAKE_INSTALL_PREFIX  PROPERTY VALUE "${INSTALL_PATH}" )
elseif( INSTALL_TO_SOURCE )
	set( INSTALL_TO "source"  CACHE INTERNAL "" )
	set_property( CACHE CMAKE_INSTALL_PREFIX  PROPERTY VALUE "${CMAKE_CURRENT_SOURCE_DIR}" )
elseif( INSTALL_TO_SUBDIR )
	set( INSTALL_TO "subdir"  CACHE INTERNAL "" )
	set_property( CACHE CMAKE_INSTALL_PREFIX  PROPERTY VALUE "${CMAKE_CURRENT_BINARY_DIR}/install" )
endif()
set( SVN_FOLDER_PATTERN "[\\.]svn"  CACHE STRING "pattern of svn folder that we exclude from instalations" )
mark_as_advanced( SVN_FOLDER_PATTERN )
set( DEVELOPMENT_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
	"${CMAKE_CURRENT_SOURCE_DIR}/configure"
	"${CMAKE_CURRENT_SOURCE_DIR}/configure.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/eAthena-6.dsw"
	"${CMAKE_CURRENT_SOURCE_DIR}/eAthena-7.1.sln"
	"${CMAKE_CURRENT_SOURCE_DIR}/eAthena-8.sln"
	"${CMAKE_CURRENT_SOURCE_DIR}/eAthena-9.sln"
	"${CMAKE_CURRENT_SOURCE_DIR}/eAthena-10.sln"
	)
set( DEVELOPMENT_DIRECTORIES
	"3rdparty"
	"conf/import-tmpl"
	"save-tmpl"
	"src"
	"vcproj-6"
	"vcproj-7.1"
	"vcproj-8"
	"vcproj-9"
	"vcproj-10"
	)
set( RUNTIME_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/athena-start"
	"${CMAKE_CURRENT_SOURCE_DIR}/Changelog-Trunk.txt"
	"${CMAKE_CURRENT_SOURCE_DIR}/char-server.sh"
	"${CMAKE_CURRENT_SOURCE_DIR}/charserv-sql.bat"
	"${CMAKE_CURRENT_SOURCE_DIR}/charserv.bat"
	"${CMAKE_CURRENT_SOURCE_DIR}/dbghelp.dll"
	"${CMAKE_CURRENT_SOURCE_DIR}/libmysql.dll"
	"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
	"${CMAKE_CURRENT_SOURCE_DIR}/login-server.sh"
	"${CMAKE_CURRENT_SOURCE_DIR}/logserv-sql.bat"
	"${CMAKE_CURRENT_SOURCE_DIR}/logserv.bat"
	"${CMAKE_CURRENT_SOURCE_DIR}/map-server.sh"
	"${CMAKE_CURRENT_SOURCE_DIR}/mapserv-sql.bat"
	"${CMAKE_CURRENT_SOURCE_DIR}/mapserv.bat"
	"${CMAKE_CURRENT_SOURCE_DIR}/notice.txt"
	"${CMAKE_CURRENT_SOURCE_DIR}/pcre3.dll"
	"${CMAKE_CURRENT_SOURCE_DIR}/readme.html"
	"${CMAKE_CURRENT_SOURCE_DIR}/runserver-sql.bat"
	"${CMAKE_CURRENT_SOURCE_DIR}/runserver.bat"
	"${CMAKE_CURRENT_SOURCE_DIR}/serv.bat"
	"${CMAKE_CURRENT_SOURCE_DIR}/start"
	"${CMAKE_CURRENT_SOURCE_DIR}/zlib1.dll"
	)
set( RUNTIME_DIRECTORIES
	"conf"
	"db"
	"doc"
	"log"
	"npc"
	"plugins"
	"readme"
	"sql-files"
	"tools"
	)
if( INSTALL_TO_SOURCE )# skip, already in the source dir
else()
	if( WITH_COMPONENT_RUNTIME )
		install( FILES ${RUNTIME_FILES}
			DESTINATION "."
			COMPONENT Runtime_base )
		foreach( DIR IN ITEMS ${RUNTIME_DIRECTORIES} )
			install( DIRECTORY "${DIR}/"
				DESTINATION "${DIR}"
				COMPONENT Runtime_base
				PATTERN ${SVN_FOLDER_PATTERN} EXCLUDE
				PATTERN "conf/import-tmpl" EXCLUDE )
		endforeach()
	endif()
	if( WITH_COMPONENT_DEVELOPMENT )
		install( FILES ${DEVELOPMENT_FILES}
			DESTINATION "."
			COMPONENT Development_base )
		foreach( DIR IN ITEMS ${DEVELOPMENT_DIRECTORIES} )
			install( DIRECTORY "${DIR}/"
				DESTINATION "${DIR}"
				COMPONENT Development_base
				PATTERN ${SVN_FOLDER_PATTERN} EXCLUDE )
		endforeach()
	endif()
endif()
if( WITH_COMPONENT_RUNTIME )
	# templates
	install( DIRECTORY "save-tmpl/"
		DESTINATION "save"
		COMPONENT Runtime_templates
		PATTERN ${SVN_FOLDER_PATTERN} EXCLUDE )
	install( DIRECTORY "conf/import-tmpl/"
		DESTINATION "conf/import"
		COMPONENT Runtime_templates
		PATTERN ${SVN_FOLDER_PATTERN} EXCLUDE )
endif()


#
# sources
#
set( TARGET_LIST  CACHE INTERNAL "" )
add_subdirectory( src )


#####################################################################
# final checks and warnings
#
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	message( WARNING "64bit should work, but is not recommended." )
elseif( NOT CMAKE_SIZEOF_VOID_P EQUAL 4 )
	message( FATAL_ERROR "unexpected architecture (CMAKE_SIZEOF_VOID_P is ${CMAKE_SIZEOF_VOID_P})" )
endif()
list( LENGTH  TARGET_LIST  _LEN )
if( _LEN EQUAL 0 )
	message( FATAL_ERROR "no targets available" )
endif()
message( STATUS "Available targets:" )
foreach( _TARGET IN ITEMS ${TARGET_LIST} )
	message( STATUS "\t${_TARGET}" )
endforeach()
