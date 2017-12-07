
#
# basic_test
#

option( BUILD_TESTS "build basic_test executable" ON )

if( BUILD_TESTS )

	set( TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )
	message( STATUS "Creating target basic_test" )
	file(GLOB TESTS_HEADERS ${TESTS_SOURCE_DIR}/*.hpp)
	file(GLOB TESTS_SOURCES ${TESTS_SOURCE_DIR}/*.cpp)
	
	
	set( DEPENDENCIES common)
	set( LIBRARIES ${GLOBAL_LIBRARIES})
	set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${COMMON_BASE_INCLUDE_DIRS} )
	set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_BASE_DEFINITIONS} -DCMAKE" )
	set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} ${TESTS_HEADERS} ${TESTS_SOURCES} )
	source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} )
	source_group( test FILES ${TESTS_HEADERS} ${TESTS_SOURCES} )
	include_directories( ${INCLUDE_DIRS} )

	#message( STATUS "basic_test SOURCE_FILES=${SOURCE_FILES}")
	add_executable( basic_test ${SOURCE_FILES} )
	#message( STATUS "basic_test LIBRARIES=${LIBRARIES}, DEPENDENCIES=${DEPENDENCIES} DEFINITIONS=${DEFINITIONS}")
	add_dependencies( basic_test ${DEPENDENCIES} )
	target_link_libraries( basic_test ${LIBRARIES} ${DEPENDENCIES} )
	set_target_properties( basic_test PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )

	if( INSTALL_COMPONENT_RUNTIME )
		cpack_add_component( Runtime_tests DESCRIPTION "basic integration test" DISPLAY_NAME "basic_test" GROUP Runtime )
		install( TARGETS basic_test
			DESTINATION "."
			COMPONENT Runtime_tests )
	endif( INSTALL_COMPONENT_RUNTIME )
	set( TARGET_LIST ${TARGET_LIST} basic_test  CACHE INTERNAL "" )
	message( STATUS "Creating target basic_test - done" )

endif( BUILD_TESTS )
