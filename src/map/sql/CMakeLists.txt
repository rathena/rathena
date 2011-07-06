
#
# map sql
#
if( HAVE_common_sql )
message( STATUS "Creating target map-server_sql" )
set( SQL_MAP_HEADERS
	"${MAP_SOURCE_DIR}/atcommand.h"
	"${MAP_SOURCE_DIR}/battle.h"
	"${MAP_SOURCE_DIR}/battleground.h"
	"${MAP_SOURCE_DIR}/buyingstore.h"
	"${MAP_SOURCE_DIR}/chat.h"
	"${MAP_SOURCE_DIR}/chrif.h"
	"${MAP_SOURCE_DIR}/clif.h"
	"${MAP_SOURCE_DIR}/date.h"
	"${MAP_SOURCE_DIR}/duel.h"
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
	)
set( SQL_MAP_SOURCES
	"${MAP_SOURCE_DIR}/atcommand.c"
	"${MAP_SOURCE_DIR}/battle.c"
	"${MAP_SOURCE_DIR}/battleground.c"
	"${MAP_SOURCE_DIR}/buyingstore.c"
	"${MAP_SOURCE_DIR}/chat.c"
	"${MAP_SOURCE_DIR}/chrif.c"
	"${MAP_SOURCE_DIR}/clif.c"
	"${MAP_SOURCE_DIR}/date.c"
	"${MAP_SOURCE_DIR}/duel.c"
	"${MAP_SOURCE_DIR}/guild.c"
	"${MAP_SOURCE_DIR}/homunculus.c"
	"${MAP_SOURCE_DIR}/instance.c"
	"${MAP_SOURCE_DIR}/intif.c"
	"${MAP_SOURCE_DIR}/itemdb.c"
	"${MAP_SOURCE_DIR}/log.c"
	"${MAP_SOURCE_DIR}/mail.c"
	"${MAP_SOURCE_DIR}/map.c"
	"${MAP_SOURCE_DIR}/mapreg_sql.c"
	"${MAP_SOURCE_DIR}/mercenary.c"
	"${MAP_SOURCE_DIR}/mob.c"
	"${MAP_SOURCE_DIR}/npc.c"
	"${MAP_SOURCE_DIR}/npc_chat.c"
	"${MAP_SOURCE_DIR}/party.c"
	"${MAP_SOURCE_DIR}/path.c"
	"${MAP_SOURCE_DIR}/pc.c"
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
	)
set( DEPENDENCIES common_sql )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} )
if( USE_PCRE )
	message( STATUS "Using PCRE" )
	list( APPEND LIBRARIES ${PCRE_LIBRARIES} )
	list( APPEND INCLUDE_DIRS ${PCRE_INCLUDE_DIRS} )
	list( APPEND DEFINITIONS PCRE_SUPPORT )
endif()
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_SQL_HEADERS} ${SQL_MAP_HEADERS} ${SQL_MAP_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_SQL_HEADERS} )
source_group( map FILES ${SQL_MAP_HEADERS} ${SQL_MAP_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( map-server_sql ${SOURCE_FILES} )
add_dependencies( map-server_sql ${DEPENDENCIES} )
target_link_libraries( map-server_sql ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( map-server_sql PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( INSTALL_RUN_DATA )
	install( TARGETS map-server_sql
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		COMPONENT "map-server_sql" )
endif()
message( STATUS "Creating target map-server_sql - done" )
set( HAVE_map-server_sql ON  CACHE BOOL "map-server_sql target is available" )
mark_as_advanced( HAVE_map-server_sql )
else()
message( STATUS "Skipping target map-server_sql (requires common_sql; optional PCRE)" )
unset( HAVE_map-server_sql CACHE )
endif()
