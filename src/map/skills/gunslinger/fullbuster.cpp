// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fullbuster.hpp"

SkillFullBuster::SkillFullBuster() : WeaponSkillImpl(GS_FULLBUSTER) {
}

void SkillFullBuster::calculateSkillRatio(Damage *wd, block_list *src, block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
	base_skillratio += 100 * (skill_lv + 2);
}
