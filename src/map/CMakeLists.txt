set(MAP_SOURCES
	"achievement.cpp"
	"atcommand.cpp"
	"battle.cpp"
	"battleground.cpp"
	"buyingstore.cpp"
	"cashshop.cpp"
	"channel.cpp"
	"chat.cpp"
	"chrif.cpp"
	"clan.cpp"
	"clif.cpp"
	"date.cpp"
	"duel.cpp"
	"elemental.cpp"
	"guild.cpp"
	"homunculus.cpp"
	"instance.cpp"
	"intif.cpp"
	"itemdb.cpp"
	"log.cpp"
	"mail.cpp"
	"map.cpp"
	"mapreg.cpp"
	"mercenary.cpp"
	"mob.cpp"
	"navi.cpp"
	"npc.cpp"
	"npc_chat.cpp"
	"party.cpp"
	"path.cpp"
	"pc.cpp"
	"pc_groups.cpp"
	"pet.cpp"
	"quest.cpp"
	"script.cpp"
	"searchstore.cpp"
	"skill.cpp"
	"status.cpp"
	"storage.cpp"
	"trade.cpp"
	"unit.cpp"
	"vending.cpp"
)

set(MAP_HEADERS
	"achievement.hpp"
	"atcommand.hpp"
	"battleground.hpp"
	"battle.hpp"
	"buyingstore.hpp"
	"cashshop.hpp"
	"channel.hpp"
	"chat.hpp"
	"chrif.hpp"
	"clan.hpp"
	"clif.hpp"
	"clif_obfuscation.hpp"
	"clif_packetdb.hpp"
	"clif_shuffle.hpp"
	"date.hpp"
	"duel.hpp"
	"elemental.hpp"
	"guild.hpp"
	"homunculus.hpp"
	"instance.hpp"
	"intif.hpp"
	"itemdb.hpp"
	"log.hpp"
	"mail.hpp"
	"map.hpp"
	"mapreg.hpp"
	"mercenary.hpp"
	"mob.hpp"
	"navi.hpp"
	"npc.hpp"
	"packets.hpp"
	"packets_struct.hpp"
	"party.hpp"
	"path.hpp"
	"pc_groups.hpp"
	"pc.hpp"
	"pet.hpp"
	"quest.hpp"
	"script_constants.hpp"
	"script.hpp"
	"searchstore.hpp"
	"skill.hpp"
	"status.hpp"
	"storage.hpp"
	"trade.hpp"
	"unit.hpp"
	"vending.hpp"
)

add_executable(map-server)

target_sources(map-server PRIVATE ${MAP_SOURCES})

if(WIN32)
	target_sources(map-server PRIVATE ${MAP_HEADERS})
	set_target_properties(map-server PROPERTIES FOLDER "Servers")
endif()

target_link_libraries(map-server PUBLIC
	common
	${PCRE_LIBRARIES}
)

target_include_directories(map-server PUBLIC
	${PCRE_INCLUDE_DIRS}
)

if(WITH_PCRE)
	target_compile_definitions(map-server PUBLIC "-DPCRE_SUPPORT")
endif()

# map-server-generator
add_executable(map-server-generator)

target_sources(map-server-generator PRIVATE ${MAP_SOURCES})

if(WIN32)
	target_sources(map-server-generator PRIVATE ${MAP_HEADERS})
	set_target_properties(map-server-generator PROPERTIES FOLDER "Tools")
endif()

set_target_properties(map-server-generator PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_compile_definitions(map-server-generator PUBLIC "-DMAP_GENERATOR")

target_link_libraries(map-server-generator PUBLIC
	common
	json
	${PCRE_LIBRARIES}
)

target_include_directories(map-server-generator PUBLIC
	${PCRE_INCLUDE_DIRS}
)

if(PCRE_FOUND)
	target_compile_definitions(map-server-generator PUBLIC "-DPCRE_SUPPORT")
endif()
