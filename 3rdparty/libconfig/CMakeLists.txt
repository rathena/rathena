
find_path( LIBCONFIG_INCLUDE_DIRS "libconfig.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}"
	NO_DEFAULT_PATH )
find_path( LIBCONFIG_SOURCE_DIR "libconfig.c"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}"
	NO_DEFAULT_PATH )
mark_as_advanced( LIBCONFIG_INCLUDE_DIRS )
mark_as_advanced( LIBCONFIG_SOURCE_DIR )

set( LIBCONFIG_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/grammar.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/libconfig.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/parsectx.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/scanctx.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/scanner.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/strbuf.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/wincompat.h"
	CACHE INTERNAL "libconfig headers" )
set( LIBCONFIG_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/grammar.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/libconfig.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/scanctx.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/scanner.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/strbuf.c"
	CACHE INTERNAL "libconfig sources" )
set( LIBCONFIG_DEFINITIONS
	"-DLIBCONFIG_STATIC"
	CACHE INTERNAL "libconfig definitions" )

include(CheckSymbolExists)
if(APPLE)
    check_symbol_exists(uselocale "xlocale.h" HAVE_USELOCALE)
    check_symbol_exists(newlocale "xlocale.h" HAVE_NEWLOCALE)
    check_symbol_exists(freelocale "xlocale.h" HAVE_FREELOCALE)
else()
    check_symbol_exists(uselocale "locale.h" HAVE_USELOCALE)
    check_symbol_exists(newlocale "locale.h" HAVE_NEWLOCALE)
    check_symbol_exists(freelocale "locale.h" HAVE_FREELOCALE)
endif()

if(HAVE_USELOCALE)
    set( LIBCONFIG_DEFINITIONS "${LIBCONFIG_DEFINITIONS} -DHAVE_USELOCALE" CACHE INTERNAL "libconfig definitions" )
endif()

if(HAVE_NEWLOCALE)
    set( LIBCONFIG_DEFINITIONS "${LIBCONFIG_DEFINITIONS} -DHAVE_NEWLOCALE" CACHE INTERNAL "libconfig definitions" )
endif()

if(HAVE_FREELOCALE)
    set( LIBCONFIG_DEFINITIONS "${LIBCONFIG_DEFINITIONS} -DHAVE_FREELOCALE" CACHE INTERNAL "libconfig definitions" )
endif()
