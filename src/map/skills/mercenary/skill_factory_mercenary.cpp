// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_mercenary.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

#include "mercenary_bash.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryMercenary::create(const e_skill skill_id) const {
	switch( skill_id ){
		case ABR_BATTLE_BUSTER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case ABR_DUAL_CANNON_FIRE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case ABR_INFINITY_BUSTER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case HAMI_BLOODLUST:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case HFLI_FLEET:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case HFLI_MOON:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case HFLI_SBR44:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case HFLI_SPEED:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case MA_CHARGEARROW:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case MA_DOUBLE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case MER_AUTOBERSERK:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case MER_CRASH:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case MER_QUICKEN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case MH_ANGRIFFS_MODUS:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case MH_GOLDENE_FERSE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case ML_AUTOGUARD:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case ML_DEFENDER:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case ML_PIERCE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case ML_SPIRALPIERCE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case MS_BASH:
			return std::make_unique<SkillMercenaryBash>();
		case MS_BERSERK:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case MS_PARRYING:
			return std::make_unique<StatusSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
