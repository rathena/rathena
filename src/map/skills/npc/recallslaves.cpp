// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "recallslaves.hpp"

SkillRecallSlaves::SkillRecallSlaves() : SkillImpl(NPC_CALLSLAVE) {
}

void SkillRecallSlaves::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_warpslave(src,MOB_SLAVEDISTANCE);
}
