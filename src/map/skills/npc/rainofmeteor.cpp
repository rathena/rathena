// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rainofmeteor.hpp"

#include "map/pc.hpp"

SkillRainOfMeteor::SkillRainOfMeteor() : SkillImpl(NPC_RAINOFMETEOR) {
}

void SkillRainOfMeteor::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 350;	// unknown ratio
}

void SkillRainOfMeteor::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = 0;

	int32 area = skill_get_splash(getSkillId(), skill_lv);
	int16 tmpx = 0, tmpy = 0;

	for (i = 1; i <= (skill_get_time(getSkillId(), skill_lv)/skill_get_unit_interval(getSkillId())); i++) {
		// Casts a double meteor in the first interval.
		if (i == 1) {
			// The first meteor is at the center
			skill_unitsetting(src, getSkillId(), skill_lv, x, y, flag+skill_get_unit_interval(getSkillId()));

			// The second meteor is near the first
			tmpx = x - 1 + rnd()%3;
			tmpy = y - 1 + rnd()%3;
			skill_unitsetting(src, getSkillId(), skill_lv, tmpx, tmpy, flag+skill_get_unit_interval(getSkillId()));
		}
		else {	// Casts 1 meteor per interval in the splash area
			tmpx = x - area + rnd()%(area * 2 + 1);
			tmpy = y - area + rnd()%(area * 2 + 1);
			skill_unitsetting(src, getSkillId(), skill_lv, tmpx, tmpy, flag+i*skill_get_unit_interval(getSkillId()));
		}
	}
}
