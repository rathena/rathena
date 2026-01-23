// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "allbloomattack2.hpp"


SkillAllBloomAttack2::SkillAllBloomAttack2() : SkillImpl(AG_ALL_BLOOM_ATK2) {
}

void SkillAllBloomAttack2::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 85000;
	// Skill not affected by Baselevel and SPL
}
