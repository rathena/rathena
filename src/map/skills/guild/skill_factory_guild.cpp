// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_guild.hpp"

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "battleorders.cpp"
#include "chargeshoutbeating.cpp"
#include "chargeshoutflag.cpp"
#include "emergencymove.cpp"
#include "itememergencycall.cpp"
#include "regeneration.cpp"
#include "restoration.cpp"
#include "urgentcall.cpp"

std::unique_ptr<const SkillImpl> SkillFactoryGuild::create(const e_skill skill_id) const {
	switch (static_cast<uint16>(skill_id)) {
		case GD_BATTLEORDER:
			return std::make_unique<SkillBattleOrders>();
		case GD_CHARGESHOUT_BEATING:
			return std::make_unique<SkillChargeShoutBeating>();
		case GD_CHARGESHOUT_FLAG:
			return std::make_unique<SkillChargeShoutFlag>();
		case GD_EMERGENCYCALL:
			return std::make_unique<SkillUrgentCall>();
		case GD_EMERGENCY_MOVE:
			return std::make_unique<SkillEmergencyMove>();
		case GD_ITEMEMERGENCYCALL:
			return std::make_unique<SkillItemEmergencyCall>();
		case GD_REGENERATION:
			return std::make_unique<SkillRegeneration>();
		case GD_RESTORE:
			return std::make_unique<SkillRestoration>();

		default:
			return nullptr;
	}
}
