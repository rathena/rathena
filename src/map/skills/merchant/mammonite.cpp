// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mammonite.hpp"

SkillMammonite::SkillMammonite() : WeaponSkillImpl(MC_MAMMONITE) {
}

void SkillMammonite::calculateSkillRatio(Damage *wd, block_list *src, block_list *target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 50 * skill_lv;
}
