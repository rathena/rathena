
# setup
set( COMMON_SOURCES
	"${COMMON_SOURCE_DIR}/core.c"
#	"${COMMON_SOURCE_DIR}/db.c"
#	"${COMMON_SOURCE_DIR}/ers.c"
	"${COMMON_SOURCE_DIR}/grfio.c"
#	"${COMMON_SOURCE_DIR}/lock.c"
	"${COMMON_SOURCE_DIR}/malloc.c"
#	"${COMMON_SOURCE_DIR}/mapindex.c"
#	"${COMMON_SOURCE_DIR}/md5calc.c"
#	"${COMMON_SOURCE_DIR}/nullpo.c"
#	"${COMMON_SOURCE_DIR}/plugins.c"
#	"${COMMON_SOURCE_DIR}/random.c"
	"${COMMON_SOURCE_DIR}/showmsg.c"
#	"${COMMON_SOURCE_DIR}/socket.c"
	"${COMMON_SOURCE_DIR}/strlib.c"
#	"${COMMON_SOURCE_DIR}/timer.c"
	"${COMMON_SOURCE_DIR}/utils.c"
	)
set( MAPCACHE_SOURCES
	mapcache.c
	)


# mapcache
if( USE_ZLIB )
message( STATUS "Creating target mapcache" )
set( COMMON_HEADERS
	${COMMON_MINI_HEADERS}
	"${COMMON_SOURCE_DIR}/grfio.h"
	"${COMMON_SOURCE_DIR}/utils.h"
	)
set( COMMON_SOURCES
	${COMMON_MINI_SOURCES}
	"${COMMON_SOURCE_DIR}/grfio.c"
	"${COMMON_SOURCE_DIR}/utils.c"
	)
set( MAPCACHE_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/mapcache.c"
	)
set( LIBRARIES ${GLOBAL_LIBRARIES} ${ZLIB_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} ${COMMON_MINI_DEFINITIONS} )
set( SOURCE_FILES ${COMMON_HEADERS} ${COMMON_SOURCES} ${MAPCACHE_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )
source_group( mapcache FILES ${MAPCACHE_SOURCES} )
add_executable( mapcache ${SOURCE_FILES} )
include_directories( ${INCLUDE_DIRS} )
target_link_libraries( mapcache ${ZLIB_LIBRARIES} )
set_target_properties( mapcache PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( INSTALL_RUN_DATA )
	install( TARGETS mapcache
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		COMPONENT "mapcache" )
endif()
message( STATUS "Creating target mapcache - done" )
set( HAVE_mapcache ON  CACHE BOOL "mapcache target is available" )
mark_as_advanced( HAVE_mapcache )
else()
message( STATUS "Skipping target mapcache (requires ZLIB)" )
unset( HAVE_mapcache CACHE )
endif()
