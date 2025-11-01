// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rapidshower.hpp"

SkillRapidShower::SkillRapidShower() : WeaponSkillImpl(GS_RAPIDSHOWER) {
}

void SkillRapidShower::calculateSkillRatio(Damage *wd, block_list *src, block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
	base_skillratio += 400 + 50 * skill_lv;
}
