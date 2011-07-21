
#
# setup
#
set( CONVERTER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )
if( WITH_MYSQL )
	option( BUILD_CONVERTERS "build login-converter and char-converter" OFF )
endif()


#
# targets
#
add_subdirectory( login )
add_subdirectory( char )
