
#
# map txt
#
if( HAVE_common_base )
message( STATUS "Creating target map-server" )
set( TXT_MAP_HEADERS
	"${TXT_MAP_SOURCE_DIR}/atcommand.h"
	"${TXT_MAP_SOURCE_DIR}/battle.h"
	"${TXT_MAP_SOURCE_DIR}/battleground.h"
	"${TXT_MAP_SOURCE_DIR}/buyingstore.h"
	"${TXT_MAP_SOURCE_DIR}/chat.h"
	"${TXT_MAP_SOURCE_DIR}/chrif.h"
	"${TXT_MAP_SOURCE_DIR}/clif.h"
	"${TXT_MAP_SOURCE_DIR}/date.h"
	"${TXT_MAP_SOURCE_DIR}/duel.h"
	"${TXT_MAP_SOURCE_DIR}/guild.h"
	"${TXT_MAP_SOURCE_DIR}/homunculus.h"
	"${TXT_MAP_SOURCE_DIR}/instance.h"
	"${TXT_MAP_SOURCE_DIR}/intif.h"
	"${TXT_MAP_SOURCE_DIR}/itemdb.h"
	"${TXT_MAP_SOURCE_DIR}/log.h"
	"${TXT_MAP_SOURCE_DIR}/mail.h"
	"${TXT_MAP_SOURCE_DIR}/map.h"
	"${TXT_MAP_SOURCE_DIR}/mapreg.h"
	"${TXT_MAP_SOURCE_DIR}/mercenary.h"
	"${TXT_MAP_SOURCE_DIR}/mob.h"
	"${TXT_MAP_SOURCE_DIR}/npc.h"
	"${TXT_MAP_SOURCE_DIR}/party.h"
	"${TXT_MAP_SOURCE_DIR}/path.h"
	"${TXT_MAP_SOURCE_DIR}/pc.h"
	"${TXT_MAP_SOURCE_DIR}/pet.h"
	"${TXT_MAP_SOURCE_DIR}/quest.h"
	"${TXT_MAP_SOURCE_DIR}/script.h"
	"${TXT_MAP_SOURCE_DIR}/searchstore.h"
	"${TXT_MAP_SOURCE_DIR}/skill.h"
	"${TXT_MAP_SOURCE_DIR}/status.h"
	"${TXT_MAP_SOURCE_DIR}/storage.h"
	"${TXT_MAP_SOURCE_DIR}/trade.h"
	"${TXT_MAP_SOURCE_DIR}/unit.h"
	"${TXT_MAP_SOURCE_DIR}/vending.h"
	)
set( TXT_MAP_SOURCES
	"${TXT_MAP_SOURCE_DIR}/atcommand.c"
	"${TXT_MAP_SOURCE_DIR}/battle.c"
	"${TXT_MAP_SOURCE_DIR}/battleground.c"
	"${TXT_MAP_SOURCE_DIR}/buyingstore.c"
	"${TXT_MAP_SOURCE_DIR}/chat.c"
	"${TXT_MAP_SOURCE_DIR}/chrif.c"
	"${TXT_MAP_SOURCE_DIR}/clif.c"
	"${TXT_MAP_SOURCE_DIR}/date.c"
	"${TXT_MAP_SOURCE_DIR}/duel.c"
	"${TXT_MAP_SOURCE_DIR}/guild.c"
	"${TXT_MAP_SOURCE_DIR}/homunculus.c"
	"${TXT_MAP_SOURCE_DIR}/instance.c"
	"${TXT_MAP_SOURCE_DIR}/intif.c"
	"${TXT_MAP_SOURCE_DIR}/itemdb.c"
	"${TXT_MAP_SOURCE_DIR}/log.c"
	"${TXT_MAP_SOURCE_DIR}/mail.c"
	"${TXT_MAP_SOURCE_DIR}/map.c"
	"${TXT_MAP_SOURCE_DIR}/mapreg_txt.c"
	"${TXT_MAP_SOURCE_DIR}/mercenary.c"
	"${TXT_MAP_SOURCE_DIR}/mob.c"
	"${TXT_MAP_SOURCE_DIR}/npc.c"
	"${TXT_MAP_SOURCE_DIR}/npc_chat.c"
	"${TXT_MAP_SOURCE_DIR}/party.c"
	"${TXT_MAP_SOURCE_DIR}/path.c"
	"${TXT_MAP_SOURCE_DIR}/pc.c"
	"${TXT_MAP_SOURCE_DIR}/pet.c"
	"${TXT_MAP_SOURCE_DIR}/quest.c"
	"${TXT_MAP_SOURCE_DIR}/script.c"
	"${TXT_MAP_SOURCE_DIR}/searchstore.c"
	"${TXT_MAP_SOURCE_DIR}/skill.c"
	"${TXT_MAP_SOURCE_DIR}/status.c"
	"${TXT_MAP_SOURCE_DIR}/storage.c"
	"${TXT_MAP_SOURCE_DIR}/trade.c"
	"${TXT_MAP_SOURCE_DIR}/unit.c"
	"${TXT_MAP_SOURCE_DIR}/vending.c"
	)
set( DEPENDENCIES common_base )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} -DTXT_ONLY" )
if( WITH_PCRE )
	message( STATUS "Using PCRE" )
	set( LIBRARIES ${LIBRARIES} ${PCRE_LIBRARIES} )
	set( INCLUDE_DIRS ${INCLUDE_DIRS} ${PCRE_INCLUDE_DIRS} )
	set( DEFINITIONS "${DEFINITIONS} -DPCRE_SUPPORT" )
endif()
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${TXT_MAP_HEADERS} ${TXT_MAP_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} )
source_group( map FILES ${TXT_MAP_HEADERS} ${TXT_MAP_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( map-server ${SOURCE_FILES} )
add_dependencies( map-server ${DEPENDENCIES} )
target_link_libraries( map-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( map-server PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( WITH_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_mapserver_txt DESCRIPTION "map-server (txt version)" DISPLAY_NAME "map-server" GROUP Runtime )
	install( TARGETS map-server
		DESTINATION "."
		COMPONENT Runtime_mapserver_txt )
endif()
set( HAVE_map-server ON  CACHE BOOL "map-server target is available" )
mark_as_advanced( HAVE_map-server )
set( TARGET_LIST ${TARGET_LIST} map-server  CACHE INTERNAL "" )
message( STATUS "Creating target map-server - done" )
else()
message( STATUS "Skipping target map-server (requires common_base; optional PCRE)" )
unset( HAVE_map-server CACHE )
endif()
