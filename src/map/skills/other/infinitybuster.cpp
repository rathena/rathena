// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "infinitybuster.hpp"

SkillInfinityBuster::SkillInfinityBuster() : WeaponSkillImpl(ABR_INFINITY_BUSTER) {
}

void SkillInfinityBuster::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	// Need official formula.
	base_skillratio += -100 + 50000;
}
