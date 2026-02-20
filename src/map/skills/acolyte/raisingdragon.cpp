// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "raisingdragon.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillRaisingDragon::SkillRaisingDragon() : StatusSkillImpl(SR_RAISINGDRAGON) {
}

void SkillRaisingDragon::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd ) {
		int16 max = 5 + skill_lv;
		sc_start(src,target, SC_EXPLOSIONSPIRITS, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
		for( int16 i = 0; i < max; i++ ) // Don't call more than max available spheres.
			pc_addspiritball(sd, skill_get_time(getSkillId(), skill_lv), max);

		StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
	}
}
