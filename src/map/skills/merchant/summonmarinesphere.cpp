// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "summonmarinesphere.hpp"

#include "map/mob.hpp"

SkillSummonMarineSphere::SkillSummonMarineSphere() : SkillImpl(AM_SPHEREMINE) {
}

void SkillSummonMarineSphere::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 class_ = MOBID_MARINE_SPHERE;
	enum mob_ai ai = AI_SPHERE;
	mob_data *md;

	// Correct info, don't change any of this! [celest]
	md = mob_once_spawn_sub(src, src->m, x, y, status_get_name(*src), class_, "", SZ_SMALL, ai);
	if (md) {
		md->master_id = src->id;
		md->special_state.ai = ai;
		if( md->deletetimer != INVALID_TIMER )
			delete_timer(md->deletetimer, mob_timer_delete);
		md->deletetimer = add_timer (gettick() + skill_get_time(getSkillId(),skill_lv), mob_timer_delete, md->id, 0);
		mob_spawn (md); //Now it is ready for spawning.
	}
}
