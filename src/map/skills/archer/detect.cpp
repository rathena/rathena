// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "detect.hpp"

#include "map/status.hpp"

SkillDetect::SkillDetect() : SkillImpl(HT_DETECTING) {
}

void SkillDetect::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinallarea( status_change_timer_sub,
		src->m, x-i, y-i, x+i,y+i,BL_CHAR,
		src,nullptr,SC_SIGHT,tick);
	skill_reveal_trap_inarea(src, i, x, y);
}
