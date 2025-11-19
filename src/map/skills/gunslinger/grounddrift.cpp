// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "grounddrift.hpp"

SkillGroundDrift::SkillGroundDrift() : WeaponSkillImpl(GS_GROUNDDRIFT) {
}

void SkillGroundDrift::calculateSkillRatio(Damage *wd, block_list *src, block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
#ifdef RENEWAL
	base_skillratio += 100 + 20 * skill_lv;
#endif
}
