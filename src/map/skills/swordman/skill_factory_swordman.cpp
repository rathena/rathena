// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_swordman.hpp"

#include "bash.hpp"

std::unique_ptr<const SkillImpl> SkillFactorySwordman::create(const e_skill skill_id) const {
	switch( skill_id ){
		case SM_BASH:
			return std::make_unique<SkillBash>();

		default:
			return nullptr;
	}
}
