// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "caneofevileye.hpp"

#include "map/clif.hpp"

SkillCaneOfEvilEye::SkillCaneOfEvilEye() : SkillImpl(NPC_CANE_OF_EVIL_EYE) {
}

void SkillCaneOfEvilEye::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;
	if(skill_unitsetting(src,getSkillId(),skill_lv,x,y,0))
		clif_skill_poseffect( *src, getSkillId(), skill_lv, x, y, tick );
}
