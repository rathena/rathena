// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "roundtripplusattack.hpp"

SkillRoundTripPlusAttack::SkillRoundTripPlusAttack() : WeaponSkillImpl(RL_R_TRIP_PLUSATK) {
}

void SkillRoundTripPlusAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 300 + 300 * skill_lv;
}
