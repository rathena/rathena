
#
# char-converter
#
if( BUILD_CONVERTERS )
message( STATUS "Creating target char-converter" )
set( COMMON_HEADERS
	${COMMON_MINI_HEADERS}
	"${COMMON_SOURCE_DIR}/mapindex.h"
	"${COMMON_SOURCE_DIR}/sql.h"
	"${COMMON_SOURCE_DIR}/timer.h"
	)
set( COMMON_SOURCES
	${COMMON_MINI_SOURCES}
	"${COMMON_SOURCE_DIR}/mapindex.c"
	"${COMMON_SOURCE_DIR}/sql.c"
	"${COMMON_SOURCE_DIR}/timer.c"
	)
set( TXT_HEADERS
	"${TXT_CHAR_SOURCE_DIR}/char.h"
	"${TXT_CHAR_SOURCE_DIR}/int_pet.h"
	"${TXT_CHAR_SOURCE_DIR}/int_storage.h"
	"${TXT_CHAR_SOURCE_DIR}/inter.h"
	"${TXT_CHAR_SOURCE_DIR}/int_party.h"
	"${TXT_CHAR_SOURCE_DIR}/int_guild.h"
	)
set( TXT_SOURCES
	"${TXT_CHAR_SOURCE_DIR}/char.c"
	"${TXT_CHAR_SOURCE_DIR}/int_pet.c"
	"${TXT_CHAR_SOURCE_DIR}/int_storage.c"
	"${TXT_CHAR_SOURCE_DIR}/inter.c"
	"${TXT_CHAR_SOURCE_DIR}/int_party.c"
	"${TXT_CHAR_SOURCE_DIR}/int_guild.c"
	)
set( SQL_HEADERS
	"${SQL_CHAR_SOURCE_DIR}/char.h"
	"${SQL_CHAR_SOURCE_DIR}/int_pet.h"
	"${SQL_CHAR_SOURCE_DIR}/int_storage.h"
	"${SQL_CHAR_SOURCE_DIR}/inter.h"
	"${SQL_CHAR_SOURCE_DIR}/int_party.h"
	"${SQL_CHAR_SOURCE_DIR}/int_guild.h"
	"${SQL_CHAR_SOURCE_DIR}/int_mercenary.h"
	)
set( SQL_SOURCES
	"${SQL_CHAR_SOURCE_DIR}/char.c"
	"${SQL_CHAR_SOURCE_DIR}/int_pet.c"
	"${SQL_CHAR_SOURCE_DIR}/int_storage.c"
	"${SQL_CHAR_SOURCE_DIR}/inter.c"
	"${SQL_CHAR_SOURCE_DIR}/int_party.c"
	"${SQL_CHAR_SOURCE_DIR}/int_guild.c"
	"${SQL_CHAR_SOURCE_DIR}/int_mercenary.c"
	)
set( CONVERTER_SOURCES
	"${CONVERTER_SOURCE_DIR}/char-converter.c"
	)
set( LIBRARIES ${GLOBAL_LIBRARIES} ${MYSQL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_MINI_DEFINITIONS} -DTXT_SQL_CONVERT" )
set( SOURCE_FILES ${COMMON_HEADERS} ${COMMON_SOURCES} ${TXT_HEADERS} ${TXT_SOURCES} ${SQL_HEADERS} ${SQL_SOURCES} ${CONVERTER_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )
source_group( txt FILES ${TXT_HEADERS} ${TXT_SOURCES} )
source_group( sql FILES ${SQL_HEADERS} ${SQL_SOURCES} )
source_group( converter FILES ${CONVERTER_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( char-converter ${SOURCE_FILES} )
target_link_libraries( char-converter ${LIBRARIES} )
set_target_properties( char-converter PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_charconverter DESCRIPTION "char-converter" DISPLAY_NAME "char-converter" GROUP Runtime )
	install( TARGETS char-converter
		DESTINATION "tools"
		COMPONENT Runtime_charconverter )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} char-converter  CACHE INTERNAL "" )
message( STATUS "Creating target char-converter - done" )
endif( BUILD_CONVERTERS )
