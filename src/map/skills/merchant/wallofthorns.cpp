// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wallofthorns.hpp"

SkillWallOfThorns::SkillWallOfThorns() : SkillImpl(GN_WALLOFTHORN) {
}

void SkillWallOfThorns::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 10 * skill_lv;
}

void SkillWallOfThorns::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
