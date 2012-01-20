
#
# setup and static libraries
#
add_subdirectory( common )
if( HAVE_common_sql )
	option( BUILD_SQL_SERVERS "build sql server executables" ON )
else()
	message( STATUS "Disabled sql server targets (requires common_sql)" )
endif()


#
# targets
#
add_subdirectory( login )
add_subdirectory( char )
add_subdirectory( map )
add_subdirectory( tool )
add_subdirectory( plugins )
