
# macro to configure the use of local or system version of a package
# Uses:
#	HAVE_LOCAL_${name} - is local version available?
#	${name}_LOCAL_LIBRARIES - libraries of the local version
#	${name}_LOCAL_INCLUDE_DIRS - include directories of the local version
#	HAVE_SYSTEM_${name} - is system version available?
#	${name}_SYSTEM_LIBRARIES - libraries of the system version
#	${name}_SYSTEM_INCLUDE_DIRS - include directories of the system version
# Generates:
#	WITH_LOCAL_${name} - use the local version of the package (only when local is available)
#	WITH_${name} - use this package
#	${name}_LIBRARIES - libraries
#	${name}_INCLUDE_DIRS - include directories
macro( CONFIGURE_WITH_LOCAL_OR_SYSTEM name )
	unset( ${name}_LIBRARIES CACHE )
	unset( ${name}_INCLUDE_DIRS CACHE )
	if( HAVE_LOCAL_${name} )
		set( WITH_LOCAL_${name} ON
			CACHE BOOL "use local version of ${name}" )
	else()
		unset( WITH_LOCAL_${name} CACHE )
	endif()
	if( WITH_LOCAL_${name} )
		message( STATUS "Configuring for local ${name}" )
		set( ${name}_LIBRARIES ${${name}_LOCAL_LIBRARIES} )
		set( ${name}_INCLUDE_DIRS ${${name}_LOCAL_INCLUDE_DIRS} )
		message( STATUS "Configuring for local ${name} - done" )
	elseif( HAVE_SYSTEM_${name} )
		message( STATUS "Configuring for system ${name}" )
		set( ${name}_LIBRARIES ${${name}_SYSTEM_LIBRARIES} )
		set( ${name}_INCLUDE_DIRS ${${name}_SYSTEM_INCLUDE_DIRS} )
		message( STATUS "Configuring for system ${name} - done" )
	endif()
	if( WITH_LOCAL_${name} OR HAVE_SYSTEM_${name} )
		set( WITH_${name} ON
			CACHE BOOL "use ${name}" )
	else()
		unset( WITH_${name} CACHE )
	endif()
	set( ${name}_LIBRARIES ${${name}_LIBRARIES}
		CACHE PATH "${name} libraries" )
	set( ${name}_INCLUDE_DIRS ${${name}_INCLUDE_DIRS}
		CACHE PATH "${name} include directories" )
	mark_as_advanced( ${name}_LIBRARIES )
	mark_as_advanced( ${name}_INCLUDE_DIRS )
endmacro( CONFIGURE_WITH_LOCAL_OR_SYSTEM )


add_subdirectory( httplib )
add_subdirectory( json )
add_subdirectory( libconfig )
add_subdirectory( mysql )
add_subdirectory( pcre )
add_subdirectory( rapidyaml )
add_subdirectory( yaml-cpp )
add_subdirectory( zlib )

#
# system only
#
option( ENABLE_PROMETHEUS_CPP "Enable prometheus-cpp metrics backend (default=OFF)" OFF )
if( ENABLE_PROMETHEUS_CPP )
message( STATUS "Detecting system PROMETHEUS_CPP" )
set( PROMETHEUS_CPP_LOCAL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/prometheus-cpp" )

if( EXISTS "${PROMETHEUS_CPP_LOCAL_SOURCE_DIR}/CMakeLists.txt" )
	message( STATUS "Using local prometheus-cpp at ${PROMETHEUS_CPP_LOCAL_SOURCE_DIR}" )

	set( PROMETHEUS_CPP_PREV_ENABLE_PULL ${ENABLE_PULL} )
	set( PROMETHEUS_CPP_PREV_ENABLE_PUSH ${ENABLE_PUSH} )
	set( PROMETHEUS_CPP_PREV_ENABLE_TESTING ${ENABLE_TESTING} )
	set( PROMETHEUS_CPP_PREV_USE_THIRDPARTY_LIBRARIES ${USE_THIRDPARTY_LIBRARIES} )
	set( PROMETHEUS_CPP_PREV_OVERRIDE_CXX_STANDARD_FLAGS ${OVERRIDE_CXX_STANDARD_FLAGS} )

	set( ENABLE_PULL OFF CACHE BOOL "" FORCE )
	set( ENABLE_PUSH OFF CACHE BOOL "" FORCE )
	set( ENABLE_TESTING OFF CACHE BOOL "" FORCE )
	set( USE_THIRDPARTY_LIBRARIES OFF CACHE BOOL "" FORCE )
	set( OVERRIDE_CXX_STANDARD_FLAGS OFF CACHE BOOL "" FORCE )

	add_subdirectory( prometheus-cpp EXCLUDE_FROM_ALL )

	set( ENABLE_PULL ${PROMETHEUS_CPP_PREV_ENABLE_PULL} CACHE BOOL "" FORCE )
	set( ENABLE_PUSH ${PROMETHEUS_CPP_PREV_ENABLE_PUSH} CACHE BOOL "" FORCE )
	set( ENABLE_TESTING ${PROMETHEUS_CPP_PREV_ENABLE_TESTING} CACHE BOOL "" FORCE )
	set( USE_THIRDPARTY_LIBRARIES ${PROMETHEUS_CPP_PREV_USE_THIRDPARTY_LIBRARIES} CACHE BOOL "" FORCE )
	set( OVERRIDE_CXX_STANDARD_FLAGS ${PROMETHEUS_CPP_PREV_OVERRIDE_CXX_STANDARD_FLAGS} CACHE BOOL "" FORCE )
else()
	find_package( prometheus-cpp CONFIG QUIET )
endif()

if( TARGET prometheus-cpp::core )
	set( WITH_PROMETHEUS_CPP ON
		CACHE BOOL "use prometheus-cpp" )
	set( PROMETHEUS_CPP_LIBRARIES prometheus-cpp::core
		CACHE STRING "prometheus-cpp libraries" )
	if( EXISTS "${PROMETHEUS_CPP_LOCAL_SOURCE_DIR}/core/include" )
		set( PROMETHEUS_CPP_INCLUDE_DIRS "${PROMETHEUS_CPP_LOCAL_SOURCE_DIR}/core/include"
			CACHE PATH "prometheus-cpp include directories" FORCE )
	endif()
	if( TARGET prometheus-cpp::pull )
		set( PROMETHEUS_CPP_LIBRARIES ${PROMETHEUS_CPP_LIBRARIES} prometheus-cpp::pull
			CACHE STRING "prometheus-cpp libraries" FORCE )
	endif()
	if( TARGET prometheus-cpp::push )
		set( PROMETHEUS_CPP_LIBRARIES ${PROMETHEUS_CPP_LIBRARIES} prometheus-cpp::push
			CACHE STRING "prometheus-cpp libraries" FORCE )
	endif()
	message( STATUS "Detecting system PROMETHEUS_CPP - done" )
else()
	unset( WITH_PROMETHEUS_CPP CACHE )
	unset( PROMETHEUS_CPP_LIBRARIES CACHE )
	unset( PROMETHEUS_CPP_INCLUDE_DIRS CACHE )
	message( STATUS "Could NOT find PROMETHEUS_CPP (set ENABLE_PROMETHEUS_CPP=OFF to silence this)." )
	message( STATUS "Detecting system PROMETHEUS_CPP - done" )
endif()
else()
	unset( WITH_PROMETHEUS_CPP CACHE )
	unset( PROMETHEUS_CPP_LIBRARIES CACHE )
	unset( PROMETHEUS_CPP_INCLUDE_DIRS CACHE )
endif()
