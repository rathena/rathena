
#
# login txt
#
if( HAVE_common_base )
message( STATUS "Creating target login-server" )
set( TXT_LOGIN_HEADERS
	"${LOGIN_SOURCE_DIR}/account.h"
	"${LOGIN_SOURCE_DIR}/ipban.h"
	"${LOGIN_SOURCE_DIR}/login.h"
	"${LOGIN_SOURCE_DIR}/loginlog.h"
	)
set( TXT_LOGIN_SOURCES
	"${LOGIN_SOURCE_DIR}/account_txt.c"
	"${LOGIN_SOURCE_DIR}/ipban_txt.c"
	"${LOGIN_SOURCE_DIR}/login.c"
	"${LOGIN_SOURCE_DIR}/loginlog_txt.c"
	)
set( DEPENDENCIES common_base )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} WITH_TXT )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${TXT_LOGIN_HEADERS} ${TXT_LOGIN_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} )
source_group( login FILES ${TXT_LOGIN_HEADERS} ${TXT_LOGIN_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( login-server ${SOURCE_FILES} )
add_dependencies( login-server ${DEPENDENCIES} )
target_link_libraries( login-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( login-server PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( INSTALL_RUN_DATA )
	install( TARGETS login-server
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		COMPONENT "login-server" )
endif()
message( STATUS "Creating target login-server - done" )
set( HAVE_login-server ON  CACHE BOOL "login-server target is available" )
mark_as_advanced( HAVE_login-server )
else()
message( STATUS "Skipping target login-server (requires common_base)" )
unset( HAVE_login-server CACHE )
endif()
