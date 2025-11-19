// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dust.hpp"

SkillDust::SkillDust() : WeaponSkillImpl(GS_DUST) {
}

void SkillDust::calculateSkillRatio(Damage *wd, block_list *src, block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
	base_skillratio += 50 * skill_lv;
}
