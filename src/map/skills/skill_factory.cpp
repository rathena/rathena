// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory.hpp"

#include <memory>
#include <vector>

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "./skill_impl.cpp"
#include "./acolyte/skill_factory_acolyte.cpp"
#include "./archer/skill_factory_archer.cpp"
#include "./custom/skill_factory_custom.cpp"
#include "./gunslinger/skill_factory_gunslinger.cpp"
#include "./mage/skill_factory_mage.cpp"
#include "./mercenary/skill_factory_mercenary.cpp"
#include "./merchant/skill_factory_merchant.cpp"
#include "./npc/skill_factory_npc.cpp"
#include "./ninja/skill_factory_ninja.cpp"
#include "./novice/skill_factory_novice.cpp"
#include "./summoner/skill_factory_summoner.cpp"
#include "./swordman/skill_factory_swordman.cpp"
#include "./taekwon/skill_factory_taekwon.cpp"
#include "./thief/skill_factory_thief.cpp"

std::unique_ptr<const SkillImpl> SkillFactoryImpl::create(const e_skill skill_id) const {
	static const std::vector<std::shared_ptr<SkillFactory>> factories = {
		// Custom Skills (Always first to allow overwriting skills)
		std::make_shared<SkillFactoryCustom>(),
		// Normal Skills
		std::make_shared<SkillFactoryAcolyte>(),
		std::make_shared<SkillFactoryArcher>(),
		std::make_shared<SkillFactoryGunslinger>(),
		std::make_shared<SkillFactoryMage>(),
		std::make_shared<SkillFactoryMercenary>(),
		std::make_shared<SkillFactoryMerchant>(),
		std::make_shared<SkillFactoryNinja>(),
		std::make_shared<SkillFactoryNpc>(),
		std::make_shared<SkillFactoryNovice>(),
		std::make_shared<SkillFactorySummoner>(),
		std::make_shared<SkillFactorySwordman>(),
		std::make_shared<SkillFactoryTaekwon>(),
		std::make_shared<SkillFactoryThief>(),
	};

	for (const std::shared_ptr<SkillFactory>& factory : factories) {
		if (std::unique_ptr<const SkillImpl> impl = factory->create(skill_id); impl != nullptr) {
			return impl;
		}
	}

	return nullptr;
}
