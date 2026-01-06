// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_archer.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

#include "arrowshower.hpp"
#include "chargearrow.hpp"
#include "concentration.hpp"
#include "doublestrafe.hpp"
#include "makingarrow.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryArcher::create(const e_skill skill_id) const {
	switch( skill_id ){
		case AC_CHARGEARROW:
			return std::make_unique<SkillChargeArrow>();
		case AC_CONCENTRATION:
			return std::make_unique<SkillConcentration>();
		case AC_DOUBLE:
			return std::make_unique<SkillDoubleStrafe>();
		case AC_MAKINGARROW:
			return std::make_unique<SkillMakingArrow>();
		case AC_SHOWER:
			return std::make_unique<SkillArrowShower>();
		case BA_DISSONANCE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case BA_MUSICALSTRIKE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case CG_ARROWVULCAN:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case DC_THROWARROW:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case HT_PHANTASMIC:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case RA_AIMEDBOLT:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case RA_UNLIMIT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case TR_KVASIR_SONATA:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case TR_MYSTIC_SYMPHONY:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case WH_CALAMITYGALE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case WH_WIND_SIGN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case WM_GREAT_ECHO:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case WM_SEVERE_RAINSTORM_MELEE:
			return std::make_unique<WeaponSkillImpl>(skill_id);

		default:
			return nullptr;
	}

	return nullptr;
}
