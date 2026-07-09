// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcrun.hpp"

#include "map/mob.hpp"
#include "map/unit.hpp"

SkillNpcRun::SkillNpcRun() : SkillImpl(NPC_RUN) {
}

void SkillNpcRun::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* md = BL_CAST(BL_MOB, src);

	if (md) {
		block_list* tbl = map_id2bl(md->target_id);

		if (tbl) {
			md->state.can_escape = 1;
			mob_unlocktarget(md, tick);
			// Official distance is 7, if level > 1, distance = level
			t_tick time = unit_escape(src, tbl, skill_lv > 1 ? skill_lv : 7, 3);

			if (time) {
				// Need to set state here as it's not set otherwise
				mob_setstate(*md, MSS_WALK);
				// Set AI to inactive for the duration of this movement
				md->next_thinktime = tick + time;
			}
		}
	}
}
