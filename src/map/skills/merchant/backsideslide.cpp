// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "backsideslide.hpp"

#include "map/clif.hpp"
#include "map/unit.hpp"

SkillBackSideSlide::SkillBackSideSlide() : SkillImpl(NC_B_SIDESLIDE) {
}

void SkillBackSideSlide::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	uint8 dir = unit_getdir(src);
	skill_blown(src,target,skill_get_blewcount(getSkillId(),skill_lv),dir,BLOWN_IGNORE_NO_KNOCKBACK);
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
}
