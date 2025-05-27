add_executable(login-server)

target_sources(login-server PRIVATE
	"account.cpp"
	"ipban.cpp"
	"login.cpp"
	"loginchrif.cpp"
	"loginclif.cpp"
	"logincnslif.cpp"
	"loginlog.cpp"
)

if(WIN32)
	target_sources(login-server PRIVATE
		"account.hpp"
		"ipban.hpp"
		"loginchrif.hpp"
		"loginclif.hpp"
		"logincnslif.hpp"
		"login.hpp"
		"loginlog.hpp"
	)

	set_target_properties(login-server PROPERTIES FOLDER "Servers")
endif()

target_link_libraries(login-server PUBLIC
	common
)
