// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "warmer.hpp"

SkillWarmer::SkillWarmer() : SkillImpl(SO_WARMER) {
}

void SkillWarmer::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag |= 8;
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
