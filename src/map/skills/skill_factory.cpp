// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory.hpp"

#include <memory>
#include <vector>

#include "./mercenary/skill_factory_mercenary.hpp"
#include "./swordsman/skill_factory_swordsman.hpp"

SkillFactoryImpl::SkillFactoryImpl() : factories_{
	std::make_shared<SkillFactoryMercenary>(),
	std::make_shared<SkillFactorySwordsman>()
} {}

std::unique_ptr<const SkillImpl> SkillFactoryImpl::create(const e_skill skill_id) const {
	for (const std::shared_ptr<SkillFactory>& factory : factories_) {
		if (std::unique_ptr<const SkillImpl> impl = factory->create(skill_id); impl != nullptr) {
			return impl;
		}
	}

	return nullptr;
}
