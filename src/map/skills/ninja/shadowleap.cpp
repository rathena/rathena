// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shadowleap.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillShadowLeap::SkillShadowLeap() : SkillImpl(NJ_SHADOWJUMP) {
}

void SkillShadowLeap::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	if( map_getcell(src->m,x,y,CELL_CHKREACH) && skill_check_unit_movepos(5, src, x, y, 1, 0) ) //You don't move on GVG grounds.
		clif_blown(src);
	status_change_end(src, SC_HIDING);
}
