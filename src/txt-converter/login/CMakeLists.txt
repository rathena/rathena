
#
# login-converter
#
if( WITH_MYSQL )
message( STATUS "Creating target login-converter" )
set( COMMON_HEADERS
	${COMMON_MINI_HEADERS}
	"${COMMON_SOURCE_DIR}/db.h"
	"${COMMON_SOURCE_DIR}/ers.h"
	"${COMMON_SOURCE_DIR}/lock.h"
	"${COMMON_SOURCE_DIR}/sql.h"
	"${COMMON_SOURCE_DIR}/timer.h"
	"${COMMON_SOURCE_DIR}/utils.h"
	)
set( COMMON_SOURCES
	${COMMON_MINI_SOURCES}
	"${COMMON_SOURCE_DIR}/db.c"
	"${COMMON_SOURCE_DIR}/ers.c"
	"${COMMON_SOURCE_DIR}/lock.c"
	"${COMMON_SOURCE_DIR}/sql.c"
	"${COMMON_SOURCE_DIR}/timer.c"
	"${COMMON_SOURCE_DIR}/utils.c"
	)
set( TXT_HEADERS
	"${TXT_LOGIN_SOURCE_DIR}/account.h"
	)
set( TXT_SOURCES
	"${TXT_LOGIN_SOURCE_DIR}/account_txt.c"
	)
set( SQL_HEADERS
	"${SQL_LOGIN_SOURCE_DIR}/account.h"
	)
set( SQL_SOURCES
	"${SQL_LOGIN_SOURCE_DIR}/account_sql.c"
	)
set( CONVERTER_SOURCES
	"${CONVERTER_SOURCE_DIR}/login-converter.c"
	)
set( LIBRARIES ${GLOBAL_LIBRARIES} ${MYSQL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} ${COMMON_MINI_DEFINITIONS} WITH_TXT WITH_SQL )
set( SOURCE_FILES ${COMMON_HEADERS} ${COMMON_SOURCES} ${TXT_HEADERS} ${TXT_SOURCES} ${SQL_HEADERS} ${SQL_SOURCES} ${CONVERTER_SOURCES} )
source_group( common FILES ${COMMON_HEADERS} ${COMMON_SOURCES} )
source_group( txt FILES ${TXT_HEADERS} ${TXT_SOURCES} )
source_group( sql FILES ${SQL_HEADERS} ${SQL_SOURCES} )
source_group( converter FILES ${CONVERTER_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( login-converter ${SOURCE_FILES} )
target_link_libraries( login-converter ${LIBRARIES} )
set_target_properties( login-converter PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( WITH_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_loginconverter DESCRIPTION "login-converter" DISPLAY_NAME "login-converter" GROUP Runtime )
	install( TARGETS login-converter
		DESTINATION "tools"
		COMPONENT Runtime_loginconverter )
endif()
set( HAVE_login-converter ON  CACHE BOOL "login-converter target is available" )
mark_as_advanced( HAVE_login-converter )
set( TARGET_LIST ${TARGET_LIST} login-converter  CACHE INTERNAL "" )
message( STATUS "Creating target login-converter - done" )
else()
message( STATUS "Skipping target login-converter (requires MYSQL)" )
unset( HAVE_login-converter CACHE )
endif()
