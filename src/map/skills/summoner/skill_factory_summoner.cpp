// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_summoner.hpp"

#include "../status_skill_impl.hpp"

std::unique_ptr<const SkillImpl> SkillFactorySummoner::create(const e_skill skill_id) const {
	switch( skill_id ){
		case SH_TEMPORARY_COMMUNION:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SU_ARCLOUSEDASH:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SU_FRESHSHRIMP:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SU_HIDE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case SU_STOOP:
			return std::make_unique<StatusSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
