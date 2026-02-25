// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_guild.hpp"

#include "../skill_impl.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryGuild::create(const e_skill skill_id) const {
	switch (static_cast<uint16>(skill_id)) {
		case GD_APPROVAL:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GD_KAFRACONTRACT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GD_GUARDRESEARCH:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GD_GUARDUP:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GD_EXTENSION:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GD_GLORYGUILD:
			return std::make_unique<StatusSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
