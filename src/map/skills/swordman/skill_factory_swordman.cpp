// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_swordman.hpp"

#include "../weapon_skill_impl.hpp"
#include "../status_skill_impl.hpp"

#include "autoberserk.hpp"
#include "bash.hpp"
#include "magnum.hpp"
#include "provoke.hpp"
#include "selfprovoke.hpp"

std::unique_ptr<const SkillImpl> SkillFactorySwordman::create(const e_skill skill_id) const {
	switch( skill_id ){
		case CR_HOLYCROSS:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case CR_SHIELDBOOMERANG:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case CR_SHIELDCHARGE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case KN_PIERCE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case KN_SPEARBOOMERANG:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case LG_BANISHINGPOINT:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case LG_HESPERUSLIT:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case LG_RAGEBURST:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case LG_SHIELDPRESS:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case LK_SPIRALPIERCE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case PA_SACRIFICE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case PA_SHIELDCHAIN:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case RK_SONICWAVE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SM_AUTOBERSERK:
			return std::make_unique<SkillAutoBerserk>();
		case SM_BASH:
			return std::make_unique<SkillBash>();
		case SM_ENDURE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SM_MAGNUM:
			return std::make_unique<SkillMagnumBreak>();
		case SM_PROVOKE:
			return std::make_unique<SkillProvoke>();
		case SM_SELFPROVOKE:
			return std::make_unique<SkillProvokeSelf>();

		default:
			return nullptr;
	}
}
