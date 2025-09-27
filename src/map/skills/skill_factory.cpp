// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory.hpp"

#include <memory>
#include <vector>

#include "./acolyte/skill_factory_acolyte.hpp"
#include "./archer/skill_factory_archer.hpp"
#include "./custom/skill_factory_custom.hpp"
#include "./gunslinger/skill_factory_gunslinger.hpp"
#include "./mage/skill_factory_mage.hpp"
#include "./mercenary/skill_factory_mercenary.hpp"
#include "./merchant/skill_factory_merchant.hpp"
#include "./npc/skill_factory_npc.hpp"
#include "./ninja/skill_factory_ninja.hpp"
#include "./novice/skill_factory_novice.hpp"
#include "./summoner/skill_factory_summoner.hpp"
#include "./swordman/skill_factory_swordman.hpp"
#include "./thief/skill_factory_thief.hpp"

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
		std::make_shared<SkillFactoryThief>(),
	};

	for (const std::shared_ptr<SkillFactory>& factory : factories) {
		if (std::unique_ptr<const SkillImpl> impl = factory->create(skill_id); impl != nullptr) {
			return impl;
		}
	}

	return nullptr;
}
