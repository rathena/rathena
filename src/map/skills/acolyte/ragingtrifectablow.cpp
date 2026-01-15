// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ragingtrifectablow.hpp"

SkillRagingTrifectaBlow::SkillRagingTrifectaBlow() : SkillImpl(MO_TRIPLEATTACK) {
}

void SkillRagingTrifectaBlow::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_WEAPON,src,src,target,getSkillId(),skill_lv,tick,flag|SD_ANIMATION);
}

void SkillRagingTrifectaBlow::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 20 * skill_lv;
}
