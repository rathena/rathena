// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "waterball.hpp"

SkillWaterBall::SkillWaterBall() : SkillImpl(WZ_WATERBALL) {
}

void SkillWaterBall::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Deploy waterball cells, these are used and turned into waterballs via the timerskill
	skill_unitsetting(src, getSkillId(), skill_lv, src->x, src->y, 0);
	skill_addtimerskill(src, tick, target->id, src->x, src->y, getSkillId(), skill_lv, 0, flag);
}

void SkillWaterBall::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 30 * skill_lv;
}
