
add_library(libconfig STATIC)

target_sources(libconfig PRIVATE
	"grammar.c"
	"libconfig.c"
	"scanctx.c"
	"scanner.c"
	"strbuf.c"
)

target_include_directories(libconfig PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_definitions(libconfig PUBLIC "-DLIBCONFIG_STATIC")

if(WIN32)
	target_sources(libconfig PRIVATE
		"grammar.h"
		"libconfig.h"
		"parsectx.h"
		"scanctx.h"
		"scanner.h"
		"strbuf.h"
		"wincompat.h"
	)
	set_target_properties(libconfig PROPERTIES FOLDER "3rdparty")
endif()
