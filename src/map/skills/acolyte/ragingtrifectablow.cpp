// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ragingtrifectablow.hpp"

SkillRagingTrifectaBlow::SkillRagingTrifectaBlow() : WeaponSkillImpl(MO_TRIPLEATTACK) {
}

void SkillRagingTrifectaBlow::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 sflag = flag|SD_ANIMATION;

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, sflag);
}

void SkillRagingTrifectaBlow::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 20 * skill_lv;
}
