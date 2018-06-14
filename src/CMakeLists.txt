
#
# setup and static libraries
#
add_subdirectory( common )
if( HAVE_common )
	option( BUILD_SERVERS "build server executables" ON )
else()
	message( STATUS "Disabled server targets (requires common)" )
endif()


#
# targets
#
add_subdirectory( login )
add_subdirectory( char )
add_subdirectory( map )
add_subdirectory( tool )

