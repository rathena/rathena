// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulstrike.hpp"

SkillSoulStrike::SkillSoulStrike() : SkillImpl(MG_SOULSTRIKE) {
}

void SkillSoulStrike::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}