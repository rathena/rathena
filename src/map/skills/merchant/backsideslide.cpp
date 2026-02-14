// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "backsideslide.hpp"

SkillBackSideSlide::SkillBackSideSlide() : SkillImpl(NC_B_SIDESLIDE) {
}

void SkillBackSideSlide::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const {
	skill_blown(src, target, skill_get_blewcount(getSkillId(), skill_lv), unit_getdir(src), BLOWN_IGNORE_NO_KNOCKBACK);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
