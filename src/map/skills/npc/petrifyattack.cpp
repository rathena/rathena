// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "petrifyattack.hpp"

#include "map/status.hpp"

SkillPetrifyAttack::SkillPetrifyAttack() : WeaponSkillImpl(NPC_PETRIFYATTACK) {
}

void SkillPetrifyAttack::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start2(src,target,SC_STONEWAIT,(20*skill_lv),skill_lv,src->id,skill_get_time2(getSkillId(),skill_lv),skill_get_time(getSkillId(), skill_lv));
}

void SkillPetrifyAttack::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	hit_rate += hit_rate * 20 / 100;
}
