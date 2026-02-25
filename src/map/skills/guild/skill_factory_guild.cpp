// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_guild.hpp"

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "coldheart.cpp"
#include "gloriouswounds.cpp"
#include "greatleadership.cpp"
#include "sharpgaze.cpp"

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
		case GD_LEADERSHIP:
			return std::make_unique<SkillGreatLeadership>();
		case GD_GLORYWOUNDS:
			return std::make_unique<SkillGloriousWounds>();
		case GD_SOULCOLD:
			return std::make_unique<SkillColdHeart>();
		case GD_HAWKEYES:
			return std::make_unique<SkillSharpGaze>();

		default:
			return nullptr;
	}
}
