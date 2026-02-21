// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <config/core.hpp>

#include "roundtrip.hpp"

SkillRoundTrip::SkillRoundTrip() : WeaponSkillImpl(RL_R_TRIP) {
}

void SkillRoundTrip::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 350 * skill_lv;
	RE_LVL_DMOD(100);
}
