// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "armcannon.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillArmCannon::SkillArmCannon() : SkillImplRecursiveDamageSplash(NC_ARMSCANNON) {
}

void SkillArmCannon::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 400 + 350 * skill_lv;
	RE_LVL_DMOD(100);
}

int32 SkillArmCannon::getSplashTarget(block_list* src) const {
	return splash_target(src);
}

void SkillArmCannon::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_area_temp[1] = 0;

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
