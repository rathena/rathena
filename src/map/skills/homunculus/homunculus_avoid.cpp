// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_avoid.hpp"

SkillAvoid::SkillAvoid() : SkillImpl(HLIF_AVOID) {
}

void SkillAvoid::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	// Master
	sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	// Homunculus
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv, sc_start(src, src, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
