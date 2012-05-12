
#
# setup
#
get_property( CAN_BUILD_SHARED_LIBS  GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS )
if( NOT CAN_BUILD_SHARED_LIBS )
	return()
endif()

#
# console
#
option( BUILD_PLUGIN_console "build console plugin" OFF )
if( BUILD_PLUGIN_console )
message( STATUS "Creating target console" )
set( CONSOLE_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/console.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/console.def"
	)
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} )
set( SOURCE_FILES ${CONSOLE_SOURCES} )
source_group( console FILES ${CONSOLE_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_library( console SHARED ${SOURCE_FILES} )
target_link_libraries( console ${LIBRARIES} )
set_target_properties( console PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
set_target_properties( console PROPERTIES PREFIX "" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_console DESCRIPTION "console plugin" DISPLAY_NAME "console" GROUP Runtime )
	install( TARGETS console
		DESTINATION "plugins"
		COMPONENT Runtime_console )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} console  CACHE INTERNAL "" )
message( STATUS "Creating target console - done" )
endif( BUILD_PLUGIN_console )


#
# dbghelpplug
#
if( WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4 )
	find_path( HAVE_DBGHELP_H dbghelp.h )
	mark_as_advanced( HAVE_DBGHELP_H )
	if( HAVE_DBGHELP_H )
		option( BUILD_PLUGIN_dbghelpplug "build dbghelpplug plugin" OFF )
	endif()
endif()
if( NOT DEFINED BUILD_PLUGIN_dbghelpplug )
	message( STATUS "Disabled dbghelpplug plugin target (requires WIN32 and CMAKE_SIZEOF_VOID_P=4 and HAVE_DBGHELP_H)" )
endif()
if( BUILD_PLUGIN_dbghelpplug )
message( STATUS "Creating target dbghelpplug" )
set( DBGHELPPLUG_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/dbghelpplug.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/dbghelpplug.rc"
	)
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} )
set( SOURCE_FILES ${DBGHELPPLUG_SOURCES} )
source_group( dbghelpplug FILES ${DBGHELPPLUG_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_library( dbghelpplug SHARED ${SOURCE_FILES} )
target_link_libraries( dbghelpplug ${LIBRARIES} )
set_target_properties( dbghelpplug PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
set_target_properties( dbghelpplug PROPERTIES PREFIX "" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_dbghelpplug DESCRIPTION "dbghelpplug plugin" DISPLAY_NAME "dbghelpplug" GROUP Runtime )
	install( TARGETS dbghelpplug
		DESTINATION "plugins"
		COMPONENT Runtime_dbghelpplug )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} dbghelpplug  CACHE INTERNAL "" )
message( STATUS "Creating target dbghelpplug - done" )
endif( BUILD_PLUGIN_dbghelpplug )


#
# pid
#
if( WIN32 OR HAVE_GETPID )
	option( BUILD_PLUGIN_pid "build pid plugin" OFF )
else()
	message( STATUS "Disabled pid plugin target (requires WIN32 or HAVE_GETPID)" )
endif()
if( BUILD_PLUGIN_pid )
message( STATUS "Creating target pid" )
set( PID_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/pid.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/pid.def"
	)
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} )
set( SOURCE_FILES ${PID_SOURCES} )
source_group( pid FILES ${PID_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_library( pid SHARED ${SOURCE_FILES} )
target_link_libraries( pid ${LIBRARIES} )
set_target_properties( pid PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
set_target_properties( pid PROPERTIES PREFIX "" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_pid DESCRIPTION "pid plugin" DISPLAY_NAME "pid" GROUP Runtime )
	install( TARGETS pid
		DESTINATION "plugins"
		COMPONENT Runtime_pid )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} pid  CACHE INTERNAL "" )
message( STATUS "Creating target pid - done" )
endif( BUILD_PLUGIN_pid )


#
# sample
#
option( BUILD_PLUGIN_sample "build sample plugin" OFF )
if( BUILD_PLUGIN_sample )
message( STATUS "Creating target sample" )
set( SAMPLE_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/sample.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/sample.def"
	)
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} )
set( SOURCE_FILES ${SAMPLE_SOURCES} )
source_group( sample FILES ${SAMPLE_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_library( sample SHARED ${SOURCE_FILES} )
target_link_libraries( sample ${LIBRARIES} )
set_target_properties( sample PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
set_target_properties( sample PROPERTIES PREFIX "" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_sample DESCRIPTION "sample plugin" DISPLAY_NAME "sample" GROUP Runtime )
	install( TARGETS sample
		DESTINATION "plugins"
		COMPONENT Runtime_sample )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} sample  CACHE INTERNAL "" )
message( STATUS "Creating target sample - done" )
endif( BUILD_PLUGIN_sample )


#
# sig
#
option( BUILD_PLUGIN_sig "build sig plugin" OFF )
if( BUILD_PLUGIN_sig )
message( STATUS "Creating target sig" )
set( SIG_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/sig.c"
	)
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} -DNO_MEMMGR" )
set( SOURCE_FILES ${SIG_SOURCES} )
source_group( sig FILES ${SIG_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_library( sig SHARED ${SOURCE_FILES} )
target_link_libraries( sig ${LIBRARIES} )
set_target_properties( sig PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
set_target_properties( sig PROPERTIES PREFIX "" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_sig DESCRIPTION "sig plugin" DISPLAY_NAME "sig" GROUP Runtime )
	install( TARGETS sig
		DESTINATION "plugins"
		COMPONENT Runtime_sig )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} sig  CACHE INTERNAL "" )
message( STATUS "Creating target sig - done" )
endif( BUILD_PLUGIN_sig )
