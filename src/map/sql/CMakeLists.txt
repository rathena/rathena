
#
# map sql
#
if( BUILD_SQL_SERVERS )
message( STATUS "Creating target map-server_sql" )
set( SQL_MAP_HEADERS
	"${SQL_MAP_SOURCE_DIR}/atcommand.h"
	"${SQL_MAP_SOURCE_DIR}/battle.h"
	"${SQL_MAP_SOURCE_DIR}/battleground.h"
	"${SQL_MAP_SOURCE_DIR}/buyingstore.h"
	"${SQL_MAP_SOURCE_DIR}/chat.h"
	"${SQL_MAP_SOURCE_DIR}/chrif.h"
	"${SQL_MAP_SOURCE_DIR}/clif.h"
	"${SQL_MAP_SOURCE_DIR}/date.h"
	"${SQL_MAP_SOURCE_DIR}/duel.h"
	"${SQL_MAP_SOURCE_DIR}/elemental.h"
	"${SQL_MAP_SOURCE_DIR}/guild.h"
	"${SQL_MAP_SOURCE_DIR}/homunculus.h"
	"${SQL_MAP_SOURCE_DIR}/instance.h"
	"${SQL_MAP_SOURCE_DIR}/intif.h"
	"${SQL_MAP_SOURCE_DIR}/itemdb.h"
	"${SQL_MAP_SOURCE_DIR}/log.h"
	"${SQL_MAP_SOURCE_DIR}/mail.h"
	"${SQL_MAP_SOURCE_DIR}/map.h"
	"${SQL_MAP_SOURCE_DIR}/mapreg.h"
	"${SQL_MAP_SOURCE_DIR}/mercenary.h"
	"${SQL_MAP_SOURCE_DIR}/mob.h"
	"${SQL_MAP_SOURCE_DIR}/npc.h"
	"${SQL_MAP_SOURCE_DIR}/party.h"
	"${SQL_MAP_SOURCE_DIR}/path.h"
	"${SQL_MAP_SOURCE_DIR}/pc.h"
	"${SQL_MAP_SOURCE_DIR}/pc_groups.h"
	"${SQL_MAP_SOURCE_DIR}/pet.h"
	"${SQL_MAP_SOURCE_DIR}/quest.h"
	"${SQL_MAP_SOURCE_DIR}/script.h"
	"${SQL_MAP_SOURCE_DIR}/searchstore.h"
	"${SQL_MAP_SOURCE_DIR}/skill.h"
	"${SQL_MAP_SOURCE_DIR}/status.h"
	"${SQL_MAP_SOURCE_DIR}/storage.h"
	"${SQL_MAP_SOURCE_DIR}/trade.h"
	"${SQL_MAP_SOURCE_DIR}/unit.h"
	"${SQL_MAP_SOURCE_DIR}/vending.h"
	"${SQL_MAP_SOURCE_DIR}/cashshop.h"
	"${SQL_MAP_SOURCE_DIR}/channel.h"
	)
set( SQL_MAP_SOURCES
	"${SQL_MAP_SOURCE_DIR}/atcommand.c"
	"${SQL_MAP_SOURCE_DIR}/battle.c"
	"${SQL_MAP_SOURCE_DIR}/battleground.c"
	"${SQL_MAP_SOURCE_DIR}/buyingstore.c"
	"${SQL_MAP_SOURCE_DIR}/chat.c"
	"${SQL_MAP_SOURCE_DIR}/chrif.c"
	"${SQL_MAP_SOURCE_DIR}/clif.c"
	"${SQL_MAP_SOURCE_DIR}/date.c"
	"${SQL_MAP_SOURCE_DIR}/duel.c"
	"${SQL_MAP_SOURCE_DIR}/elemental.c"
	"${SQL_MAP_SOURCE_DIR}/guild.c"
	"${SQL_MAP_SOURCE_DIR}/homunculus.c"
	"${SQL_MAP_SOURCE_DIR}/instance.c"
	"${SQL_MAP_SOURCE_DIR}/intif.c"
	"${SQL_MAP_SOURCE_DIR}/itemdb.c"
	"${SQL_MAP_SOURCE_DIR}/log.c"
	"${SQL_MAP_SOURCE_DIR}/mail.c"
	"${SQL_MAP_SOURCE_DIR}/map.c"
	"${SQL_MAP_SOURCE_DIR}/mapreg_sql.c"
	"${SQL_MAP_SOURCE_DIR}/mercenary.c"
	"${SQL_MAP_SOURCE_DIR}/mob.c"
	"${SQL_MAP_SOURCE_DIR}/npc.c"
	"${SQL_MAP_SOURCE_DIR}/npc_chat.c"
	"${SQL_MAP_SOURCE_DIR}/party.c"
	"${SQL_MAP_SOURCE_DIR}/path.c"
	"${SQL_MAP_SOURCE_DIR}/pc.c"
	"${SQL_MAP_SOURCE_DIR}/pc_groups.c"
	"${SQL_MAP_SOURCE_DIR}/pet.c"
	"${SQL_MAP_SOURCE_DIR}/quest.c"
	"${SQL_MAP_SOURCE_DIR}/script.c"
	"${SQL_MAP_SOURCE_DIR}/searchstore.c"
	"${SQL_MAP_SOURCE_DIR}/skill.c"
	"${SQL_MAP_SOURCE_DIR}/status.c"
	"${SQL_MAP_SOURCE_DIR}/storage.c"
	"${SQL_MAP_SOURCE_DIR}/trade.c"
	"${SQL_MAP_SOURCE_DIR}/unit.c"
	"${SQL_MAP_SOURCE_DIR}/vending.c"
	"${SQL_MAP_SOURCE_DIR}/cashshop.c"
	"${SQL_MAP_SOURCE_DIR}/channel.c"
	)
set( DEPENDENCIES common_sql )
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
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_SQL_HEADERS} ${SQL_MAP_HEADERS} ${SQL_MAP_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_SQL_HEADERS} )
source_group( map FILES ${SQL_MAP_HEADERS} ${SQL_MAP_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( map-server_sql ${SOURCE_FILES} )
add_dependencies( map-server_sql ${DEPENDENCIES} )
target_link_libraries( map-server_sql ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( map-server_sql PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_mapserver_sql DESCRIPTION "map-server (sql version)" DISPLAY_NAME "map-server_sql" GROUP Runtime )
	install( TARGETS map-server_sql
		DESTINATION "."
		COMPONENT Runtime_mapserver_sql )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} map-server_sql  CACHE INTERNAL "" )
message( STATUS "Creating target map-server_sql - done" )
endif( BUILD_SQL_SERVERS )
