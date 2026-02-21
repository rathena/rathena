// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pulsestrike2.hpp"

SkillPulseStrike2::SkillPulseStrike2() : SkillImplRecursiveDamageSplash(NPC_PULSESTRIKE2) {
}

void SkillPulseStrike2::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 100;
}

void SkillPulseStrike2::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	for (int32 i = 0; i < 3; i++)
	skill_addtimerskill(src, tick + (t_tick)skill_get_time(getSkillId(), skill_lv) * i, target->id, 0, 0, getSkillId(), skill_lv, skill_get_type(getSkillId()), flag);
}
