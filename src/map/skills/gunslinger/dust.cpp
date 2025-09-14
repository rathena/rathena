// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dust.hpp"

SkillDust::SkillDust() : WeaponSkillImpl(GS_DUST) {
}

void SkillDust::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
	base_skillratio += 50 * skill_lv;
}

void SkillDust::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_DUST, skill_lv, tick, flag);
}