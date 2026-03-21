// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "safetywall.hpp"

SkillSafetyWall::SkillSafetyWall() : SkillImpl(MG_SAFETYWALL) {
}

void SkillSafetyWall::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 dummy = 1;

	if (map_foreachincell(skill_cell_overlap, src->m, x, y, BL_SKILL, getSkillId(), &dummy, src)) {
		skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
		// Don't consume gems if cast on Land Protector
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
