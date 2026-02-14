// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "coldslower.hpp"

#include <config/core.hpp>

SkillColdSlower::SkillColdSlower() : WeaponSkillImpl(NC_COLDSLOWER) {
}

void SkillColdSlower::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += 200 + 300 * skill_lv;
	RE_LVL_DMOD(150);
}
