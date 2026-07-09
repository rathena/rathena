// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "undeadattributechange.hpp"

#include "map/status.hpp"

SkillUndeadAttributeChange::SkillUndeadAttributeChange() : WeaponSkillImpl(NPC_CHANGEUNDEAD) {
}

void SkillUndeadAttributeChange::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_CHANGEUNDEAD, (10 * skill_lv), skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillUndeadAttributeChange::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	hit_rate += hit_rate * 20 / 100;
}
