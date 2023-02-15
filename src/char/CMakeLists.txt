add_executable(char-server)

target_sources(char-server PRIVATE
	"char.cpp"
	"char_clif.cpp"
	"char_cnslif.cpp"
	"char_logif.cpp"
	"char_mapif.cpp"
	"int_achievement.cpp"
	"int_auction.cpp"
	"int_clan.cpp"
	"int_elemental.cpp"
	"int_guild.cpp"
	"int_homun.cpp"
	"int_mail.cpp"
	"int_mercenary.cpp"
	"int_party.cpp"
	"int_pet.cpp"
	"int_quest.cpp"
	"int_storage.cpp"
	"inter.cpp"
)

if(WIN32)
	target_sources(char-server PRIVATE
		"char_clif.hpp"
		"char_cnslif.hpp"
		"char.hpp"
		"char_logif.hpp"
		"char_mapif.hpp"
		"int_achievement.hpp"
		"int_auction.hpp"
		"int_clan.hpp"
		"int_elemental.hpp"
		"inter.hpp"
		"int_guild.hpp"
		"int_homun.hpp"
		"int_mail.hpp"
		"int_mercenary.hpp"
		"int_party.hpp"
		"int_pet.hpp"
		"int_quest.hpp"
		"int_storage.hpp"
		"packets.hpp"
	)

	set_target_properties(char-server PROPERTIES FOLDER "Servers")
endif()

target_link_libraries(char-server PUBLIC
	common
)
