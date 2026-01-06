// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_novice.hpp"

#include "../status_skill_impl.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryNovice::create(const e_skill skill_id) const {
	switch( skill_id ){
		case NV_TRICKDEAD:
			return std::make_unique<StatusSkillImpl>(skill_id, true);

		default:
			return nullptr;
	}

	return nullptr;
}
