// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firerain.hpp"

SkillFireRain::SkillFireRain() : SkillImpl(RL_FIRE_RAIN) {
}

void SkillFireRain::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 wave = skill_lv + 5;
	int32 dir = map_calc_dir(src, x, y);
	int32 sx = src->x;
	int32 sy = src->y;

	x = src->x;
	y = src->y;

	for (int32 w = 0; w <= wave; ++w) {
		switch (dir) {
			case DIR_NORTH:
			case DIR_NORTHWEST:
			case DIR_NORTHEAST:
				sy = y + w;
				break;
			case DIR_WEST:
				sx = x - w;
				break;
			case DIR_SOUTHWEST:
			case DIR_SOUTH:
			case DIR_SOUTHEAST:
				sy = y - w;
				break;
			case DIR_EAST:
				sx = x + w;
				break;
		}
		skill_addtimerskill(src, gettick() + (80 * w), 0, sx, sy, getSkillId(), skill_lv, dir, flag);
	}
}

void SkillFireRain::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 3500 + 300 * skill_lv;
}
