
#
# Create svnversion.h
#
message( STATUS "Creating svnversion.h" )
if( SVNVERSION )
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/svnversion.h
		"#ifndef SVNVERSION\n#define SVNVERSION ${SVNVERSION}\n#endif\n" )
else()
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/svnversion.h "" )
endif()
set( GLOBAL_INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "" )
set( SVNVERSION ${SVNVERSION}
	CACHE STRING "SVN version of the source code" )
if( INSTALL_COMPONENT_DEVELOPMENT )
	install( FILES ${CMAKE_CURRENT_BINARY_DIR}/svnversion.h
		DESTINATION "src/common"
		COMPONENT Development_base )
endif( INSTALL_COMPONENT_DEVELOPMENT )
message( STATUS "Creating svnversion.h - done" )


#####################################################################
# setup
#
set( COMMON_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
	CACHE PATH "common source directory" )
mark_as_advanced( COMMON_SOURCE_DIR )

set( COMMON_ALL_HEADERS
	"${CMAKE_CURRENT_BINARY_DIR}/svnversion.h"
	"${COMMON_SOURCE_DIR}/cbasetypes.h"
	"${COMMON_SOURCE_DIR}/mmo.h"
	)

set( COMMON_MINI_HEADERS
	${COMMON_ALL_HEADERS}
	"${COMMON_SOURCE_DIR}/core.h"
	"${COMMON_SOURCE_DIR}/malloc.h"
	"${COMMON_SOURCE_DIR}/showmsg.h"
	"${COMMON_SOURCE_DIR}/strlib.h"
	${LIBCONFIG_HEADERS} # needed by showmsg.h
	CACHE INTERNAL "" )
set( COMMON_MINI_SOURCES
	"${COMMON_SOURCE_DIR}/core.c"
	"${COMMON_SOURCE_DIR}/malloc.c"
	"${COMMON_SOURCE_DIR}/showmsg.c"
	"${COMMON_SOURCE_DIR}/strlib.c"
	${LIBCONFIG_SOURCES} # needed by showmsg.c
	CACHE INTERNAL "" )
set( COMMON_MINI_INCLUDE_DIRS ${LIBCONFIG_INCLUDE_DIRS} CACHE INTERNAL "" )
set( COMMON_MINI_DEFINITIONS "-DMINICORE ${LIBCONFIG_DEFINITIONS}" CACHE INTERNAL "" )


#
# common_base
#
if( WITH_ZLIB )
message( STATUS "Creating target common_base" )
set( COMMON_BASE_HEADERS
	${COMMON_ALL_HEADERS}
	"${COMMON_SOURCE_DIR}/conf.h"
	"${COMMON_SOURCE_DIR}/core.h"
	"${COMMON_SOURCE_DIR}/db.h"
	"${COMMON_SOURCE_DIR}/des.h"
	"${COMMON_SOURCE_DIR}/ers.h"
	"${COMMON_SOURCE_DIR}/grfio.h"
	"${COMMON_SOURCE_DIR}/malloc.h"
	"${COMMON_SOURCE_DIR}/mapindex.h"
	"${COMMON_SOURCE_DIR}/md5calc.h"
	"${COMMON_SOURCE_DIR}/nullpo.h"
	"${COMMON_SOURCE_DIR}/random.h"
	"${COMMON_SOURCE_DIR}/showmsg.h"
	"${COMMON_SOURCE_DIR}/socket.h"
	"${COMMON_SOURCE_DIR}/strlib.h"
	"${COMMON_SOURCE_DIR}/timer.h"
	"${COMMON_SOURCE_DIR}/utils.h"
	"${COMMON_SOURCE_DIR}/atomic.h"
	"${COMMON_SOURCE_DIR}/spinlock.h"
	"${COMMON_SOURCE_DIR}/thread.h"
	"${COMMON_SOURCE_DIR}/mutex.h"
	"${COMMON_SOURCE_DIR}/raconf.h"
	"${COMMON_SOURCE_DIR}/mempool.h"
	"${COMMON_SOURCE_DIR}/msg_conf.h"
	"${COMMON_SOURCE_DIR}/cli.h"
	${LIBCONFIG_HEADERS} # needed by conf.h/showmsg.h
	CACHE INTERNAL "common_base headers" )
set( COMMON_BASE_SOURCES
	"${COMMON_SOURCE_DIR}/conf.c"
	"${COMMON_SOURCE_DIR}/core.c"
	"${COMMON_SOURCE_DIR}/db.c"
	"${COMMON_SOURCE_DIR}/des.c"
	"${COMMON_SOURCE_DIR}/ers.c"
	"${COMMON_SOURCE_DIR}/grfio.c"
	"${COMMON_SOURCE_DIR}/malloc.c"
	"${COMMON_SOURCE_DIR}/mapindex.c"
	"${COMMON_SOURCE_DIR}/md5calc.c"
	"${COMMON_SOURCE_DIR}/nullpo.c"
	"${COMMON_SOURCE_DIR}/random.c"
	"${COMMON_SOURCE_DIR}/showmsg.c"
	"${COMMON_SOURCE_DIR}/socket.c"
	"${COMMON_SOURCE_DIR}/strlib.c"
	"${COMMON_SOURCE_DIR}/timer.c"
	"${COMMON_SOURCE_DIR}/utils.c"
	"${COMMON_SOURCE_DIR}/thread.c"
	"${COMMON_SOURCE_DIR}/mutex.c"
	"${COMMON_SOURCE_DIR}/mempool.c"
	"${COMMON_SOURCE_DIR}/raconf.c"
	"${COMMON_SOURCE_DIR}/msg_conf.c"
	"${COMMON_SOURCE_DIR}/cli.c"
	${LIBCONFIG_SOURCES} # needed by conf.c/showmsg.c
	CACHE INTERNAL "common_base sources" )
set( COMMON_BASE_INCLUDE_DIRS
	${LIBCONFIG_INCLUDE_DIRS}
	CACHE INTERNAL "common_base include dirs" )
set( COMMON_BASE_DEFINITIONS
	${LIBCONFIG_DEFINITIONS}
	CACHE INTERNAL "common_base definitions" )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${ZLIB_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MT19937AR_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${COMMON_BASE_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_BASE_DEFINITIONS}" )
set( SOURCE_FILES ${MT19937AR_HEADERS} ${MT19937AR_SOURCES} ${COMMON_BASE_HEADERS} ${COMMON_BASE_SOURCES} )
source_group( mt19937ar FILES ${MT19937AR_HEADERS} ${MT19937AR_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_BASE_SOURCES} )
add_library( common_base ${SOURCE_FILES} )
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
	"${CMAKE_CURRENT_SOURCE_DIR}/sql.h"
	CACHE INTERNAL "common headers" )
set( COMMON_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/sql.c"
	CACHE INTERNAL "common sources" )
set( DEPENDENCIES common_base )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${MYSQL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS}" )
set( SOURCE_FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )
add_library( common ${SOURCE_FILES} )
add_dependencies( common ${DEPENDENCIES} )
target_link_libraries( common ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( common PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
include_directories( ${INCLUDE_DIRS} )
set( HAVE_common ON  CACHE INTERNAL "" )
set( TARGET_LIST ${TARGET_LIST} common  CACHE INTERNAL "" )
message( STATUS "Creating target common - done" )
else()
message( STATUS "Skipping target common (requires common_base and MYSQL)" )
unset( HAVE_common CACHE )
endif()
