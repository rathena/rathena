// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cracker.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillCracker::SkillCracker() : SkillImpl(GS_CRACKER) {
}

void SkillCracker::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	map_session_data *sd = BL_CAST(BL_PC, src);
	map_session_data *dstsd = BL_CAST(BL_PC, target);
	mob_data *dstmd = BL_CAST(BL_MOB, target);

	/* per official standards, this skill works on players and mobs. */
	if (sd && (dstsd || dstmd)) {
		int32 i = 65 - 5 * distance_bl(src, target); // Base rate
		if (i < 30)
			i = 30;
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		sc_start(src, target, SC_STUN, i, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	}
}
