// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "grounddrift.hpp"

SkillGrounddrift::SkillGrounddrift() : WeaponSkillImpl(GS_GROUNDDRIFT) {
}

void SkillGrounddrift::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
#ifdef RENEWAL
	base_skillratio += 100 + 20 * skill_lv;
#endif
}