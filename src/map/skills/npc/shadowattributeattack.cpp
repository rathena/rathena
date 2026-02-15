// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shadowattributeattack.hpp"

SkillShadowAttributeAttack::SkillShadowAttributeAttack() : WeaponSkillImpl(NPC_DARKNESSATTACK) {
}

void SkillShadowAttributeAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 100 * (skill_lv - 1);
}

void SkillShadowAttributeAttack::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	nullpo_retv(src);
	nullpo_retv(target);
	(void)skill_lv;
	hit_rate += hit_rate * 20 / 100;
}
