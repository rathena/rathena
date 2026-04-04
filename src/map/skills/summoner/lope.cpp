// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lope.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/unit.hpp"

SkillLope::SkillLope() : SkillImpl(SU_LOPE) {
}

void SkillLope::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Fails on noteleport maps, except for GvG and BG maps
	if (map_getmapflag(src->m, MF_NOTELEPORT) && !(map_getmapflag(src->m, MF_BATTLEGROUND) || map_flag_gvg2(src->m))) {
		x = src->x;
		y = src->y;
	}

	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
	if (!map_count_oncell(src->m, x, y, BL_PC|BL_NPC|BL_MOB, 0) && map_getcell(src->m, x, y, CELL_CHKREACH) && unit_movepos(src, x, y, 1, 0))
		clif_blown(src);
}
