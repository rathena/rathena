
# macro to configure the use of local or system version of a package
# Uses:
#	HAVE_LOCAL_${name} - is local version available?
#	${name}_LOCAL_DEPENDENCIES - dependencies of the local version
#	${name}_LOCAL_LIBRARIES - libraries of the local version
#	${name}_LOCAL_INCLUDE_DIRS - include directories of the local version
#	${name}_LOCAL_DEFINITIONS - definitions of the local version
#	HAVE_SYSTEM_${name} - is system version available?
#	${name}_SYSTEM_DEPENDENCIES - dependencies of the system version
#	${name}_SYSTEM_LIBRARIES - libraries of the system version
#	${name}_SYSTEM_INCLUDE_DIRS - include directories of the system version
#	${name}_SYSTEM_DEFINITIONS - definitions of the system version
# Generates:
#	WITH_LOCAL_${name} - use the local version of the package (only when local is available)
#	WITH_${name} - use this package
#	${name}_DEPENDENCIES - dependencies
#	${name}_LIBRARIES - libraries
#	${name}_INCLUDE_DIRS - include directories
#	${name}_DEFINITIONS - definitions
macro( CONFIGURE_WITH_LOCAL_OR_SYSTEM name )
	if( HAVE_LOCAL_${name} )
		set( WITH_LOCAL_${name} ON
			CACHE BOOL "use local version of ${name}" )
	else()
		unset( WITH_LOCAL_${name} CACHE )
	endif()
	if( WITH_LOCAL_${name} )
		set( _type "LOCAL" )
	elseif( HAVE_SYSTEM_${name} )
		set( _type "SYSTEM" )
		message( STATUS "Configuring for system ${name} - done" )
	endif()
	if( WITH_LOCAL_${name} OR HAVE_SYSTEM_${name} )
		set( WITH_${name} ON
			CACHE BOOL "use ${name}" )
	else()
		unset( WITH_${name} CACHE )
	endif()
	message( STATUS "Configuring for ${_type} ${name}" )
	unset( ${name}_DEPENDENCIES CACHE )
	unset( ${name}_LIBRARIES CACHE )
	unset( ${name}_INCLUDE_DIRS CACHE )
	unset( ${name}_DEFINITIONS CACHE )
	set( ${name}_DEPENDENCIES ${${name}_${_type}_DEPENDENCIES}
		CACHE PATH "${name} dependencies" )
	set( ${name}_LIBRARIES ${${name}_${_type}_LIBRARIES}
		CACHE PATH "${name} libraries" )
	set( ${name}_INCLUDE_DIRS ${${name}_${_type}_INCLUDE_DIRS}
		CACHE PATH "${name} include directories" )
	set( ${name}_DEFINITIONS ${${name}_${_type}_DEFINITIONS}
		CACHE PATH "${name} definitions" )
	mark_as_advanced( ${name}_DEPENDENCIES )
	mark_as_advanced( ${name}_LIBRARIES )
	mark_as_advanced( ${name}_INCLUDE_DIRS )
	mark_as_advanced( ${name}_DEFINITIONS )
	message( STATUS "Configuring for ${_type} ${name} - done" )
endmacro( CONFIGURE_WITH_LOCAL_OR_SYSTEM )


add_subdirectory( libconfig )
add_subdirectory( msinttypes )
add_subdirectory( mt19937ar )
add_subdirectory( mysql )
add_subdirectory( pcre )
add_subdirectory( zlib )
