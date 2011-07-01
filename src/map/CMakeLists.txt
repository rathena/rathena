
#
# setup
#
set( MT19937AR_SOURCES
	"${MT19937AR_SOURCE_DIR}/mt19937ar.c"
	)
set( COMMON_SOURCES
	"${COMMON_SOURCE_DIR}/core.c"
	"${COMMON_SOURCE_DIR}/db.c"
	"${COMMON_SOURCE_DIR}/ers.c"
	"${COMMON_SOURCE_DIR}/grfio.c"
	"${COMMON_SOURCE_DIR}/lock.c"
	"${COMMON_SOURCE_DIR}/malloc.c"
	"${COMMON_SOURCE_DIR}/mapindex.c"
	"${COMMON_SOURCE_DIR}/md5calc.c"
	"${COMMON_SOURCE_DIR}/nullpo.c"
	"${COMMON_SOURCE_DIR}/plugins.c"
	"${COMMON_SOURCE_DIR}/random.c"
	"${COMMON_SOURCE_DIR}/showmsg.c"
	"${COMMON_SOURCE_DIR}/socket.c"
	"${COMMON_SOURCE_DIR}/strlib.c"
	"${COMMON_SOURCE_DIR}/timer.c"
	"${COMMON_SOURCE_DIR}/utils.c"
	)
set( COMMON_SQL_SOURCES
	"${COMMON_SOURCE_DIR}/sql.c"
	)
set( MAP_SOURCES
	"atcommand.c"
	"battle.c"
	"battleground.c"
	"buyingstore.c"
	"chat.c"
	"chrif.c"
	"clif.c"
	"date.c"
	"duel.c"
	"guild.c"
	"homunculus.c"
	"instance.c"
	"intif.c"
	"itemdb.c"
	"log.c"
	"mail.c"
	"map.c"
	"mercenary.c"
	"mob.c"
	"npc.c"
	"npc_chat.c"
	"party.c"
	"path.c"
	"pc.c"
	"pet.c"
	"quest.c"
	"script.c"
	"searchstore.c"
	"skill.c"
	"status.c"
	"storage.c"
	"trade.c"
	"unit.c"
	"vending.c"
	)
set( MAP_TXT_SOURCES
	"mapreg_txt.c"
	)
set( MAP_SQL_SOURCES
	"mapreg_sql.c"
	)


#
# map txt
#
if( USE_ZLIB )
message ( STATUS "Creating target map-server" )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${ZLIB_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MT19937AR_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} TXT_ONLY )
if( USE_PCRE )
	message( STATUS "Using PCRE" )
	list( APPEND LIBRARIES ${PCRE_LIBRARIES} )
	list( APPEND INCLUDE_DIRS ${PCRE_INCLUDE_DIRS} )
endif()
if( PACKETVER )
	message( STATUS "Using PACKETVER=${PACKETVER}" )
	list( APPEND DEFINITIONS PACKETVER=${PACKETVER} )
endif()
if( WIN32 )
	list( APPEND LIBRARIES "oldnames.lib" "ws2_32.lib" )
	list( APPEND INCLUDE_DIRS ${MSINTTYPES_INCLUDE_DIRS} )
endif()
set( SOURCE_FILES ${MT19937AR_SOURCES} ${COMMON_SOURCES} ${MAP_SOURCES} ${MAP_TXT_SOURCES} )
source_group( 3rdparty FILES ${MT19937AR_SOURCES} )
source_group( common FILES ${COMMON_SOURCES} )
source_group( map FILES ${MAP_SOURCES} ${MAP_TXT_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( map-server ${SOURCE_FILES} )
target_link_libraries( map-server ${LIBRARIES} )
set_target_properties( map-server PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( INSTALL_RUN_DATA )
	install( TARGETS map-server
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		COMPONENT "map-server" )
endif()
message ( STATUS "Creating target map-server - done" )
else()
message ( STATUS "Skipping target map-server (requires ZLIB; optional PCRE)" )
endif()


#
# map sql
#
if( USE_ZLIB AND USE_MYSQL )
message ( STATUS "Creating target map-server_sql" )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${ZLIB_LIBRARIES} ${MYSQL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MT19937AR_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} ${COMMON_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} )
if( USE_PCRE )
	message( STATUS "Using PCRE" )
	list( APPEND LIBRARIES ${PCRE_LIBRARIES} )
	list( APPEND INCLUDE_DIRS ${PCRE_INCLUDE_DIRS} )
endif()
if( PACKETVER )
	message( STATUS "Using PACKETVER=${PACKETVER}" )
	list( APPEND DEFINITIONS PACKETVER=${PACKETVER} )
endif()
if( WIN32 )
	list( APPEND LIBRARIES "oldnames.lib" "ws2_32.lib" )
	list( APPEND INCLUDE_DIRS ${MSINTTYPES_INCLUDE_DIRS} )
endif()
set( SOURCE_FILES ${MT19937AR_SOURCES} ${COMMON_SOURCES} ${COMMON_SQL_SOURCES} ${MAP_SOURCES} ${MAP_SQL_SOURCES} )
source_group( 3rdparty FILES ${MT19937AR_SOURCES} )
source_group( common FILES ${COMMON_SOURCES} ${COMMON_SQL_SOURCES} )
source_group( map FILES ${MAP_SOURCES} ${MAP_SQL_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( map-server_sql ${SOURCE_FILES} )
target_link_libraries( map-server_sql ${LIBRARIES} )
set_target_properties( map-server_sql PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( INSTALL_RUN_DATA )
	install( TARGETS map-server_sql
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		COMPONENT "map-server_sql" )
endif()
message ( STATUS "Creating target map-server_sql - done" )
else()
message ( STATUS "Skipping target map-server_sql (requires ZLIB and MYSQL; optional PCRE)" )
endif()
