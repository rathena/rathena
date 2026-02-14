// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "armcannon.hpp"

#include <config/core.hpp>

SkillArmCannon::SkillArmCannon() : WeaponSkillImpl(NC_ARMSCANNON) {
}

void SkillArmCannon::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 400 + 350 * skill_lv;
	RE_LVL_DMOD(100);
}
