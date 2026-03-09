// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "anklesnare.hpp"

SkillAnkleSnare::SkillAnkleSnare() : SkillImpl(HT_ANKLESNARE) {
}

void SkillAnkleSnare::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
