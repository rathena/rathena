
#
# mapcache
#
if( WITH_ZLIB )
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
target_link_libraries( mapcache ${LIBRARIES} )
set_target_properties( mapcache PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( WITH_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_mapcache DESCRIPTION "mapcache generator" DISPLAY_NAME "mapcache" GROUP Runtime )
	install( TARGETS mapcache
		DESTINATION "."
		COMPONENT Runtime_mapcache )
endif()
set( HAVE_mapcache ON  CACHE BOOL "mapcache target is available" )
mark_as_advanced( HAVE_mapcache )
set( TARGET_LIST ${TARGET_LIST} mapcache  CACHE INTERNAL "" )
message( STATUS "Creating target mapcache - done" )
else()
message( STATUS "Skipping target mapcache (requires ZLIB)" )
unset( HAVE_mapcache CACHE )
endif()
