// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crimsonrock.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillCrimsonRock::SkillCrimsonRock() : SkillImplRecursiveDamageSplash(WL_CRIMSONROCK) {
}

void SkillCrimsonRock::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 700 + 600 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillCrimsonRock::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_area_temp[4] = target->x;
	skill_area_temp[5] = target->y;

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
