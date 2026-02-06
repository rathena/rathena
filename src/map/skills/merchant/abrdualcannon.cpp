// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "abrdualcannon.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillAbrDualCannon::SkillAbrDualCannon() : StatusSkillImpl(MT_SUMMON_ABR_DUAL_CANNON) {
}

void SkillAbrDualCannon::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);

	mob_data *md = mob_once_spawn_sub(src, src->m, src->x, src->y, "--ja--", MOBID_ABR_DUAL_CANNON, "", SZ_SMALL, AI_ABR);

	if (md) {
		md->master_id = src->id;
		md->special_state.ai = AI_ABR;

		if (md->deletetimer != INVALID_TIMER)
			delete_timer(md->deletetimer, mob_timer_delete);
		md->deletetimer = add_timer(gettick() + skill_get_time(getSkillId(), skill_lv), mob_timer_delete, md->id, 0);
		mob_spawn(md);
	}
}
