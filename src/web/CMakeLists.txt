add_executable(web-server)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_sources(web-server PRIVATE
	"auth.cpp"
	"charconfig_controller.cpp"
	"emblem_controller.cpp"
	"merchantstore_controller.cpp"
	"partybooking_controller.cpp"
	"sqllock.cpp"
	"userconfig_controller.cpp"
	"web.cpp"
	"webutils.cpp"
)

if(WIN32)
	target_sources(web-server PRIVATE
		"auth.hpp"
		"charconfig_controller.hpp"
		"emblem_controller.hpp"
		"http.hpp"
		"merchantstore_controller.hpp"
		"partybooking_controller.hpp"
		"sqllock.hpp"
		"userconfig_controller.hpp"
		"webcnslif.hpp"
		"web.hpp"
		"webutils.hpp"
	)

	set_target_properties(web-server PROPERTIES FOLDER "Servers")
endif()

target_link_libraries(web-server PUBLIC
	common
	httplib
	Threads::Threads
	json
)
