#
# setup
#
set( LOGIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )

# login server
#
if( BUILD_SERVERS )
message( STATUS "Creating target login-server" )
file(GLOB LOGIN_HEADERS ${LOGIN_SOURCE_DIR}/*.hpp)
file(GLOB LOGIN_SOURCES_CXX ${LOGIN_SOURCE_DIR}/*.cpp)
set(LOGIN_SOURCES ${LOGIN_SOURCES_C} ${LOGIN_SOURCES_CXX})
#message( STATUS "LOGIN_SOURCES="${LOGIN_SOURCES})
set( DEPENDENCIES common )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${COMMON_BASE_INCLUDE_DIRS} ${RA_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_BASE_DEFINITIONS} -DWITH_SQL" )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} ${LOGIN_HEADERS} ${LOGIN_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} )
source_group( login FILES ${LOGIN_HEADERS} ${LOGIN_SOURCES} )
include_directories( ${INCLUDE_DIRS} )

add_executable( login-server ${SOURCE_FILES} )
add_dependencies( login-server ${DEPENDENCIES} )
target_link_libraries( login-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( login-server PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_loginserver DESCRIPTION "login-server" DISPLAY_NAME "login-server" GROUP Runtime )
	install( TARGETS login-server
		DESTINATION "."
		COMPONENT Runtime_loginserver )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} login-server  CACHE INTERNAL "" )
message( STATUS "Creating target login-server - done" )
endif( BUILD_SERVERS )
