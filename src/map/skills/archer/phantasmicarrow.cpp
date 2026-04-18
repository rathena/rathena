// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "phantasmicarrow.hpp"

#include <config/core.hpp>

SkillPhantasmicArrow::SkillPhantasmicArrow() : WeaponSkillImpl(HT_PHANTASMIC) {
}

void SkillPhantasmicArrow::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 400;
#else
	base_skillratio += 50;
#endif
}
