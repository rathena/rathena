// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fawsilversniper.hpp"

#include <common/timer.hpp>

#include "map/mob.hpp"

SkillFawSilverSniper::SkillFawSilverSniper() : SkillImpl(NC_SILVERSNIPER) {
}

void SkillFawSilverSniper::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* md = mob_once_spawn_sub(src, src->m, x, y, status_get_name(*src), MOBID_SILVERSNIPER, "", SZ_SMALL, AI_NONE);
	if (md) {
		md->master_id = src->id;
		md->special_state.ai = AI_FAW;
		if (md->deletetimer != INVALID_TIMER) {
			delete_timer(md->deletetimer, mob_timer_delete);
		}
		md->deletetimer = add_timer(gettick() + skill_get_time(getSkillId(), skill_lv), mob_timer_delete, md->id, 0);
		mob_spawn(md);
	}
}
