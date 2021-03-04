#
# setup
#
set( CHAR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )


#
# char server
#
if( BUILD_SERVERS )
message( STATUS "Creating target char-server" )
file(GLOB CHAR_HEADERS ${CHAR_SOURCE_DIR}/*.hpp)
file(GLOB CHAR_SOURCES_CXX ${CHAR_SOURCE_DIR}/*.cpp)
set(CHAR_SOURCES ${CHAR_SOURCES_C} ${CHAR_SOURCES_CXX})
#message( STATUS "CHAR_SOURCES="${CHAR_SOURCES})
set( DEPENDENCIES common )
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${COMMON_BASE_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_BASE_DEFINITIONS}" )
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} ${CHAR_HEADERS} ${CHAR_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} )
source_group( char FILES ${CHAR_HEADERS} ${CHAR_SOURCES} )
include_directories( ${INCLUDE_DIRS} )

#message( STATUS "char-server SOURCE_FILES=${SOURCE_FILES}")
add_executable( char-server ${SOURCE_FILES} )
#message( STATUS "char-server LIBRARIES=${LIBRARIES}, DEPENDENCIES=${DEPENDENCIES} DEFINITIONS=${DEFINITIONS}")
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
