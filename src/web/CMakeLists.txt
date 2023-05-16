#
# setup
#
set( WEB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )


#
# web server
#
if ( NOT ENABLE_WEB_SERVER )
	return()
endif ( NOT ENABLE_WEB_SERVER )

if( BUILD_SERVERS AND ENABLE_WEB_SERVER )
message( STATUS "Creating target web-server" )
file(GLOB WEB_HEADERS ${WEB_SOURCE_DIR}/*.hpp)
file(GLOB WEB_SOURCES ${WEB_SOURCE_DIR}/*.cpp)
set( DEPENDENCIES common yaml-cpp httplib)
set( LIBRARIES ${GLOBAL_LIBRARIES})
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${COMMON_BASE_INCLUDE_DIRS} ${HTTPLIB_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS} ${RA_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_BASE_DEFINITIONS}" )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} ${WEB_HEADERS} ${WEB_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} )
source_group( web FILES ${WEB_HEADERS} ${WEB_SOURCES} )
include_directories( ${INCLUDE_DIRS} )

#message( STATUS "web-server SOURCE_FILES=${SOURCE_FILES}")
add_executable( web-server ${SOURCE_FILES} )
#message( STATUS "web-server LIBRARIES=${LIBRARIES}, DEPENDENCIES=${DEPENDENCIES} DEFINITIONS=${DEFINITIONS}")
add_dependencies( web-server ${DEPENDENCIES} )
target_link_libraries( web-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( web-server PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_webserver DESCRIPTION "web-server" DISPLAY_NAME "web-server" GROUP Runtime )
	install( TARGETS web-server
		DESTINATION "."
		COMPONENT Runtime_webserver )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} web-server  CACHE INTERNAL "" )
message( STATUS "Creating target web-server - done" )
endif( BUILD_SERVERS AND ENABLE_WEB_SERVER)
