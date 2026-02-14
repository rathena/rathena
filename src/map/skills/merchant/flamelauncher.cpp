// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "flamelauncher.hpp"

#include <config/core.hpp>

SkillFlameLauncher::SkillFlameLauncher() : WeaponSkillImpl(NC_FLAMELAUNCHER) {
}

void SkillFlameLauncher::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += 200 + 300 * skill_lv;
	RE_LVL_DMOD(150);
}
