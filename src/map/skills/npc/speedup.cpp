// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "speedup.hpp"

#include "map/mob.hpp"

SkillSpeedUp::SkillSpeedUp() : SkillImpl(NPC_SPEEDUP) {
}

void SkillSpeedUp::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* md = BL_CAST(BL_MOB, src);

	if (md != nullptr) {
		// Officially, trickcasting continues as long as there are more than 700ms left
		int32 trickstop = (MOB_SKILL_INTERVAL * 7) / 10;
		if (DIFF_TICK(md->trickcasting, tick) >= trickstop) {
			// This skill directly modifies a monster's base speed value
			md->base_status->speed = std::max(md->base_status->speed - 250, MIN_WALK_SPEED);
			// Need to recalc speed based on new base value
			status_calc_bl(md, { SCB_SPEED });
			// We use skills only on each full cell, to fix the inaccuracy we do this on last move interval
			if (DIFF_TICK(md->trickcasting, tick) < trickstop + MOB_SKILL_INTERVAL)
				md->last_skillcheck = tick + 100;
		}
		else {
			// Synchronize skill usage
			md->last_skillcheck = md->trickcasting;
			// Causes monster to stop and get ready for next alchemist skill
			md->trickcasting = 0;
			md->state.can_escape = 0;
		}
	}
}
