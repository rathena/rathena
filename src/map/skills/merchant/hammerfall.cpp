// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hammerfall.hpp"

SkillHammerFall::SkillHammerFall() : SkillImpl(BS_HAMMERFALL) {
}

void SkillHammerFall::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_addtimerskill(src, tick+1000, target->id, 0, 0, getSkillId(), skill_lv, min(20+10*skill_lv, 50+5*skill_lv), flag);
}

void SkillHammerFall::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinallarea(skill_area_sub,
		src->m, x-i, y-i, x+i, y+i, BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|2,
		skill_castend_nodamage_id);
}
