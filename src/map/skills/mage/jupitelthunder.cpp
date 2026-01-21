// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "jupitelthunder.hpp"

SkillJupitelThunder::SkillJupitelThunder() : SkillImpl(WZ_JUPITEL) {
}

void SkillJupitelThunder::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Jupitel Thunder is delayed by 150ms, you can cast another spell before the knockback
	skill_addtimerskill(src, tick + TIMERSKILL_INTERVAL, target->id, 0, 0, getSkillId(), skill_lv, 1, flag);
}
