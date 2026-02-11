// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "darkillusion.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/unit.hpp"

SkillDarkIllusion::SkillDarkIllusion() : WeaponSkillImpl(GC_DARKILLUSION) {
}

void SkillDarkIllusion::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int16 x, y;
	int16 dir = map_calc_dir(src,target->x,target->y);

	if( dir > 0 && dir < 4) x = 2;
	else if( dir > 4 ) x = -2;
	else x = 0;
	if( dir > 2 && dir < 6 ) y = 2;
	else if( dir == 7 || dir < 2 ) y = -2;
	else y = 0;

	if( unit_movepos(src, target->x+x, target->y+y, 1, 1) ) {
		clif_blown(src);
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);

		if( rnd()%100 < 4 * skill_lv )
			skill_castend_damage_id(src,target,GC_CROSSIMPACT,skill_lv,tick,flag);
	}
}
