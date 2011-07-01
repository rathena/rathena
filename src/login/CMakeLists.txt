
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
#	"${COMMON_SOURCE_DIR}/grfio.c"
	"${COMMON_SOURCE_DIR}/lock.c"
	"${COMMON_SOURCE_DIR}/malloc.c"
#	"${COMMON_SOURCE_DIR}/mapindex.c"
	"${COMMON_SOURCE_DIR}/md5calc.c"
#	"${COMMON_SOURCE_DIR}/nullpo.c"
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
set( LOGIN_SOURCES
	"login.c"
	)
set( LOGIN_TXT_SOURCES
	"account_txt.c"
	"ipban_txt.c"
	"loginlog_txt.c"
	)
set( LOGIN_SQL_SOURCES
	"account_sql.c"
	"ipban_sql.c"
	"loginlog_sql.c"
	)


#
# login txt
#
if( USE_ZLIB )
message ( STATUS "Creating target login-server" )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${ZLIB_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MT19937AR_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${COMMON_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} WITH_TXT )
if( PACKETVER )
	message( STATUS "Using PACKETVER=${PACKETVER}" )
	list( APPEND DEFINITIONS PACKETVER=${PACKETVER} )
endif()
if( WIN32 )
	list( APPEND LIBRARIES "oldnames.lib" "ws2_32.lib" )
	list( APPEND INCLUDE_DIRS ${MSINTTYPES_INCLUDE_DIRS} )
endif()
set( SOURCE_FILES ${MT19937AR_SOURCES} ${COMMON_SOURCES} ${LOGIN_SOURCES} ${LOGIN_TXT_SOURCES} )
source_group( 3rdparty FILES ${MT19937AR_SOURCES} )
source_group( common FILES ${COMMON_SOURCES} )
source_group( login FILES ${LOGIN_SOURCES} ${LOGIN_TXT_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( login-server ${SOURCE_FILES} )
target_link_libraries( login-server ${LIBRARIES} )
set_target_properties( login-server PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( INSTALL_RUN_DATA )
	install( TARGETS login-server
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		COMPONENT "login-server" )
endif()
message ( STATUS "Creating target login-server - done" )
else()
message ( STATUS "Skipping target login-server (requires ZLIB)" )
endif()


#
# login sql
#
if( USE_ZLIB AND USE_MYSQL )
message ( STATUS "Creating target login-server_sql" )
set( LIBRARIES ${GLOBAL_LIBRARIES} ${ZLIB_LIBRARIES} ${MYSQL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${MT19937AR_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} ${COMMON_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} WITH_SQL )
if( PACKETVER )
	message( STATUS "Using PACKETVER=${PACKETVER}" )
	list( APPEND DEFINITIONS PACKETVER=${PACKETVER} )
endif()
if( WIN32 )
	list( APPEND LIBRARIES "oldnames.lib" "ws2_32.lib" )
	list( APPEND INCLUDE_DIRS ${MSINTTYPES_INCLUDE_DIRS} )
endif()
set( SOURCE_FILES ${MT19937AR_SOURCES} ${COMMON_SOURCES} ${COMMON_SQL_SOURCES} ${LOGIN_SOURCES} ${LOGIN_SQL_SOURCES} )
source_group( 3rdparty FILES ${MT19937AR_SOURCES} )
source_group( common FILES ${COMMON_SOURCES} ${COMMON_SQL_SOURCES} )
source_group( login FILES ${LOGIN_SOURCES} ${LOGIN_SQL_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_executable( login-server_sql ${SOURCE_FILES} )
target_link_libraries( login-server_sql ${LIBRARIES} )
set_target_properties( login-server_sql PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}" )
if( INSTALL_RUN_DATA )
	install( TARGETS login-server_sql
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		COMPONENT "login-server_sql" )
endif()
message ( STATUS "Creating target login-server_sql - done" )
else()
message ( STATUS "Skipping target login-server_sql (requires ZLIB and MYSQL)" )
endif()
