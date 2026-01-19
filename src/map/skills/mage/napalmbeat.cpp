// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "napalmbeat.hpp"

SkillNapalmBeat::SkillNapalmBeat() : SkillImplRecursiveDamageSplash(MG_NAPALMBEAT) {
}

void SkillNapalmBeat::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -30 + 10 * skill_lv;
}

void SkillNapalmBeat::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}
