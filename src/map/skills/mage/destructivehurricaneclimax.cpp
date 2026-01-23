// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "destructivehurricaneclimax.hpp"


SkillDestructiveHurricaneClimax::SkillDestructiveHurricaneClimax() : SkillImpl(AG_DESTRUCTIVE_HURRICANE_CLIMAX) {
}

void SkillDestructiveHurricaneClimax::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 12500;
	// Skill not affected by Baselevel and SPL
}

void SkillDestructiveHurricaneClimax::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}
