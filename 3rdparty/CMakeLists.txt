
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


add_subdirectory( libconfig )
add_subdirectory( mysql )
add_subdirectory( pcre )
add_subdirectory( rapidyaml )
add_subdirectory( yaml-cpp )
add_subdirectory( zlib )
