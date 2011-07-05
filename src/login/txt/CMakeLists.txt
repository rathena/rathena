
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
