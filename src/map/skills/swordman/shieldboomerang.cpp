// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shieldboomerang.hpp"

#include <config/core.hpp>

SkillShieldBoomerang::SkillShieldBoomerang() : WeaponSkillImpl(CR_SHIELDBOOMERANG) {
}

void SkillShieldBoomerang::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += -100 + skill_lv * 80;
#else
	base_skillratio += 30 * skill_lv;
#endif
}
