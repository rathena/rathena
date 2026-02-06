// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ultimatesacrifice.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillUltimateSacrifice::SkillUltimateSacrifice() : SkillImpl(IG_ULTIMATE_SACRIFICE) {
}

void SkillUltimateSacrifice::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	// Is the animation on this skill correct? Check if its on caster only or all affected. [Rytech]
	if( sd == nullptr || sd->status.party_id == 0 || (flag & 1) )
		clif_skill_nodamage(target, *target, getSkillId(), skill_lv, sc_start(src,target,skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
	else if (sd)
	{
		status_set_hp(src, 1, 0);
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
	}
}
