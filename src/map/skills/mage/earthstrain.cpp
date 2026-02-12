// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "earthstrain.hpp"

#include <config/core.hpp>

#include "map/map.hpp"
#include "map/status.hpp"

SkillEarthStrain::SkillEarthStrain() : SkillImpl(WL_EARTHSTRAIN) {
}

void SkillEarthStrain::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 1000 + 600 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillEarthStrain::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 w, wave = skill_lv + 4, dir = map_calc_dir(src,x,y);
	int32 sx = x = src->x, sy = y = src->y; // Store first caster's location to avoid glitch on unit setting

	for( w = 1; w <= wave; w++ )
	{
		switch( dir ){
			case 0: case 1: case 7: sy = y + w; break;
			case 3: case 4: case 5: sy = y - w; break;
			case 2: sx = x - w; break;
			case 6: sx = x + w; break;
		}
		skill_addtimerskill(src,gettick() + (140 * w),0,sx,sy,getSkillId(),skill_lv,dir,flag&2);
	}
}
