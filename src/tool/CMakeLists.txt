
# Define the tools interface library, all tools will inherit properties

add_library(tools INTERFACE)
target_include_directories(tools INTERFACE 
	${YAML_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
)

target_sources(tools INTERFACE
	"${COMMON_SOURCE_DIR}/utils.cpp"
	"${COMMON_SOURCE_DIR}/des.cpp"
	"${COMMON_SOURCE_DIR}/grfio.cpp"
	"${COMMON_SOURCE_DIR}/nullpo.cpp"
	"${COMMON_SOURCE_DIR}/database.cpp"
)

target_compile_definitions(tools INTERFACE
	"MINICORE"
)

target_link_libraries(tools INTERFACE
	${GLOBAL_LIBRARIES}
	${ZLIB_LIBRARIES}
	yaml-cpp
	ryml
	minicore
)

# mapcache
message( STATUS "Creating target mapcache" )
add_executable(mapcache)
target_link_libraries(mapcache PRIVATE tools)
target_sources(mapcache PRIVATE "mapcache.cpp")


# csv2yaml
message( STATUS "Creating target csv2yaml" )
add_executable(csv2yaml)
target_link_libraries(csv2yaml PRIVATE tools)
target_sources(csv2yaml PRIVATE "csv2yaml.cpp")

# yaml2sql
message( STATUS "Creating target yaml2sql" )
add_executable(yaml2sql)
target_link_libraries(yaml2sql PRIVATE tools)
target_sources(yaml2sql PRIVATE "yaml2sql.cpp")

# yamlupgrade
message( STATUS "Creating target yamlupgrade" )
add_executable(yamlupgrade)
target_link_libraries(yamlupgrade PRIVATE tools)
target_sources(yamlupgrade PRIVATE "yamlupgrade.cpp")

set( TARGET_LIST ${TARGET_LIST} mapcache csv2yaml yaml2sql yamlupgrade  CACHE INTERNAL "" )

if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_mapcache DESCRIPTION "mapcache generator" DISPLAY_NAME "mapcache" GROUP Runtime )
	install( TARGETS mapcache
		DESTINATION "."
		COMPONENT Runtime_mapcache
	)
	cpack_add_component( Runtime_csv2yaml DESCRIPTION "yaml converter" DISPLAY_NAME "csv2yaml" GROUP Runtime )
	install( TARGETS csv2yaml
		DESTINATION "."
		COMPONENT Runtime_csv2yaml
	)
	cpack_add_component( Runtime_yaml2sql DESCRIPTION "sql converter" DISPLAY_NAME "yaml2sql" GROUP Runtime )
	install( TARGETS yaml2sql
		DESTINATION "."
		COMPONENT Runtime_yaml2sql
	)
	cpack_add_component( Runtime_yamlupgrade DESCRIPTION "yaml upgrader" DISPLAY_NAME "yamlupgrade" GROUP Runtime )
	install( TARGETS yamlupgrade
		DESTINATION "."
		COMPONENT Runtime_yamlupgrade
	)
	install (TARGETS )
endif( INSTALL_COMPONENT_RUNTIME )
