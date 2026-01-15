// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gravity.hpp"

#include "map/clif.hpp"

SkillGravity::SkillGravity() : SkillImpl(SA_GRAVITY) {
}

void SkillGravity::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
}
