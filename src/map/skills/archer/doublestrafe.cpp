// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "doublestrafe.hpp"

SkillDoubleStrafe::SkillDoubleStrafe() : WeaponSkillImpl(AC_DOUBLE) {
}

void SkillDoubleStrafe::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 10 * (skill_lv - 1);
}
