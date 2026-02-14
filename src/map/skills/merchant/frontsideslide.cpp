// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "frontsideslide.hpp"

SkillFrontSideSlide::SkillFrontSideSlide() : SkillImpl(NC_F_SIDESLIDE) {
}

void SkillFrontSideSlide::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32&) const {
	uint8 dir = (unit_getdir(src) + 4) % 8;
	skill_blown(src, target, skill_get_blewcount(getSkillId(), skill_lv), dir, BLOWN_IGNORE_NO_KNOCKBACK);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
