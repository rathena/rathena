
if( WIN32 )
find_path( MSINTTYPES_INCLUDE_DIRS "inttypes.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include"
	NO_DEFAULT_PATH )
mark_as_advanced( MSINTTYPES_INCLUDE_DIRS )
endif()
