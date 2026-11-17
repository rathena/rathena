// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gravitycontrol.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillGravityControl::SkillGravityControl() : SkillImpl(SJ_GRAVITYCONTROL) {
}

void SkillGravityControl::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* sstatus = status_get_status_data(*src);
	status_data* tstatus = status_get_status_data(*target);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	int32 fall_damage = sstatus->batk + sstatus->rhw.atk - tstatus->def2;

	if (target->type == BL_PC)
		fall_damage += dstsd->weight / 10 - tstatus->def;
	else // Monster's don't have weight. Put something in its place.
		fall_damage += 50 * status_get_lv(src) - tstatus->def;

	fall_damage = max(1, fall_damage);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start2(src, target, skill_get_sc(getSkillId()), 100, skill_lv, fall_damage, skill_get_time(getSkillId(), skill_lv)));
}
