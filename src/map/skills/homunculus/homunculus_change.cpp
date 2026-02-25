// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_change.hpp"

SkillChange::SkillChange() : SkillImpl(HLIF_CHANGE) {
}

void SkillChange::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

#ifndef RENEWAL
	status_percent_heal(target, 100, 100);
#endif
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
