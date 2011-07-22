
#
# setup
#
set( CONVERTER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )
if( WITH_MYSQL )
	option( BUILD_CONVERTERS "build converter executables" OFF )
else()
	message( STATUS "Disabled converter targets  (requires MYSQL)" )
endif()


#
# targets
#
add_subdirectory( login )
add_subdirectory( char )
