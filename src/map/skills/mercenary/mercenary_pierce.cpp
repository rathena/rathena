// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_pierce.hpp"

SkillMercenaryPierce::SkillMercenaryPierce() : WeaponSkillImpl(ML_PIERCE) {
}

void SkillMercenaryPierce::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 10 * skill_lv;
}

void SkillMercenaryPierce::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	hit_rate += hit_rate * 5 * skill_lv / 100;
}
