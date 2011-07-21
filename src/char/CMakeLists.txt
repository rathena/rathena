
#
# setup
#
set( TXT_CHAR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )


#
# char txt
#
if( HAVE_common_base )
message( STATUS "Creating target char-server" )
set( TXT_CHAR_HEADERS
	"${TXT_CHAR_SOURCE_DIR}/char.h"
	"${TXT_CHAR_SOURCE_DIR}/int_guild.h"
	"${TXT_CHAR_SOURCE_DIR}/int_homun.h"
	"${TXT_CHAR_SOURCE_DIR}/int_party.h"
	"${TXT_CHAR_SOURCE_DIR}/int_pet.h"
	"${TXT_CHAR_SOURCE_DIR}/int_status.h"
	"${TXT_CHAR_SOURCE_DIR}/int_storage.h"
	"${TXT_CHAR_SOURCE_DIR}/inter.h"
	)
set( TXT_CHAR_SOURCES
	"${TXT_CHAR_SOURCE_DIR}/char.c"
	"${TXT_CHAR_SOURCE_DIR}/int_guild.c"
	"${TXT_CHAR_SOURCE_DIR}/int_homun.c"
	"${TXT_CHAR_SOURCE_DIR}/int_party.c"
	"${TXT_CHAR_SOURCE_DIR}/int_pet.c"
	"${TXT_CHAR_SOURCE_DIR}/int_status.c"
	"${TXT_CHAR_SOURCE_DIR}/int_storage.c"
	"${TXT_CHAR_SOURCE_DIR}/inter.c"
	)
set( DEPENDENCIES common_base )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} -DTXT_ONLY" )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${TXT_CHAR_HEADERS} ${TXT_CHAR_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} )
source_group( char FILES ${TXT_CHAR_HEADERS} ${TXT_CHAR_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( char-server ${SOURCE_FILES} )
add_dependencies( char-server ${DEPENDENCIES} )
target_link_libraries( char-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( char-server PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_charserver_txt DESCRIPTION "char-server (txt version)" DISPLAY_NAME "char-server" GROUP Runtime )
	install( TARGETS char-server
		DESTINATION "."
		COMPONENT Runtime_charserver_txt )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} char-server  CACHE INTERNAL "" )
message( STATUS "Creating target char-server - done" )
else()
message( STATUS "Skipping target char-server (requires common_base)" )
endif()
