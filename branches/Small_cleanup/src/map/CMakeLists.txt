#
# setup
#
set( MAP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )


#
# map server
#
if( BUILD_SERVERS )
message( STATUS "Creating target map-server" )
set( MAP_HEADERS
	"${MAP_SOURCE_DIR}/atcommand.h"
	"${MAP_SOURCE_DIR}/battle.h"
	"${MAP_SOURCE_DIR}/battleground.h"
	"${MAP_SOURCE_DIR}/buyingstore.h"
	"${MAP_SOURCE_DIR}/chat.h"
	"${MAP_SOURCE_DIR}/chrif.h"
	"${MAP_SOURCE_DIR}/clif.h"
	"${MAP_SOURCE_DIR}/date.h"
	"${MAP_SOURCE_DIR}/duel.h"
	"${MAP_SOURCE_DIR}/elemental.h"
	"${MAP_SOURCE_DIR}/guild.h"
	"${MAP_SOURCE_DIR}/homunculus.h"
	"${MAP_SOURCE_DIR}/instance.h"
	"${MAP_SOURCE_DIR}/intif.h"
	"${MAP_SOURCE_DIR}/itemdb.h"
	"${MAP_SOURCE_DIR}/log.h"
	"${MAP_SOURCE_DIR}/mail.h"
	"${MAP_SOURCE_DIR}/map.h"
	"${MAP_SOURCE_DIR}/mapreg.h"
	"${MAP_SOURCE_DIR}/mercenary.h"
	"${MAP_SOURCE_DIR}/mob.h"
	"${MAP_SOURCE_DIR}/npc.h"
	"${MAP_SOURCE_DIR}/party.h"
	"${MAP_SOURCE_DIR}/path.h"
	"${MAP_SOURCE_DIR}/pc.h"
	"${MAP_SOURCE_DIR}/pc_groups.h"
	"${MAP_SOURCE_DIR}/pet.h"
	"${MAP_SOURCE_DIR}/quest.h"
	"${MAP_SOURCE_DIR}/script.h"
	"${MAP_SOURCE_DIR}/searchstore.h"
	"${MAP_SOURCE_DIR}/skill.h"
	"${MAP_SOURCE_DIR}/status.h"
	"${MAP_SOURCE_DIR}/storage.h"
	"${MAP_SOURCE_DIR}/trade.h"
	"${MAP_SOURCE_DIR}/unit.h"
	"${MAP_SOURCE_DIR}/vending.h"
	"${MAP_SOURCE_DIR}/cashshop.h"
	"${MAP_SOURCE_DIR}/channel.h"
	)
set( MAP_SOURCES
	"${MAP_SOURCE_DIR}/atcommand.c"
	"${MAP_SOURCE_DIR}/battle.c"
	"${MAP_SOURCE_DIR}/battleground.c"
	"${MAP_SOURCE_DIR}/buyingstore.c"
	"${MAP_SOURCE_DIR}/chat.c"
	"${MAP_SOURCE_DIR}/chrif.c"
	"${MAP_SOURCE_DIR}/clif.c"
	"${MAP_SOURCE_DIR}/date.c"
	"${MAP_SOURCE_DIR}/duel.c"
	"${MAP_SOURCE_DIR}/elemental.c"
	"${MAP_SOURCE_DIR}/guild.c"
	"${MAP_SOURCE_DIR}/homunculus.c"
	"${MAP_SOURCE_DIR}/instance.c"
	"${MAP_SOURCE_DIR}/intif.c"
	"${MAP_SOURCE_DIR}/itemdb.c"
	"${MAP_SOURCE_DIR}/log.c"
	"${MAP_SOURCE_DIR}/mail.c"
	"${MAP_SOURCE_DIR}/map.c"
	"${MAP_SOURCE_DIR}/mapreg.c"
	"${MAP_SOURCE_DIR}/mercenary.c"
	"${MAP_SOURCE_DIR}/mob.c"
	"${MAP_SOURCE_DIR}/npc.c"
	"${MAP_SOURCE_DIR}/npc_chat.c"
	"${MAP_SOURCE_DIR}/party.c"
	"${MAP_SOURCE_DIR}/path.c"
	"${MAP_SOURCE_DIR}/pc.c"
	"${MAP_SOURCE_DIR}/pc_groups.c"
	"${MAP_SOURCE_DIR}/pet.c"
	"${MAP_SOURCE_DIR}/quest.c"
	"${MAP_SOURCE_DIR}/script.c"
	"${MAP_SOURCE_DIR}/searchstore.c"
	"${MAP_SOURCE_DIR}/skill.c"
	"${MAP_SOURCE_DIR}/status.c"
	"${MAP_SOURCE_DIR}/storage.c"
	"${MAP_SOURCE_DIR}/trade.c"
	"${MAP_SOURCE_DIR}/unit.c"
	"${MAP_SOURCE_DIR}/vending.c"
	"${MAP_SOURCE_DIR}/cashshop.c"
	"${MAP_SOURCE_DIR}/channel.c"
	)
set( DEPENDENCIES common )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${COMMON_BASE_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_BASE_DEFINITIONS}" )
if( WITH_PCRE )
	message( STATUS "Enabled PCRE code" )
	set( LIBRARIES ${LIBRARIES} ${PCRE_LIBRARIES} )
	set( INCLUDE_DIRS ${INCLUDE_DIRS} ${PCRE_INCLUDE_DIRS} )
	set( DEFINITIONS "${DEFINITIONS} -DPCRE_SUPPORT" )
else()
	message( STATUS "Disabled PCRE code" )
endif()
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} ${MAP_HEADERS} ${MAP_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} )
source_group( map FILES ${MAP_HEADERS} ${MAP_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( map-server ${SOURCE_FILES} )
add_dependencies( map-server ${DEPENDENCIES} )
target_link_libraries( map-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( map-server PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_mapserver DESCRIPTION "map-server" DISPLAY_NAME "map-server" GROUP Runtime )
	install( TARGETS map-server
		DESTINATION "."
		COMPONENT Runtime_mapserver )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} map-server  CACHE INTERNAL "" )
message( STATUS "Creating target map-server - done" )
endif( BUILD_SERVERS )
