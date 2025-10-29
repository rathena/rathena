add_executable(mapcache)
target_link_libraries(mapcache PRIVATE minicore)
target_sources(mapcache PRIVATE "mapcache.cpp")
if(WIN32)
	set_target_properties(mapcache PROPERTIES FOLDER "Tools")
endif()

add_executable(csv2yaml)
target_link_libraries(csv2yaml PRIVATE minicore)
target_sources(csv2yaml PRIVATE "csv2yaml.cpp")
if(WIN32)
	set_target_properties(csv2yaml PROPERTIES FOLDER "Tools")
	target_sources(csv2yaml PRIVATE
		"csv2yaml.hpp"
		"yaml.hpp"
	)
endif()

add_executable(yaml2sql)
target_link_libraries(yaml2sql PRIVATE minicore)
target_sources(yaml2sql PRIVATE "yaml2sql.cpp")
if(WIN32)
	set_target_properties(yaml2sql PROPERTIES FOLDER "Tools")
endif()

add_executable(yamlupgrade)
target_link_libraries(yamlupgrade PRIVATE minicore)
target_sources(yamlupgrade PRIVATE "yamlupgrade.cpp")
if(WIN32)
	set_target_properties(yamlupgrade PROPERTIES FOLDER "Tools")
	target_sources(yamlupgrade PRIVATE
		"yamlupgrade.hpp"
		"yaml.hpp"
	)
endif()

add_custom_target(tools DEPENDS mapcache csv2yaml yaml2sql yamlupgrade map-server-generator)
