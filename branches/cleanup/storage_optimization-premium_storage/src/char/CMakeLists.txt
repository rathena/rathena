#
# setup
#
set( SERVER_CHAR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )


#
# char server
#
if( BUILD_SERVERS )
message( STATUS "Creating target char-server" )
set( CHAR_HEADERS
	"${SERVER_CHAR_SOURCE_DIR}/char.h"
	"${SERVER_CHAR_SOURCE_DIR}/char_clif.h"
	"${SERVER_CHAR_SOURCE_DIR}/char_cnslif.h"
	"${SERVER_CHAR_SOURCE_DIR}/char_logif.h"
	"${SERVER_CHAR_SOURCE_DIR}/char_mapif.h"
	"${SERVER_CHAR_SOURCE_DIR}/int_auction.h"
	"${SERVER_CHAR_SOURCE_DIR}/int_elemental.h"
	"${SERVER_CHAR_SOURCE_DIR}/int_guild.h"
	"${SERVER_CHAR_SOURCE_DIR}/int_homun.h"
	"${SERVER_CHAR_SOURCE_DIR}/int_mail.h"
	"${SERVER_CHAR_SOURCE_DIR}/int_mercenary.h"
	"${SERVER_CHAR_SOURCE_DIR}/int_party.h"
	"${SERVER_CHAR_SOURCE_DIR}/int_pet.h"
	"${SERVER_CHAR_SOURCE_DIR}/int_quest.h"
	"${SERVER_CHAR_SOURCE_DIR}/int_storage.h"
	"${SERVER_CHAR_SOURCE_DIR}/inter.h"
	)
set( CHAR_SOURCES
	"${SERVER_CHAR_SOURCE_DIR}/char.c"
	"${SERVER_CHAR_SOURCE_DIR}/char_clif.c"
	"${SERVER_CHAR_SOURCE_DIR}/char_cnslif.c"
	"${SERVER_CHAR_SOURCE_DIR}/char_logif.c"
	"${SERVER_CHAR_SOURCE_DIR}/char_mapif.c"
	"${SERVER_CHAR_SOURCE_DIR}/int_auction.c"
	"${SERVER_CHAR_SOURCE_DIR}/int_elemental.c"
	"${SERVER_CHAR_SOURCE_DIR}/int_guild.c"
	"${SERVER_CHAR_SOURCE_DIR}/int_homun.c"
	"${SERVER_CHAR_SOURCE_DIR}/int_mail.c"
	"${SERVER_CHAR_SOURCE_DIR}/int_mercenary.c"
	"${SERVER_CHAR_SOURCE_DIR}/int_party.c"
	"${SERVER_CHAR_SOURCE_DIR}/int_pet.c"
	"${SERVER_CHAR_SOURCE_DIR}/int_quest.c"
	"${SERVER_CHAR_SOURCE_DIR}/int_storage.c"
	"${SERVER_CHAR_SOURCE_DIR}/inter.c"
	)
set( DEPENDENCIES common )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${COMMON_BASE_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_BASE_DEFINITIONS}" )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} ${CHAR_HEADERS} ${CHAR_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} )
source_group( char FILES ${CHAR_HEADERS} ${CHAR_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( char-server ${SOURCE_FILES} )
add_dependencies( char-server ${DEPENDENCIES} )
target_link_libraries( char-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( char-server PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_charserver DESCRIPTION "char-server" DISPLAY_NAME "char-server" GROUP Runtime )
	install( TARGETS char-server
		DESTINATION "."
		COMPONENT Runtime_charserver )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} char-server  CACHE INTERNAL "" )
message( STATUS "Creating target char-server - done" )
endif( BUILD_SERVERS )
