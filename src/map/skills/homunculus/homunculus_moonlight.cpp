// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_moonlight.hpp"

SkillMoonlight::SkillMoonlight() : WeaponSkillImpl(HFLI_MOON) {
}

void SkillMoonlight::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 10 + 110 * skill_lv;
}
