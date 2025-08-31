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

	"skills/skill_factory.cpp"
	"skills/skill_impl.cpp"
	"skills/weapon_skill_impl.cpp"
	"skills/status_skill_impl.cpp"

	"skills/acolyte/skill_factory_acolyte.cpp"

	"skills/archer/skill_factory_archer.cpp"

	"skills/custom/skill_factory_custom.cpp"

	"skills/gunslinger/skill_factory_gunslinger.cpp"

	"skills/mage/skill_factory_mage.cpp"

	"skills/mercenary/mercenary_bash.cpp"
	"skills/mercenary/skill_factory_mercenary.cpp"

	"skills/merchant/skill_factory_merchant.cpp"

	"skills/ninja/skill_factory_ninja.cpp"
	
	"skills/novice/skill_factory_novice.cpp"
	
	"skills/npc/skill_factory_npc.cpp"
	
	"skills/summoner/skill_factory_summoner.cpp"

	"skills/swordman/autoberserk.cpp"
	"skills/swordman/bash.cpp"
	"skills/swordman/magnum.cpp"
	"skills/swordman/provoke.cpp"
	"skills/swordman/selfprovoke.cpp"
	"skills/swordman/skill_factory_swordman.cpp"
	
	"skills/thief/skill_factory_thief.cpp"
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

	"skills/skill_factory.hpp"
	"skills/skill_impl.hpp"
	"skills/weapon_skill_impl.hpp"
	"skills/status_skill_impl.hpp"

	"skills/acolyte/skill_factory_acolyte.hpp"

	"skills/archer/skill_factory_archer.hpp"

	"skills/custom/skill_factory_custom.hpp"

	"skills/gunslinger/skill_factory_gunslinger.hpp"

	"skills/mage/skill_factory_mage.hpp"

	"skills/mercenary/mercenary_bash.hpp"
	"skills/mercenary/skill_factory_mercenary.hpp"

	"skills/merchant/skill_factory_merchant.hpp"

	"skills/ninja/skill_factory_ninja.hpp"
	
	"skills/novice/skill_factory_novice.hpp"
	
	"skills/npc/skill_factory_npc.hpp"
	
	"skills/summoner/skill_factory_summoner.hpp"

	"skills/swordman/autoberserk.hpp"
	"skills/swordman/bash.hpp"
	"skills/swordman/magnum.hpp"
	"skills/swordman/provoke.hpp"
	"skills/swordman/selfprovoke.hpp"
	"skills/swordman/skill_factory_swordman.hpp"
	
	"skills/thief/skill_factory_thief.hpp"
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
	target_compile_definitions(map-server PUBLIC "PCRE_SUPPORT")
endif()

# map-server-generator
add_executable(map-server-generator)

target_sources(map-server-generator PRIVATE ${MAP_SOURCES})

if(WIN32)
	target_sources(map-server-generator PRIVATE ${MAP_HEADERS})
	set_target_properties(map-server-generator PROPERTIES FOLDER "Tools")
endif()

target_compile_definitions(map-server-generator PUBLIC "MAP_GENERATOR")

target_link_libraries(map-server-generator PUBLIC
	common
	json
	${PCRE_LIBRARIES}
)

target_include_directories(map-server-generator PUBLIC
	${PCRE_INCLUDE_DIRS}
)

if(WITH_PCRE)
	target_compile_definitions(map-server-generator PUBLIC "PCRE_SUPPORT")
endif()
