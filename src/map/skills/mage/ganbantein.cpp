// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ganbantein.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillGanbantein::SkillGanbantein() : SkillImpl(HW_GANBANTEIN) {
}

void SkillGanbantein::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (rnd()%100 < 80) {
		int32 dummy = 1;
		clif_skill_poseffect( *src, getSkillId(), skill_lv, x, y, tick );
		bool i = skill_get_splash(getSkillId(), skill_lv);
		map_foreachinallarea(skill_cell_overlap, src->m, x-i, y-i, x+i, y+i, BL_SKILL, getSkillId(), &dummy, src);
	} else {
		if (sd) clif_skill_fail( *sd, getSkillId() );
	
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
}
