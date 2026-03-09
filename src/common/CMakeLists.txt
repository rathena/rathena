
#
# Create version.hpp
#
message( STATUS "Creating version.hpp" )
if(GIT_VERSION)
	# those 2 was done in parent to produce this
	#include(GetGitVersion)
	#get_git_version()
	string(SUBSTRING ${GIT_VERSION} 0 10 SHORT_GIT_VERSION)
	string(SUBSTRING ${GIT_HEAD_VERSION} 0 10 SHORT_GIT_HEAD_VERSION)
	string(CONCAT GIT_STR_VERSIONS ${SHORT_GIT_VERSION} "_" ${SHORT_GIT_HEAD_VERSION})
	#message( STATUS "git version=${GIT_STR_VERSIONS}" )
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/version.hpp
		"#ifndef SVNVERSION\n#define SVNVERSION ${GIT_STR_VERSIONS}\n#endif\n" )
elseif( SVNVERSION )
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/version.hpp
		"#ifndef SVNVERSION\n#define SVNVERSION ${SVNVERSION}\n#endif\n" )
else()
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/version.hpp "" )
endif()
set( GLOBAL_INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "" )
set( SVNVERSION ${SVNVERSION}
	CACHE STRING "SVN version of the source code" )
if( INSTALL_COMPONENT_DEVELOPMENT )
	install( FILES ${CMAKE_CURRENT_BINARY_DIR}/version.hpp
		DESTINATION "src/common"
		COMPONENT Development_base )
endif( INSTALL_COMPONENT_DEVELOPMENT )
message( STATUS "Creating version.hpp - done" )

set( COMMON_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
	CACHE PATH "common source directory" )
#message( STATUS "DEBUG COMMON_SOURCE_DIR=${COMMON_SOURCE_DIR}" )
mark_as_advanced( COMMON_SOURCE_DIR )

#####################################################################
# setup
#
if( WIN32 )
	set( PROJECT_LIBS Ws2_32.lib )

	if( MSVC )
		add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
	endif()

	set( COMMON_ADDITIONALL_CPP	"${COMMON_SOURCE_DIR}/winapi.cpp" )

	set( COMMON_ADDITIONALL_HPP	"${COMMON_SOURCE_DIR}/winapi.hpp" )
endif()

set( COMMON_ALL_HEADERS
	"${CMAKE_CURRENT_BINARY_DIR}/version.hpp"
	"${COMMON_SOURCE_DIR}/cbasetypes.hpp"
	"${COMMON_SOURCE_DIR}/mmo.hpp"
	)

add_library(minicore)

target_sources(minicore PRIVATE
	"${COMMON_SOURCE_DIR}/core.cpp"
	"${COMMON_SOURCE_DIR}/malloc.cpp"
	"${COMMON_SOURCE_DIR}/showmsg.cpp"
	"${COMMON_SOURCE_DIR}/strlib.cpp"
	${LIBCONFIG_SOURCES} # needed by showmsg.cpp
	${COMMON_ADDITIONALL_CPP} # needed by Windows
)

target_include_directories(minicore PUBLIC
	${COMMON_SOURCE_DIR}
	${LIBCONFIG_INCLUDE_DIRS}
	${COMMON_ADDITIONALL_HPP} # needed by Windows
)

set_target_properties( minicore PROPERTIES COMPILE_FLAGS "${GLOBAL_DEFINITIONS}" )
target_compile_definitions(minicore PRIVATE "-DMINICORE" ${LIBCONFIG_DEFINITIONS})

#
# common_base
#
if( WITH_ZLIB )
message( STATUS "Creating target common_base" )
set( COMMON_BASE_HEADERS
	${COMMON_ALL_HEADERS}
	"${COMMON_SOURCE_DIR}/conf.hpp"
	"${COMMON_SOURCE_DIR}/core.hpp"
	"${COMMON_SOURCE_DIR}/database.hpp"
	"${COMMON_SOURCE_DIR}/db.hpp"
	"${COMMON_SOURCE_DIR}/des.hpp"
	"${COMMON_SOURCE_DIR}/ers.hpp"
	"${COMMON_SOURCE_DIR}/grfio.hpp"
	"${COMMON_SOURCE_DIR}/malloc.hpp"
	"${COMMON_SOURCE_DIR}/mapindex.hpp"
	"${COMMON_SOURCE_DIR}/md5calc.hpp"
	"${COMMON_SOURCE_DIR}/nullpo.hpp"
	"${COMMON_SOURCE_DIR}/random.hpp"
	"${COMMON_SOURCE_DIR}/showmsg.hpp"
	"${COMMON_SOURCE_DIR}/socket.hpp"
	"${COMMON_SOURCE_DIR}/strlib.hpp"
	"${COMMON_SOURCE_DIR}/timer.hpp"
	"${COMMON_SOURCE_DIR}/utils.hpp"
	"${COMMON_SOURCE_DIR}/msg_conf.hpp"
	"${COMMON_SOURCE_DIR}/cli.hpp"
	"${COMMON_SOURCE_DIR}/utilities.hpp"
	${LIBCONFIG_HEADERS} # needed by conf.hpp/showmsg.hpp
	${COMMON_ADDITIONALL_HPP} # needed by Windows
	CACHE INTERNAL "common_base headers" )
