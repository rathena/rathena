// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "detonator.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillDetonator::SkillDetonator() : SkillImpl(RA_DETONATOR) {
}

void SkillDetonator::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinallarea(skill_detonator, src->m, x-i, y-i, x+i, y+i, BL_SKILL, src);
	clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
}
