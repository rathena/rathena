// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fullbuster.hpp"

SkillFullbuster::SkillFullbuster() : WeaponSkillImpl(GS_FULLBUSTER) {
}

void SkillFullbuster::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
	base_skillratio += 100 * (skill_lv + 2);
}

void SkillFullbuster::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_FULLBUSTER, skill_lv, tick, flag);
}