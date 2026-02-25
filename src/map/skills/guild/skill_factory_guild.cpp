// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_guild.hpp"

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "battleorders.cpp"
#include "chargeshoutbeating.cpp"
#include "chargeshoutflag.cpp"
#include "coldheart.cpp"
#include "emergencymove.cpp"
#include "gloriouswounds.cpp"
#include "greatleadership.cpp"
#include "itememergencycall.cpp"
#include "regeneration.cpp"
#include "restoration.cpp"
#include "sharpgaze.cpp"
#include "urgentcall.cpp"

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
		case GD_BATTLEORDER:
			return std::make_unique<SkillBattleOrders>();
		case GD_REGENERATION:
			return std::make_unique<SkillRegeneration>();
		case GD_RESTORE:
			return std::make_unique<SkillRestoration>();
		case GD_EMERGENCYCALL:
			return std::make_unique<SkillUrgentCall>();
		case GD_DEVELOPMENT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GD_ITEMEMERGENCYCALL:
			return std::make_unique<SkillItemEmergencyCall>();
		case GD_GUILD_STORAGE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GD_CHARGESHOUT_FLAG:
			return std::make_unique<SkillChargeShoutFlag>();
		case GD_CHARGESHOUT_BEATING:
			return std::make_unique<SkillChargeShoutBeating>();
		case GD_EMERGENCY_MOVE:
			return std::make_unique<SkillEmergencyMove>();

		default:
			return nullptr;
	}
}
