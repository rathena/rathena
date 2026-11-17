// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculusresurrection.hpp"

#include "map/clif.hpp"
#include "map/homunculus.hpp"
#include "map/pc.hpp"

SkillHomunculusResurrection::SkillHomunculusResurrection() : SkillImpl(AM_RESURRECTHOMUN) {
}

void SkillHomunculusResurrection::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd)
	{
		if (!hom_ressurect(sd, 20*skill_lv, x, y))
		{
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
	}
}
