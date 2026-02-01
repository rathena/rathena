// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_ninja.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryNinja::create(const e_skill skill_id) const {
	switch( skill_id ){
		case KO_MEIKYOUSISUI:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case KO_SETSUDAN:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NJ_KASUMIKIRI:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NJ_KUNAI:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NJ_NEN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NJ_SYURIKEN:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NJ_UTSUSEMI:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SS_FUUMAKOUCHIKU:
			return std::make_unique<WeaponSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
