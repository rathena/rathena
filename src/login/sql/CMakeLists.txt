
#
# login sql
#
if( HAVE_common_sql )
message( STATUS "Creating target login-server_sql" )
set( SQL_LOGIN_HEADERS
	"${LOGIN_SOURCE_DIR}/account.h"
	"${LOGIN_SOURCE_DIR}/ipban.h"
	"${LOGIN_SOURCE_DIR}/login.h"
	"${LOGIN_SOURCE_DIR}/loginlog.h"
	)
set( SQL_LOGIN_SOURCES
	"${LOGIN_SOURCE_DIR}/account_sql.c"
	"${LOGIN_SOURCE_DIR}/ipban_sql.c"
	"${LOGIN_SOURCE_DIR}/login.c"
	"${LOGIN_SOURCE_DIR}/loginlog_sql.c"
	)
set( DEPENDENCIES common_sql )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} WITH_SQL )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_SQL_HEADERS} ${SQL_LOGIN_HEADERS} ${SQL_LOGIN_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_SQL_HEADERS} )
source_group( login FILES ${SQL_LOGIN_HEADERS} ${SQL_LOGIN_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( login-server_sql ${SOURCE_FILES} )
add_dependencies( login-server_sql ${DEPENDENCIES} )
target_link_libraries( login-server_sql ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( login-server_sql PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( WITH_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_loginserver_sql DESCRIPTION "login-server (sql version)" DISPLAY_NAME "login-server_sql" GROUP Runtime )
	install( TARGETS login-server_sql
		DESTINATION "."
		COMPONENT Runtime_loginserver_sql )
endif()
message( STATUS "Creating target login-server_sql - done" )
set( HAVE_login-server_sql ON  CACHE BOOL "login-server_sql target is available" )
mark_as_advanced( HAVE_login-server_sql )
else()
message( STATUS "Skipping target login-server_sql (requires common_sql)" )
unset( HAVE_login-server_sql CACHE )
endif()
