// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_sbr44.hpp"

SkillSBR44::SkillSBR44() : WeaponSkillImpl(HFLI_SBR44) {
}

void SkillSBR44::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 100 * (skill_lv - 1);
}
