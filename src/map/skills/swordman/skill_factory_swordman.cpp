// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_swordman.hpp"

#include "autoberserk.hpp"
#include "bash.hpp"
#include "magnum.hpp"
#include "provoke.hpp"
#include "selfprovoke.hpp"
#include "../status_skill_impl.hpp"

std::unique_ptr<const SkillImpl> SkillFactorySwordman::create(const e_skill skill_id) const {
	switch( skill_id ){
		case SM_AUTOBERSERK:
			return std::make_unique<SkillAutoBerserk>();
		case SM_BASH:
			return std::make_unique<SkillBash>();
		case SM_ENDURE:
			return std::make_unique<StatusSkillImpl>(SM_ENDURE);
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
