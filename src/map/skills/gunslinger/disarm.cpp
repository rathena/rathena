// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "disarm.hpp"

SkillDisarm::SkillDisarm() : WeaponSkillImpl(GS_DISARM) {
}

void SkillDisarm::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_DISARM, skill_lv, tick, flag);
}

void SkillDisarm::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	skill_strip_equip(src, target, GS_DISARM, skill_lv);
	clif_skill_nodamage(src, *target, GS_DISARM, skill_lv);
}