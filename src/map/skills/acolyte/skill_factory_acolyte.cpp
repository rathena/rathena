// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_acolyte.hpp"

#include "angelus.hpp"
#include "blessing.hpp"
#include "crucis.hpp"
#include "cure.hpp"
#include "decagi.hpp"
#include "heal.hpp"
#include "holylight.hpp"
#include "holywater.hpp"
#include "incagi.hpp"
#include "ruwach.hpp"


std::unique_ptr<const SkillImpl> SkillFactoryAcolyte::create(const e_skill skill_id) const {
	switch( skill_id ){
		case AL_ANGELUS:
			return std::make_unique<SkillAngelus>();
		case AL_BLESSING:
			return std::make_unique<SkillBlessing>();
		case AL_CRUCIS:
			return std::make_unique<SkillCrucis>();
		case AL_CURE:
			return std::make_unique<SkillCure>();
		case AL_DECAGI:
			return std::make_unique<SkillDecreaseAgi>();
		case AL_HEAL:
			return std::make_unique<SkillHeal>();
		case AL_HOLYLIGHT:
			return std::make_unique<SkillHolyLight>();
		case AL_HOLYWATER:
			return std::make_unique<SkillHolyWater>();
		case AL_INCAGI:
			return std::make_unique<SkillIncreaseAgi>();
		case AL_RUWACH:
			return std::make_unique<SkillRuwach>();
		default:
			return nullptr;
	}

	return nullptr;
}
