
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
