// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "venompressure.hpp"


SkillVenomPressure::SkillVenomPressure() : WeaponSkillImpl(GC_VENOMPRESSURE) {
}

void SkillVenomPressure::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 900;
}

void SkillVenomPressure::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	hit_rate += 10 + 4 * skill_lv;
}
