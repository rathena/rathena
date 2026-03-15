// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "giantgrowth.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillGiantGrowth::SkillGiantGrowth() : SkillImpl(RK_GIANTGROWTH) {
}

void SkillGiantGrowth::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (map_session_data* sd = BL_CAST(BL_PC, src); sd != nullptr) {
		if (pc_checkskill(sd, RK_RUNEMASTERY) >= 1) {
			if (sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv)))
				clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		} else
			clif_skill_fail( *sd, getSkillId() );
	}
}
