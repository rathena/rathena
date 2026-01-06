// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_acolyte.hpp"

#include "../weapon_skill_impl.hpp"

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
		case AB_DUPLELIGHT_MELEE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
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
		case CH_CHAINCRUSH:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case CH_TIGERFIST:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SR_CRESCENTELBOW_AUTOSPELL:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SR_DRAGONCOMBO:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SR_FALLENEMPIRE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SR_GATEOFHELL:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SR_GENTLETOUCH_QUIET:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		default:
			return nullptr;
	}

	return nullptr;
}
