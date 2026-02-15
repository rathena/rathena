// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "splashattack.hpp"

SkillSplashAttack::SkillSplashAttack() : SkillImplRecursiveDamageSplash(NPC_SPLASHATTACK) {
}

void SkillSplashAttack::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// A fake packet should be sent for the first target to be hit.
	flag |= SD_PREAMBLE;
	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}
