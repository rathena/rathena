
#
# map server
#
if( BUILD_SERVERS )
message( STATUS "Creating target map-server" )
set( MAP_HEADERS
	"${SERVER_MAP_SOURCE_DIR}/atcommand.h"
	"${SERVER_MAP_SOURCE_DIR}/battle.h"
	"${SERVER_MAP_SOURCE_DIR}/battleground.h"
	"${SERVER_MAP_SOURCE_DIR}/buyingstore.h"
	"${SERVER_MAP_SOURCE_DIR}/chat.h"
	"${SERVER_MAP_SOURCE_DIR}/chrif.h"
	"${SERVER_MAP_SOURCE_DIR}/clif.h"
	"${SERVER_MAP_SOURCE_DIR}/date.h"
	"${SERVER_MAP_SOURCE_DIR}/duel.h"
	"${SERVER_MAP_SOURCE_DIR}/elemental.h"
	"${SERVER_MAP_SOURCE_DIR}/guild.h"
	"${SERVER_MAP_SOURCE_DIR}/homunculus.h"
	"${SERVER_MAP_SOURCE_DIR}/instance.h"
	"${SERVER_MAP_SOURCE_DIR}/intif.h"
	"${SERVER_MAP_SOURCE_DIR}/itemdb.h"
	"${SERVER_MAP_SOURCE_DIR}/log.h"
	"${SERVER_MAP_SOURCE_DIR}/mail.h"
	"${SERVER_MAP_SOURCE_DIR}/map.h"
	"${SERVER_MAP_SOURCE_DIR}/mapreg.h"
	"${SERVER_MAP_SOURCE_DIR}/mercenary.h"
	"${SERVER_MAP_SOURCE_DIR}/mob.h"
	"${SERVER_MAP_SOURCE_DIR}/npc.h"
	"${SERVER_MAP_SOURCE_DIR}/party.h"
	"${SERVER_MAP_SOURCE_DIR}/path.h"
	"${SERVER_MAP_SOURCE_DIR}/pc.h"
	"${SERVER_MAP_SOURCE_DIR}/pc_groups.h"
	"${SERVER_MAP_SOURCE_DIR}/pet.h"
	"${SERVER_MAP_SOURCE_DIR}/quest.h"
	"${SERVER_MAP_SOURCE_DIR}/script.h"
	"${SERVER_MAP_SOURCE_DIR}/searchstore.h"
	"${SERVER_MAP_SOURCE_DIR}/skill.h"
	"${SERVER_MAP_SOURCE_DIR}/status.h"
	"${SERVER_MAP_SOURCE_DIR}/storage.h"
	"${SERVER_MAP_SOURCE_DIR}/trade.h"
	"${SERVER_MAP_SOURCE_DIR}/unit.h"
	"${SERVER_MAP_SOURCE_DIR}/vending.h"
	"${SERVER_MAP_SOURCE_DIR}/cashshop.h"
	"${SERVER_MAP_SOURCE_DIR}/channel.h"
	)
set( MAP_SOURCES
	"${SERVER_MAP_SOURCE_DIR}/atcommand.c"
	"${SERVER_MAP_SOURCE_DIR}/battle.c"
	"${SERVER_MAP_SOURCE_DIR}/battleground.c"
	"${SERVER_MAP_SOURCE_DIR}/buyingstore.c"
	"${SERVER_MAP_SOURCE_DIR}/chat.c"
	"${SERVER_MAP_SOURCE_DIR}/chrif.c"
	"${SERVER_MAP_SOURCE_DIR}/clif.c"
	"${SERVER_MAP_SOURCE_DIR}/date.c"
	"${SERVER_MAP_SOURCE_DIR}/duel.c"
	"${SERVER_MAP_SOURCE_DIR}/elemental.c"
	"${SERVER_MAP_SOURCE_DIR}/guild.c"
	"${SERVER_MAP_SOURCE_DIR}/homunculus.c"
	"${SERVER_MAP_SOURCE_DIR}/instance.c"
	"${SERVER_MAP_SOURCE_DIR}/intif.c"
	"${SERVER_MAP_SOURCE_DIR}/itemdb.c"
	"${SERVER_MAP_SOURCE_DIR}/log.c"
	"${SERVER_MAP_SOURCE_DIR}/mail.c"
	"${SERVER_MAP_SOURCE_DIR}/map.c"
	"${SERVER_MAP_SOURCE_DIR}/mapreg.c"
	"${SERVER_MAP_SOURCE_DIR}/mercenary.c"
	"${SERVER_MAP_SOURCE_DIR}/mob.c"
	"${SERVER_MAP_SOURCE_DIR}/npc.c"
	"${SERVER_MAP_SOURCE_DIR}/npc_chat.c"
	"${SERVER_MAP_SOURCE_DIR}/party.c"
	"${SERVER_MAP_SOURCE_DIR}/path.c"
	"${SERVER_MAP_SOURCE_DIR}/pc.c"
	"${SERVER_MAP_SOURCE_DIR}/pc_groups.c"
	"${SERVER_MAP_SOURCE_DIR}/pet.c"
	"${SERVER_MAP_SOURCE_DIR}/quest.c"
	"${SERVER_MAP_SOURCE_DIR}/script.c"
	"${SERVER_MAP_SOURCE_DIR}/searchstore.c"
	"${SERVER_MAP_SOURCE_DIR}/skill.c"
	"${SERVER_MAP_SOURCE_DIR}/status.c"
	"${SERVER_MAP_SOURCE_DIR}/storage.c"
	"${SERVER_MAP_SOURCE_DIR}/trade.c"
	"${SERVER_MAP_SOURCE_DIR}/unit.c"
	"${SERVER_MAP_SOURCE_DIR}/vending.c"
	"${SERVER_MAP_SOURCE_DIR}/cashshop.c"
	"${SERVER_MAP_SOURCE_DIR}/channel.c"
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
