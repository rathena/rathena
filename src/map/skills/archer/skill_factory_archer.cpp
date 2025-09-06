// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_archer.hpp"

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
		default:
			return nullptr;
	}

	return nullptr;
}
