// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "melodystrike.hpp"

#include <config/core.hpp>

SkillMelodyStrike::SkillMelodyStrike() : WeaponSkillImpl(BA_MUSICALSTRIKE) {
}

void SkillMelodyStrike::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 10 + 40 * skill_lv;
#else
	base_skillratio += -40 + 40 * skill_lv;
#endif
}
