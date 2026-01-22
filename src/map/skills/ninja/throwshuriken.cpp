// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "throwshuriken.hpp"

#include <config/core.hpp>

SkillThrowShuriken::SkillThrowShuriken() : WeaponSkillImpl(NJ_SYURIKEN) {
}

void SkillThrowShuriken::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 5 * skill_lv;
#endif
}
