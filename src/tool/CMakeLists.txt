
#
# mapcache
#
if( WITH_ZLIB )
	option( BUILD_MAPCACHE "build mapcache executable" ON )
else()
	message( STATUS "Disabled mapcache target (required ZLIB)" )
endif()
if( BUILD_MAPCACHE )
message( STATUS "Creating target mapcache" )
set( COMMON_HEADERS
	${COMMON_MINI_HEADERS}
	"${COMMON_SOURCE_DIR}/des.h"
	"${COMMON_SOURCE_DIR}/grfio.h"
	"${COMMON_SOURCE_DIR}/utils.h"
	)
set( COMMON_SOURCES
	${COMMON_MINI_SOURCES}
	"${COMMON_SOURCE_DIR}/des.c"
	"${COMMON_SOURCE_DIR}/grfio.c"
	"${COMMON_SOURCE_DIR}/utils.c"
	)
set( MAPCACHE_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/mapcache.c"
	)
set( LIBRARIES ${GLOBAL_LIBRARIES} ${ZLIB_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${COMMON_MINI_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_MINI_DEFINITIONS}" )
set( SOURCE_FILES ${COMMON_HEADERS} ${COMMON_SOURCES} ${MAPCACHE_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )
source_group( mapcache FILES ${MAPCACHE_SOURCES} )
add_executable( mapcache ${SOURCE_FILES} )
include_directories( ${INCLUDE_DIRS} )
target_link_libraries( mapcache ${LIBRARIES} )
set_target_properties( mapcache PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_mapcache DESCRIPTION "mapcache generator" DISPLAY_NAME "mapcache" GROUP Runtime )
	install( TARGETS mapcache
		DESTINATION "."
		COMPONENT Runtime_mapcache )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} mapcache  CACHE INTERNAL "" )
message( STATUS "Creating target mapcache - done" )
endif( BUILD_MAPCACHE )
