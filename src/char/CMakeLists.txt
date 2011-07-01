
#
# setup
#
set( MT19937AR_SOURCES
#	"${MT19937AR_SOURCE_DIR}/mt19937ar.c"
	)
set( COMMON_SOURCES
	"${COMMON_SOURCE_DIR}/core.c"
	"${COMMON_SOURCE_DIR}/db.c"
	"${COMMON_SOURCE_DIR}/ers.c"
#	"${COMMON_SOURCE_DIR}/grfio.c"
	"${COMMON_SOURCE_DIR}/lock.c"
	"${COMMON_SOURCE_DIR}/malloc.c"
	"${COMMON_SOURCE_DIR}/mapindex.c"
#	"${COMMON_SOURCE_DIR}/md5calc.c"
#	"${COMMON_SOURCE_DIR}/nullpo.c"
	"${COMMON_SOURCE_DIR}/plugins.c"
#	"${COMMON_SOURCE_DIR}/random.c"
	"${COMMON_SOURCE_DIR}/showmsg.c"
	"${COMMON_SOURCE_DIR}/socket.c"
	"${COMMON_SOURCE_DIR}/strlib.c"
	"${COMMON_SOURCE_DIR}/timer.c"
	"${COMMON_SOURCE_DIR}/utils.c"
	)
set( CHAR_SOURCES
	"char.c"
	"int_guild.c"
	"int_homun.c"
	"int_party.c"
	"int_pet.c"
	"int_status.c"
	"int_storage.c"
	"inter.c"
	)


#
# char txt
#
if( USE_ZLIB )
message ( STATUS "Creating target char-server" )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${ZLIB_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MT19937AR_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${COMMON_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} TXT_ONLY )
if( PACKETVER )
	message( STATUS "Using PACKETVER=${PACKETVER}" )
	list( APPEND DEFINITIONS PACKETVER=${PACKETVER} )
endif()
if( WIN32 )
	list( APPEND LIBRARIES "oldnames.lib" "ws2_32.lib" )
	list( APPEND INCLUDE_DIRS ${MSINTTYPES_INCLUDE_DIRS} )
endif()
set( SOURCE_FILES ${MT19937AR_SOURCES} ${COMMON_SOURCES} ${CHAR_SOURCES} )
source_group( 3rdparty FILES ${MT19937AR_SOURCES} )
source_group( common FILES ${COMMON_SOURCES} )
source_group( char FILES ${CHAR_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( char-server ${SOURCE_FILES} )
target_link_libraries( char-server ${LIBRARIES} )
set_target_properties( char-server PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( INSTALL_RUN_DATA )
	install( TARGETS char-server
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		COMPONENT "char-server" )
endif()
message ( STATUS "Creating target char-server - done" )
else()
message ( STATUS "Skipping target char-server (requires ZLIB)" )
endif()
