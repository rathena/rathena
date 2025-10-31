// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_mercenary.hpp"

#include "mercenary_bash.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryMercenary::create(const e_skill skill_id) const {
	switch( skill_id ){
		case MS_BASH:
			return std::make_unique<SkillMercenaryBash>();

		default:
			return nullptr;
	}
}
