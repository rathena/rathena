// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "demonshockattack.hpp"

#include "map/status.hpp"

SkillDemonShockAttack::SkillDemonShockAttack() : SkillImpl(NPC_MAGICALATTACK) {
}

void SkillDemonShockAttack::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	sc_start(src,src,SC_MAGICALATTACK,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
}
