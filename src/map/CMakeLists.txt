#
# setup
#
set( MAP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )


#
# map server
#
if( BUILD_SERVERS )
message( STATUS "Creating target map-server" )
file(GLOB MAP_HEADERS ${MAP_SOURCE_DIR}/*.hpp)
file(GLOB MAP_SOURCES ${MAP_SOURCE_DIR}/*.cpp)
set( DEPENDENCIES common ryml)
set( LIBRARIES ${GLOBAL_LIBRARIES} ${MYSQL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} ${COMMON_BASE_INCLUDE_DIRS} ${RA_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} )
set( DEFINITIONS "${GLOBAL_DEFINITIONS} ${COMMON_BASE_DEFINITIONS}" )
if( WITH_PCRE )
	message( STATUS "Enabled PCRE code" )
	set( LIBRARIES ${LIBRARIES} ${PCRE_LIBRARIES} )
	set( INCLUDE_DIRS ${INCLUDE_DIRS} ${PCRE_INCLUDE_DIRS} )
	set( DEFINITIONS "${DEFINITIONS} -DPCRE_SUPPORT" )
else()
	message( STATUS "Disabled PCRE code" )
endif()
if( ENABLE_TS_SCRIPTING )
	message( STATUS "Enabled TS scripting (V8) — linking src/map/scripting/*" )

	# Generated files live OUT of tree:
	#   - C++ artifacts go into the CMake binary dir (gitignored by
	#     virtue of being under build/)
	#   - TS artifacts go into npc-ts/generated/ which is gitignored
	#     via npc-ts/.gitignore
	# Authors never see codegen output committed.
	set( TS_GEN_CPP ${CMAKE_BINARY_DIR}/generated/scripting/player_binding.generated.cpp )
	set( TS_GEN_DTS ${CMAKE_SOURCE_DIR}/npc-ts/generated/player.generated.d.ts )
	set( TS_GEN_BUILTIN_HPP ${CMAKE_BINARY_DIR}/generated/scripting/surface_stubs.generated.hpp )
	# Placeholder file kept for legacy CMake DEPENDS — the surface .d.ts
	# isn't separately generated (api.d.ts IS the contract).
	set( TS_GEN_BUILTIN_DTS ${CMAKE_SOURCE_DIR}/npc-ts/generated/.surface_proxy.d.ts )

	add_custom_command(
		OUTPUT ${TS_GEN_CPP} ${TS_GEN_DTS} ${TS_GEN_BUILTIN_HPP} ${TS_GEN_BUILTIN_DTS}
		COMMAND ${CMAKE_COMMAND} -E echo "[ts-scripting] regenerating bindings..."
		COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/scripting/gen_bindings.py
			--cpp-out ${TS_GEN_CPP}
			--dts-out ${TS_GEN_DTS}
			--buildin-cpp-out ${TS_GEN_BUILTIN_HPP}
			--buildin-dts-out ${TS_GEN_BUILTIN_DTS}
		DEPENDS
			${CMAKE_SOURCE_DIR}/tools/scripting/gen_bindings.py
			${CMAKE_SOURCE_DIR}/tools/scripting/bindings.yaml
			${CMAKE_SOURCE_DIR}/npc-ts/types/api.d.ts
			${CMAKE_SOURCE_DIR}/npc-ts/types/registrations.d.ts
			${CMAKE_SOURCE_DIR}/npc-ts/types/npc.d.ts
			${CMAKE_SOURCE_DIR}/npc-ts/types/player.d.ts
			${CMAKE_SOURCE_DIR}/npc-ts/types/player-ops.d.ts
			${CMAKE_SOURCE_DIR}/npc-ts/types/world.d.ts
			${CMAKE_SOURCE_DIR}/npc-ts/types/social.d.ts
			${CMAKE_SOURCE_DIR}/npc-ts/types/item.d.ts
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Generating TS scripting bindings via libclang + api.d.ts"
		VERBATIM
	)
	add_custom_target( ts-scripting-codegen
		DEPENDS ${TS_GEN_CPP} ${TS_GEN_DTS} ${TS_GEN_BUILTIN_HPP} ${TS_GEN_BUILTIN_DTS} )

	file( GLOB MAP_SCRIPTING_HEADERS ${MAP_SOURCE_DIR}/scripting/*.hpp )
	file( GLOB MAP_SCRIPTING_SOURCES ${MAP_SOURCE_DIR}/scripting/*.cpp )
	# Splice in the auto-generated binding cpp from the binary dir.
	set( MAP_HEADERS ${MAP_HEADERS} ${MAP_SCRIPTING_HEADERS} )
	set( MAP_SOURCES ${MAP_SOURCES} ${MAP_SCRIPTING_SOURCES} ${TS_GEN_CPP} )
	set( LIBRARIES ${LIBRARIES} ${V8_LIBRARIES} )
	# The generated .cpp lives at CMAKE_BINARY_DIR/generated/scripting/,
	# outside the source tree, so the usual same-dir #include resolution
	# doesn't apply. Add src/map/ and src/map/scripting/ explicitly so
	# the generated file can write plain "map.hpp" / "pc.hpp" /
	# "player_binding.hpp" includes. Also add the generated dir itself
	# so dialog_context.cpp can `#include "surface_stubs.generated.hpp"`.
	set( INCLUDE_DIRS ${INCLUDE_DIRS} ${V8_INCLUDE_DIRS}
		${MAP_SOURCE_DIR}
		${MAP_SOURCE_DIR}/scripting
		${CMAKE_BINARY_DIR}/generated/scripting )
endif()
set( SOURCE_FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} ${MAP_HEADERS} ${MAP_SOURCES} )
source_group( common FILES ${COMMON_BASE_HEADERS} ${COMMON_HEADERS} )
source_group( map FILES ${MAP_HEADERS} ${MAP_SOURCES} )
include_directories( ${INCLUDE_DIRS} )

#message( STATUS "map-server SOURCE_FILES=${SOURCE_FILES}")
add_executable( map-server ${SOURCE_FILES} )
#message( STATUS "map-server LIBRARIES=${LIBRARIES}, DEPENDENCIES=${DEPENDENCIES} DEFINITIONS=${DEFINITIONS}")
add_dependencies( map-server ${DEPENDENCIES} )
if( ENABLE_TS_SCRIPTING )
	add_dependencies( map-server ts-scripting-codegen )
endif()
target_link_libraries( map-server ${LIBRARIES} ${DEPENDENCIES} )
set_target_properties( map-server PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_mapserver DESCRIPTION "map-server" DISPLAY_NAME "map-server" GROUP Runtime )
	install( TARGETS map-server
		DESTINATION "."
		COMPONENT Runtime_mapserver )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} map-server  CACHE INTERNAL "" )
message( STATUS "Creating target map-server - done" )
endif( BUILD_SERVERS )
