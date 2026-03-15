// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "earthattributeattack.hpp"

SkillEarthAttributeAttack::SkillEarthAttributeAttack() : WeaponSkillImpl(NPC_GROUNDATTACK) {
}

void SkillEarthAttributeAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 100 * (skill_lv - 1);
}

void SkillEarthAttributeAttack::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	hit_rate += hit_rate * 20 / 100;
}
