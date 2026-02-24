// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fourcolorscharm.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillFourColorsCharm::SkillFourColorsCharm() : StatusSkillImpl(SS_FOUR_CHARM) {
}

void SkillFourColorsCharm::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd != nullptr) {
		sc_type type = skill_get_sc(getSkillId());

		switch (sd->spiritcharm_type) {
			case CHARM_TYPE_FIRE:  type = SC_FIRE_CHARM_POWER;    break;
			case CHARM_TYPE_WATER: type = SC_WATER_CHARM_POWER;   break;
			case CHARM_TYPE_LAND:  type = SC_GROUND_CHARM_POWER;  break;
			case CHARM_TYPE_WIND:  type = SC_WIND_CHARM_POWER;    break;
			default:  type = SC_NONE;    break;
		}
		if (type != SC_NONE) {
			clif_skill_nodamage(src, *target, getSkillId(), skill_lv,
				sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
		}
	}
}
