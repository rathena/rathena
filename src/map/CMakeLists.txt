#
# setup
#
set( MAP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )


#
# map server
#
if( BUILD_SERVERS )
message( STATUS "Creating target map-server" )
file(GLOB MAP_HEADERS ${MAP_SOURCE_DIR}/*.hpp)
#file(GLOB MAP_SOURCES ${MAP_SOURCE_DIR}/*.cpp)
#message( STATUS "${MAP_SOURCES} ")
set (MAP_SOURCES 
achievement.cpp
atcommand.cpp
battle.cpp
battleground.cpp
buyingstore.cpp
cashshop.cpp
channel.cpp
chat.cpp
chrif.cpp
clan.cpp
clif.cpp
date.cpp
duel.cpp
elemental.cpp
guild.cpp
homunculus.cpp
instance.cpp
intif.cpp
itemdb.cpp
log.cpp
mail.cpp
map.cpp
mapreg.cpp
mercenary.cpp
mob.cpp
npc.cpp
npc_chat.cpp
party.cpp
path.cpp
pc.cpp
pc_groups.cpp
pet.cpp
quest.cpp
script.cpp
searchstore.cpp
skill.cpp
status.cpp
storage.cpp
trade.cpp
unit.cpp
vending.cpp)
file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/tests/map/*.cpp)
set( DEPENDENCIES common yaml-cpp)
set( LIBRARIES ${GLOBAL_LIBRARIES})
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${COMMON_BASE_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_BASE_DEFINITIONS}" )
if( WITH_PCRE )
	message( STATUS "Enabled PCRE code" )
	set( LIBRARIES ${LIBRARIES} ${PCRE_LIBRARIES} )
	set( INCLUDE_DIRS ${INCLUDE_DIRS} ${PCRE_INCLUDE_DIRS} )
	set( DEFINITIONS "${DEFINITIONS} -DPCRE_SUPPORT" )
else()
	message( STATUS "Disabled PCRE code" )
endif()
if ( TESTING )
	set ( DEFINITIONS "${DEFINITIONS} -DTESTING" )
endif()
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} ${MAP_HEADERS} ${MAP_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} )
source_group( map FILES ${MAP_HEADERS} ${MAP_SOURCES} )
include_directories( ${INCLUDE_DIRS} )

#message( STATUS "map-server SOURCE_FILES=${SOURCE_FILES}")
if ( TESTING )
	add_library( map-server ${SOURCE_FILES})
else()
	add_executable( map-server ${SOURCE_FILES} ${TEST_SOURCES} )
endif()
#message( STATUS "map-server LIBRARIES=${LIBRARIES}, DEPENDENCIES=${DEPENDENCIES} DEFINITIONS=${DEFINITIONS}")
add_dependencies( map-server ${DEPENDENCIES} )
target_link_libraries( map-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( map-server PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_mapserver DESCRIPTION "map-server" DISPLAY_NAME "map-server" GROUP Runtime )
	install( TARGETS map-server
		DESTINATION "."
		COMPONENT Runtime_mapserver )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} map-server  CACHE INTERNAL "" )
message( STATUS "Creating target map-server - done" )
endif( BUILD_SERVERS )
