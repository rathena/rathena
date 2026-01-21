// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "counterattack.hpp"

#include "map/status.hpp"

SkillCounterAttack::SkillCounterAttack() : SkillImpl(KN_AUTOCOUNTER) {
}

void SkillCounterAttack::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	skill_addtimerskill(src, tick + 100, target->id, 0, 0, getSkillId(), skill_lv, BF_WEAPON, flag);
}

void SkillCounterAttack::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	hit_rate += hit_rate * 20 / 100;
}
