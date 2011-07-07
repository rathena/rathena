
#
# char txt
#
if( HAVE_common_base )
message( STATUS "Creating target char-server" )
set( TXT_CHAR_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/char.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_guild.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_homun.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_party.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_pet.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_status.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_storage.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/inter.h"
	)
set( TXT_CHAR_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/char.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_guild.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_homun.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_party.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_pet.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_status.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/int_storage.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/inter.c"
	)
set( DEPENDENCIES common_base )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} TXT_ONLY )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${TXT_CHAR_HEADERS} ${TXT_CHAR_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} )
source_group( char FILES ${TXT_CHAR_HEADERS} ${TXT_CHAR_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( char-server ${SOURCE_FILES} )
add_dependencies( char-server ${DEPENDENCIES} )
target_link_libraries( char-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( char-server PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( WITH_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_charserver_txt DESCRIPTION "char-server (txt version)" DISPLAY_NAME "char-server" GROUP Runtime )
	install( TARGETS char-server
		DESTINATION "."
		COMPONENT Runtime_charserver_txt )
endif()
message( STATUS "Creating target char-server - done" )
set( HAVE_char-server ON  CACHE BOOL "char-server target is available" )
mark_as_advanced( HAVE_char-server )
else()
message( STATUS "Skipping target char-server (requires common_base)" )
unset( HAVE_char-server CACHE )
endif()
