// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "abrinfinity.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/status.hpp"

SkillAbrInfinity::SkillAbrInfinity() : SkillImpl(MT_SUMMON_ABR_INFINITY) {
}

void SkillAbrInfinity::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));

	mob_data *md = mob_once_spawn_sub(src, src->m, src->x, src->y, "--ja--", MOBID_ABR_INFINITY, "", SZ_SMALL, AI_ABR);

	if (md) {
		md->master_id = src->id;
		md->special_state.ai = AI_ABR;

		if (md->deletetimer != INVALID_TIMER)
			delete_timer(md->deletetimer, mob_timer_delete);
		md->deletetimer = add_timer(gettick() + skill_get_time(getSkillId(), skill_lv), mob_timer_delete, md->id, 0);
		mob_spawn(md);
	}
}