set( COMMON_BASE_SOURCES
	"${COMMON_SOURCE_DIR}/conf.cpp"
	"${COMMON_SOURCE_DIR}/core.cpp"
	"${COMMON_SOURCE_DIR}/database.cpp"
	"${COMMON_SOURCE_DIR}/db.cpp"
	"${COMMON_SOURCE_DIR}/des.cpp"
	"${COMMON_SOURCE_DIR}/ers.cpp"
	"${COMMON_SOURCE_DIR}/grfio.cpp"
	"${COMMON_SOURCE_DIR}/malloc.cpp"
	"${COMMON_SOURCE_DIR}/mapindex.cpp"
	"${COMMON_SOURCE_DIR}/md5calc.cpp"
	"${COMMON_SOURCE_DIR}/nullpo.cpp"
	"${COMMON_SOURCE_DIR}/random.cpp"
	"${COMMON_SOURCE_DIR}/showmsg.cpp"
	"${COMMON_SOURCE_DIR}/socket.cpp"
	"${COMMON_SOURCE_DIR}/strlib.cpp"
	"${COMMON_SOURCE_DIR}/timer.cpp"
	"${COMMON_SOURCE_DIR}/utils.cpp"
	"${COMMON_SOURCE_DIR}/msg_conf.cpp"
	"${COMMON_SOURCE_DIR}/cli.cpp"
	"${COMMON_SOURCE_DIR}/utilities.cpp"
	${LIBCONFIG_SOURCES} # needed by conf.cpp/showmsg.cpp
	${COMMON_ADDITIONALL_CPP} # needed by Windows
	CACHE INTERNAL "common_base sources" )
set( COMMON_BASE_INCLUDE_DIRS
	${LIBCONFIG_INCLUDE_DIRS}
	${YAML_INCLUDE_DIRS}
	CACHE INTERNAL "common_base include dirs" )
set( COMMON_BASE_DEFINITIONS
	${LIBCONFIG_DEFINITIONS}
	CACHE INTERNAL "common_base definitions" )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${ZLIB_LIBRARIES} yaml-cpp ryml )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${RA_INCLUDE_DIRS} ${YAML_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${COMMON_BASE_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_BASE_DEFINITIONS}" )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_BASE_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_BASE_SOURCES} )

add_library( common_base ${SOURCE_FILES} )
#message( STATUS "common_base LIBRARIES=${LIBRARIES}, DEFINITIONS=${DEFINITIONS}")
target_link_libraries( common_base ${LIBRARIES} )
set_target_properties( common_base PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
include_directories( ${INCLUDE_DIRS} )

set( HAVE_common_base ON  CACHE INTERNAL "" )
set( TARGET_LIST ${TARGET_LIST} common_base  CACHE INTERNAL "" )
message( STATUS "Creating target common_base - done" )
else()
message( STATUS "Skipping target common_base (requires ZLIB)" )
unset( HAVE_common_base CACHE )
endif()


#
# common
#
if( HAVE_common_base AND WITH_MYSQL )
message( STATUS "Creating target common" )
set( COMMON_HEADERS
	${COMMON_ALL_HEADERS}
	"${CMAKE_CURRENT_SOURCE_DIR}/sql.hpp"
	CACHE INTERNAL "common headers" )
set( COMMON_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/sql.cpp"
	CACHE INTERNAL "common sources" )
set( DEPENDENCIES common_base yaml-cpp ryml )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${MYSQL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS}" )
set( SOURCE_FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )

add_library( common ${SOURCE_FILES} )
#message( STATUS "common LIBRARIES=${LIBRARIES}, DEPENDENCIES=${DEPENDENCIES} DEFINITIONS=${DEFINITIONS}")
add_dependencies( common ${DEPENDENCIES} )
target_link_libraries( common ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( common PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
include_directories( ${INCLUDE_DIRS} ${YAML_INCLUDE_DIRS} )

set( HAVE_common ON  CACHE INTERNAL "" )
set( TARGET_LIST ${TARGET_LIST} common  CACHE INTERNAL "" )
message( STATUS "Creating target common - done" )
else()
message( FATAL_ERROR "Stopping target common (requires common_base and MYSQL)" )
endif()
