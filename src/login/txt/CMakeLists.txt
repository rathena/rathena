
#
# login txt
#
if( HAVE_common_base )
message( STATUS "Creating target login-server" )
set( TXT_LOGIN_HEADERS
	"${TXT_LOGIN_SOURCE_DIR}/account.h"
	"${TXT_LOGIN_SOURCE_DIR}/ipban.h"
	"${TXT_LOGIN_SOURCE_DIR}/login.h"
	"${TXT_LOGIN_SOURCE_DIR}/loginlog.h"
	)
set( TXT_LOGIN_SOURCES
	"${TXT_LOGIN_SOURCE_DIR}/account_txt.c"
	"${TXT_LOGIN_SOURCE_DIR}/ipban_txt.c"
	"${TXT_LOGIN_SOURCE_DIR}/login.c"
	"${TXT_LOGIN_SOURCE_DIR}/loginlog_txt.c"
	)
set( DEPENDENCIES common_base )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} -DWITH_TXT" )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${TXT_LOGIN_HEADERS} ${TXT_LOGIN_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} )
source_group( login FILES ${TXT_LOGIN_HEADERS} ${TXT_LOGIN_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( login-server ${SOURCE_FILES} )
add_dependencies( login-server ${DEPENDENCIES} )
target_link_libraries( login-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( login-server PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( WITH_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_loginserver_txt DESCRIPTION "login-server (txt version)" DISPLAY_NAME "login-server" GROUP Runtime )
	install( TARGETS login-server
		DESTINATION "."
		COMPONENT Runtime_loginserver_txt )
endif()
set( HAVE_login-server ON  CACHE BOOL "login-server target is available" )
mark_as_advanced( HAVE_login-server )
set( TARGET_LIST ${TARGET_LIST} login-server  CACHE INTERNAL "" )
message( STATUS "Creating target login-server - done" )
else()
message( STATUS "Skipping target login-server (requires common_base)" )
unset( HAVE_login-server CACHE )
endif()
