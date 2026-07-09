// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "epiclesis.hpp"

SkillEpiclesis::SkillEpiclesis() : SkillImpl(AB_EPICLESIS) {
}

void SkillEpiclesis::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	std::shared_ptr<s_skill_unit_group> sg;

	if( (sg = skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0)) ) {
		int32 i = skill_get_splash(getSkillId(), skill_lv);
		map_foreachinallarea(skill_area_sub, src->m, x - i, y - i, x + i, y + i, BL_CHAR, src, ALL_RESURRECTION, 1, tick, flag|BCT_NOENEMY|1,skill_castend_nodamage_id);
	}
}
